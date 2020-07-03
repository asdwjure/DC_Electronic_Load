/*
 * system.c
 *
 *  Created on: 29. avg. 2018
 *      Author: jrreb
 */

#include <system_jure.h>

TimerHandle_t debounceTimer, timer10msHandle, timerBuzzerHandle;

Load_Observer_T gLoadObserver;
Timer_T gEncoderPushTimer, gDisplayErrorTimer;
Encoder_Observer_T gEncoderObserver;
extern ADC_Observer_T gADCObserver[MAX_ADC_CHN];
extern Display_Observer_T gDisplayObserver;

extern TaskHandle_t displayTaskHandle;

uint32_t gGPIO_DEB_PORT0 = 0, gGPIO_DEB_PORT1 = 0;	// debounced ports


void system_task (void *pvParameters) {
	uint8_t systemState = SYS_STATE_INIT;

	timer10msHandle = xTimerCreate("10ms tmr", pdMS_TO_TICKS(10), pdTRUE, NULL, timer_10ms_callback);
	xTimerStart(timer10msHandle, 0);

	debounceTimer = xTimerCreate("Debounce", pdMS_TO_TICKS(2), pdTRUE, NULL, timer_debounce_callback);
	xTimerStart(debounceTimer, 0);

	/* Timer for buzzer beep */
	timerBuzzerHandle = xTimerCreate("Buzzer tmr", pdMS_TO_TICKS(50), pdFALSE, NULL, timer_buzzer_callback);

	static bool encoderPush = 0, encoderPushFalling = 0, prvEncoderPushFalling = 0;
	static bool loadSwitch = 0, loadSwitchPrv = 0;


	while (1) {

		/* Falling edge of encoder push */
		encoderPush = (bool)(gGPIO_DEB_PORT0 & (1<<ENCODER_PUSH_PIN));
		encoderPushFalling = (prvEncoderPushFalling ^ encoderPush) & ~encoderPush;
		prvEncoderPushFalling = encoderPush;

		loadSwitch = (bool)(gGPIO_DEB_PORT1 & (1<<LOAD_SWITCH_PIN));



		switch (systemState) {
		case SYS_STATE_INIT:

			/* Set all variables to inital state */
			gLoadObserver.setCurrent = 0;
			gLoadObserver.setVoltage = 0;
			gLoadObserver.setPower = 0;
			gLoadObserver.setResistance = 0;
			gLoadObserver.current = 0;
			gLoadObserver.voltage = 0;
			gLoadObserver.power = 0;
			gLoadObserver.temperature = 0;
			gLoadObserver.mode = LOAD_MODE_CC;
			gLoadObserver.status = LOAD_STATUS_OK;
			gLoadObserver.enable = FALSE;

			gEncoderObserver.value = 0;
			gEncoderObserver.timerVal = 0;
			gEncoderObserver.timerAvg = 0;

			ADC_Init();
			gADCObserver[ADC_CURRENT].done = TRUE; gADCObserver[ADC_VOLTAGE].done = TRUE; gADCObserver[ADC_TEMPERATURE].done = TRUE;	// set all flags to true for first conversion
			ADC_Start_Conversion();	// start first conversion

			systemState = SYS_STATE_IDLE;
			break;


		case SYS_STATE_IDLE:

			/* Calculate and do some averaging on voltage, current and temperature */
			System_Calculate_Averages();

			/* Check if all values are in boundaries */
			if (gLoadObserver.voltage > LOAD_MAX_VOLTAGE) {
				// Overvoltage condition
				gLoadObserver.status = LOAD_STATUS_OVERVOLTAGE;
			}
			if (gLoadObserver.current >= LOAD_MAX_CURRENT) {
				// Overcurrent condition
				gLoadObserver.status = LOAD_STATUS_OVERCURRENT;
			}
			if (gLoadObserver.power > LOAD_MAX_POWER) {
				// Overpower condition
				gLoadObserver.status = LOAD_STATUS_OVERPOWER;
			}

			if (gLoadObserver.temperature > HEATSINK_TEMP_CRITICAL || gLoadObserver.status == LOAD_STATUS_OVERTEMPERATURE) {	// Prevelika temperatura

				gLoadObserver.enable = FALSE;
				gLoadObserver.status = LOAD_STATUS_OVERTEMPERATURE;
				gDisplayObserver.currentScreen = DISPLAY_TEMPERATURE;
				Chip_GPIO_SetPinState(LPC_GPIO, FAN_PORT, FAN_PIN, 1);

				if (gLoadObserver.temperature < HEATSINK_TEMP_CRITICAL - 5) {
					gLoadObserver.status = LOAD_STATUS_OK;
					gDisplayObserver.currentScreen = DISPLAY_HOME;
				}
			}


			/* Fan control */
			if (gLoadObserver.power >= 5000 || gLoadObserver.temperature >= HEATSINK_TEMP_NORMAL) {
				Chip_GPIO_SetPinState(LPC_GPIO, FAN_PORT, FAN_PIN, 1);
			}
			else if (gLoadObserver.power < 5000 && gLoadObserver.temperature < HEATSINK_TEMP_NORMAL && gLoadObserver.status != LOAD_STATUS_OVERTEMPERATURE) {
				Chip_GPIO_SetPinState(LPC_GPIO, FAN_PORT, FAN_PIN, 0);
			}


			/* Check if encoder was turned and write new values to DAC */
			if (gEncoderObserver.value != 0) {
				if (gDisplayObserver.currentScreen == DISPLAY_HOME) {
					if (gLoadObserver.mode == LOAD_MODE_CC) {
						gLoadObserver.setCurrent += gEncoderObserver.value;
						if (gLoadObserver.setCurrent < 0) gLoadObserver.setCurrent = 0;
						else if (gLoadObserver.setCurrent > LOAD_MAX_CURRENT) gLoadObserver.setCurrent = LOAD_MAX_CURRENT;
					}
					else if (gLoadObserver.mode == LOAD_MODE_CP) {
						gLoadObserver.setPower += gEncoderObserver.value;
						if (gLoadObserver.setPower < 0) gLoadObserver.setPower = 0;
						else if (gLoadObserver.setPower > LOAD_MAX_POWER) gLoadObserver.setPower = LOAD_MAX_POWER;
					}
					else if (gLoadObserver.mode == LOAD_MODE_CR) {
						gLoadObserver.setResistance += gEncoderObserver.value;
						if (gLoadObserver.setResistance < 0) gLoadObserver.setResistance = 0;
						else if (gLoadObserver.setResistance > LOAD_MAX_RESISTANCE) gLoadObserver.setResistance = LOAD_MAX_RESISTANCE;
					}
				}
				else if (gDisplayObserver.currentScreen == DISPLAY_MENU) {
					gDisplayObserver.menuIndex += gEncoderObserver.value;
					if (gDisplayObserver.menuIndex < 0) gDisplayObserver.menuIndex = 0;
					else if (gDisplayObserver.menuIndex > 4) gDisplayObserver.menuIndex = 4;
				}

				gEncoderObserver.value = 0;
			}


			/* Write new values to DAC */
			if (gLoadObserver.enable) {
				if (gLoadObserver.mode == LOAD_MODE_CC) {
					Load_Set_Current((uint32_t)gLoadObserver.setCurrent);
				}
				else if (gLoadObserver.mode == LOAD_MODE_CP) {
					Load_Set_Power((uint32_t)gLoadObserver.setPower);
				}
				else if (gLoadObserver.mode == LOAD_MODE_CR) {
					Load_Set_Resistance((uint32_t)gLoadObserver.setResistance);
				}
			}
			else {
				Load_Set_Current(0);
			}



			/* Check for encoder push falling edge */
			if ( encoderPushFalling ) {
				System_Buzzer_Beep();

				if (gDisplayObserver.currentScreen == DISPLAY_MENU) {
					if (gDisplayObserver.menuIndex == 0) {
						gLoadObserver.mode = LOAD_MODE_CC;
						gDisplayObserver.currentScreen = DISPLAY_HOME;
					}
					else if (gDisplayObserver.menuIndex == 1) {
						gLoadObserver.mode = LOAD_MODE_CP;
						gDisplayObserver.currentScreen = DISPLAY_HOME;
					}
					else if (gDisplayObserver.menuIndex == 2) {
						gLoadObserver.mode = LOAD_MODE_CR;
						gDisplayObserver.currentScreen = DISPLAY_HOME;
					}
					else if (gDisplayObserver.menuIndex == 3) gDisplayObserver.currentScreen = DISPLAY_STAT;
					else if (gDisplayObserver.menuIndex == 4) gDisplayObserver.currentScreen = DISPLAY_HOME;
				}

				else if (gDisplayObserver.currentScreen == DISPLAY_STAT) {	// se smo na statistiki in pritisnemo nazaj
					gDisplayObserver.currentScreen = DISPLAY_MENU;
				}

			}

			// TODO: FALLING EDGE
			/* Check if encoder was long pressed */
			if (gEncoderPushTimer.out) {
				gEncoderPushTimer.out = FALSE;

				System_Buzzer_Beep();
				gDisplayObserver.currentScreen = DISPLAY_MENU;
				gDisplayObserver.menuIndex = 0;
			}

			/* Check for load switch */
			if ( loadSwitch != loadSwitchPrv) {
				loadSwitchPrv = loadSwitch;
				System_Buzzer_Beep();
				gLoadObserver.enable = !gLoadObserver.enable;
			}



			break;


		} // end switch



	}
}
//--------------------------------------------------------------------------------


