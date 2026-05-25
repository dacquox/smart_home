#include "ui_listening.h"

extern TFT_eSPI tft;

static int wave = 0;
static bool growing = true;

/*
    Listening screen
*/
void drawListeningScreen() {
    
    //2 lan moi xoa het
    tft.fillScreen(TFT_BLACK);
    tft.fillScreen(TFT_BLACK);

    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.setTextSize(2);

    tft.drawCentreString("DANG LANG NGHE", 120, 40, 2);

    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(1);

    tft.drawCentreString("Hay noi lenh....", 120, 280, 2);
}

/*
    Mic animation
*/
void updateListeningAnimation() {
    int radius = 25 + wave;

    // clear
    tft.fillCircle(120, 140, 55, TFT_BLACK);

    // outer wave
    tft.drawCircle(120, 140, radius + 10, TFT_DARKCYAN);

    // main wave
    tft.drawCircle(120, 140, radius, TFT_CYAN);

    // mic center
    tft.fillCircle(120, 140, 12, TFT_CYAN);

    // animate
    if (growing) {

        wave++;

        if (wave > 15) {
            growing = false;
        }

    } else {

        wave--;

        if (wave < 0) {
            growing = true;
        }
    }
}