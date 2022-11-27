// LIBRARIES
#include "Arduino.h"
#include "NewPing.h"
#include "Servo.h"
#include "FastLED.h"


// PIN DEFINITIONS

// Sonar sensors pins
#define HCSR04_PIN_TRIG 2
#define HCSR04_PIN_ECHO_NORTH A5
#define HCSR04_PIN_ECHO_WEST A3
#define HCSR04_PIN_ECHO_SOUTH A4
#define HCSR04_PIN_ECHO_EAST A2

// Servo motors pins
#define SERVOMD_PIN_SIG 11
#define SERVO9G_PIN_SIG 12
#define SERVO9G_2_PIN_SIG 13

// Motor A
#define enA 3
#define in1 4
#define in2 8

// Motor B
#define in3 7
#define in4 6
#define enB 5

// Led Strip pins
#define NUM_LEDS 73
#define DATA_PIN 9

// Infrared sensors pins
#define RIGHT A0
#define LEFT A1


// GLOBAL VARIABLES

// Sonar sensors
NewPing sonar_north(HCSR04_PIN_TRIG, HCSR04_PIN_ECHO_NORTH);
NewPing sonar_south(HCSR04_PIN_TRIG, HCSR04_PIN_ECHO_SOUTH);
NewPing sonar_west(HCSR04_PIN_TRIG, HCSR04_PIN_ECHO_WEST);
NewPing sonar_east(HCSR04_PIN_TRIG, HCSR04_PIN_ECHO_EAST);

// Servo motors
Servo servoMD;
Servo servo9g;
Servo servo9g_2;

// Led strip variables
CRGB leds[NUM_LEDS];
CRGB colors[3] = {CRGB::Red, CRGB::Green, CRGB::Blue};
CRGB phase0_colors[3] = {CRGB::Red, CRGB::Red, CRGB::Red};

// Variables used for the Led Strip animation
int loading_index = 0;
int led_index[3] = {0, 25, 50};

// Data received from serial communication w/ raspberry
int data;

// Arduino state: 0 is busy, 1 is free 
int status = '1';

// Phase selected at the robot's startup
int phase;

// Sonar's sensing distance
int dist = 50;

// Time the motors go in reverse, for when the robots backs off, after being scared.
int reverse_time = 750;

// Variable for storing time
unsigned long myTime = 0;


// *********
// FUNCTIONS
// *********


// Read serial data from raspberry, and set the internal status of arduino to free, when receiving '1'
void wait_serial_data(){
    data = Serial.read();
    if (data == '1') status = data; 
}

// The same of wait_serial_data(), but adapted for phase2, with different signals for each planet
void get_serial_data(){
     data = Serial.read();
     if(data == '1' || data == 'A' || data == 'B' || data == 'C' || data == 'D' || data == 'E' || data == 'F' || data == 'G' || data == 'H'){
      if(data == '1') led_schema_color(CRGB::Navy);
      status = data; 
     }
}

// Send signal over serial to raspberry, corresponding to the "scared" or the "normal" state (in the raspberry)
void send_serial_state(String state){
    if(state.equals("scared")) Serial.write('0');
    if(state.equals("normal")) Serial.write('1');
}

// Led strip animation: blink 5 times with green color
void led_schema_green(){
  for(int i=0;i < 5; i++){
    for(int j=0; j < NUM_LEDS; j++){
      leds[j] = CRGB::Green;
    }
    FastLED.show();
    delay(500);
    if(i<4){
      FastLED.clear();
      FastLED.show();
      delay(500);
    }
  }
}

// Led strip animation: turn on all leds with a color
void led_schema_color(CRGB color){
    for(int j=0; j < NUM_LEDS; j++){
      leds[j] = color;
    }
    FastLED.show();
}

// Led strip animation: 3 segments with different colors, rotate around the spaceship
void led_schema_normal(CRGB colors[]){
    if(millis() - myTime >= 50){
      FastLED.clear();
      for(int j = 0; j < NUM_LEDS; j++){
        for(int i = 0; i < 3; i++){
          int distance = j - led_index[i];
          if(distance < 10 && distance > 0){
            leds[j] = colors[i];
          }
          else if((distance * - 1) > NUM_LEDS - 10){
            leds[j] = colors[i];
          }
        }
       }
      
      for(int i=0; i<3; i++){
        if(led_index[i] < NUM_LEDS - 1){
          led_index[i] ++;
        }
        else{
         led_index[i] = 0;
        }
      }

      myTime = millis();
      FastLED.show();
    }
}

