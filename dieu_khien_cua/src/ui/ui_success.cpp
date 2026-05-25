#include "ui_success.h"

extern TFT_eSPI tft;

// animation
static int pulse = 0;
static bool growing = true;

/*
    Success screen
*/
void drawSuccessScreen() {

    tft.fillScreen(TFT_BLACK);
    tft.fillScreen(TFT_BLACK);
    // icon
    tft.drawCircle(120, 100, 40, TFT_GREEN);

    // dấu check
    tft.drawLine(100, 100, 115, 115, TFT_GREEN);
    tft.drawLine(115, 115, 145, 85, TFT_GREEN);

    // title
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.setTextSize(2);

    tft.drawCentreString("XAC THUC OK", 120, 170, 2);

    // sub text
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(1);

    tft.drawCentreString("DANG MO CUA...", 120, 220, 2);
}

/*
    Success animation
*/
void updateSuccessAnimation() {

    int radius = 40 + pulse;

    // clear animation area
    tft.fillCircle(120, 100, 60, TFT_BLACK);

    // outer glow
    tft.drawCircle(120, 100, radius, TFT_DARKGREEN);

    // main circle
    tft.drawCircle(120, 100, 40, TFT_GREEN);

    // check mark
    tft.drawLine(100, 100, 115, 115, TFT_GREEN);
    tft.drawLine(115, 115, 145, 85, TFT_GREEN);

    // animate
    if (growing) {

        pulse++;

        if (pulse > 10) {
            growing = false;
        }

    } else {

        pulse--;

        if (pulse < 0) {
            growing = true;
        }
    }
}