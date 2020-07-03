/*
===============================================================================
 Name        : main.c
 Author      : $(author)
 Version     :
 Copyright   : $(copyright)
 Description : main definition
===============================================================================
*/

/*
 * JURE REBERNIK
 * MATURITETNI PROJEKT 2018 - ELEKTRONSKO BREME
 *
 * R 2.0 - izboljšana verzija programa z uporabo operacijskega sistema FreeRTOS
 */

#ifndef	DEBUG_BUILD
	#define DEBUG_BUILD
	#warning BUILDING DEBUG VERSION!
#endif

#if defined (__USE_LPCOPEN)
#if defined(NO_BOARD_LIB)
#include "chip.h"
#else
#include "board.h"
#endif
#endif

#include <cr_section_macros.h>
#include <cmsis.h>
#include <stdint.h>

// Includes
#include "hardware.h"
#include <system_jure.h>
#include <display_jure.h>


// RTOS includes
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"






/* Debug functions */
#ifdef DEBUG_BUILD
uint32_t freeHeap, freeHeapMin;
UBaseType_t freeStack[4];

void vApplicationStackOverflowHook( TaskHandle_t xTask, signed char *pcTaskName );
void vApplicationStackOverflowHook( TaskHandle_t xTask, signed char *pcTaskName ) {
	while (1){
		configASSERT(0);
	}
}
#endif




/* Task handles */
TaskHandle_t ledTaskHandle = NULL;
TaskHandle_t displayTaskHandle = NULL;
TaskHandle_t systemTaskHandle = NULL;
TaskHandle_t AdcCallbackHandle = NULL;



void led_task (void *pvParameters);		// prototype
void led_task (void *pvParameters) {

	/* make hrb pin output */
	LPC_GPIO->DIR[HRB_LED_PORT] |= (1<<HRB_LED_PIN);

	while (1) {
		LPC_GPIO->NOT[HRB_LED_PORT] |= (1<<HRB_LED_PIN);		// toggle Hrb led

#ifdef DEBUG_BUILD
		/* Some system statuses */
		freeHeap = xPortGetFreeHeapSize();
		freeHeapMin = xPortGetMinimumEverFreeHeapSize();
		if (freeStack [0] < uxTaskGetStackHighWaterMark(ledTaskHandle)) freeStack[0] = uxTaskGetStackHighWaterMark(ledTaskHandle);
		if (freeStack [1] < uxTaskGetStackHighWaterMark(displayTaskHandle)) freeStack[1] = uxTaskGetStackHighWaterMark(displayTaskHandle);
		if (freeStack [2] < uxTaskGetStackHighWaterMark(systemTaskHandle)) freeStack[2] = uxTaskGetStackHighWaterMark(systemTaskHandle);
		// Check stack sizes
#endif

		vTaskDelay(1000);
	}
}




int main(void) {

	/* Initialization */
	Hardware_Init();


	/* Create all tasks */
	xTaskCreate(led_task, 		"LED", 		configMINIMAL_STACK_SIZE, 	NULL, 	tskIDLE_PRIORITY, 	&ledTaskHandle);
	xTaskCreate(display_task, 	"DISPLAY", 	256, 						NULL, 	1, 					&displayTaskHandle);
	xTaskCreate(system_task, 	"SYSTEM", 	256, 						NULL, 	tskIDLE_PRIORITY, 	&systemTaskHandle);
	xTaskCreate(ADC_Callback, 	"ADC CB", 	100, 						NULL, 	2, 					&AdcCallbackHandle);



	vTaskStartScheduler();
	return 0;


}
