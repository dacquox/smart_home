#include "read_ir.h"
#include "phat_ir.h"
#include "control.h"
#include "servo.h"
#define BAUD_RATE 115200

// UART1 configuration
#define UART1_TX 17
#define UART1_RX 16
#define UART1_BAUD 115200

// Hàm gửi phản hồi qua UART1
void sendVoiceResponse(String message) {
    Serial1.println(message);
}

void handleCommand_AC(String cmd) {
    cmd.trim();

    // Bật điều hòa 1
    if (cmd == "AC1_ON") {
        servo_togle(0);
        delay(1200);
        On_AC();
        sendVoiceResponse("Dạ, đã bật điều hòa số 1.");
    }
    // Tắt điều hòa 1
    else if (cmd == "AC1_OFF") {
        servo_togle(0);
        delay(1200);
        Off_AC();
        sendVoiceResponse("Dạ, đã tắt điều hòa số 1.");
    }
    // Bật điều hòa 2
    else if (cmd == "AC2_ON") {
        servo_togle(135);
        delay(1200);
        On_AC();
        sendVoiceResponse("Dạ, đã bật điều hòa số 2.");
    }
    // Tắt điều hòa 2
    else if (cmd == "AC2_OFF") {
        servo_togle(135);
        delay(1200);
        Off_AC();
        sendVoiceResponse("Dạ, đã tắt điều hòa số 2.");
    }
    // Bật đèn
    else if (cmd == "LIGHT_ON") {
        servo_togle(180);
        delay(1200);
        On_led();
        sendVoiceResponse("Dạ, đã bật đèn.");
    }
    // Tắt đèn
    else if (cmd == "LIGHT_OFF") {
        servo_togle(180);
        delay(1200);
        Off_led();
        sendVoiceResponse("Dạ, đã tắt đèn.");
    }
    // Lấy nhiệt độ điều hòa 1
    else if (cmd == "AC1_TEMP_GET") {
        servo_togle(0);
        delay(1200);
        int temp = AC_Get_Temp();
        Serial.print("[AC1] Nhiệt độ hiện tại: ");
        Serial.print(temp);
        Serial.println(" °C");
        sendVoiceResponse("Dạ, nhiệt độ điều hòa số 1 hiện tại là " + String(temp) + " độ C.");
    }
    // Lấy nhiệt độ điều hòa 2
    else if (cmd == "AC2_TEMP_GET") {
        servo_togle(135);
        delay(1200);
        int temp = AC_Get_Temp();
        Serial.print("[AC2] Nhiệt độ hiện tại: ");
        Serial.print(temp);
        Serial.println(" °C");
        sendVoiceResponse("Dạ, nhiệt độ điều hòa số 2 hiện tại là " + String(temp) + " độ C.");
    }
    // Tăng nhiệt độ điều hòa 1
    else if (cmd.startsWith("AC1_TEMP_UP:")) {
        String valStr = cmd.substring(12);
        valStr.trim();
        int temp = valStr.toInt();
        servo_togle(0);
        delay(1200);
        AC_Set_Temp((uint8_t)temp);
        sendVoiceResponse("Dạ, đã tăng nhiệt độ điều hòa số 1 lên " + String(temp) + " độ C.");
    }
    // Giảm nhiệt độ điều hòa 1
    else if (cmd.startsWith("AC1_TEMP_DOWN:")) {
        String valStr = cmd.substring(14);
        valStr.trim();
        int temp = valStr.toInt();
        servo_togle(0);
        delay(1200);
        AC_Set_Temp((uint8_t)temp);
        sendVoiceResponse("Dạ, đã giảm nhiệt độ điều hòa số 1 xuống " + String(temp) + " độ C.");
    }
    // Tăng nhiệt độ điều hòa 2
    else if (cmd.startsWith("AC2_TEMP_UP:")) {
        String valStr = cmd.substring(12);
        valStr.trim();
        int temp = valStr.toInt();
        servo_togle(135);
        delay(1200);
        AC_Set_Temp((uint8_t)temp);
        sendVoiceResponse("Dạ, đã tăng nhiệt độ điều hòa số 2 lên " + String(temp) + " độ C.");
    }
    // Giảm nhiệt độ điều hòa 2
    else if (cmd.startsWith("AC2_TEMP_DOWN:")) {
        String valStr = cmd.substring(14);
        valStr.trim();
        int temp = valStr.toInt();
        servo_togle(135);
        delay(1200);
        AC_Set_Temp((uint8_t)temp);
        sendVoiceResponse("Dạ, đã giảm nhiệt độ điều hòa số 2 xuống " + String(temp) + " độ C.");
    }
}

void setup() {
    Serial.begin(BAUD_RATE);
    Serial1.begin(UART1_BAUD, SERIAL_8N1, UART1_RX, UART1_TX);
    set_servo();
    setupReadIR();
    setup_sendir(4);
    delay(300);
}

void loop() {
    loopReadIR();
    if (Serial1.available()) {
        String cmd = Serial1.readStringUntil('\n');
        cmd.trim();
        if (cmd.length() > 0) handleCommand_AC(cmd);
    }
}