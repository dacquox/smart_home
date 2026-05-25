#include "states.h"
#include "../ui/ui_idle.h"
#include "../ui/ui_listening.h"
#include "../ui/ui_password.h"
#include "../ui/ui_success.h"
#include "../ui/ui_fail.h"
#include "../voice/voice_manager.h"
#include "../touch/touch_manager.h"

State currentState = IDLE;
bool passwordEntered = false;
static unsigned long listeningStartTime = 0;
static unsigned long successStartTime = 0;
static unsigned long failStartTime = 0;
/*
    Enter listening
*/
void enterListeningState() 
{
    currentState = LISTENING;

    listeningStartTime = millis();

    drawListeningScreen();
}

/*
    Update listening
*/
void updateListeningState() 
{
    updateListeningAnimation();
    /*
        Nhận lệnh giọng nói
    */
    String command = getVoiceCommand();

    /*
        Mở cửa
    */
    if(command == CMD_OPEN)
    {
        voiceUART.println(CMD_MK);
        enterPasswordState();

        return;
    }

    /*
        Đóng cửa
    */
    if(command == CMD_CLOSE)
    {
        voiceUART.println(CMD_CLOSED);
        closeDoor();

        currentState = IDLE;

        drawIdleScreen();

        return;
    }

    /*
        timeout 5s
    */
    if(millis() - listeningStartTime > 30000)
    {
        currentState = IDLE;

        drawIdleScreen();
    }
}

/*
    Enter password
*/
void enterPasswordState()
{
    currentState = PASSWORD;
    drawPasswordScreen();
}

void updatePasswordState()
{
    /*
        cập nhật keypad
    */
    updatePasswordInput();

    if(isPasswordEntered())
    {
        if(inputPassword == "1234")
        {
            openDoor();
            enterSuccessState();
        }
        else
        {
            enterFailState();
        }

        inputPassword = "";
    }
}
/*
    update password
*/
void updatePasswordInput()
{
    TS_Point p = touch.getPoint();
    uint16_t x, y;
    /*
        có touch
    */
    if(getTouchPosition(x, y))
    {
        Serial.print("X: ");
        Serial.print(p.x);

        Serial.print(" Y: ");
        Serial.println(p.y);
        char key = getTouchKey(x, y);
        if(key != 0)
        {
            /*
                OK
            */
            if(key == 'O')
            {
                passwordEntered = true;
            }

            /*
                Xóa
            */
            else if(key == 'X')
            {
                if(inputPassword.length() > 0)
                {
                    inputPassword.remove(inputPassword.length() - 1);
                }
            }

            /*
                Thêm số
            */
            else
            {
                if(inputPassword.length() < 4)
                {
                    inputPassword += key;
                }
            }

            /*
                Hiển thị password
            */
            String displayText = inputPassword;

            while(displayText.length() < 4)
            {
                displayText += "_";
            }

            /*
                clear vùng password
            */
            tft.fillRect(70, 60, 100, 40, TFT_BLACK);

            tft.setTextColor(TFT_WHITE, TFT_BLACK);

            tft.drawCentreString(displayText, 120, 55, 4);

            delay(200);
        }
    }
}

bool isPasswordEntered()
{
    if(passwordEntered)
    {
        passwordEntered = false;

        return true;
    }

    return false;
}
/*
    Enter success
*/
void enterSuccessState()
{
    currentState = ACCESS_GRANTED;

    successStartTime = millis();

    drawSuccessScreen();

    // TODO:
    // relay ON ở đây
}
/*
    Update success
*/
void updateSuccessState()
{
    updateSuccessAnimation();

    // 3s quay lại idle
    if (millis() - successStartTime > 3000) {

        currentState = IDLE;

        drawIdleScreen();
    }
}
/*
    Enter Fail
*/
void enterFailState() {

    currentState = ACCESS_DENIED;

    failStartTime = millis();

    drawFailScreen();
}

/*
    Update Fail
*/
void updateFailState() {

    updateFailAnimation();

    // 3s quay về idle
    if (millis() - failStartTime > 3000) {

        currentState = IDLE;

        drawIdleScreen();
    }
}