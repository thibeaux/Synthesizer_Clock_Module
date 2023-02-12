/*
 * Clock_Control.h
 *
 *  Created on: Jan 2, 2023
 *      Author: Brandon Thibeaux
 */

#ifndef INC_CLOCK_CONTROL_H_
#define INC_CLOCK_CONTROL_H_
#include "Clock_Control.h"
#include "Structures.h"
void UpdateBPM(uint16_t newBPM, Clock* clockObj);
void UpdateDutyCycle(Clock* clockObj);
float CalculateFrequency(uint32_t ms);
uint32_t CalculateBPM(float frequency);
uint32_t GetTimeSample(uint32_t sample);

#endif /* INC_CLOCK_CONTROL_H_ */
