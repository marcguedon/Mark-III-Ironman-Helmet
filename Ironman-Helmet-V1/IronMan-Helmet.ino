/*  
 *  AUTHOR : MARC GUEDON
 *  WEBSITE : http://thedraill.e-monsite.com/
 *  INSTAGRAM : https://www.instagram.com/guedonmarc/
 */

#include <ServoEasing.hpp>

#define EYES_PIN          3
#define TACTILE_KEY_PIN   2
#define OPENED_HELMET     0
#define OPENED_FACE       0
#define CLOSED_HELMET     155
#define CLOSED_FACE       175
#define HELMET_SERVO_PIN  5
#define FACE_SERVO_PIN    6
#define SERVOS_SPEED      190

ServoEasing HelmetServo;
ServoEasing FaceServo;

bool helmetOpen;
bool eyesLit;

void setup() {
  pinMode(EYES_PIN, OUTPUT);
  pinMode(TACTILE_KEY_PIN, INPUT);

  //Initialisation of servomotors
  HelmetServo.attach(HELMET_SERVO_PIN);
  FaceServo.attach(FACE_SERVO_PIN);
  setSpeedForAllServos(SERVOS_SPEED);
  HelmetServo.setEasingType(EASE_CUBIC_IN_OUT);
  FaceServo.setEasingType(EASE_CUBIC_IN_OUT);

  //Initialisation of variables
  helmetOpen = false;
  eyesLit = true;

  //Functions performing the actions
  helmetFunction();
  eyesFunction();
}

void loop() {
  //If contact on the left side 
  if(digitalRead(TACTILE_KEY_PIN) == HIGH){
    //If helmet opened, helmet and eyes state change
    if(helmetOpen == true){
      helmetOpen = false;
      eyesLit = true;
    }
    //If helmet closed, helmet and eyes state change 
    else{
      helmetOpen = true;
      eyesLit = false;
    }
    
    //Functions performing the actions
    helmetFunction();
    eyesFunction();

    //While contact on the left side
    while(digitalRead(TACTILE_KEY_PIN) == HIGH){}
  }
}

//Eyes on/off function
void eyesFunction(){
  //If eyes on
  if(eyesLit == true){
    analogWrite(EYES_PIN, 255);
    delay(10);
    analogWrite(EYES_PIN, 0);
    delay(500);
    analogWrite(EYES_PIN, 255);
    delay(50);
    analogWrite(EYES_PIN, 0);
    delay(75);
    analogWrite(EYES_PIN, 255);
    delay(200);
    analogWrite(EYES_PIN, 0);
    delay(50);
    for(int i = 32; i < 255; i++){
      analogWrite(EYES_PIN, i);
      delay(8);
    }
  }
  //If eyes off
  else{
    analogWrite(EYES_PIN, 0);
  }
}

//Helmet open/close function
void helmetFunction(){
  //If helmet opened
  if(helmetOpen == true){
    HelmetServo.setEaseTo(OPENED_HELMET);
    FaceServo.setEaseTo(OPENED_FACE);
  }
  //If helmet closed
  else{
    HelmetServo.setEaseTo(CLOSED_HELMET);
    FaceServo.setEaseTo(CLOSED_FACE);
  }
  synchronizeAllServosStartAndWaitForAllServosToStop();
}
