#include "relay.h"

void Relay_Init()
{
    pinMode(RELAY_PIN, OUTPUT);
}
void openDoor()
{
    digitalWrite(RELAY_PIN, HIGH);
}
void closeDoor()
{
    digitalWrite(RELAY_PIN, LOW);
}