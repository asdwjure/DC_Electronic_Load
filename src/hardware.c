/*
 * hardware.c
 *
 *  Created on: 22. feb. 2018
 *      Author: jrreb
 */


/* INITIALIZE ALL HARDWARE */

#include "hardware.h"



void Hardware_Init(void) {
	SystemCoreClockUpdate();

	Chip_GPIO_Init(LPC_GPIO);	// Initialize GPIO block...
	Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_IOCON);	// Enable clock for IOCON (pullups, pulldowns, ...)
	Chip_SWM_Init();			// Initialize switch matrix

	LCD_Init();					// Zaženi display
	DAC_Init();
	Hardware_MRT_Timer_Init();


	/* Initialize buzzer */
	Chip_GPIO_SetPinDIROutput(LPC_GPIO, BUZZER_PORT, BUZZER_PIN);


	/* Initializiraj pine za releje */
	Chip_IOCON_PinMuxSet(LPC_IOCON, 1, RELAY_VOLTAGE_RANGE_1_PIN, 	IOCON_MODE_INACT); // Disable internal pullups
	Chip_IOCON_PinMuxSet(LPC_IOCON, 1, RELAY_VOLTAGE_RANGE_2_PIN, 	IOCON_MODE_INACT);
	Chip_IOCON_PinMuxSet(LPC_IOCON, 1, RELAY_EXT_VOLTAGE_PIN, 		IOCON_MODE_INACT);
	Chip_IOCON_PinMuxSet(LPC_IOCON, 1, RELAY_CURRENT_RANGE_PIN, 	IOCON_MODE_INACT);

	Chip_GPIO_SetPinDIR(LPC_GPIO, 	1, RELAY_VOLTAGE_RANGE_1_PIN, 	1);			// GPIO 1.5 = output (VOLTAGE_RANGE_1)
	Chip_GPIO_SetPinDIR(LPC_GPIO, 	1, RELAY_VOLTAGE_RANGE_2_PIN, 	1);			// GPIO 1.6 = output (VOLTAGE_RANGE_2)
	Chip_GPIO_SetPinDIR(LPC_GPIO, 	1, RELAY_EXT_VOLTAGE_PIN, 		1);			// GPIO 1.7 = output (EXT_VOLTAGE)
	Chip_GPIO_SetPinDIR(LPC_GPIO, 	1, RELAY_CURRENT_RANGE_PIN, 	1);			// GPIO 1.4 = output (CURRENT_RANGE)

	Chip_GPIO_SetPinState(LPC_GPIO, 1, RELAY_VOLTAGE_RANGE_1_PIN, 	1);
	Chip_GPIO_SetPinState(LPC_GPIO, 1, RELAY_VOLTAGE_RANGE_2_PIN, 	0);			// On startup select 10V
	Chip_GPIO_SetPinState(LPC_GPIO, 1, RELAY_EXT_VOLTAGE_PIN, 		0);
	Chip_GPIO_SetPinState(LPC_GPIO, 1, RELAY_CURRENT_RANGE_PIN, 	1);			// On startup select 4.096 range

	/* Init fan */
	Chip_GPIO_SetPinDIROutput(LPC_GPIO, FAN_PORT, FAN_PIN);

	/* Initialize Rotary Encoder w/ interrupts */
	Chip_PININT_Init(LPC_GPIO_PIN_INT);
	Chip_INMUX_PinIntSel(0, ENCODER_PORT, ENCODER_A_PIN);			// Attach GPIO1.1 (ENCODER_A) to interrput 0
	LPC_GPIO_PIN_INT->SIENF |= PININTCH0;	// Enable interrupts on falling edge
	NVIC_SetPriority(PIN_INT0_IRQn, 6);		// lowest priority - 1
	NVIC_ClearPendingIRQ(PIN_INT0_IRQn);	// Enable interrupts in NVIC
	NVIC_EnableIRQ(PIN_INT0_IRQn);
}
//--------------------------------------------------------------------------------


void Hardware_MRT_Timer_Init() {
	Chip_MRT_Init();				// Enable clock and disable periph. reset

	LPC_MRT_CH0->INTVAL = 7200 & MRT_INTVAL_IVALUE;	// Yelids 0.1ms interrupt interval (for adc sampling)

	LPC_MRT_CH0->CTRL |= (1<<0);	// Enable interrupts
	LPC_MRT_CH1->CTRL |= MRT_MODE_ONESHOT | (1<<0);	// Timer za encoder A debounce and enable interrupts

	NVIC_ClearPendingIRQ(MRT_IRQn);
	NVIC_SetPriority(MRT_IRQn, 7);	// lowest priority
	NVIC_EnableIRQ(MRT_IRQn);
}
//--------------------------------------------------------------------------------


void LCD_SPI_Init(){
	Chip_SPI_Init(LPC_SPI0);
	Chip_SWM_MovablePortPinAssign(SWM_SPI0_SCK_IO, 0, LCD_CLK_PIN);
	Chip_SWM_MovablePortPinAssign(SWM_SPI0_MOSI_IO, 0, LCD_DATA_PIN);
	Chip_SWM_MovablePortPinAssign(SWM_SPI0_SSELSN_0_IO, 0, LCD_CS_PIN);
	LPC_SPI0->DIV = 179;	// 400kHz clock
	LPC_SPI0->DLY = (1<<1) | (1<<4) | (1<<8) | (1<<12);	// delays - doesn't work without this

	LPC_SPI0->CFG |= SPI_CFG_SPI_EN | SPI_CFG_MASTER_EN;

	NVIC_ClearPendingIRQ(SPI0_IRQn);
	NVIC_SetPriority(SPI0_IRQn, 7);	// lowest priority
	NVIC_EnableIRQ(SPI0_IRQn);

}
//--------------------------------------------------------------------------------
