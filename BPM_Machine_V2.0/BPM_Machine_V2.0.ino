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
 
// Global Variables
uint32_t firstTimeSample = 0 ;
uint32_t secondTimeSample = 0 ;
uint32_t avgTime = 0;
unsigned char numOfTaps = 0;

// Timer Global Settings
unsigned char timerInterruptFlag = 0;
uint32_t miliseconds = 1;

// Prototypes
uint32_t GetTimeSample(uint32_t sample);
uint16_t CalculateFrequency(uint32_t ms);
void SetTimerSettings();

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
    uint16_t freqHz = 0 ;
    unsigned char toggleButton = 0 ; // make sure code executes on falling edge

    unsigned long time = millis();
    
    uint32_t period = 500; // hopefully in ms
    int counter = 0 ;
     while (1)
     {
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
           }
           else
           {
              avgTime = GetTimeSample(firstTimeSample);// get second sample
              // debug lines
              Serial.print("average time is ");
              Serial.print(firstTimeSample); Serial.print(" - "); Serial.print(millis()); Serial.print(" = ");
              Serial.println(avgTime);
              
              // Set PWM to match button time difference
              freqHz  = CalculateFrequency(avgTime);

              period = avgTime;   
              SetTimerSettings();
                         
              //reset variables when done
              numOfTaps = 0;
              firstTimeSample = 0;
              avgTime = 0;
           }

        }
        else
        {
        }

        if(millis() - time >= period)// pulse 
        {
            time = millis();
            PORTB ^= (1<<5);  // PB5 output toggle// debug line
            //Serial.print("Made it in the pulse task, period interval is "); Serial.println(period);
        }
        //while(!timerInterruptFlag==1){}//wait for timer to tick
        //counter++;
        //timerInterruptFlag = 0 ;
     } 
}


uint16_t CalculateFrequency(uint32_t ms)
{
  float hertz; // we use floats temporarily for precision
  hertz = (float)((float)(1/(float)(ms)*1000));
  int roundedHertz = round(hertz);
  Serial.print("Frequency calculated (HZ): ");
  Serial.print(hertz);
  Serial.print(" from the miliseconds value (ms) "); Serial.println(ms);
  return hertz;
}

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

uint32_t GetTimeSample(uint32_t sample)
{
  int avg = 0 ;
  return avg = millis() - sample; // get second sample
}

ISR(TIMER1_COMPA_vect)
{
  // WARNING, it seems with arduino's 16MHZ CPU speed the smallest delay amount that we can set the 
  //OCR1A to is 24ms when the value is set to 1 ms
  timerInterruptFlag = 1;
  //PORTB ^= (1<<5);  // PB5 output toggle// debug line
  //Serial.println("Interrupt");
}