// Led strip animation: color schema used during boot (blue segment rotating around)
void led_schema_loading(CRGB color){
  FastLED.clear();
  for(int j=0; j < NUM_LEDS; j++){
    int distance = (j - loading_index);
    if(distance < 10 && distance > 0){
      leds[j] = color;
    }
    else if((distance * - 1) > NUM_LEDS - 10){
      leds[j] = color;
    }
  }
  
  if(loading_index < NUM_LEDS - 1){
    loading_index ++;
  }
  else{
     loading_index = 0;
  }
  FastLED.show();
  delay(50);
}

// Turn on motor A
void left_motor_on(){
    // accende il motore a
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
    // imposta la velocità a 200 (0~255)
    analogWrite(enA, 200);  
}

// Turn on motor B
void right_motor_on(){
    // accende il motore b
    digitalWrite(in3, HIGH);
    digitalWrite(in4, LOW);
    // imposta la velocità a 200 (0~255)
    analogWrite(enB, 200);
}

// Turn both motors on
void motors_on(){
    left_motor_on();
    right_motor_on();
}

// Turn both motors on, in reverse mode
void motors_reverse(){
    // accende il motore a
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
    // imposta la velocità a 200 (0~255)
    analogWrite(enA, 200);  
    // accende il motore b
    digitalWrite(in3, LOW);
    digitalWrite(in4, HIGH);
    // imposta la velocità a 200 (0~255)
    analogWrite(enB, 200);
}

// Turn off both motors
void motors_off(){
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);  
    digitalWrite(in3, LOW);
    digitalWrite(in4, LOW);
    analogWrite(enA, 0);  
    analogWrite(enB, 0);  
}

// Head movement animation procedure
void head_movement()
{
    //First rotation of the head
    servo9g.attach(SERVO9G_PIN_SIG);
    servo9g.write(180);  
    delay(700);                          
    servo9g.write(0);    
    delay(700); 
    servo9g.write(90);   
    delay(700);        
    servo9g.detach(); 
      
    //The head hides going down
    servoMD.attach(SERVOMD_PIN_SIG);  
    servoMD.write(120);
    delay(1000);
    servoMD.detach(); 

    //The eye moves looking on who is there
    servo9g_2.attach(SERVO9G_2_PIN_SIG);         
    servo9g_2.write(160);  
    delay(700);                              
    servo9g_2.write(70);    
    delay(700); 
    servo9g_2.write(90);    
    delay(700);
    servo9g_2.detach();
     
    //Second rotation of the head 
    servo9g.attach(SERVO9G_PIN_SIG);         
    servo9g.write(180);  
    delay(700);                              
    servo9g.write(0);    
    delay(700); 
    servo9g.write(90);    
    delay(700);
    servo9g.write(0);
    delay(700);
    servo9g.detach();

    //The head goes up slowly
    servoMD.attach(SERVOMD_PIN_SIG); 
    for(int i=120; i>=0; i-- ){
        servoMD.write(i);  
        delay(20); 
    }
    delay(700);
    servoMD.detach(); 
}

// Reset the head to the initial zero position
void reset_head(){
  
  servoMD.attach(SERVOMD_PIN_SIG);   
  servo9g.attach(SERVO9G_PIN_SIG);
  servo9g_2.attach(SERVO9G_2_PIN_SIG);   
  
  servoMD.write(0);
  servo9g.write(0);
  servo9g_2.write(90);
  
  delay(3000); 
  
  servoMD.detach(); 
  servo9g.detach(); 
  servo9g_2.detach();

}

// Procedure for perfoming the line-following strategy
void line_follow(){

  // Compare both sensor to decide the direction
  if (digitalRead(RIGHT)==0 && digitalRead(LEFT)==0) 
  {
    //MOVE FORWARD//
    
    analogWrite(enA, 120); // set right motors speed
    analogWrite(enB, 120); // set left motors speed
    
    //run right motors clockwise
    digitalWrite(in2, LOW);
    digitalWrite(in1, HIGH);
    //run left motors clockwise
    digitalWrite(in3, HIGH);
    digitalWrite(in4, LOW);
  }
  else if (digitalRead(RIGHT)==0 && digitalRead(LEFT)==1)
  {
    //MOVE RIGHT//
    
    analogWrite(enA, 200); //set right motors speed
    analogWrite(enB, 200); //set left motors speed
    
    //run right motors clockwise
    digitalWrite(in2, LOW);
    digitalWrite(in1, HIGH);
    //run left motors anti-clockwise
    digitalWrite(in3, LOW);
    digitalWrite(in4, HIGH);
  }
  else if (digitalRead(RIGHT)==1 && digitalRead(LEFT)==0)
  { 
    //MOVE-LEFT//
    
    analogWrite(enA, 200); //set right motors speed
    analogWrite(enB, 200); //set left motors speed
  
    //run right motors anti-clockwise
    digitalWrite(in2, HIGH);
    digitalWrite(in1, LOW);
    //run left motors clockwise
    digitalWrite(in3, HIGH);
    digitalWrite(in4, LOW);
  }
  else if (digitalRead(RIGHT)==1 && digitalRead(LEFT)==1)
  { 
    //STOP//
    
    analogWrite(enA, 0); //set right motors speed
    analogWrite(enB, 0); //set left motors speed
  
    //stop right motors
    digitalWrite(in2, LOW);
    digitalWrite(in1, LOW);
    //stop left motors
    digitalWrite(in3, LOW);
    digitalWrite(in4, LOW);
    }
}


