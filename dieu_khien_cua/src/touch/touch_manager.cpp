#include "touch_manager.h"

XPT2046_Touchscreen touch(TOUCH_CS);

/*
    Init touch
*/
void initTouch() 
{
    touch.begin();
    touch.setRotation(0);
}

/*
    Touch detect
*/
bool isTouched() 
{
    static bool locked = false;
    static uint32_t lastTouch = 0;

    const uint32_t debounceTime = 250;

    // đang chạm
    if (touch.touched())
    {
        // chưa lock
        if (!locked)
        {
            // debounce thời gian
            if (millis() - lastTouch > debounceTime)
            {
                locked = true;
                lastTouch = millis();

                return true;
            }
        }
    }
    else
    {
        // nhả tay -> unlock
        locked = false;
    }

    return false;
}
/*
    Get touch position
*/
bool getTouchPosition(uint16_t &x, uint16_t &y)
{
    if(!touch.touched())
        return false;

    TS_Point p = touch.getPoint();

    /*
        map tọa độ
        chỉnh lại theo màn hình thực tế
    */
    x = map(p.x, 320, 3400, 0, 240);

    y = map(p.y, 200, 3555, 0, 320);

    return true;
}
/*
    kiểm tra nút touch
*/
char getTouchKey(uint16_t x, uint16_t y)
{
    const int startX = 30;
    const int startY = 120;

    const int w = 50;
    const int h = 40;
    const int gap = 10;

    char keypad[4][3] =
    {
        {'1','2','3'},
        {'4','5','6'},
        {'7','8','9'},
        {'X','0','O'}
    };

    for(int row = 0; row < 4; row++)
    {
        for(int col = 0; col < 3; col++)
        {
            int btnX = startX + col * (w + gap);
            int btnY = startY + row * (h + gap);

            if(x >= btnX &&
               x <= btnX + w &&
               y >= btnY &&
               y <= btnY + h)
            {
                return keypad[row][col];
            }
        }
    }

    return '\0';
}