#ifndef _TOUCH_MANAGER_H_
#define _TOUCH_MANAGER_H_

#include <XPT2046_Touchscreen.h>
#include <SPI.h>
#include <TFT_eSPI.h>

extern XPT2046_Touchscreen touch;

void initTouch();
bool isTouched();
bool getTouchPosition(uint16_t &x, uint16_t &y);
char getTouchKey(uint16_t x, uint16_t y);
#endif