// Phase 0: Just an easter egg, nothing special
void phase0()
{
  led_schema_normal(phase0_colors);
}


// Phase 1
void phase1() 
{
    wait_serial_data();

    // Perform a reading from sonar sensors
    int hcsr04Dist = sonar_north.ping_cm();
    int hcsr04Dist_1 = sonar_south.ping_cm();
    int hcsr04Dist_2 = sonar_west.ping_cm();
    int hcsr04Dist_3 = sonar_east.ping_cm();

    if (status == '1') 
    {
        motors_on();
        led_schema_normal(colors);

        // Check which sonar was triggered: north, south, weast, east
        
        if(hcsr04Dist <= dist && hcsr04Dist > 0) {                //NORTH
            motors_reverse();
            delay(reverse_time);
            status = '0';
        }
        
        if(hcsr04Dist_1 <= dist && hcsr04Dist_1 > 0) {            //SOUTH
            motors_off();
            right_motor_on();
            delay(1800); //to be tuned
            motors_reverse();
            delay(reverse_time);
            status = '0';
        }
        
        if(hcsr04Dist_2 <= dist && hcsr04Dist_2 > 0) {            //WEST
            motors_off();
            left_motor_on();
            delay(900); //to be tuned
            motors_reverse();
            delay(reverse_time);
            status = '0';
        }
        
        if(hcsr04Dist_3 <= dist && hcsr04Dist_3 > 0) {            //EAST
            motors_off();
            right_motor_on();
            delay(900); //to be tuned
            motors_reverse();
            delay(reverse_time);
            status = '0';
        }
        
    } else {
        motors_off();
        send_serial_state("scared");
        led_schema_color(CRGB::Red);
        head_movement();
        delay(1000);
        led_schema_green();
        delay(1000);
        send_serial_state("normal"); 
        delay(1000); //to be tuned

        // Wait the raspberry for sending the free signal '1' to the arduino
        while(1) {
            led_schema_normal(colors);
            wait_serial_data();
            if(data == '1') break;
        }
        status = '1';

    }
}


void phase2(){

  if(status == '1'){
      get_serial_data();
      line_follow();
  }
  else{    
    motors_off();

    // Blink the led strip with the color corresponding to the received planet
    if(status == 'A')
      led_schema_color(CRGB::Aqua);
    if(status == 'B')
      led_schema_color(CRGB::BurlyWood);
    if(status == 'C')
      led_schema_color(CRGB::Red);
    if(status == 'D')
      led_schema_color(CRGB::DarkSlateBlue);
    if(status == 'E')
      led_schema_color(CRGB::Blue);
    if(status == 'F')
      led_schema_color(CRGB::Moccasin);
    if(status == 'G')
      led_schema_color(CRGB::MediumSpringGreen);
    if(status == 'H')
      led_schema_color(CRGB::DarkOrange);
      
     while(1) {
        get_serial_data();
        if(status == '1') break;
     }
  }
}


// Setup
void setup() 
{
    delay(2000);
    //Init serial communication
    Serial.begin(9600);
    
    pinMode(in1, OUTPUT);
    pinMode(in2, OUTPUT);
    pinMode(in3, OUTPUT);
    pinMode(in4, OUTPUT); 
    pinMode(enA, OUTPUT);
    pinMode(enB, OUTPUT);

    // Reset the head to zero position
    reset_head();

    // Init Led Strip
    FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
    FastLED.clear();
    FastLED.show();

    // Wait the raspberry for sending the selected phase and perform the led strip boot animation
    while (1){
      led_schema_loading(CRGB::Blue);
      phase = Serial.read();

      if (phase == 'a' || phase == 'b' || phase == 'c'){
        if(phase == 'b') {
          led_schema_color(CRGB::Navy);
          }
        else {
          FastLED.clear();
          FastLED.show();
          }
        break;
      }
    }
  
}


// Main Loop: based on the signal received during setup, select the correct phase
void loop()
{
  if(phase == 'a'){
    phase1();
  }
  if(phase == 'b'){
    phase2();
  }
  if(phase == 'c'){
    phase0();
  }
}
