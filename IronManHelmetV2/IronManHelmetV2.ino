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

// MAY NEED TO CHANGE
#define OPENED_HELMET     0
#define OPENED_FACE       0
#define CLOSED_HELMET     150
#define CLOSED_FACE       170
#define SERVOS_SPEED      190

// COMMANDS NUMBER
#define JarvisRecord      (0)
#define HelmetOnRecord    (1)
#define HelmetOffRecord   (2)
#define EyesOnRecord      (3)
#define EyesOffRecord     (4)

ServoEasing helmet_servo;
ServoEasing face_servo;
VR myVR1(&Serial1);

bool helmet_open = false;
bool eyes_lit = true;
bool eyes_already_on = false;
uint8_t buf[64];

void setup()
{
    pinMode(EYES_PIN, OUTPUT);
    pinMode(TACTILE_KEY_PIN, INPUT);

    Serial1.begin(9600);

    //Initialisation of servomotors
    helmet_servo.attach(HELMET_SERVO_PIN);
    face_servo.attach(FACE_SERVO_PIN);
    setSpeedForAllServos(SERVOS_SPEED);
    helmet_servo.setEasingType(EASE_CUBIC_IN_OUT);
    face_servo.setEasingType(EASE_CUBIC_IN_OUT);

// NOT NECESSARY / TO ENSURE PROPER OPERATION
    // Serial.begin(115200);
    // Serial.println("Elechouse Voice Recognition V3 Module\r\nControl LED sample");

    // if(myVR1.clear() == 0)
    //     Serial.println("Recognizer cleared.");

    // else
    // {
    //     Serial.println("Not find VoiceRecognitionModule.");
    //     Serial.println("Please check connection and restart Arduino.");
    //     while(1);
    // }

    // if(myVR1.load((uint8_t)JarvisRecord) >= 0)
    //     Serial.println("JarvisRecord loaded");

    // if(myVR1.load((uint8_t)HelmetOnRecord) >= 0)
    //     Serial.println("HelmetOnRecord loaded");

    // if(myVR1.load((uint8_t)HelmetOffRecord) >= 0)
    //     Serial.println("HelmetOffRecord loaded");

    // if(myVR1.load((uint8_t)EyesOnRecord) >= 0)
    //     Serial.println("EyesOnRecord loaded");

    // if(myVR1.load((uint8_t)EyesOffRecord) >= 0)
    //     Serial.println("EyesOffRecord loaded");

    // Intialisation of the helmet and eyes
    helmet_function();
    eyes_function();
}

void loop()
{
    
    if(digitalRead(TACTILE_KEY_PIN) == HIGH) // Contact on the left side 
    {
        eyes_lit = helmet_open;
        helmet_open = !helmet_open;

        helmet_function();
        eyes_function();

        while(digitalRead(TACTILE_KEY_PIN) == HIGH) {}
    }
  
    if(myVR1.recognize(buf, 50) > 0) // A command has been recognized
    {
        switch(buf[1])
        {
            case JarvisRecord:
                break;
            case HelmetOnRecord:
                helmet_open = true;
                eyes_lit = false;
                break;
            case HelmetOffRecord:
                helmet_open = false;
                eyes_lit = true;
                break;
            case EyesOnRecord:
                eyes_lit = true;
                break;
            case EyesOffRecord:
                eyes_lit = false;
                break;
            default:
                break;
        }

        printVR(buf);

        helmet_function();
        eyes_function();
    }
}

//Eyes on/off function
void eyes_function()
{
    if(eyes_lit == true && eyes_already_on == false) // If not already lit
    {
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

        for(int i = 32; i < 255; i++)
        {
            analogWrite(EYES_PIN, i);
            delay(8);
        }

        eyes_already_on = true;
    }
  
    else if(eyes_lit == false)
    {
        analogWrite(EYES_PIN, 0);
        eyes_already_on = false;
    }
}

//Helmet open/close function
void helmet_function()
{
    if(helmet_open == true)
    {
        helmet_servo.setEaseTo(OPENED_HELMET);
        face_servo.setEaseTo(OPENED_FACE);
    }
    
    else
    {
        helmet_servo.setEaseTo(CLOSED_HELMET);
        face_servo.setEaseTo(CLOSED_FACE);
    }
    
    synchronizeAllServosStartAndWaitForAllServosToStop();
}

void printSignature(uint8_t *buf, int len)
{
    for(int i = 0; i < len; i++)
    {
        if(buf[i]>0x19 && buf[i]<0x7F)
            Serial.write(buf[i]);

        else
        {
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

    if(buf[0] == 0xFF) Serial.print("NONE");

    else if(buf[0] & 0x80)
    {
        Serial.print("UG ");
        Serial.print(buf[0] & (~0x80), DEC);
    }

    else
    {
        Serial.print("SG ");
        Serial.print(buf[0], DEC);
    }

    Serial.print("\t");
    Serial.print(buf[1], DEC);
    Serial.print("\t\t");

    if(buf[3]>0) printSignature(buf + 4, buf[3]);
    else Serial.print("NONE");
    
    Serial.println("\r\n");
}
