#ifndef SERVO_H
#define SERVO_H
#include <ESP32Servo.h>

#define SERVO_PIN 13
void servo_togle(uint16_t togle);
void set_servo(void);
#endif