/*
 * Globals.h
 *
 *  Created on: Jan 1, 2023
 *      Author: Brandon Thibeaux
 */

#ifndef INC_GLOBALS_H_
#define INC_GLOBALS_H_

// Global Variables
uint32_t firstTimeSample = 0 ;
uint32_t secondTimeSample = 0 ;
uint32_t avgTime = 0;
unsigned char numOfTaps = 0;

// Global variables for the ISR
uint16_t divider = 2;
uint16_t dividerCount = 0;
uint16_t dividerCount2 = 0;

// DisplayGlobal Variables



// Timer Global Settings
unsigned char timerInterruptFlag = 0;
uint32_t multiplier = 1;
uint16_t timeoutValue = 3000; // in miliseconds

// FIXME add these to clock object
unsigned long time1 = millis();
unsigned long time2 = millis();
uint8_t pulseToggle = 0 ; // to sequence clock1 pulse
uint8_t pulseToggle2 = 0 ; // to sequence clock2 pulse
int  delaybuffer = 0; // for clock1
int  delaybuffer2 = 0; // for clock2
int calculatedWaitPeriod = 0; uint8_t calculatedEnd =0;

#endif /* INC_GLOBALS_H_ */
