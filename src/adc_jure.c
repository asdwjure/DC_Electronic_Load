/*
 * adc.c
 *
 *  Created on: 29. avg. 2018
 *      Author: jrreb
 */

#include <adc_jure.h>


ADC_Observer_T gADCObserver[MAX_ADC_CHN];		// three channels

uint32_t gADCDataReg_Current = 0, gADCDataReg_Voltage = 0, gADCDataReg_Temperature = 0;

extern TaskHandle_t AdcCallbackHandle;

void ADC_Init (){
	/* Initialize ADC0 for measuring current and temperature */
	LPC_SYSCON->SYSAHBCLKCTRL[0] |= (1<<27);	// Enable ADC0 clock
	LPC_SYSCON->PRESETCTRL[0] &= ~(1<<27);		// Clear ADC0 reset
	LPC_SYSCON->PDRUNCFG &= ~(1<<10);			// Power up ADC0 block

	vTaskDelay(pdMS_TO_TICKS(20));	// Wait for stabilization
	Chip_ADC_StartCalibration(LPC_ADC0);		// Sprozi kalibracijo ADC0
	while (LPC_ADC0->CTRL & ADC_CR_CALMODEBIT);	// Cakaj dokler se kalibracija ne konca

	LPC_ADC0->CTRL |= (1 & ADC_CR_CLKDIV_MASK);	// Clock devision - yeilds 36MHz clock
	LPC_ADC0->CTRL &= ~(ADC_CR_ASYNMODE);		// Disable asynchronous mode (enable synchronous mode)

	LPC_ADC0->SEQ_CTRL[ADC_SEQB_IDX] &= ~(ADC_SEQ_CTRL_SEQ_ENA);		// Disable trigger sequence
	LPC_ADC0->SEQ_CTRL[ADC_SEQB_IDX] |=	(ADC_SEQ_CTRL_HWTRIG_POLPOS) |	// TRIGPOL = 1
										(ADC_SEQ_CTRL_MODE_EOS);		// MODE = 1: Interrupts happen at the end of sequence

	/* Enable ADC pins */
	Chip_SWM_FixedPinEnable(SWM_FIXED_ADC0_8, 1);	// Enable ADC0_8 on GPIO0_31 (voltage)
	Chip_SWM_FixedPinEnable(SWM_FIXED_ADC0_9, 1);	// Enable ADC0_9 on GPIO0_31 (current)
	Chip_SWM_FixedPinEnable(SWM_FIXED_ADC0_11, 1);	// Enable ADC0_11 on GPIO0_30 (temperature)


	/* Interrupts */
	Chip_ADC_EnableInt(LPC_ADC0, ADC_INTEN_SEQB_ENABLE);
	NVIC_SetPriority(ADC0_SEQB_IRQn, 7);	// lowest priority
	NVIC_ClearPendingIRQ(ADC0_SEQB_IRQn);
	NVIC_EnableIRQ(ADC0_SEQB_IRQn);

	/* Init variables */
	gADCObserver[ADC_VOLTAGE].done = FALSE;
	gADCObserver[ADC_CURRENT].done = FALSE;
}


void ADC_Start_Conversion() {

	if (gADCObserver[ADC_CURRENT].done && gADCObserver[ADC_VOLTAGE].done && gADCObserver[ADC_TEMPERATURE].done) { // check if all ADC are done converting

		LPC_ADC0->SEQ_CTRL[ADC_SEQB_IDX] &= ~(ADC_SEQ_CTRL_SEQ_ENA);	// Disable trigger sequence
		LPC_ADC0->SEQ_CTRL[ADC_SEQB_IDX] |= ( (1<<ADC_VOLTAGE_CHN) | (1<<ADC_CURRENT_CHN) | (1<<ADC_TEMPERATURE_CHN) ) & ADC_SEQ_CTRL_CHANSEL_MASK;	// Izberi kanal 8(napetost) in 9 (tok)
		LPC_ADC0->SEQ_CTRL[ADC_SEQB_IDX] |= ADC_SEQ_CTRL_SEQ_ENA;		// Enable trigger sequence

		LPC_ADC0->SEQ_CTRL[ADC_SEQB_IDX] |= ADC_SEQ_CTRL_START;			// Start conversion

		gADCObserver[ADC_CURRENT].done = FALSE;
		gADCObserver[ADC_VOLTAGE].done = FALSE;
		gADCObserver[ADC_TEMPERATURE].done = FALSE;
	}
}


