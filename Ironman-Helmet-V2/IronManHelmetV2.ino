/*  
 *  AUTHOR : MARC GUEDON
 *  WEBSITE : http://thedraill.e-monsite.com/
 *  INSTAGRAM : https://www.instagram.com/guedonmarc/
 */

#include <ServoEasing.hpp>
#include <VoiceRecognitionV4.h>

#define EYES_PIN          3
#define TACTILE_KEY_PIN   2
#define HELMET_SERVO_PIN  5
#define FACE_SERVO_PIN    6

#define OPENED_HELMET     0
#define OPENED_FACE       0
#define CLOSED_HELMET     150
#define CLOSED_FACE       170
#define SERVOS_SPEED      190

#define JarvisRecord      (0)
#define HelmetOnRecord    (1)
#define HelmetOffRecord   (2)
#define EyesOnRecord      (3)
#define EyesOffRecord     (4)

ServoEasing HelmetServo;
ServoEasing FaceServo;

VR myVR1(&Serial1);

bool helmetOpen = false;
bool eyesLit = true;
bool eyesAlreadyOn = false;

uint8_t buf[64];

void setup() {
  pinMode(EYES_PIN, OUTPUT);
  pinMode(TACTILE_KEY_PIN, INPUT);

  Serial1.begin(9600);

  //Initialisation of servomotors
  HelmetServo.attach(HELMET_SERVO_PIN);
  FaceServo.attach(FACE_SERVO_PIN);
  setSpeedForAllServos(SERVOS_SPEED);
  HelmetServo.setEasingType(EASE_CUBIC_IN_OUT);
  FaceServo.setEasingType(EASE_CUBIC_IN_OUT);

//NOT NECESSARY / TO ENSURE PROPER OPERATION
////////////////////////////////////////////////////////////////////////////////////////////////////
  Serial.begin(115200);
  Serial.println("Elechouse Voice Recognition V3 Module\r\nControl LED sample");
    
  if(myVR1.clear() == 0){
    Serial.println("Recognizer cleared.");
  }else{
    Serial.println("Not find VoiceRecognitionModule.");
    Serial.println("Please check connection and restart Arduino.");
    while(1);
  }
  
  if(myVR1.load((uint8_t)JarvisRecord) >= 0){
    Serial.println("JarvisRecord loaded");
  }
  
  if(myVR1.load((uint8_t)HelmetOnRecord) >= 0){
    Serial.println("HelmetOnRecord loaded");
  }

  if(myVR1.load((uint8_t)HelmetOffRecord) >= 0){
    Serial.println("HelmetOffRecord loaded");
  }

  if(myVR1.load((uint8_t)EyesOnRecord) >= 0){
    Serial.println("EyesOnRecord loaded");
  }

  if(myVR1.load((uint8_t)EyesOffRecord) >= 0){
    Serial.println("EyesOffRecord loaded");
  }
////////////////////////////////////////////////////////////////////////////////////////////////////


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

  int ret;
  ret = myVR1.recognize(buf, 50);
  
  //If a recognized command
  if(ret>0){
    //Recognize the command and act
    switch(buf[1]){
      case JarvisRecord:
        break;
      case HelmetOnRecord:
        helmetOpen = true;
        eyesLit = false;
        break;
      case HelmetOffRecord:
        helmetOpen = false;
        eyesLit = true;
        break;
      case EyesOnRecord:
        eyesLit = true;
        break;
      case EyesOffRecord:
        eyesLit = false;
        break;
      default:
        break;
    }
    
    printVR(buf);

    //Functions performing the actions
    helmetFunction();
    eyesFunction();
  }
}

//Eyes on/off function
void eyesFunction(){
  //If eyes on
  if(eyesLit == true){
    //If not already lit
    if(eyesAlreadyOn == false){
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
      eyesAlreadyOn = true;
    }
  }
  //If eyes off
  else{
    analogWrite(EYES_PIN, 0);
    eyesAlreadyOn = false;
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

void printSignature(uint8_t *buf, int len)
{
  int i;
  for(i=0; i<len; i++){
    if(buf[i]>0x19 && buf[i]<0x7F){
      Serial.write(buf[i]);
    }
    else{
      Serial.print("[");
      Serial.print(buf[i], HEX);
      Serial.print("]");
    }
  }
}

void printVR(uint8_t *buf)
{
  Serial.println("VR Index\tGroup\tRecordNum\tSignature");

  Serial.print(buf[2], DEC);
  Serial.print("\t\t");

  if(buf[0] == 0xFF){
    Serial.print("NONE");
  }
  else if(buf[0]&0x80){
    Serial.print("UG ");
    Serial.print(buf[0]&(~0x80), DEC);
  }
  else{
    Serial.print("SG ");
    Serial.print(buf[0], DEC);
  }
  Serial.print("\t");

  Serial.print(buf[1], DEC);
  Serial.print("\t\t");
  if(buf[3]>0){
    printSignature(buf+4, buf[3]);
  }
  else{
    Serial.print("NONE");
  }
  Serial.println("\r\n");
}
