#include <Arduino.h>

void loaMAX98357Init(int bclkPin, int lrcPin, int dinPin, int volume);
void loaMAX98357Speak(const String &text);
void loaMAX98357Stop();