int32_t ADC_Is_Done(uint32_t channel) {
	// Retirn an adc value if adc on specified channel has finished conversion, or return -1 if adc on specified channel hasnt finished its conversion
	if (gADCObserver[channel].done)
		return gADCObserver[channel].value;

	return (-1);
}


void ADC_Callback(void *pvParameters) {
	while (1){
		vTaskSuspend(NULL);

		/* Preveri za vsak kanal posebej, če so podatki validni in vpiši v fifo buffer */
		if(gADCDataReg_Current & ADC_DR_DATAVALID) {
			gADCObserver[ADC_CURRENT].fifo[gADCObserver[ADC_CURRENT].head] = ADC_DR_RESULT( gADCDataReg_Current );
			gADCObserver[ADC_CURRENT].head++; gADCObserver[ADC_CURRENT].head &= ADC_FIFO_LENGTH - 1;
			if (gADCObserver[ADC_CURRENT].head == gADCObserver[ADC_CURRENT].tail) gADCObserver[ADC_CURRENT].full = TRUE;
			//else gADCObserver[ADC_CURRENT].full = FALSE;

			gADCObserver[ADC_CURRENT].value = ADC_DR_RESULT( gADCDataReg_Current );
			gADCObserver[ADC_CURRENT].done = TRUE;
		}

		if(gADCDataReg_Voltage & ADC_DR_DATAVALID) {
			gADCObserver[ADC_VOLTAGE].fifo[gADCObserver[ADC_VOLTAGE].head] = ADC_DR_RESULT( gADCDataReg_Voltage );
			gADCObserver[ADC_VOLTAGE].head++; gADCObserver[ADC_VOLTAGE].head &= ADC_FIFO_LENGTH - 1;
			if (gADCObserver[ADC_VOLTAGE].head == gADCObserver[ADC_VOLTAGE].tail) gADCObserver[ADC_VOLTAGE].full = TRUE;
			//else gADCObserver[ADC_VOLTAGE].full = FALSE;

			gADCObserver[ADC_VOLTAGE].value = ADC_DR_RESULT( gADCDataReg_Voltage );
			gADCObserver[ADC_VOLTAGE].done = TRUE;
		}

		if(gADCDataReg_Temperature & ADC_DR_DATAVALID) {
			gADCObserver[ADC_TEMPERATURE].fifo[gADCObserver[ADC_TEMPERATURE].head] = ADC_DR_RESULT( gADCDataReg_Temperature );
			gADCObserver[ADC_TEMPERATURE].head++; gADCObserver[ADC_TEMPERATURE].head &= ADC_FIFO_LENGTH - 1;
			if (gADCObserver[ADC_TEMPERATURE].head == gADCObserver[ADC_TEMPERATURE].tail) gADCObserver[ADC_TEMPERATURE].full = TRUE;
			//else gADCObserver[ADC_TEMPERATURE].full = FALSE;

			gADCObserver[ADC_TEMPERATURE].value = ADC_DR_RESULT( gADCDataReg_Temperature );
			gADCObserver[ADC_TEMPERATURE].done = TRUE;
		}
	}
}


/* Tok in napetost in temperatura */
void ADC0B_IRQHandler() {
	/* Clear interrupts */
	LPC_ADC0->FLAGS |= (ADC_FLAGS_SEQB_INT_MASK);
	NVIC_ClearPendingIRQ(ADC0_SEQB_IRQn);

	BaseType_t checkIfYieldRequired = pdFALSE;

	/* Grab data and GTFO */
	gADCDataReg_Current = Chip_ADC_GetDataReg(LPC_ADC0, ADC_CURRENT_CHN);
	gADCDataReg_Voltage = Chip_ADC_GetDataReg(LPC_ADC0, ADC_VOLTAGE_CHN);
	gADCDataReg_Temperature = Chip_ADC_GetDataReg(LPC_ADC0, ADC_TEMPERATURE_CHN);

	checkIfYieldRequired = xTaskResumeFromISR(AdcCallbackHandle);
	portYIELD_FROM_ISR(checkIfYieldRequired);
}
