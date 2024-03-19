#include <Arduino.h>
#include <ESP32Servo.h>

#define SERVO 23

// put function declarations here:
Servo myServo;
int pos = 0;

void setup() {
    // put your setup code here, to run once:
    myServo.attach(SERVO);
}

void loop() {
    // put your main code here, to run repeatedly:
    for (pos = 0; pos <= 180; pos++) {
        myServo.write(pos);
        delay(15);
    }
    for (pos = 180; pos >= 0; pos--) {
        myServo.write(pos);
        delay(15);
    }
}

// put function definitions here: