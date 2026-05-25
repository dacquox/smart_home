#include "display_manager.h"

TFT_eSPI tft = TFT_eSPI();

/*
    - init TFT
    - rotation
    - brightness
    - clear screen
    - font
    - backlight
    - SPI display
*/

void initDisplay() {

    tft.init();
    tft.setRotation(0);
}