/*
 * display.c
 *
 *  Created on: 29. avg. 2018
 *      Author: jrreb
 */

#include <display_jure.h>

extern const char logo_atlan [];


Display_Observer_T gDisplayObserver;
Display_Buffer_T gDisplayBuffer;
extern Load_Observer_T gLoadObserver;
extern Timer_T gDisplayErrorTimer;


void display_task (void *pvParameters) {

	gDisplayObserver.currentScreen = DISPLAY_INIT;

	while (1) {

		memset(gDisplayBuffer.buffer, 0, DISPLAY_BUFFER_SIZE);
		gDisplayBuffer.index = 0;

		ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(300)); // block in case system task didn't calculate screen values until now

		switch (gDisplayObserver.currentScreen) {
		case DISPLAY_INIT: /* Začetni zaslon */
		    LCD_Set_Cursor(9,0); LCD_Write_Text("MATURITETNI"); LCD_Set_Cursor(21,1); LCD_Write_Text("IZDELEK");
		    LCD_Set_Cursor(3,2); LCD_Write_Text("JURE REBERNIK");
		    LCD_Set_Cursor(6,3); LCD_Write_Text("September 2018");
		    LCD_Set_Cursor(3,5); LCD_Write_Text("Software r2.1");
			for(uint32_t i=0; i<0xFFFFF; i++);
			LCD_Draw_Bitmap(logo_atlan);
			for(uint32_t i=0; i<0xFFFFF; i++);

			gDisplayObserver.currentScreen = DISPLAY_HOME;
			break;

//--------------------------------------------------------------------------------
		case DISPLAY_HOME:		// Domac zaslon ki prikazuje merjene podatke

			/* napiši izmerjene velicine glede na nacin bremena */
			if(gLoadObserver.enable) {
				LCD_Set_Cursor(66,5);
				LCD_Write_Text(" "); LCD_Write_Text_Inverted("ON");
			}
			else {
				LCD_Set_Cursor(66,5);
				LCD_Write_Text_Inverted("OFF");
			}

			switch (gLoadObserver.mode) {

			case LOAD_MODE_CC:
				LCD_Set_Cursor(0,5); LCD_Write_Text_Inverted("CC mode");
				LCD_Write_Float(gLoadObserver.current, 3, 0, 0, 1); LCD_Write_Text_11x16("A", 72, 0);
				LCD_Set_Cursor(0,2); LCD_Write_Float(gLoadObserver.voltage, 2, 0, 0, 0); LCD_Write_Text("V");
				LCD_Set_Cursor(0,3); LCD_Write_Float(gLoadObserver.power, 3, 0, 0, 0); LCD_Write_Text("W");
				LCD_Set_Cursor(0,4); LCD_Write_Text("Set:"); LCD_Write_Float(gLoadObserver.setCurrent, 3, 0, 0, 0); LCD_Write_Text("A   ");
				break;

			case LOAD_MODE_CP:
				LCD_Set_Cursor(0,5); LCD_Write_Text_Inverted("CP mode");
				LCD_Write_Float(gLoadObserver.power, 3, 0, 0, 1); LCD_Write_Text_11x16("W", 72, 0);
				LCD_Set_Cursor(0,2); LCD_Write_Float(gLoadObserver.current, 2, 0, 0, 0); LCD_Write_Text("A");
				LCD_Set_Cursor(0,3); LCD_Write_Float(gLoadObserver.voltage, 3, 0, 0, 0); LCD_Write_Text("V");
				LCD_Set_Cursor(0,4); LCD_Write_Text("Set:"); LCD_Write_Float(gLoadObserver.setPower, 3, 0, 0, 0); LCD_Write_Text("W");
				break;


			case LOAD_MODE_CR:
				LCD_Set_Cursor(0,5); LCD_Write_Text_Inverted("CR mode");
				LCD_Write_Float(gLoadObserver.current, 3, 0, 0, 1); LCD_Write_Text_11x16("A", 72, 0);
				LCD_Set_Cursor(0,2); LCD_Write_Float(gLoadObserver.voltage, 2, 0, 0, 0); LCD_Write_Text("V");
				LCD_Set_Cursor(0,3); LCD_Write_Float(gLoadObserver.power, 3, 0, 0, 0); LCD_Write_Text("W");
				LCD_Set_Cursor(0,4); LCD_Write_Text("Set:"); LCD_Write_Float(gLoadObserver.setResistance, 3, 0, 0, 0); LCD_Write_Custom(2);
				break;
			}

			break;
//--------------------------------------------------------------------------------

		case DISPLAY_MENU:
			if(gDisplayObserver.menuIndex == 0){
				LCD_Set_Cursor(0,0); LCD_Write_Text(">");

				LCD_Set_Cursor(8,0); LCD_Write_Text_Inverted("CC mode");
				LCD_Set_Cursor(8,1); LCD_Write_Text("CP mode");
				LCD_Set_Cursor(8,2); LCD_Write_Text("CR mode");
				LCD_Set_Cursor(8,3); LCD_Write_Text("Statistics");
				LCD_Set_Cursor(8,4); LCD_Write_Text("Back");
			}
			else if(gDisplayObserver.menuIndex == 1){
				LCD_Set_Cursor(0,1); LCD_Write_Text(">");

				LCD_Set_Cursor(8,0); LCD_Write_Text("CC mode");
				LCD_Set_Cursor(8,1); LCD_Write_Text_Inverted("CP mode");
				LCD_Set_Cursor(8,2); LCD_Write_Text("CR mode");
				LCD_Set_Cursor(8,3); LCD_Write_Text("Statistics");
				LCD_Set_Cursor(8,4); LCD_Write_Text("Back");
			}
			else if(gDisplayObserver.menuIndex == 2){
				LCD_Set_Cursor(0,2); LCD_Write_Text(">");

				LCD_Set_Cursor(8,0); LCD_Write_Text("CC mode");
				LCD_Set_Cursor(8,1); LCD_Write_Text("CP mode");
				LCD_Set_Cursor(8,2); LCD_Write_Text_Inverted("CR mode");
				LCD_Set_Cursor(8,3); LCD_Write_Text("Statistics");
				LCD_Set_Cursor(8,4); LCD_Write_Text("Back");
			}
			else if(gDisplayObserver.menuIndex == 3){
				LCD_Set_Cursor(0,3); LCD_Write_Text(">");

				LCD_Set_Cursor(8,0); LCD_Write_Text("CC mode");
				LCD_Set_Cursor(8,1); LCD_Write_Text("CP mode");
				LCD_Set_Cursor(8,2); LCD_Write_Text("CR mode");
				LCD_Set_Cursor(8,3); LCD_Write_Text_Inverted("Statistics");
				LCD_Set_Cursor(8,4); LCD_Write_Text("Back");
			}
			else if(gDisplayObserver.menuIndex == 4){
				LCD_Set_Cursor(0,4); LCD_Write_Text(">");

				LCD_Set_Cursor(8,0); LCD_Write_Text("CC mode");
				LCD_Set_Cursor(8,1); LCD_Write_Text("CP mode");
				LCD_Set_Cursor(8,2); LCD_Write_Text("CR mode");
				LCD_Set_Cursor(8,3); LCD_Write_Text("Statistics");
				LCD_Set_Cursor(8,4); LCD_Write_Text_Inverted("Back");
			}
			break;
//--------------------------------------------------------------------------------

		case DISPLAY_STAT:
			LCD_Set_Cursor(0,0); LCD_Write_Text("Temp: "); LCD_Write_Int( (uint32_t)gLoadObserver.temperature); LCD_Write_Custom(0); LCD_Write_Text("C");	// Znakec za stopinje

			LCD_Set_Cursor(0,5); LCD_Write_Text(">"); LCD_Set_Cursor(8,5); LCD_Write_Text_Inverted("Back");
			break;
//--------------------------------------------------------------------------------

		case DISPLAY_TEMPERATURE:
			LCD_Set_Cursor(0,0); LCD_Write_Text("HEATSINK TEMP");
			LCD_Set_Cursor(0,1); LCD_Write_Text("CRITICAL!");
			LCD_Set_Cursor(0,2); LCD_Write_Int( (uint32_t)gLoadObserver.temperature); LCD_Write_Custom(0); LCD_Write_Text("C");	// Znakec za stopinje
			LCD_Set_Cursor(0,4); LCD_Write_Text("Cooling down..");
			break;
//--------------------------------------------------------------------------------

		case DISPLAY_ERROR:		// prikazuj napako za 2 sekundi
			LCD_Set_Cursor(3,1); LCD_Write_Text("Turn the load");
			LCD_Set_Cursor(12,2); LCD_Write_Text("off before");
			LCD_Set_Cursor(0,3); LCD_Write_Text("changing mode!");

			if (!gDisplayErrorTimer.enable) {
				gDisplayErrorTimer.value = 2000;
				gDisplayErrorTimer.enable = TRUE;
			}

			if (gDisplayErrorTimer.out) {
				gDisplayErrorTimer.enable = FALSE;
				gDisplayObserver.currentScreen = DISPLAY_MENU;
			}
			break;
//--------------------------------------------------------------------------------

		case DISPLAY_LOAD_ERROR:
			if (gLoadObserver.status == LOAD_STATUS_OVERPOWER) {
				LCD_Set_Cursor(0,1); LCD_Write_Text("LOAD");
				LCD_Set_Cursor(0,2); LCD_Write_Text("OVERPOWER!");
				LCD_Set_Cursor(0,3); LCD_Write_Text("Max power is");
				LCD_Set_Cursor(0,4); LCD_Write_Text("100W");
			}
			else if (gLoadObserver.status == LOAD_STATUS_OVERCURRENT) {
				LCD_Set_Cursor(0,1); LCD_Write_Text("LOAD");
				LCD_Set_Cursor(0,2); LCD_Write_Text("OVERCURRENT!");
				LCD_Set_Cursor(0,3); LCD_Write_Text("Max current is");
				LCD_Set_Cursor(0,4); LCD_Write_Text("4.096A");
			}
			else if (gLoadObserver.status == LOAD_STATUS_OVERVOLTAGE) {
				LCD_Set_Cursor(0,1); LCD_Write_Text("LOAD");
				LCD_Set_Cursor(0,2); LCD_Write_Text("OVERVOLTAGE!");
				LCD_Set_Cursor(0,3); LCD_Write_Text("Max current is");
				LCD_Set_Cursor(0,4); LCD_Write_Text("50V");
			}

			if (!gDisplayErrorTimer.enable) {
				gDisplayErrorTimer.value = 2000;
				gDisplayErrorTimer.enable = TRUE;
			}

			if (gDisplayErrorTimer.out) {
				gDisplayErrorTimer.enable = FALSE;
				gDisplayObserver.currentScreen = DISPLAY_MENU;
			}
			break;


		}	// end main switch

		Display_Update();
		vTaskDelay(pdMS_TO_TICKS(100));

	}	// end while
}
//--------------------------------------------------------------------------------


