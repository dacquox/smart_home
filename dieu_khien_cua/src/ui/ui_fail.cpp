#include "ui_fail.h"

extern TFT_eSPI tft;

// animation
static int pulse = 0;
static bool growing = true;

/*
    Fail screen
*/
void drawFailScreen() {

    tft.fillScreen(TFT_BLACK);

    // vòng tròn đỏ
    tft.drawCircle(120, 100, 40, TFT_RED);

    // dấu X
    tft.drawLine(100, 80, 140, 120, TFT_RED);
    tft.drawLine(140, 80, 100, 120, TFT_RED);

    // title
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.setTextSize(2);

    tft.drawCentreString("TRUY CAP TU CHOI", 120, 170, 2);

    // sub text
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(1);

    tft.drawCentreString("MAT KHAU KHONG DUNG", 120, 220, 2);
}

/*
    Fail animation
*/
void updateFailAnimation() {

    int radius = 40 + pulse;

    // clear vùng animation
    tft.fillCircle(120, 100, 60, TFT_BLACK);

    // glow ngoài
    tft.drawCircle(120, 100, radius, TFT_DARKGREY);

    // vòng chính
    tft.drawCircle(120, 100, 40, TFT_RED);

    // dấu X
    tft.drawLine(100, 80, 140, 120, TFT_RED);
    tft.drawLine(140, 80, 100, 120, TFT_RED);

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
