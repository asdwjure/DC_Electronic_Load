/*
 * system.h
 *
 *  Created on: 29. avg. 2018
 *      Author: jrreb
 */

#ifndef SYSTEM_JURE_H_
#define SYSTEM_JURE_H_

#include "stdint.h"
#include "stdbool.h"

#include "chip.h"
#include "hardware.h"
#include <adc_jure.h>

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"



enum {
	SYS_STATE_INIT,
	SYS_STATE_IDLE,
	SYS_STATE_LOAD_MODE,	// changing load mode
};

typedef struct {
	int32_t setCurrent;	// vse v je v milivoltih
	int32_t setVoltage;
	int32_t setPower;
	int32_t setResistance;

	float current;
	float voltage;
	float power;
	float temperature;

	bool enable;
	uint8_t mode;
	uint8_t status;
}Load_Observer_T;

enum {
	LOAD_MODE_CC = 0,
	LOAD_MODE_CP,
	LOAD_MODE_CR,

	LOAD_STATUS_OK,
	LOAD_STATUS_OVERVOLTAGE,
	LOAD_STATUS_OVERCURRENT,
	LOAD_STATUS_OVERPOWER,
	LOAD_STATUS_OVERTEMPERATURE,
};

typedef struct {
	int32_t value;
	uint32_t timerVal;	// in microseconds
	uint32_t timerAvg;
}Encoder_Observer_T;


#define LOAD_MAX_CURRENT	4096	// 4.096A
#define LOAD_MAX_VOLTAGE	50000	// 50V
#define LOAD_MAX_POWER		99000	// 99W
#define LOAD_MAX_RESISTANCE	100000	// 100kohm

#define HEATSINK_TEMP_CRITICAL	65
#define HEATSINK_TEMP_NORMAL	35


typedef struct {
	bool enable;
	bool out;
	int32_t value;
}Timer_T;



void system_task (void *pvParameters);

void System_Calculate_Averages();

void Load_Set_Current(uint32_t value);
void Load_Set_Power(uint32_t value);
void Load_Set_Resistance(uint32_t value);

void System_Buzzer_Beep();

void timer_buzzer_callback (TimerHandle_t xTimer);
void timer_debounce_callback( TimerHandle_t xTimer );
void timer_10ms_callback( TimerHandle_t xTimer );

#endif /* SYSTEM_JURE_H_ */
