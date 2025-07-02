#define CUSTOM_SETTINGS
#define INCLUDE_GAMEPAD_MODULE
#include <DabbleESP32.h>
#include <GamePadModule.h>


// Motor pins
#define motor_left_pin_L 27
#define motor_left_pin_R 14
#define motor_right_pin_L 32
#define motor_right_pin_R 33

// PWM pins
#define motor_left_speedPin_L 13
#define motor_left_speedPin_R 23
#define motor_right_speedPin_L 25
#define motor_right_speedPin_R 26
// Sensors
// hồng ngoại
#define infrared_left 35
#define infrared_right 4
// Siêu âm
#define trig1 18
#define echo1 19
#define trig2 22
#define echo2 21
#define trig3 17
#define echo3 16


// Globals
unsigned long echo_time1=0, echo_time2=0, echo_time3=0;
int distLeft=100, distFront=100, distRight=100;
int car_speed=135;
bool automatic=false, isBrake=false, isLeft = false, isRight = false, doneTurn = false;
const int DETECTION_THRESHOLD= 50, ATTACK_SPEED=200, TURN_SPEED=165;

// State machine
enum RobotState { WAITING, SEARCHING, ROTATING_LEFT, ROTATING_RIGHT, MOVING_FORWARD, ESCAPING };
RobotState currentState = WAITING;
unsigned long stateStartTime=0;
unsigned long stateDuration=0;

void setup() {
  Serial.begin(115200);
  Dabble.begin("Liberobot-SUMO");
  // setup pins...
  int motorPins[] = {motor_left_pin_L, motor_left_pin_R, motor_right_pin_L, motor_right_pin_R};
  for(int p: motorPins) pinMode(p, OUTPUT);
  int speedPins[] = {motor_left_speedPin_L, motor_left_speedPin_R, motor_right_speedPin_L, motor_right_speedPin_R};
  for(int p: speedPins) pinMode(p, OUTPUT);
  pinMode(infrared_left, INPUT);
  pinMode(infrared_right, INPUT);
  pinMode(trig1, OUTPUT); pinMode(echo1, INPUT);
  pinMode(trig2, OUTPUT); pinMode(echo2, INPUT);
  pinMode(trig3, OUTPUT); pinMode(echo3, INPUT);
  // Setup PWM
  ledcAttachChannel(motor_left_speedPin_L, 5000, 8, 0);
  ledcAttachChannel(motor_left_speedPin_R, 5000, 8, 1);
  ledcAttachChannel(motor_right_speedPin_L, 5000, 8, 2);
  ledcAttachChannel(motor_right_speedPin_R, 5000, 8, 3);
}

void loop() {
  Dabble.processInput();
  if(GamePad.isStartPressed()) { automatic=false; stop(); }
  else if(GamePad.isSelectPressed()) { automatic=true;  stop(); }
  if(GamePad.isCrossPressed()) isBrake = !isBrake;
  if(!automatic) {
    setDefault();
    gamepad_control_mode();
  }
  else updateStateMachine();
}

void setDefault(){
  currentState = WAITING;
  isLeft = isRight = doneTurn = false;
}

