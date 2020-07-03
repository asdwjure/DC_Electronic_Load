/*
 * adc.h
 *
 *  Created on: 29. avg. 2018
 *      Author: jrreb
 */

#ifndef ADC_JURE_H_
#define ADC_JURE_H_

#include "chip.h"
#include "stdint.h"
#include "stdbool.h"

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"


/* ADC fifo buffer structure */
#define ADC_FIFO_LENGTH		64
typedef struct {
	uint32_t fifo[ADC_FIFO_LENGTH];
	uint8_t head;
	uint8_t tail;
	bool full;

	uint32_t value;
	bool done;
}ADC_Observer_T;

/* Enums for adc channel variable */
enum {
	ADC_VOLTAGE = 0,
	ADC_CURRENT,
	ADC_TEMPERATURE,
};
#define MAX_ADC_CHN			3

#define ADC_TEMPERATURE_CHN 11
#define ADC_VOLTAGE_CHN		8
#define ADC_CURRENT_CHN		9




void ADC_Init();
void ADC_Start_Conversion();
int32_t ADC_Is_Done(uint32_t channel);
void ADC_Callback(void *pvParameters);

#endif /* ADC_JURE_H_ */
