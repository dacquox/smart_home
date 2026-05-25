#ifndef _VOICE_MANAGER_H_
#define _VOICE_MANAGER_H_

#include <Arduino.h>
#include "../system/states.h"

#define CMD_OPEN   "DOOR_OPEN"
#define CMD_CLOSE  "DOOR_CLOSE"
#define CMD_MK     "anh ơi nhập mật khẩu"
#define CMD_CLOSED "em đóng cửa cho anh rồi đây"

extern HardwareSerial voiceUART;

void initVoiceManager();
String getVoiceCommand();

#endif