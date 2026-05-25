#include "ui_idle.h"

extern TFT_eSPI tft;
// animation
static int pulse = 0;
static bool growing = true;

/*
    Idle screen
*/
void drawIdleScreen()
{
    tft.fillScreen(TFT_BLACK);

    // title
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.setTextSize(2);

    tft.drawCentreString("CHATBOX", 120, 40, 2);

    // hint
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(1);

    tft.drawCentreString("Cham de bat dau", 120, 280, 2);
}
/*
    Animation
*/
void updateIdleAnimation()
{
    int radius = 20 + pulse;

    // clear old circle
    tft.fillCircle(120, 140, 45, TFT_BLACK);

    // outer glow
    tft.drawCircle(120, 140, radius + 8, TFT_DARKCYAN);

    // main circle
    tft.drawCircle(120, 140, radius, TFT_CYAN);

    // center dot
    tft.fillCircle(120, 140, 8, TFT_CYAN);

    // pulse animation
    if (growing) {

        pulse++;

        if (pulse > 12) {
            growing = false;
        }

    } else {

        pulse--;

        if (pulse < 0) {
            growing = true;
        }
    }
}