void System_Calculate_Averages() {
	float temp;
	/* Calculate averages form raw adc values to milliamps, millivolts and degrees C*/

	// TODO: NEDELA POVPRECJE (neki glitcha)

	/* Avg of voltage */
	temp = 0;
	for (uint32_t i=0; i<ADC_FIFO_LENGTH; i++) {
		temp += gADCObserver[ADC_VOLTAGE].fifo[i];
	}
	temp /= (float)ADC_FIFO_LENGTH;
	gLoadObserver.voltage = temp * 2.5;	// 1 bit = 2.5mV

	/* Avg of current */
	temp = 0;
	for (uint32_t i=0; i<ADC_FIFO_LENGTH; i++) {
		temp += gADCObserver[ADC_CURRENT].fifo[i];
	}
	temp /= (float)ADC_FIFO_LENGTH;
	gLoadObserver.current = temp;	// 1 bit = 0.5mV = 1mA

	/* Avg of temperature */
	temp = 0;
	for (uint32_t i=0; i<ADC_FIFO_LENGTH; i++) {
		temp += gADCObserver[ADC_TEMPERATURE].fifo[i];
	}
	temp /= (float)ADC_FIFO_LENGTH;
	gLoadObserver.temperature = temp / 20;


	/* Calculate power (in milliwatts) */
	gLoadObserver.power = (gLoadObserver.voltage * gLoadObserver.current) / 1000;


	xTaskNotifyGive(displayTaskHandle);	// only now the display task can update the display

}
//--------------------------------------------------------------------------------


