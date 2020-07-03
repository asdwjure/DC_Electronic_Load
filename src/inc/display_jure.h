/*
 * display.h
 *
 *  Created on: 29. avg. 2018
 *      Author: jrreb
 */

#ifndef DISPLAY_JURE_H_
#define DISPLAY_JURE_H_

#include "string.h"
#include "stdint.h"

#include "chip.h"

#include <system_jure.h>

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"


/* User interface observer */
typedef struct {
	uint8_t currentScreen;
	uint8_t lastScreen;
	int8_t menuIndex;
} Display_Observer_T;

enum {
	DISPLAY_INIT = 0,
	DISPLAY_HOME,
	DISPLAY_MENU,
	DISPLAY_STAT,
	DISPLAY_TEMPERATURE,
	DISPLAY_ERROR,
	DISPLAY_LOAD_ERROR,
};

#define DISPLAY_BUFFER_SIZE		504
typedef struct {
	uint8_t buffer[DISPLAY_BUFFER_SIZE];
	uint16_t index;
	uint16_t sendIndex;
}Display_Buffer_T;





void display_task (void *pvParameters);
void Display_Update();

#endif /* DISPLAY_JURE_H_ */
