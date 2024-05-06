#ifndef CONSTANTS
#define CONSTANTS

// Change pins
#define BELT 1
#define BELT_FORWARD 1
#define BELT_BACK 1
#define LED 1
#define PUMP 1
#define PUMP_FORWARD 1
#define PUMP_BACK 1
#define SENSOR_START 4
#define SENSOR_MILK 5
#define SENSOR_CEREAL_1 1
#define SENSOR_CEREAL_2 2
#define SENSOR_CEREAL_3 3
#define SENSOR_END 4
#define SERVO_CEREAL_1 10
#define SERVO_CEREAL_2 11
#define SERVO_CEREAL_3 12
#define SOLENOID_1 1
#define SOLENOID_2 2

// Adjust sensor sensitivity and engine speeds
#define BELT_SPEED 100
#define PUMP_SPEED 100
#define SENSOR_SENSITIVITY 100

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