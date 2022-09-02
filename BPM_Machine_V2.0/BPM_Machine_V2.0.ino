/*
 * Author:  Brandon Thibeaux
 * Date:    7/17/2022
 * Decription:
 *          This program is to provide a clock signal to a sequencer (4017 decade counter) so that it may control both
 *          the the speed of the sequence and the duration of each not using the clocks frequency 
 *          as the BPM and the duty cycle as note duration.
 *          
 *          A few notes about the state of the code. You may find some code not being used. 
 *          I plan on optimizing thes lines out. I'm just leaving them here incase I need them for later features. 
 *          You may also notice some code that is labeled debug. This is for help with debugging. 
 *          I also plan on optimizing these outby commenting out the lines of code.
 */
#include<stdio.h>
#include <avr/io.h> // Contains all the I/O Register Macros
#include <stdint.h>
// State Machine
enum DutyCycleRatio{TwentyFive=0, Fifty=1,SeventyFive=2}; // These are estimates, can be fine tuned. Measured in percentagesEX: 25%,50%,75%
enum RotaryEnocderFlag{NONE=0,INCREMENT=1,DECREMENT=2,SWITCH=4};
// Structs
typedef struct RotaryKnob
{
    RotaryEnocderFlag flag;
    volatile uint8_t *port; 
    volatile uint8_t *in;
    volatile uint8_t sw;
    volatile uint8_t dt;
    volatile uint8_t clk;

    volatile bool buttonState;
    volatile uint8_t incrementState;
    volatile uint8_t decrementState;
}RotaryKnob;

RotaryKnob bpmAdjust;

typedef struct ClockObject
{
    volatile uint8_t *port; 
    volatile uint8_t pin ;
    DutyCycleRatio dutyCycle;
    unsigned int posDutyCycleDelay;
    unsigned int negDutyCycleDelay ;

    //clock stats
    uint32_t period; // in ms
    float freqHz ;
    uint16_t bpm;
}Clock;

Clock clock1;

// Global Variables
uint32_t firstTimeSample = 0 ;
uint32_t secondTimeSample = 0 ;
uint32_t avgTime = 0;
unsigned char numOfTaps = 0;
 
// Timer Global Settings
unsigned char timerInterruptFlag = 0;
uint32_t multiplier = 1;
uint16_t timeoutValue = 5000; // in miliseconds


// Prototypes
uint32_t GetTimeSample(uint32_t sample);
float CalculateFrequency(uint32_t ms);
void SetTimerSettings();
uint32_t CalculateBPM(float frequency);
void UpdateDutyCycle(Clock* clockObj);
void UpdateBPM(uint16_t newBPM, Clock* clockObj);
void CheckSwitch( RotaryKnob* knob);
void CheckIncrement(RotaryKnob* knob);
void CheckDecrement(RotaryKnob* knob);

void setup() {
  // debug features
  Serial.begin(9600);
  //Serial.print("TCCR1A Settings in HEX: ");
  //Serial.println(TCCR1A,HEX);
  //Serial.print("TCCR1B Settings in HEX: ");
  //Serial.println(TCCR1B,HEX);
  Serial.print("CPU Speed ");
  Serial.println(F_CPU);
}

