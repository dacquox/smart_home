#include "ui_password.h"

extern TFT_eSPI tft;
/*
    Password screen
*/
void drawPasswordScreen() 
{
    tft.fillScreen(TFT_BLACK);
    tft.fillScreen(TFT_BLACK);
    // title
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.setTextSize(2);

    tft.drawCentreString("NHAP MAT KHAU", 120, 20, 2);

    // password dots
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawCentreString("____", 120, 55, 4);

    // keypad
    int startX = 30;
    int startY = 120;

    int w = 50;
    int h = 40;
    int gap = 10;

    int number = 1;

    for (int row = 0; row < 3; row++) {

        for (int col = 0; col < 3; col++) {

            int x = startX + col * (w + gap);
            int y = startY + row * (h + gap);

            tft.drawRoundRect(x, y, w, h, 5, TFT_CYAN);

            tft.drawCentreString(String(number), x + w/2, y + 10, 2);

            number++;
        }
    }

    // row cuối
    tft.drawRoundRect(startX + 0 * (w + gap), startY + 3 * (h + gap), 50, 40, 5, TFT_GREEN);
    tft.drawCentreString("X", startX + 0 * (w + gap) + w/2, startY + 3 * (h + gap) + 10, 2);

    tft.drawRoundRect(startX + 1 * (w + gap), startY + 3 * (h + gap), 50, 40, 5, TFT_CYAN);
    tft.drawCentreString("0", startX + 1 * (w + gap) + w/2, startY + 3 * (h + gap) + 10, 2);

    tft.drawRoundRect(startX + 2 * (w + gap), startY + 3 * (h + gap), 50, 40, 5, TFT_DARKGREEN);
    tft.drawCentreString("OK", startX + 2 * (w + gap) + w/2, startY + 3 * (h + gap) + 10, 2);
}