/*
 * Structures.h
 *
 *  Created on: Jan 1, 2023
 *      Author: Brandon Thibeaux
 */

#ifndef INC_STRUCTURES_H_
#define INC_STRUCTURES_H_

#include "main.h"
#include "stdint.h"
#include "stdbool.h"
#include<stdio.h>


// State Machine
//enum DutyCycleRatio{TwentyFive=0, Fifty=1,SeventyFive=2}; // These are estimates, can be fine tuned. Measured in percentagesEX: 25%,50%,75% // Delete if not used 9/5/2022
enum RotaryEnocderFlag{NONE=0x00,INCREMENT=0x01,DECREMENT=0x02,SWITCH=0x04};
enum ApplicationModes {FREQUENCY_CONTROL=0,DUTYCYCLE_CONTROL=1,GATE_CONTROL=2};
enum ButtonState {RELEASED = 0, PRESSED = 1};

// Structs
typedef struct ButtonObject // template
{
	GPIO_TypeDef *port;
	volatile uint16_t pin ;
	volatile enum ButtonState buttonState;
}Button;

typedef struct Window
{
  const uint8_t screenWidth ;//= 128;
  const uint8_t screenHeight;// = 32;
  const uint8_t address;// = 0x3C;

//  const int8_t OLEDReset = -1;// Reset pin # (or -1 if sharing Arduino reset pin)// delete this if not used 1/2/2023
  uint8_t refreshFlag ; // needs to be one for refresh
}Window;
//Window window; // delete this if not used 1/2/2023

typedef struct AppMode
{
  enum ApplicationModes appMode;
}Application;

typedef struct RotaryKnob
{
	// TODO: Delete legacy code if not used
    enum RotaryEnocderFlag flag;
//    GPIO_TypeDef *port_sw;
    GPIO_TypeDef *port_dt;
    GPIO_TypeDef *port_clk;

//    volatile uint16_t *in;
//    volatile uint8_t sw; // for legacy purposes, move away from this.
    volatile uint16_t dt;
    volatile uint16_t clk;

    volatile Button sw;

//    volatile bool buttonState; // legacy code, integrate abstract struct Button into this
    volatile int incrementState;
    volatile int decrementState;
}RotaryKnob;

//RotaryKnob bpmAdjust;// delete this if not used 1/2/2023

typedef struct ClockObject
{
	GPIO_TypeDef *port;
    volatile uint8_t pin ;
    int posDutyCycleDelay;
    int negDutyCycleDelay ;
    float dutyCycleValue ;//= 0.5; // this can be thought of a a ratio percentage. See update duty cycle function for deatils how to use this.

    //clock stats
    uint32_t period; // in ms
    float freqHz ;
    uint16_t bpm;
}Clock;

//Clock clock1, clock2;// delete this if not used 1/2/2023

typedef struct TempoButton
{
	volatile Button button;
	volatile uint8_t tap_count;
	volatile uint8_t tap_interval_buffer_size;
	volatile uint32_t *tap_intervals; // This is an array of time intervals. You need to allocate this using malloc and tap_interval_buffer_size * sizeof(uint32_t) to make this work.
}TempoButton;



#endif /* INC_STRUCTURES_H_ */