void loop() {
    // local variables
    uint8_t bpmButton = 0; 
    unsigned char toggleButton = 0 ; // make sure code executes on falling edge

    unsigned long time = millis();
    unsigned long timeoutCount ; // in miliseconds to reset tap count
    bool startTimeout = false;
    uint8_t counter = 0 ;
    
    // Initialize Local Variables and Periphrials 
    // initialize struct
    clock1.dutyCycle = TwentyFive;
    clock1.pin = (1<<5); // assign pin number
    clock1.port = &PORTB; // assign port number
    
    clock1.period = 576;

  // Rotary Encoder Initizlization 
    bpmAdjust.flag= NONE;
    bpmAdjust.port  = &PORTD;
    bpmAdjust.in = &PIND;
    bpmAdjust.sw = (1<<4);
    bpmAdjust.dt = (1<<5);
    bpmAdjust.clk = (1<<6);
    bpmAdjust.buttonState=0;
    bpmAdjust.incrementState = 0;
    bpmAdjust.decrementState = 0;

    DDRD &= ~(bpmAdjust.sw + bpmAdjust.dt + bpmAdjust.clk); // make pins inputs 
    //Timer Initilization
    SetTimerSettings();

    
    // initialize pins
    // PortD pin 3 is the beat button
    PORTD |= 1 << 3; // Enabling Internal Pull Up on single pin PD3 with bit masking :
    DDRD &= ~(1<<3); // Configuring PD3 as Input
   
    DDRB |= clock1.pin;  // PB5 output

    // init values
    // Get Frequency 
    clock1.freqHz  = CalculateFrequency(clock1.period);
    // Calculate BPM 
    clock1.bpm = CalculateBPM(clock1.freqHz);
    Serial.print("BPM: ");
    Serial.println(CalculateBPM(clock1.freqHz));  

     
     while (1)
     {
      // Determine Duty Cylce State
      UpdateDutyCycle(&clock1);

      // Get Beat Button Input 
        bpmButton = PIND & (1<<3) ; //FIXME make a butto object that we can iniate and store a port reference and pin # to
        //Serial.println(port_value); // debug
        if((!bpmButton)) //|| (port_value  <115))// if button pressed, should be less than 5. Seem to be around max 9
        {
          while((!bpmButton)) //|| (port_value <120))//wait for button to release
          {    
            bpmButton = PIND & (1<<3); // get pin input
          }
           numOfTaps++;
           Serial.print("Number of taps "); Serial.println(numOfTaps); // debug line
           if (numOfTaps == 1)
           {
              firstTimeSample = millis(); // get first time sample
              timeoutCount = millis(); startTimeout = true; // start timout routine
           }
           else
           {
              avgTime = GetTimeSample(firstTimeSample);// get second sample
              
              // debug lines
              //Serial.print("average time is ");
              //Serial.print(firstTimeSample); Serial.print(" - "); Serial.print(millis()); Serial.print(" = ");
              //Serial.println(avgTime);
              
              // Get Frequency 
              clock1.freqHz  = CalculateFrequency(avgTime);

              // Get period
              clock1.period = avgTime;   

              // Calculate BPM 
              clock1.bpm = CalculateBPM(clock1.freqHz);
              Serial.print("BPM: ");
              Serial.println(CalculateBPM(clock1.freqHz));   

                
              //reset variables when done
              numOfTaps = 0;
              firstTimeSample = 0;
              avgTime = 0;
              startTimeout = false;
           }
        }
        // pulse clocks output pin
        if(millis() - time >= clock1.period)// pulse 
        {
            time = millis();
            // hard code pin high and pin low using counter%2 ==  0 
            // so we can add small delay to the high if or the low else to adjust duty cycle
            if((counter%2) == 0)
            {
              delay(clock1.negDutyCycleDelay);
              *clock1.port |=  clock1.pin ;
              counter = 0;
            }
            else
            {
              delay(clock1.posDutyCycleDelay);
              *clock1.port &= ~ clock1.pin ;
            }
            
            counter++;

            //PORTB ^= (1<<5);  // PB5 output toggle// debug line
            //Serial.print("Made it in the pulse task, period interval is "); Serial.println(period); // debug line
         } 
        
        // Goal is to prevent a button press hanging and ruining the user experiance. 
        //User must tap two times to set device frequency/BPM.
        if((millis() - timeoutCount >= timeoutValue) && startTimeout == true)
        {
          //Serial.println("Resetting tap count"); // debug line
          //Serial.println(millis() - timeoutCount);
          startTimeout = false;
          numOfTaps = 0;
        }

        //while(!timerInterruptFlag==1){}//wait for timer to tick
        //counter++;
        //timerInterruptFlag = 0 ;
     } 
}

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
  /* DELETE IF NOT USED 9/1/2022
  while(!(*knob->in & (knob->sw)))
  {
    //Serial.println("You pressed the button");
    uint8_t test = 2+2;
    if(*knob->in & (knob->sw))
    {
      //Serial.println("You released the button");
      knob->flag = SWITCH;
      //Serial.print("Knob->Flag: ");
      //Serial.println(knob->flag);
    }
    
  }
  */
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
    //Serial.println("I One");
  }
  /* DELETE IF NOT USED 9/1/2022
  if((!(*knob->in & knob->dt)) && knob->incrementState == 1)
  {
    knob->incrementState =2;
    Serial.println("I Two");
  }
  */
  if(((*knob->in & (knob->clk)) && (*knob->in & knob->dt)) && knob->incrementState == 2) // wait for both lines to come back up
  {
    knob->incrementState = 0;
    knob->flag = INCREMENT;
    //Serial.println("I Three");
  }
  delay(10); // to try to prevent debouncing
  /* DELETE IF NOT USED 9/1/2022
  if((*knob->in & (knob->clk)) && !(*knob->in & knob->dt)) // if clk is high and dt is low
  {
    while(!(*knob->in & knob->dt))
    {
       while(!(*knob->in & knob->clk))
       {
        // wait and do nothing
       }
    }
    delay(10); // to try to prevent debouncing
    knob->flag = INCREMENT;
    //Serial.print("Increment: ");
    //Serial.println(counter);
  }
  */
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
    //Serial.println("D One");
  }
  /* DELETE IF NOT USED 9/1/2022
  if((!(*knob->in & knob->clk)) && knob->decrementState == 1)
  {
    knob->decrementState = 2;
        Serial.println("D Two");
  }*/
  if(((*knob->in & (knob->dt)) && (*knob->in & knob->clk)) &&( knob->decrementState == 2))
  {
    knob->decrementState = 0;
    knob->flag = DECREMENT;
    //Serial.println("D Three");
  }
  delay(10); // to try to prevent debouncing
  /* DELETE IF NOT USED 9/1/2022
  if((*knob->in & (knob->dt)) && !(*knob->in & knob->clk)) // if dt is high and clock is low
  {
    while(!(*knob->in & knob->clk))
    {
      while(!(*knob->in & knob->dt))
      {
        // wait and do nothing
      }
    }
    delay(10); // to try to prevent debouncing
    knob->flag = DECREMENT;
    //Serial.print("Decrement: ");
    //Serial.println(counter);
  }  
  */
}
// SUMMARY: It is passed a new BPM variable. We then calculate the frequency of that BPM, then we get the period 
// length of that BPM value. We then set the clock's attributes to match these new updated values.
void UpdateBPM(uint16_t newBPM, Clock* clockObj)
{
  // FIXME, make this update the clock's frequency and bpm varibles too while it is in here
  float freq = (float)((float)newBPM / 60.0)*2; // returns frequency of that BPM 
  clockObj->period = (int)((((float)1.0/freq)*1000.0));
  //debug lines
  //Serial.print("Hz");
  //Serial.println(freq);
  //Serial.print("ms");
  //Serial.println(clockObj->period);
}
// SUMMARY: Updates clock's duty cycle based on clock object's duty cycle state
void UpdateDutyCycle(Clock* clockObj)
{
  switch(clockObj->dutyCycle)
  {
    case(0):
    {
      clockObj->posDutyCycleDelay = 0;
      clockObj->negDutyCycleDelay = 100;
      break;
    }
    case(1):
    {
      clockObj->posDutyCycleDelay = 0;
      clockObj->negDutyCycleDelay = 0;
      break;
    }
    case(2):
    {
      clockObj->posDutyCycleDelay = 100;
      clockObj->negDutyCycleDelay = 0;
      break;
    }
    default:
    {
      clockObj->dutyCycle = TwentyFive;
      break;
    }
  }

}
// SUMMARY: Calculates a frequency based on a period of time in miliseconds
float CalculateFrequency(uint32_t ms)
{
  float hertz; // we use floats temporarily for precision
  hertz = (float)((float)(1/(float)(ms)*1000)/2);
  int roundedHertz = round(hertz);
  
  // Debug print lines
  //Serial.print("Frequency calculated (HZ): ");
  //Serial.print(hertz);
  //Serial.print(" from the miliseconds value (ms) "); Serial.println(ms);
  
  return hertz;
}
// SUMMARY: Calculates BPM based on a frequency input in Hertz.
uint32_t CalculateBPM(float frequency)
{
  return (int)((float)frequency * (float)60.0);
}
// SUMMARY: Gets the difference between two time samples, pass it the first time sample
uint32_t GetTimeSample(uint32_t sample)
{
  int avg = 0 ;
  return avg = millis() - sample; // get second sample
}
//***************** DEAD FUNCTIONS *****************
void SetTimerSettings()
{
  // see ref https://create.arduino.cc/projecthub/dhorton668/pardon-me-for-interrupting-acbd1a
 // 4Also see section 15.11 in datasheet
  // Disable interrupts while things are being set up.
  cli();
  
  // Set up Timer Control Registers. See ATmega328P data sheet section 15.11 (Remember, bits are numbered from zero.)
  TCCR1A = B00000000;  // All bits zero for "normal" port operation. 
  TCCR1B = B00001100;  // Bit 3 is Clear Timer on Compare match (CTC) and bits 0..2 specifies a divide by 256 prescaler.
  TIMSK1 = B00000010;  // Bit 1 will raise an interrupt on timer Output Compare Register A (OCR1A) match.
  
  //FIXME Verify that period, frequency are the same in the osciliscope. I think that the calculate frequency function has inverted 
  // the period. Or the formula to set the OCR1A register is wrong. 
  
  //ONLY USE FOR EXAMPLE DIVISIONS ARE NOT ACCURATE ON ARDUNIO ********************8
  //unsigned int timerVal =((unsigned int)(F_CPU / 256) * (unsigned int)(multiplier/20));// = 3125       // Counter compare match value. 16MHz / prescaler * delay time (in seconds.)
  //*********************************************************************************
  uint16_t oscVal= 625;//3125 should be about 20 interrupts a second. 625 is about 100 interrupts a second
  OCR1A = oscVal;
  Serial.print("OCR1A Value: "); Serial.println(oscVal);
  // Reenable the interrupts 
  sei();
}


