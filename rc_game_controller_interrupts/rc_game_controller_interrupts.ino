
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

#define UPDATE_RATE_MS 10
#define RC_MIN_PULSE_US   1000
#define RC_MID_PULSE_US   1500
#define RC_MAX_PULSE_US   2000
#define PULSE_IN_WAIT_US  20000 //RC Time Between Pulses 20000us 

//Pin to Channel
#define RC_PIN_CH1 7 //THR0
#define RC_PIN_CH2 3 //AILE
#define RC_PIN_CH3 2 //ELEV
#define RC_PIN_CH4 0 //RUDD
#define RC_PIN_CH5 1 //MODE

#define BUTTON_1 8 //MODE
#define BUTTON_2 9 //AUX

//Channel to Drone controls
#define CH_ROLL   RC_PIN_CH1
#define CH_PITCH  RC_PIN_CH2
#define CH_THRO   RC_PIN_CH3
#define CH_YAW    RC_PIN_CH4
#define CH_AUX_1  RC_PIN_CH5

#define JOY_THROTTLE_LOW  1000 
#define JOY_THROTTLE_HIGH 2000 
#define JOY_RUDDER_LOW    1000 
#define JOY_RUDDER_HIGH   2000 
#define JOY_RANGE_LOW     1000
#define JOY_RANGE_HIGH    2000

#define NUM_READINGS 10
// Create the Joystick
Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID,JOYSTICK_TYPE_GAMEPAD,
  2, 0,                   // Button Count, Hat Switch Count
  true, true, false,      // X,Y,Z Axis
  true, true, false,    // Rx,Ry,Rz
  true, true,             // rudder,throttle
  false, false, false);   // accelerator, brake, or steering

volatile unsigned long input_thro_rising  = 0;
volatile unsigned long input_yaw_rising   = 0;
volatile unsigned long input_pitch_rising = 0;
volatile unsigned long input_roll_rising  = 0;
volatile unsigned long input_aux_1_rising = 0;

volatile unsigned long input_thro_falling  = 0;
volatile unsigned long input_yaw_falling   = 0;
volatile unsigned long input_pitch_falling = 0;
volatile unsigned long input_roll_falling  = 0;
volatile unsigned long input_aux_1_falling = 0;
//RC Controller Values
volatile unsigned long input_thro  = 0;
volatile unsigned long input_yaw   = 0;
volatile unsigned long input_pitch = 0;
volatile unsigned long input_roll  = 0;
volatile unsigned long input_aux_1 = 0;

int32_t readings_throttle[NUM_READINGS];
int32_t readings_rudder[NUM_READINGS];
int32_t readings_x [NUM_READINGS];
int32_t readings_y[NUM_READINGS];
int32_t readings_aux_x[NUM_READINGS];
int32_t readings_aux_y[NUM_READINGS];

int8_t index_throttle = 0;
int8_t index_rudder = 0;
int8_t index_x = 0;
int8_t index_y = 0;
int8_t index_aux_x = 0;
int8_t index_aux_y = 0;

int32_t total_throttle = 0;
int32_t total_rudder = 0;
int32_t total_x = 0;
int32_t total_y = 0;
int32_t total_aux_x = 0;
int32_t total_aux_y = 0;

//Simulator Values
int32_t sim_throttle = 0;
int32_t sim_rudder = 0;
int32_t sim_x = 0;
int32_t sim_y = 0;
int32_t sim_aux_x = 0;
int32_t sim_aux_y = 0;

void CH_THRO_ISR(){
  if(digitalRead(CH_THRO) == HIGH){
    input_thro_rising = micros();
  }
  else{
    input_thro_falling = micros();
    if(input_thro_falling>input_thro_rising){//Check for overflow
    input_thro = input_thro_falling-input_thro_rising;
    }
  }  
}

void CH_YAW_ISR(){
  if(digitalRead(CH_YAW) == HIGH){
    input_yaw_rising = micros();
  }
  else
  {
    input_yaw_falling = micros();
    if(input_yaw_falling>input_yaw_rising){ //Check for overflow
    input_yaw = input_yaw_falling-input_yaw_rising;
    }
  }  
}
void CH_PITCH_ISR(){
  if(digitalRead(CH_PITCH) == HIGH){
    input_pitch_rising = micros();
  }
  else
  {
    input_pitch_falling = micros();
    if(input_pitch_falling>input_pitch_rising){//Check for overflow
    input_pitch = input_pitch_falling-input_pitch_rising;
    }
  }  
}
void CH_ROLL_ISR(){
  if(digitalRead(CH_ROLL) == HIGH){
    input_roll_rising = micros();
  }
  else
  {
    input_roll_falling = micros();
    if(input_roll_falling>input_roll_rising){//Check for overflow
    input_roll = input_roll_falling-input_roll_rising;
    }
  }  
}
void CH_AUX_1_ISR(){
  if(digitalRead(CH_AUX_1) == HIGH){
    input_aux_1_rising = micros();
  }
  else
  {
    input_aux_1_falling = micros();
    if(input_aux_1_falling>input_aux_1_rising){//Check for overflow
    input_aux_1 = input_aux_1_falling-input_aux_1_rising;
    }
  }  
}

