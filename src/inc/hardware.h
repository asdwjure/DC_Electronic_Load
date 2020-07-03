/*
 * hardware.h
 *
 *  Created on: 22. feb. 2018
 *      Author: jrreb
 */

#ifndef HARDWARE_H_
#define HARDWARE_H_

#include "chip.h"
#include <stdint.h>

#include "nokia5110_lcd.h"
#include <dac_jure.h>


/* Hardware pin definitions */
#define HRB_LED_PORT	1
#define HRB_LED_PIN		8

#define FAN_PORT		0
#define FAN_PIN			26

#define BUZZER_PORT		1
#define BUZZER_PIN		10

#define ENCODER_PORT	1
#define ENCODER_A_PIN	1
#define ENCODER_B_PIN	2

#define ENCODER_PUSH_PORT	0
#define ENCODER_PUSH_PIN	18

#define LOAD_SWITCH_PORT	1
#define LOAD_SWITCH_PIN		3

/* Definicija pinov od relejev */
#define RELAY_VOLTAGE_RANGE_1_PIN	5
#define RELAY_VOLTAGE_RANGE_2_PIN	6
#define RELAY_EXT_VOLTAGE_PIN		7
#define RELAY_CURRENT_RANGE_PIN		4




/* Function prototypes */
void Hardware_Init(void);	// Init all hardware
void Hardware_MRT_Timer_Init();
void LCD_SPI_Init();


#endif /* HARDWARE_H_ */
