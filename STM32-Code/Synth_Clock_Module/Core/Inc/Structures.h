/*
 * Structures.h
 *
 *  Created on: Jan 1, 2023
 *      Author: Brandon Thibeaux
 */

#ifndef INC_STRUCTURES_H_
#define INC_STRUCTURES_H_

// State Machine
//enum DutyCycleRatio{TwentyFive=0, Fifty=1,SeventyFive=2}; // These are estimates, can be fine tuned. Measured in percentagesEX: 25%,50%,75% // Delete if not used 9/5/2022
enum RotaryEnocderFlag{NONE=0x00,INCREMENT=0x01,DECREMENT=0x02,SWITCH=0x04};
enum ApplicationModes {FREQUENCY_CONTROL=0,DUTYCYCLE_CONTROL=1,GATE_CONTROL=2};


// Structs
typedef struct Window
{
  const uint8_t screenWidth = 128;
  const uint8_t screenHeight = 32;
  const uint8_t address = 0x3C;

  const int8_t OLEDReset = -1;// Reset pin # (or -1 if sharing Arduino reset pin)
  uint8_t refreshFlag = 0; // needs to be one for refresh
}Window;
Window window;
typedef struct AppMode
{
  ApplicationModes appMode;
}Application;

typedef struct RotaryKnob
{
    RotaryEnocderFlag flag;
    volatile uint8_t *port;
    volatile uint8_t *in;
    volatile uint8_t sw;
    volatile uint8_t dt;
    volatile uint8_t clk;

    volatile bool buttonState;
    volatile int incrementState;
    volatile int decrementState;
}RotaryKnob;

RotaryKnob bpmAdjust;

typedef struct ClockObject
{
    volatile uint8_t *port;
    volatile uint8_t pin ;
    int posDutyCycleDelay;
    int negDutyCycleDelay ;
    float dutyCycleValue = 0.5; // this can be thought of a a ratio percentage. See update duty cycle function for deatils how to use this.

    //clock stats
    uint32_t period; // in ms
    float freqHz ;
    uint16_t bpm;
}Clock;

Clock clock1, clock2;

#endif /* INC_STRUCTURES_H_ */
