#ifndef PHAT_IR_H
#define PHAT_IR_H

#include <Arduino.h>
#include <IRSend.h>

#define DEFAULT_SEND_PIN 4

void setup_sendir(uint8_t pin = DEFAULT_SEND_PIN);
void setSendPin(uint8_t pin);

void sendRaw(const uint16_t buf[], uint16_t len);

#endif