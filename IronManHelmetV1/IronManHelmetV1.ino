/*  
 *  AUTHOR : MARC GUEDON
 *  WEBSITE : http://thedraill.e-monsite.com/
 *  INSTAGRAM : https://www.instagram.com/guedonmarc/
 */

#include <ServoEasing.hpp>

#define EYES_PIN          3
#define TACTILE_KEY_PIN   2
#define HELMET_SERVO_PIN  5
#define FACE_SERVO_PIN    6

// MAY NEED TO CHANGE
#define OPENED_HELMET     0
#define OPENED_FACE       0
#define CLOSED_HELMET     155
#define CLOSED_FACE       175
#define SERVOS_SPEED      190

ServoEasing helmet_servo;
ServoEasing face_servo;

bool helmet_open = false;
bool eyes_lit = true;

void setup()
{
    pinMode(EYES_PIN, OUTPUT);
    pinMode(TACTILE_KEY_PIN, INPUT);

    // Initialisation of servomotors
    helmet_servo.attach(HELMET_SERVO_PIN);
    face_servo.attach(FACE_SERVO_PIN);
    setSpeedForAllServos(SERVOS_SPEED);
    helmet_servo.setEasingType(EASE_CUBIC_IN_OUT);
    face_servo.setEasingType(EASE_CUBIC_IN_OUT);

    // Intialisation of the helmet and eyes
    helmet_function();
    eyes_function();
}

void loop()
{
    if(digitalRead(TACTILE_KEY_PIN) == HIGH) // Contact on the left side (touch sensor) 
    {
        eyes_lit = helmet_open;
        helmet_open = !helmet_open;

        helmet_function();
        eyes_function();

        while(digitalRead(TACTILE_KEY_PIN) == HIGH) {}
    }
}

//Eyes on/off function
void eyes_function()
{
    if(eyes_lit == true)
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
    }
    
    else analogWrite(EYES_PIN, 0);
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