ISR(TIMER1_COMPA_vect)
{
  // WARNING, it seems with arduino's 16MHZ CPU speed the smallest delay amount that we can set the 
  //OCR1A to is 24ms when the value is set to 1 ms
    
  // Rotary Knob State Machine 
  CheckDecrement(&bpmAdjust);
  CheckIncrement(&bpmAdjust);
  CheckSwitch(&bpmAdjust);
  //Serial.println(bpmAdjust.flag);//debug
  switch(bpmAdjust.flag)
  {
    case(0):
    {
      //Serial.println("No flag"); // debug
      break;
    }
    case(1):
    {
      UpdateBPM((clock1.bpm += 1),&clock1);
      // debug lines
      Serial.println("Increment"); // debug
      Serial.print("New BPM: "); 
      Serial.print((clock1.bpm += 1));
      Serial.print("\tNewPeriod");
      Serial.println(clock1.period);
      break;
    }
    case(2):
    {
      UpdateBPM((clock1.bpm -= 1),&clock1);
      // debug lines
      Serial.println("Decrement"); // debug
      Serial.print("New BPM: "); 
      Serial.print((clock1.bpm -= 1));
      Serial.print("\tNewPeriod");
      Serial.println(clock1.period);
      break;
    }
    case(4):
    {
      clock1.dutyCycle = clock1.dutyCycle + 1; // FIXME I don't think this is best practice and may cause compiler problems
      UpdateDutyCycle(&clock1); // can probably be taken away
      Serial.println("Toggle Duty Cycle"); // debug
      Serial.print("NewDutyCycle: ");
      Serial.println( clock1.dutyCycle);
      break;
    }
    default:
    {break;}
  }
  bpmAdjust.flag = NONE; // flag reset
  
  //Serial.println("Interrupt");
}
