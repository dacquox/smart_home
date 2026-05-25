#ifndef _RELAY_H_
#define _RELAY_H_

#include <Arduino.h>
#define RELAY_PIN 33

void Relay_Init();
void openDoor();
void closeDoor();

#endif