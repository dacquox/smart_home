#include <Arduino.h>
#include <WiFi.h>
#include "loamax98357.h"
#include "uart.h"
#include "edge_wake.h"

const char *WIFI_SSID = "Khu Tro";
const char *WIFI_PASSWORD = "11111111";


const int MAX98357_BCLK = 15;
const int MAX98357_LRC  = 16;
const int MAX98357_DIN  = 7;

const int UART2_RX_PIN = 18;
const int UART2_TX_PIN = 17;
const uint32_t UART2_BAUD = 115200;

void connectWiFi() {
    WiFi.mode(WIFI_STA);
    WiFi.setSleep(false);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    Serial.print("Dang ket noi WiFi");

    uint32_t t0 = millis();

    while (WiFi.status() != WL_CONNECTED && millis() - t0 < 15000) {
        Serial.print(".");
        delay(500);
    }

    Serial.println();

    if (WiFi.status() == WL_CONNECTED) {
        Serial.print("WiFi OK, IP: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("!! Chua ket noi duoc WiFi.");
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println();
    Serial.println("===== ESP32 S3 VOICE BOT START =====");

    connectWiFi();

    uartLinkBegin(UART2_RX_PIN, UART2_TX_PIN, UART2_BAUD);

    loaMAX98357Init(MAX98357_BCLK, MAX98357_LRC, MAX98357_DIN, 100);

    // Edge Impulse nghe tu goi bot "white"
    edgeWakeBegin();

    Serial.println("San sang. Hay goi: white");
}

void loop() {
    if (WiFi.status() != WL_CONNECTED) {
        static uint32_t lastReconnect = 0;

        if (millis() - lastReconnect > 5000) {
            lastReconnect = millis();

            Serial.println("-> Dang ket noi lai WiFi...");
            WiFi.disconnect();
            WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
        }
    }

    edgeWakeLoop();
}