void updateStateMachine() {
  // update sensors
  bool edge = (digitalRead(infrared_left)==LOW || digitalRead(infrared_right)==LOW);
  distLeft = readUltrasonicCM(trig1, echo1, echo_time1);
  distFront = readUltrasonicCM(trig2, echo2, echo_time1);
  distRight = readUltrasonicCM(trig3, echo3, echo_time3);
  Serial.print("distLeft = "); Serial.print(distLeft);
  Serial.print(" distFront = "); Serial.print(distFront);
  Serial.print(" distRight = "); Serial.println(distRight);

  switch(currentState) {
    case WAITING:
      if(!isRight && GamePad.isLeftPressed()){
        stateDuration = calculateTimeRotation(65.0);
        isLeft = true;
        stateStartTime = millis();
      } else if(!isLeft && GamePad.isRightPressed()){
        stateDuration = calculateTimeRotation(65.0);
        isRight = true;
        stateStartTime = millis();
      }
      if(isLeft && !doneTurn){
        if(millis()-stateStartTime < stateDuration) turn_left(ATTACK_SPEED);
        else { doneTurn = true; stop(); stateDuration = 350; stateStartTime = millis();}
      } else if(isRight && !doneTurn){
        if(millis()-stateStartTime < stateDuration) turn_right(ATTACK_SPEED);
        else { doneTurn = true; stop(); stateDuration = 350; stateStartTime = millis();}
      }
      if(doneTurn){
        if(edge){
          currentState=ESCAPING; stateStartTime=millis(); stateDuration=350;
        } else if(millis()-stateStartTime < stateDuration) go_straight(ATTACK_SPEED);
        else { stop(); currentState = SEARCHING;}
      }
      break;
    case SEARCHING:
      if(edge) {
        currentState = ESCAPING;
        stateStartTime = millis(); stateDuration = 300;
      } else if(distLeft<DETECTION_THRESHOLD||distFront<DETECTION_THRESHOLD||distRight<DETECTION_THRESHOLD) {
        // decide rotate or forward
        float ang = computeTargetAngle();
        if(ang < -15) { currentState = ROTATING_LEFT; stateDuration = calculateTimeRotation(ang); stateStartTime=millis(); }
        else if(ang > 15) { currentState = ROTATING_RIGHT; stateDuration = calculateTimeRotation(ang); stateStartTime=millis(); }
        else { currentState = MOVING_FORWARD; }
      } else {
        if(isLeft){
          turn_left(TURN_SPEED);
        } else if(isRight){
          turn_right(TURN_SPEED);
        }
      }
      break;
    case ROTATING_LEFT:
      if(millis()-stateStartTime < stateDuration) turn_left(ATTACK_SPEED);
      else { currentState = MOVING_FORWARD; }
      break;
    case ROTATING_RIGHT:
      if(millis()-stateStartTime < stateDuration) turn_right(ATTACK_SPEED);
      else { currentState = MOVING_FORWARD; }
      break;
    case MOVING_FORWARD:
      if(edge) { currentState=ESCAPING; stateStartTime=millis(); stateDuration=300; }
      else if(distLeft<DETECTION_THRESHOLD||distFront<DETECTION_THRESHOLD||distRight<DETECTION_THRESHOLD) go_straight(ATTACK_SPEED);
      else currentState=SEARCHING;
      break;
    case ESCAPING:
      if(millis()-stateStartTime < stateDuration) go_back(ATTACK_SPEED);
      else { stop(); currentState=SEARCHING; }
      break;
  }
}

float computeTargetAngle() {
  float sum=0, wsum=0;
  if(distLeft<DETECTION_THRESHOLD) { sum+=-55.0/distLeft; wsum+=1.0/distLeft; }
  if(distFront<DETECTION_THRESHOLD) { sum+=0.0; wsum+=1.0/distFront; }
  if(distRight<DETECTION_THRESHOLD) { sum+=45.0/distRight; wsum+=1.0/distRight; }
  return (wsum>0? sum/wsum : 0);
}

bool gamepad_control_mode() {
  Dabble.processInput(); stop();
  while(GamePad.isUpPressed()){ go_straight(car_speed); Dabble.processInput(); }
  while(GamePad.isDownPressed()){ go_back(car_speed); Dabble.processInput(); }
  while(GamePad.isLeftPressed()){ turn_left(car_speed); Dabble.processInput(); }
  while(GamePad.isRightPressed()){ turn_right(car_speed); Dabble.processInput(); }
  if(GamePad.isSquarePressed()) car_speed=135;
  if(GamePad.isTrianglePressed()) car_speed=200;
  if(GamePad.isCirclePressed()) car_speed=255;
  return true;
}

int readUltrasonicCM(uint8_t trig, uint8_t echo, unsigned long &t) {
  digitalWrite(trig, LOW); delayMicroseconds(2);
  digitalWrite(trig, HIGH); delayMicroseconds(10);
  digitalWrite(trig, LOW);
  t = pulseIn(echo, HIGH, 20000);
  int d=int(t/2/29.412);
  return d? d:100;
}

