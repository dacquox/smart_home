#include "uart.h"

void uartLinkBegin(int rxPin, int txPin, uint32_t baud) {
    Serial2.begin(baud, SERIAL_8N1, rxPin, txPin);
}

void uartSendLine(const String &line) {
    Serial2.println(line);
    Serial.println(line);
}

//clear cac buffer cũ để phát đúng tts
void clearUartReplyBuffer() {
    while (Serial2.available()) {
        Serial2.read();
    }
}

//gui lenh sang esp con lai
void uartSendCommand(const String &cmdName) {
    if (cmdName.length() == 0) return;

    String frame = "";
    frame += cmdName;

    uartSendLine(frame);
}

String uartReadLine() {
    static String line = "";

    while (Serial2.available()) {
        char c = (char)Serial2.read();

        if (c == '\n') {
            String out = line;
            out.trim();
            line = "";
            return out;
        }

        if (c != '\r') {
            line += c;
        }

        if (line.length() > 200) {
            line = "";
        }
    }
    return "";
}