void Load_Set_Current(uint32_t value) {
	// Write to internal DAC
	DAC_Set_Value(value);
}

void Load_Set_Power(uint32_t value) {
	// Write to internal DAC
	//DAC_Set_Value(value);
	// I = P/U
	DAC_Set_Value( (gLoadObserver.power / value) * 1000);
}

void Load_Set_Resistance(uint32_t value) {
	// Write to internal DAC
	// I = U/R
	DAC_Set_Value(gLoadObserver.voltage / value);
}
//--------------------------------------------------------------------------------


/*
 * @Brief 	Interrupt handler for ENCODER_A pin
 */
void PIN_INT0_IRQHandler(void) {
	LPC_GPIO_PIN_INT->IST |= PININTCH0;		// Clear status flag
	NVIC_ClearPendingIRQ(PIN_INT0_IRQn);	// Clear pending interrupt

	// TODO: Speed control
	if (gGPIO_DEB_PORT1 & (1<<ENCODER_B_PIN) ) { 			// vrtenje v desno
		if (gEncoderObserver.timerVal < 50000) {	// manj kot 50ms
			gEncoderObserver.value += (1/gEncoderObserver.timerVal) * 100000;
		}
		else
			gEncoderObserver.value++;
	}
	else {		// Vrtenje v levo
		if (gEncoderObserver.timerVal < 50000) {
			gEncoderObserver.value -= (1/gEncoderObserver.timerVal) * 100000;
		}
		else
			gEncoderObserver.value--;
	}

	gEncoderObserver.timerVal = 0;

	//LPC_GPIO_PIN_INT->CIENF |= PININTCH0;				// Disable interrupts
	//LPC_MRT_CH1->INTVAL = 216000 & MRT_INTVAL_IVALUE;	// Sproži timer za debounce; ~3ms; value = time * CPU_CLK
}