int calculateTimeRotation(float angleDeg) {
  float v=0.4, L=0.125, r=L/2;
  float omega=v/r;
  float ang=abs(angleDeg)*PI/180.0;
  return int(ang/omega*1000);
}

void stop() {
  if(isBrake) {
    digitalWrite(motor_left_pin_R, HIGH);
    digitalWrite(motor_left_pin_L, HIGH);
    digitalWrite(motor_right_pin_R, HIGH);
    digitalWrite(motor_right_pin_L, HIGH);
    analogWrite(motor_left_speedPin_L, 255);
    analogWrite(motor_left_speedPin_R, 255);
    analogWrite(motor_right_speedPin_L, 255);
    analogWrite(motor_right_speedPin_R, 255);

    // ledcWrite(0, 255);  // HIGH
    // ledcWrite(1, 255);  // HIGH
  } else {
    // ledcWrite(0, 0);
    // ledcWrite(1, 0);
    analogWrite(motor_left_speedPin_L, 0);
    analogWrite(motor_left_speedPin_R, 0);
    analogWrite(motor_right_speedPin_L, 0);
    analogWrite(motor_right_speedPin_R, 0);
    digitalWrite(motor_left_pin_R, LOW);
    digitalWrite(motor_left_pin_L, LOW);
    digitalWrite(motor_right_pin_R, LOW);
    digitalWrite(motor_right_pin_L, LOW);
  }
}

void go_back(int speed) {
  Serial.println("DOWN");
  //Motor pins
  digitalWrite(motor_left_pin_R, HIGH);
  digitalWrite(motor_left_pin_L, HIGH);
  digitalWrite(motor_right_pin_R, HIGH);
  digitalWrite(motor_right_pin_L, HIGH);

  //Speed
  analogWrite(motor_left_speedPin_L, 0);
  analogWrite(motor_left_speedPin_R, speed);
  analogWrite(motor_right_speedPin_L, 0);
  analogWrite(motor_right_speedPin_R, speed);

  // ledcWrite(0, 0);      // RPWM low
  // ledcWrite(1, speed);  // LPWM active
}

void go_straight(int speed) {
  Serial.println("UP");
  //Motor pins
  digitalWrite(motor_left_pin_R, HIGH);
  digitalWrite(motor_left_pin_L, HIGH);
  digitalWrite(motor_right_pin_R, HIGH);
  digitalWrite(motor_right_pin_L, HIGH);
  
  //Speed
  analogWrite(motor_left_speedPin_L, speed);
  analogWrite(motor_left_speedPin_R, 0);
  analogWrite(motor_right_speedPin_L, speed);
  analogWrite(motor_right_speedPin_R, 0);

  // ledcWrite(0, speed);  // RPWM active
  // ledcWrite(1, 0);
}

void turn_left(int speed) {
  Serial.println("LEFT");
  //Motor pins
  digitalWrite(motor_left_pin_R, HIGH);
  digitalWrite(motor_left_pin_L, HIGH);
  digitalWrite(motor_right_pin_R, HIGH);
  digitalWrite(motor_right_pin_L, HIGH);
  
  //Speed
  analogWrite(motor_left_speedPin_L, 0);
  analogWrite(motor_left_speedPin_R, speed);
  analogWrite(motor_right_speedPin_L, speed);
  analogWrite(motor_right_speedPin_R, 0);
}

void turn_right(int speed) {
  Serial.println("RIGHT");
  //Motor pins
  digitalWrite(motor_left_pin_R, HIGH);
  digitalWrite(motor_left_pin_L, HIGH);
  digitalWrite(motor_right_pin_R, HIGH);
  digitalWrite(motor_right_pin_L, HIGH);
  
  //Speed
  analogWrite(motor_left_speedPin_L, speed);
  analogWrite(motor_left_speedPin_R, 0);
  analogWrite(motor_right_speedPin_L, 0);
  analogWrite(motor_right_speedPin_R, speed);
}
