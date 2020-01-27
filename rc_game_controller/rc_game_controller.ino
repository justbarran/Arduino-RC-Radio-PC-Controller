
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

#define RC_MIN_PULSE_US   1000
#define RC_MID_PULSE_US   1500
#define RC_MAX_PULSE_US   2000
#define PULSE_IN_WAIT_US  25000 //RC Time Between Pulses 20000us 

#define RC_PIN_CH1 2 //THR0
#define RC_PIN_CH2 3 //AILE
#define RC_PIN_CH3 4 //ELEV
#define RC_PIN_CH4 5 //RUDD
#define RC_PIN_CH5 6 //MODE
#define RC_PIN_CH6 7 //AUX

#define BUTTON_1 8 //MODE
#define BUTTON_2 9 //AUX

#define CH_THRO   RC_PIN_CH3
#define CH_YAW    RC_PIN_CH4
#define CH_PITCH  RC_PIN_CH1
#define CH_ROLL   RC_PIN_CH2
#define CH_AUX_1  RC_PIN_CH5
#define CH_AUX_2  RC_PIN_CH6


#define JOY_THROTTLE_LOW  1000 
#define JOY_THROTTLE_HIGH 2000 
#define JOY_RUDDER_LOW    1000 
#define JOY_RUDDER_HIGH   2000 
#define JOY_RANGE_LOW     1000
#define JOY_RANGE_HIGH    2000

// Create the Joystick
Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID,JOYSTICK_TYPE_GAMEPAD,
  2, 0,                   // Button Count, Hat Switch Count
  true, true, false,      // X,Y,Z Axis
  true, true, false,    // Rx,Ry,Rz
  true, true,             // rudder,throttle
  false, false, false);   // accelerator, brake, or steering
  
//RC Controller Values
int16_t input_thro  = 0;
int16_t input_yaw   = 0;
int16_t input_pitch = 0;
int16_t input_roll  = 0;
int16_t input_aux_1 = 0;
int16_t input_aux_2 = 0;

//Simulator Values
int16_t sim_throttle = 0;
int16_t sim_rudder = 0;
int16_t sim_x = 0;
int16_t sim_y = 0;
int16_t sim_aux_x = 0;
int16_t sim_aux_y = 0;

void setup() {
  pinMode(RC_PIN_CH1, INPUT); // Set our input pins as such
  pinMode(RC_PIN_CH2, INPUT);
  pinMode(RC_PIN_CH3, INPUT);
  pinMode(RC_PIN_CH4, INPUT);
  pinMode(RC_PIN_CH5, INPUT);
  pinMode(RC_PIN_CH6, INPUT);
  pinMode(BUTTON_1, INPUT);
  pinMode(BUTTON_2, INPUT);
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
}

void loop() {  
  input_thro    = pulseIn(CH_THRO,  HIGH, PULSE_IN_WAIT_US); // Read the pulse width 
  input_yaw     = pulseIn(CH_YAW,   HIGH, PULSE_IN_WAIT_US); // each channel
  input_pitch   = pulseIn(CH_PITCH, HIGH, PULSE_IN_WAIT_US);
  input_roll    = pulseIn(CH_ROLL,  HIGH, PULSE_IN_WAIT_US); 
  input_aux_1   = pulseIn(CH_AUX_1, HIGH, PULSE_IN_WAIT_US); 
  input_aux_2   = pulseIn(CH_AUX_2, HIGH, PULSE_IN_WAIT_US);

  sim_throttle =  map(input_thro,RC_MIN_PULSE_US,RC_MAX_PULSE_US,JOY_THROTTLE_LOW,JOY_THROTTLE_HIGH);
  Joystick.setThrottle(sim_throttle);
#ifdef SERIALPRINT
  Serial.print(sim_throttle);
  Serial.print("\t");    
#endif
  sim_rudder =    map(input_yaw,RC_MIN_PULSE_US,RC_MAX_PULSE_US,JOY_RUDDER_LOW,JOY_RUDDER_HIGH);
  Joystick.setRudder(sim_rudder);
#ifdef SERIALPRINT
  Serial.print(sim_rudder);
  Serial.print("\t");
#endif
  sim_x =         map(input_pitch,RC_MIN_PULSE_US,RC_MAX_PULSE_US,JOY_RANGE_LOW,JOY_RANGE_HIGH);
  Joystick.setXAxis(sim_x);
#ifdef SERIALPRINT
  Serial.print(sim_x);
  Serial.print("\t");
#endif
  sim_y =         map(input_roll,RC_MIN_PULSE_US,RC_MAX_PULSE_US,JOY_RANGE_LOW,JOY_RANGE_HIGH);
  Joystick.setYAxis(sim_y);
#ifdef SERIALPRINT
  Serial.print(sim_y);
  Serial.print("\t"); 
#endif
  sim_aux_x =     map(input_aux_1,RC_MIN_PULSE_US,RC_MAX_PULSE_US,JOY_RANGE_LOW,JOY_RANGE_HIGH);
  Joystick.setRxAxis(sim_aux_x);
#ifdef SERIALPRINT
  Serial.print(sim_aux_x);
  Serial.print("\t");
#endif
  sim_aux_y =     map(input_aux_2,RC_MIN_PULSE_US,RC_MAX_PULSE_US,JOY_RANGE_LOW,JOY_RANGE_HIGH);
  Joystick.setRyAxis(sim_aux_y);
#ifdef SERIALPRINT
  Serial.print(sim_aux_y);
  Serial.println(); 
#endif
}
