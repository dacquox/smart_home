#include <Arduino.h>

#include "display/display_manager.h"

#include "ui/ui_idle.h"
#include "ui/ui_listening.h"
#include "ui/ui_password.h"
#include "ui/ui_success.h"
#include "ui/ui_fail.h"
#include "touch/touch_manager.h"
#include "system/states.h"
#include "config/PIN.h"
#include "relay/relay.h"
#include "voice/voice_manager.h"

void setup() 
{
    Serial.begin(115200);
    initVoiceManager();

    Relay_Init();
    initDisplay();
    initTouch();
    drawIdleScreen();
}

void loop() 
{
    switch(currentState) 
    {
        case IDLE:
            updateIdleAnimation();

            // touch detect
            if (isTouched()) 
            {
                enterListeningState();
            }   
            break;

        case LISTENING:
            updateListeningState();
            break;
        case PASSWORD:
            updatePasswordState();
            break;
        case ACCESS_GRANTED:
            updateSuccessState();
            break;
        case ACCESS_DENIED:
            updateFailState();
            break;
    }
    delay(30);
}

