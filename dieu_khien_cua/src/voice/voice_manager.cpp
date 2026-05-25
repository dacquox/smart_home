#include "voice_manager.h"

HardwareSerial voiceUART(2); // UART2
/*
    UART ESP32-S3
    RX = GPIO16, TX = GPIO17
*/
void initVoiceManager()
{
    voiceUART.begin(115200, SERIAL_8N1, 16, 17);
}

/*
    Nhan chuoi tu ESP32-S3
*/
String getVoiceCommand()
{
    if (!voiceUART.available())
    {
        return "";
    }

    String cmd = voiceUART.readStringUntil('\n');
    cmd.trim();

    Serial.print("UART RX: ");
    Serial.println(cmd);

    return cmd;
}