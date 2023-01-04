/*
 * Rotary_Encoder.c
 *
 *  Created on: Jan 2, 2023
 *      Author: Brandon Thibeaux
 */

#include "Rotary_Encoder.h"
#include "Structures.h"


// SUMMARY: Function checks a rotary knob's switch to see if it was pressed and raises a flag to prompt state machine for action to be taken later.
void CheckSwitch( RotaryKnob* knob)
{
  //Serial.println((*knob->in & (knob->sw)));
  if(!(*knob->in & (knob->sw)) && !knob->buttonState)
  {
    knob->buttonState = 1;
  }
  if((*knob->in & (knob->sw)) && knob->buttonState)
  {
    knob->flag = SWITCH;
    knob->buttonState = 0;
  }
}

// SUMMARY: Checks if Rotary Knob was rotated in the increment direction, then raises a flag if it was rotated.
void CheckIncrement(RotaryKnob* knob)
{
  if(knob->decrementState > 0)
  {
    // if we started another state, return and do not proceed
    return;
  }
  if(((*knob->in & (knob->clk)) && !(*knob->in & knob->dt)) && knob->incrementState == 0) // if clk is high and dt is low
  {
    knob->incrementState = 2;
  }
//  delay(10); // to try to prevent debouncing

  if(((*knob->in & (knob->clk)) && (*knob->in & knob->dt)) && knob->incrementState == 2) // wait for both lines to come back up
  {
    knob->incrementState = 0;
    knob->flag = INCREMENT;
  }
}

// SUMMARY: Checks if rotary knob was rotated in decremening direction. Flag is raised if it has been rotated.
void CheckDecrement(RotaryKnob* knob)
{
  if(knob->incrementState > 0)
  {
     // if we started another state, return and do not proceed
    return;
  }
  if(((*knob->in & (knob->dt)) && !(*knob->in & knob->clk)) && knob->decrementState == 0) // if dt is high and clock is low
  {
    knob->decrementState = 2;
  }
//  delay(10); // to try to prevent debouncing

  if(((*knob->in & (knob->dt)) && (*knob->in & knob->clk)) &&( knob->decrementState == 2))
  {
    knob->decrementState = 0;
    knob->flag = DECREMENT;
  }
}
