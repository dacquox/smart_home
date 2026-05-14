#include "servo.h"
Servo servo;
void servo_togle(uint16_t angle)
{
  uint16_t val = constrain(angle, 0, 180);
  servo.write(val);
}
void set_servo(void)
{
  servo.attach(SERVO_PIN);
}