void Display_Update() {
	/* start transfer of data to display */
	/* Set hardware cursor of ldc to 0,0 */
	LCD_Write_Command(0 | 0x80); // Column
	LCD_Write_Command(0 | 0x40); // Row
	LPC_GPIO->SET[0] |= (1<<LCD_DC_PIN);		// set DC pin because we're sending data

	gDisplayBuffer.sendIndex = 0;
	if (LPC_SPI0->STAT & SPI_STAT_TXRDY) {
		Chip_SPI_SendFirstFrame_RxIgnore(LPC_SPI0, gDisplayBuffer.buffer[gDisplayBuffer.sendIndex++], 8, 0);
		LPC_SPI0->INTENSET |= SPI_INTENSET_TXRDYEN;
	}

//	LPC_SPI0->TXDATCTL = SPI_TXDATCTL_EOT | SPI_TXDATCTL_RXIGNORE | SPI_TXDATCTL_LEN(7) | SPI_TXDATCTL_DATA(gDisplayBuffer.buffer[gDisplayBuffer.sendIndex++] );
}
//--------------------------------------------------------------------------------


void SPI0_IRQHandler(void) {
	uint32_t irqID = LPC_SPI0->INTSTAT & SPI_INTSTAT_BITMASK;
	uint32_t spiStat = LPC_SPI0->STAT & SPI_STAT_BITMASK;
	NVIC_ClearPendingIRQ(SPI0_IRQn);

	if ( (irqID & SPI_INTSTAT_TXRDY) && (spiStat & SPI_STAT_TXRDY) ) {
		/* šopni podatke in to je to */
		if (gDisplayBuffer.sendIndex < 503)
			Chip_SPI_SendMidFrame(LPC_SPI0, gDisplayBuffer.buffer[gDisplayBuffer.sendIndex++]);
//			LPC_SPI0->TXDATCTL = SPI_TXDATCTL_EOT | SPI_TXDATCTL_RXIGNORE | SPI_TXDATCTL_LEN(7) | SPI_TXDATCTL_DATA(gDisplayBuffer.buffer[gDisplayBuffer.sendIndex++] );
		else if (gDisplayBuffer.sendIndex == 503) {
			LPC_SPI0->INTENCLR |= SPI_INTENSET_TXRDYEN;
			Chip_SPI_SendLastFrame_RxIgnore(LPC_SPI0, gDisplayBuffer.buffer[gDisplayBuffer.sendIndex++], 8, 0);
		}
	}

}
