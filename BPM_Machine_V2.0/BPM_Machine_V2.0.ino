/*
 * Author:  Brandon Thibeaux
 * Date:    7/17/2022
 * Decription:
 *          This program is to provide a clock signal to a sequencer so that it may control both
 *          the the speed of the sequence and the duration of each not using the clocks frequency 
 *          as the BPM and the duty cycle as note duration.
 *          
 *          A few notes about the state of the code. You may find some code not being used. 
 *          I plan on optimizing thes lines out. I'm just leaving them here incase I need them for later features. 
 *          You may also notice some code that is labeled debug. This is for help with debugging. 
 *          I also plan on optimizing these outby commenting out the lines of code.
 */
#include<stdio.h>
// State Machine
enum DuctyCycleRatio{TwentyFive=0, Fifty=1,SeventyFive=2}; // These are estimates, can be fine tuned. Measured in percentagesEX: 25%,50%,75%

// Global Variables
uint32_t firstTimeSample = 0 ;
uint32_t secondTimeSample = 0 ;
uint32_t avgTime = 0;
unsigned char numOfTaps = 0;
DuctyCycleRatio dutyCycle;
unsigned int posDutyCycleDelay = 0;
unsigned int negDutyCycleDelay = 0;
    
// Timer Global Settings
unsigned char timerInterruptFlag = 0;
uint32_t miliseconds = 1;
uint16_t timeoutValue = 5000; // in miliseconds

// Prototypes
uint32_t GetTimeSample(uint32_t sample);
float CalculateFrequency(uint32_t ms);
void SetTimerSettings();
uint32_t CalculateBPM(float frequency);
void UpdateDutyCycle(DuctyCycleRatio _dutyCycle);

void setup() {
   PORTD |= 1 << 3; // Enabling Internal Pull Up on single pin PD3 with bit masking :
   DDRD &= ~(1<<3); // Configuring PD3 as Input
   
   DDRB |= 1<<5;  // PB5 output

  //SetTimerSettings();
  
  // debug features
  Serial.begin(9600);
  Serial.print("TCCR1A Settings in HEX: ");
  Serial.println(TCCR1A,HEX);
  Serial.print("TCCR1B Settings in HEX: ");
  Serial.println(TCCR1B,HEX);
  Serial.print("CPU Speed ");
  Serial.println(F_CPU);
}

void loop() {
    // local variables
    uint8_t port_value = 0; 
    float freqHz = 0 ;
    unsigned char toggleButton = 0 ; // make sure code executes on falling edge

    unsigned long time = millis();
    unsigned long timeoutCount ; // in miliseconds to reset tap count
    bool startTimeout = false;
    uint32_t period = 500; // hopefully in ms
    uint8_t counter = 0 ;

     while (1)
     {
      // Determine Duty Cylce State
      dutyCycle = TwentyFive;
      UpdateDutyCycle(dutyCycle);
      // debug
        port_value = PIND; // Using Hexadecimal Numbering System
        
        if(port_value < 5) // if button pressed, should be less than 5. Seem to be around max 9
        {
          while(port_value < 9)//wait for button to release
          {    
            port_value = PIND; // get pin input
            //Serial.print("Button press value"); // debug
            //Serial.println(port_value); // debug
            // FIXME Make a function that detects inactivity after so long
          }
           numOfTaps++;
           Serial.print("Number of taps "); Serial.println(numOfTaps);
           if (numOfTaps == 1)
           {
              firstTimeSample = millis(); // get first time sample
              timeoutCount = millis(); startTimeout = true; // start timout routine
           }
           else
           {
              avgTime = GetTimeSample(firstTimeSample);// get second sample
              // debug lines
              Serial.print("average time is ");
              Serial.print(firstTimeSample); Serial.print(" - "); Serial.print(millis()); Serial.print(" = ");
              Serial.println(avgTime);
              
              // Get Frequency 
              freqHz  = CalculateFrequency(avgTime);

              // Get period
              period = avgTime;   

              // Calculate BPM 
              Serial.print("BPM: ");
              Serial.println(CalculateBPM(freqHz));     
              //reset variables when done
              numOfTaps = 0;
              firstTimeSample = 0;
              avgTime = 0;
              startTimeout = false;
           }
        }
        else
        {
        }

        if(millis() - time >= period)// pulse 
        {
            time = millis();
            // hard code pin high and pin low using counter%2 ==  0 
            // so we can add small delay to the high if or the low else to adjust duty cycle
            if((counter%2) == 0)
            {
              delay(negDutyCycleDelay);
              PORTB |= (1<<5);
              //Serial.println("ON");// debug line
              counter = 0;
            }
            else
            {
              delay(posDutyCycleDelay);
              PORTB &= ~(1<<5);
              //Serial.println("OFF"); // debug line
            }
            
            counter++;

            //PORTB ^= (1<<5);  // PB5 output toggle// debug line
            //Serial.print("Made it in the pulse task, period interval is "); Serial.println(period);
         } 


        
        // Goal is to prevent a button press hanging and ruining the user experiance. 
        //User must tap two times to set device frequency/BPM.
        if((millis() - timeoutCount >= timeoutValue) && startTimeout == true)
        {
          Serial.println("Resetting tap count");
          //Serial.println(millis() - timeoutCount);
          startTimeout = false;
          numOfTaps = 0;
        }

        //while(!timerInterruptFlag==1){}//wait for timer to tick
        //counter++;
        //timerInterruptFlag = 0 ;
     } 
}
//FIXME Have pointers be inputed so we can make the pos and neg varables be loclaized
void UpdateDutyCycle(DuctyCycleRatio _dutyCycle)
{
  //Serial.println(_dutyCycle);
  //Serial.println(posDutyCycleDelay);
  //Serial.println(negDutyCycleDelay);
  switch(_dutyCycle)
      {
        case(0):
        {
          posDutyCycleDelay = 0;
          negDutyCycleDelay = 100;
          break;
        }
        case(1):
        {
          posDutyCycleDelay = 0;
          negDutyCycleDelay = 0;
          break;
        }
        case(2):
        {
          posDutyCycleDelay = 100;
          negDutyCycleDelay = 0;
          break;
        }
        default:
        {
          posDutyCycleDelay = 0;
          negDutyCycleDelay = 0;
          break;
        }
      }
}

float CalculateFrequency(uint32_t ms)
{
  float hertz; // we use floats temporarily for precision
  hertz = (float)((float)(1/(float)(ms)*1000)/2);
  int roundedHertz = round(hertz);
  Serial.print("Frequency calculated (HZ): ");
  Serial.print(hertz);
  Serial.print(" from the miliseconds value (ms) "); Serial.println(ms);
  return hertz;
}

uint32_t CalculateBPM(float frequency)
{
  return (int)((float)frequency * (float)60.0);
}

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
  OCR1A = (F_CPU / 256 * ((float)miliseconds)/1000);       // Counter compare match value. 16MHz / prescaler * delay time (in seconds.)

  // Reenable the interrupts 
  sei();
}


ISR(TIMER1_COMPA_vect)
{
  // WARNING, it seems with arduino's 16MHZ CPU speed the smallest delay amount that we can set the 
  //OCR1A to is 24ms when the value is set to 1 ms
  timerInterruptFlag = 1;
  //PORTB ^= (1<<5);  // PB5 output toggle// debug line
  //Serial.println("Interrupt");
}
