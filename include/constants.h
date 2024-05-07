#ifndef CONSTANTS
#define CONSTANTS

// Change pins
#define BELT 22
#define BELT_FORWARD 19
#define BELT_BACK 29
#define LED 14
#define PUMP 17
#define PUMP_FORWARD 0
#define PUMP_BACK 16
#define SENSOR_START 36
#define SENSOR_MILK 39
#define SENSOR_CEREAL_1 34
#define SENSOR_CEREAL_2 35
#define SENSOR_CEREAL_3 32
#define SENSOR_END 33
#define SERVO_CEREAL_1 25
#define SERVO_CEREAL_2 26
#define SERVO_CEREAL_3 27
#define SOLENOID_1 12
#define SOLENOID_2 13

// Adjust sensor sensitivity and engine speeds
#define BELT_SPEED 5
#define PUMP_SPEED 255
#define SENSOR_SENSITIVITY 30

// LEDs
#define LED_COUNT 20
#define LED_START 3
#define LED_MILK 5
#define LED_CEREAL_1 6
#define LED_CEREAL_2 8
#define LED_CEREAL_3 10

// Screen, better to not touch
#define BUTTON_W 135
#define BUTTON_H 110
#define NAV_BUTTON_W 100
#define NAV_BUTTON_H 60
#define OK_BUTTON_W 180
#define OK_BUTTON_H 180

#endif