/*
 * Clock_Control.c
 *
 *  Created on: Jan 2, 2023
 *      Author: Brandon Thibeaux
 */

#include "Clock_Control.h"
#include "Structures.h"
#include <math.h>
#include "App_Config.h"

// SUMMARY: It is passed a new BPM variable. We then calculate the frequency of that BPM, then we get the period
// length of that BPM value. We then set the clock's attributes to match these new updated values.
void UpdateBPM(uint16_t newBPM, Clock* clockObj)
{
  float freq = (float)((float)newBPM / 60.0)*2; // returns frequency of that BPM
  clockObj->freqHz = freq/2; // for display purposes, seems to work, so I'll leave it
  clockObj->period = (int)((((float)1.0/freq)*1000.0));
}


// SUMMARY: Updates clock's duty cycle based on clock object's duty cycle state.
    // The way this works is case 1 and case 2 are inverses of each other. When duty cycle value is at 50%,
    // we should get a case 1 duty positive duty cycle of 25% and a case 2 positive duty cycle of 75%.
    // So if we use this value as an input and we find a way to sweep this value from 0 to .99, we could get some interesting functionalities to come of it.
void UpdateDutyCycle(Clock* clockObj)
{
  clockObj->posDutyCycleDelay = (int)(((clockObj->period) * clockObj->dutyCycleValue + 0.5) * -1);
  clockObj->negDutyCycleDelay = (int)((clockObj->period) * clockObj->dutyCycleValue + 0.5);
}


// SUMMARY: Calculates a frequency based on a period of time in miliseconds
float CalculateFrequency(uint32_t ms)
{
  float hertz; // we use floats temporarily for precision
  hertz = (float)((float)(1/(float)(ms)*1000)/2);
//  int roundedHertz = round(hertz); // if needed


  return hertz;
}


// SUMMARY: Calculates BPM based on a frequency input in Hertz.
uint32_t CalculateBPM(float frequency)
{
  //Serial.print("BPM: "); Serial.println((int)((float)frequency * (float)60.0)*2);
  return (int)((float)frequency * (float)60.0)*2;
}


// SUMMARY: Gets the difference between two time samples, pass it the first time sample
uint32_t GetTimeSample(uint32_t sample)
{
  int avg = 0 ;
  return avg = (millis() - sample); // get second sample
}