//--------------------------------------------------------------------------------
void MRT_IRQHandler() {
	NVIC_ClearPendingIRQ(MRT_IRQn);				// Clear pending interrupt
	uint32_t intChannel = LPC_MRT->IRQ_FLAG;	// Preberi register da vidimo kateri kanal je sprožu interrupt

	if(intChannel & MRT0_INTFLAG) {				// 0.1ms period
		/* Trigger a new adc sample */
		LPC_MRT->IRQ_FLAG |= MRT0_INTFLAG;		// Clear int flag
		ADC_Start_Conversion();

		/* Timer for encoder speed detection */
		if (gEncoderObserver.timerVal < 100000) gEncoderObserver.timerVal += 100;	// povecaj samo ce je manjsi od 100ms
	}

	if(intChannel & MRT1_INTFLAG) {
		/* Enable interrupts for ENCODER_A after deobounce period */
		LPC_MRT->IRQ_FLAG |= MRT1_INTFLAG;		// Clear int flag in MRT timer register
		//LPC_GPIO_PIN_INT->SIENF |= PININTCH0;	// Enable interrupts on falling edge
	}
}


//--------------------------------------------------------------------------------
void System_Buzzer_Beep() {
	Chip_GPIO_SetPinState(LPC_GPIO, BUZZER_PORT, BUZZER_PIN, 1);	// Set pin high
	xTimerStart(timerBuzzerHandle, 0);
}

void timer_buzzer_callback (TimerHandle_t xTimer) {
	Chip_GPIO_SetPinState(LPC_GPIO, BUZZER_PORT, BUZZER_PIN, 0);	// Set pin low
}
//--------------------------------------------------------------------------------


void timer_debounce_callback( TimerHandle_t xTimer ) {	/* 2ms period */
	static uint32_t DEB_PORT0_A = 0, DEB_PORT0_B = 0, DEB_PORT1_A = 0, DEB_PORT1_B = 0;
	uint32_t S = 0;

	/* Debounce PORT0 */
	S = gGPIO_DEB_PORT0 ^ LPC_GPIO->PIN[0];
	DEB_PORT0_B = ~(DEB_PORT0_B & S);
	DEB_PORT0_A = DEB_PORT0_B ^ (DEB_PORT0_A & S);
	S &= DEB_PORT0_B & DEB_PORT0_A;
	gGPIO_DEB_PORT0 ^= S;

	/* Debounce PORT1 */
	S = gGPIO_DEB_PORT1 ^ LPC_GPIO->PIN[1];
	DEB_PORT1_B = ~(DEB_PORT1_B & S);
	DEB_PORT1_A = DEB_PORT1_B ^ (DEB_PORT1_A & S);
	S &= DEB_PORT1_B & DEB_PORT1_A;
	gGPIO_DEB_PORT1 ^= S;
}
//--------------------------------------------------------------------------------


void timer_10ms_callback( TimerHandle_t xTimer ) {

	// Timer for encoder long-press
	if ( !(gGPIO_DEB_PORT0 & (1<<ENCODER_PUSH_PIN) ) ) {
		if (gEncoderPushTimer.value > 0)
			gEncoderPushTimer.value -= 10;
		else {
			gEncoderPushTimer.out = TRUE;
		}

	}
	else {
		gEncoderPushTimer.out = FALSE;
		gEncoderPushTimer.value = 1000;
	}


	// Timer for error display
	if ( gDisplayErrorTimer.enable ) {
		if (gDisplayErrorTimer.value > 0)
			gDisplayErrorTimer.value -= 10;
		else
			gDisplayErrorTimer.out = TRUE;

	}
	else
		gDisplayErrorTimer.out = FALSE;
}
