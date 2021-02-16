/*
  RC Radio Flight Simulator Controller
  Created by Christian Barran 2020
  https://www.youtube.com/c/justbarran
  Released into the public domain.
  code uses library: 
  https://github.com/MHeironimus/ArduinoJoystickLibrary
  NOTE: You may need to adjust channels depending on your receiver
*/

#include <Joystick.h>
//#define SERIALPRINT

#define UPDATE_RATE_MS 1
#define RC_MIN_PULSE_US   1000
#define RC_MID_PULSE_US   1500
#define RC_MAX_PULSE_US   2000
#define RC_DEADBAND_US    10

//Pin to Channel
#define RC_PIN_CH1 7 
#define RC_PIN_CH2 3 
#define RC_PIN_CH3 2 
#define RC_PIN_CH4 0 
#define RC_PIN_CH5 1 

#define BUTTON_1 8 //MODE
#define BUTTON_2 9 //AUX

//Channel to Drone controls
#define CH_ROLL   0
#define CH_PITCH  1
#define CH_THRO   2
#define CH_YAW    3
#define CH_AUX_1  4

#define JOY_THROTTLE_LOW  1000 
#define JOY_THROTTLE_HIGH 2000 
#define JOY_RUDDER_LOW    1000 
#define JOY_RUDDER_HIGH   2000
#define JOY_RANGE_LOW     1000
#define JOY_RANGE_HIGH    2000

#define NUM_CH 5
#define NUM_READINGS 10
// Create the Joystick
Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID,JOYSTICK_TYPE_GAMEPAD,
  2, 0,                   // Button Count, Hat Switch Count
  true, true, false,      // X,Y,Z Axis
  true, true, false,    // Rx,Ry,Rz
  true, true,             // rudder,throttle
  false, false, false);   // accelerator, brake, or steering

const byte channel_pin[] = {RC_PIN_CH1,RC_PIN_CH2,RC_PIN_CH3,RC_PIN_CH4,RC_PIN_CH5};
volatile unsigned long rising_start[NUM_CH]  = {0,0,0,0,0};
volatile unsigned long channel_input[NUM_CH]  = {0,0,0,0,0};
volatile unsigned long channel_total[NUM_CH]  = {0,0,0,0,0};
volatile unsigned long channel_reading[NUM_READINGS*NUM_CH];
volatile unsigned long channel_output[NUM_CH]  = {0,0,0,0,0};
volatile unsigned long channel_dead[NUM_CH]  = {0,0,0,0,0};
int readIndex = 0;  


void processPin(byte pin) {
  uint8_t trigger = digitalRead(channel_pin[pin]);
  if(trigger == HIGH){rising_start[pin] = micros();} 
  else if(trigger == LOW) {channel_input[pin] = micros() - rising_start[pin]; }
}

void CH_THRO_ISR(){
  processPin(CH_THRO);
}

void CH_YAW_ISR(){
  processPin(CH_YAW);
}
void CH_PITCH_ISR(){
  processPin(CH_PITCH); 
}
void CH_ROLL_ISR(){
  processPin(CH_ROLL);
}
void CH_AUX_1_ISR(){
  processPin(CH_AUX_1);
}

void setup(){
  pinMode(RC_PIN_CH1, INPUT); // Set our input pins as such
  pinMode(RC_PIN_CH2, INPUT);
  pinMode(RC_PIN_CH3, INPUT);
  pinMode(RC_PIN_CH4, INPUT);
  pinMode(RC_PIN_CH5, INPUT);
  pinMode(BUTTON_1,   INPUT);
  pinMode(BUTTON_2,   INPUT);
  
#ifdef SERIALPRINT
  Serial.begin(115200);
#endif
  for(int i = 0;i<(NUM_CH*NUM_READINGS);i++)
    {
        channel_reading[i]=0;
    }
  
  Joystick.begin();
  Joystick.setThrottleRange(JOY_THROTTLE_LOW,JOY_THROTTLE_HIGH);
  Joystick.setRudderRange(JOY_RUDDER_LOW,JOY_RUDDER_HIGH);
  Joystick.setRxAxisRange(JOY_RANGE_LOW,JOY_RANGE_HIGH);
  Joystick.setRyAxisRange(JOY_RANGE_LOW,JOY_RANGE_HIGH);
  Joystick.setXAxisRange(JOY_RANGE_LOW,JOY_RANGE_HIGH);
  Joystick.setYAxisRange(JOY_RANGE_LOW,JOY_RANGE_HIGH);
  Joystick.setButton(0,0);
  Joystick.setButton(1,0); 
  
  attachInterrupt(digitalPinToInterrupt(channel_pin[CH_THRO]),   CH_THRO_ISR,  CHANGE);
  attachInterrupt(digitalPinToInterrupt(channel_pin[CH_YAW]),    CH_YAW_ISR,   CHANGE);
  attachInterrupt(digitalPinToInterrupt(channel_pin[CH_PITCH]),  CH_PITCH_ISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(channel_pin[CH_ROLL]),   CH_ROLL_ISR,  CHANGE);
  attachInterrupt(digitalPinToInterrupt(channel_pin[CH_AUX_1]),  CH_AUX_1_ISR, CHANGE);
  //delay(1);
}

void loop()
{  
  for(int i = 0;i<NUM_CH;i++)
  {
    int readingNow = (i*NUM_READINGS)+readIndex;
    channel_total[i] = channel_total[i]- channel_reading[readingNow];
    channel_reading[readingNow]= channel_input[i];
    channel_total[i] = channel_total[i]+ channel_reading[readingNow];
    channel_output[i]=channel_total[i]/NUM_READINGS;
    if(channel_output[i]> channel_dead[i])
      {
        if((channel_output[i]-channel_dead[i])<RC_DEADBAND_US)
        {
          channel_output[i]=channel_dead[i];
        }
      }
    if(channel_output[i]< channel_dead[i])
      {
        if((channel_dead[i]-channel_output[i])<RC_DEADBAND_US)
        {
          channel_output[i]=channel_dead[i];
        }
      }
     channel_dead[i]=channel_output[i];    
  }
  readIndex++;
  if(readIndex>=NUM_READINGS)
  {
    readIndex=0;
  }
  
#ifdef SERIALPRINT 
  for(int i=0;i<NUM_CH;i++)
  {
    Serial.print(channel_output[i]);
    Serial.print("\t"); 
  }
  Serial.println();
 #endif
  
  Joystick.setThrottle(channel_output[CH_THRO]); 
  Joystick.setRudder(channel_output[CH_YAW]);
  Joystick.setYAxis(channel_output[CH_PITCH]);
  Joystick.setXAxis(channel_output[CH_ROLL]);
  Joystick.setRxAxis(channel_output[CH_AUX_1]);  
  if(digitalRead(BUTTON_1)==LOW){Joystick.setButton(0,1);}
  else {Joystick.setButton(0,0);}
  if(digitalRead(BUTTON_2)==LOW){Joystick.setButton(1,1);}
  else{Joystick.setButton(1,0);}
  //delay(UPDATE_RATE_MS);
}
