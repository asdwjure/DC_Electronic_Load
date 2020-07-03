/*
 * dac.c
 *
 *  Created on: 29. avg. 2018
 *      Author: jrreb
 */

#include <dac_jure.h>


void DAC_Init () {
	Chip_DAC_Init(LPC_DAC);

	LPC_DAC->CTRL &= ~DAC_DACCTRL_MASK;
	LPC_DAC->VAL = 0;

	Chip_SWM_FixedPinEnable(SWM_FIXED_DAC_OUT, 1);
}

void DAC_Set_Value(uint32_t value) {
	if (value <= 4096)
		LPC_DAC->VAL = DAC_VALUE(value);
}