void setup() {
  pinMode(RC_PIN_CH1, INPUT); // Set our input pins as such
  pinMode(RC_PIN_CH2, INPUT);
  pinMode(RC_PIN_CH3, INPUT);
  pinMode(RC_PIN_CH4, INPUT);
  pinMode(RC_PIN_CH5, INPUT);
  pinMode(BUTTON_1,   INPUT);
  pinMode(BUTTON_2,   INPUT);
  
  for (int thisReading = 0; thisReading < NUM_READINGS; thisReading++) {
      readings_throttle[thisReading]=0;
      readings_rudder[thisReading]=0;
      readings_x [thisReading]=0;
      readings_y[thisReading]=0;
      readings_aux_x[thisReading]=0;
      readings_aux_y[thisReading]=0;
  }
#ifdef SERIALPRINT
  Serial.begin(115200);
#endif
  Joystick.begin();
  Joystick.setThrottleRange(JOY_THROTTLE_LOW,JOY_THROTTLE_HIGH);
  Joystick.setRudderRange(JOY_RUDDER_LOW,JOY_RUDDER_HIGH);
  Joystick.setRxAxisRange(JOY_RANGE_LOW,JOY_RANGE_HIGH);
  Joystick.setRyAxisRange(JOY_RANGE_LOW,JOY_RANGE_HIGH);
  Joystick.setXAxisRange(JOY_RANGE_LOW,JOY_RANGE_HIGH);
  Joystick.setYAxisRange(JOY_RANGE_LOW,JOY_RANGE_HIGH);
  Joystick.setButton(0,0);
  Joystick.setButton(1,0); 
  
  attachInterrupt(digitalPinToInterrupt(CH_THRO),   CH_THRO_ISR,  CHANGE);
  attachInterrupt(digitalPinToInterrupt(CH_YAW),    CH_YAW_ISR,   CHANGE);
  attachInterrupt(digitalPinToInterrupt(CH_PITCH),  CH_PITCH_ISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(CH_ROLL),   CH_ROLL_ISR,  CHANGE);
  attachInterrupt(digitalPinToInterrupt(CH_AUX_1),  CH_AUX_1_ISR, CHANGE);
  delay(1);
}

void loop() {  
  total_throttle = total_throttle - readings_throttle[index_throttle];
  readings_throttle[index_throttle] =  map(input_thro,RC_MIN_PULSE_US,RC_MAX_PULSE_US,JOY_THROTTLE_LOW,JOY_THROTTLE_HIGH);
  total_throttle = total_throttle + readings_throttle[index_throttle];
  index_throttle++;
  if(index_throttle>= NUM_READINGS)
  {
    index_throttle = 0;
  }
  sim_throttle = total_throttle/NUM_READINGS;
  Joystick.setThrottle(sim_throttle);  
#ifdef SERIALPRINT
  Serial.print(sim_throttle);
  Serial.print("\t");    
#endif
  total_rudder = total_rudder - readings_rudder[index_rudder];
  readings_rudder[index_rudder] = map(input_yaw,RC_MIN_PULSE_US,RC_MAX_PULSE_US,JOY_RUDDER_LOW,JOY_RUDDER_HIGH);
  total_rudder = total_rudder + readings_rudder[index_rudder];
  index_rudder++;
  if(index_rudder>= NUM_READINGS)
  {
    index_rudder = 0;
  }
  sim_rudder = total_rudder/NUM_READINGS;
  Joystick.setRudder(sim_rudder);
#ifdef SERIALPRINT
  Serial.print(sim_rudder);
  Serial.print("\t");
#endif
  total_y = total_y - readings_y[index_y];
  readings_y[index_y]= map(input_pitch,RC_MIN_PULSE_US,RC_MAX_PULSE_US,JOY_RANGE_LOW,JOY_RANGE_HIGH);
   total_y = total_y + readings_y[index_y];
  index_y++;
  if(index_y>= NUM_READINGS)
  {
    index_y = 0;
  }
  sim_y = total_y/NUM_READINGS;
  Joystick.setYAxis(sim_y);
#ifdef SERIALPRINT
  Serial.print(sim_y);
  Serial.print("\t");
#endif
  total_x = total_x - readings_x[index_x];
  readings_x[index_x]=map(input_roll,RC_MIN_PULSE_US,RC_MAX_PULSE_US,JOY_RANGE_LOW,JOY_RANGE_HIGH);
  total_x = total_x + readings_x[index_x];
  index_x++;
  if(index_x>= NUM_READINGS)
  {
    index_x = 0;
  }
  sim_x = total_x/NUM_READINGS;
  Joystick.setXAxis(sim_x);
#ifdef SERIALPRINT
  Serial.print(sim_x);
  Serial.print("\t"); 
#endif
  sim_aux_x =     map(input_aux_1,RC_MIN_PULSE_US,RC_MAX_PULSE_US,JOY_RANGE_LOW,JOY_RANGE_HIGH);
  Joystick.setRxAxis(sim_aux_x);
#ifdef SERIALPRINT
  Serial.print(sim_aux_x);
  Serial.println(); 
#endif
  if(digitalRead(BUTTON_1)==LOW){
    Joystick.setButton(0,1);
  }
  else{
    Joystick.setButton(0,0);
  }
  if(digitalRead(BUTTON_2)==LOW){
    Joystick.setButton(1,1);
  }
  else{
    Joystick.setButton(1,0);
  }
 delay(UPDATE_RATE_MS);
}
