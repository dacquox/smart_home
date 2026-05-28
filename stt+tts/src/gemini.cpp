#include "gemini.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <base64.h>
#include "dieukhien.h"
#include "loamax98357.h"
#include "uart.h"

// api key
const char *GEMINI_API_KEY = "AIzaSyCOHKKED_nMGOhX8xWI16kgKD4uXWlYsOA";
const char *GEMINI_STT_MODEL = "gemini-3-flash-preview";

// ghi 44 header vao dau buffer audio
static void writeWavHeader(uint8_t *buffer, uint32_t pcmDataSize) {
    uint32_t fileSize = 36 + pcmDataSize;
    uint32_t sampleRate = 16000;
    uint32_t byteRate = 16000 * 2;

    uint8_t header[44] = {
        'R','I','F','F',
        (uint8_t)(fileSize & 0xFF),
        (uint8_t)((fileSize >> 8) & 0xFF),
        (uint8_t)((fileSize >> 16) & 0xFF),
        (uint8_t)((fileSize >> 24) & 0xFF),

        'W','A','V','E',

        'f','m','t',' ',
        16, 0, 0, 0,
        1, 0,
        1, 0,

        (uint8_t)(sampleRate & 0xFF),
        (uint8_t)((sampleRate >> 8) & 0xFF),
        (uint8_t)((sampleRate >> 16) & 0xFF),
        (uint8_t)((sampleRate >> 24) & 0xFF),

        (uint8_t)(byteRate & 0xFF),
        (uint8_t)((byteRate >> 8) & 0xFF),
        (uint8_t)((byteRate >> 16) & 0xFF),
        (uint8_t)((byteRate >> 24) & 0xFF),

        2, 0,
        16, 0,

        'd','a','t','a',
        (uint8_t)(pcmDataSize & 0xFF),
        (uint8_t)((pcmDataSize >> 8) & 0xFF),
        (uint8_t)((pcmDataSize >> 16) & 0xFF),
        (uint8_t)((pcmDataSize >> 24) & 0xFF)
    };

    memcpy(buffer, header, 44);
}

// ham gui json len gemini api
static String postGeminiJson(const char *model, const String &payload, uint32_t timeoutMs) {
    if (WiFi.status() != WL_CONNECTED) {
        return "";
    }

    WiFiClientSecure client;
    client.setInsecure();

    HTTPClient https;
    https.setTimeout(timeoutMs);

    String url = "https://generativelanguage.googleapis.com/v1beta/models/";
    url += model;
    url += ":generateContent";

    if (!https.begin(client, url)) {
        Serial.println("!! Khong begin HTTPS duoc.");
        return "";
    }

    https.addHeader("Content-Type", "application/json");
    https.addHeader("x-goog-api-key", GEMINI_API_KEY);

    int httpCode = https.POST(payload);
    String response = https.getString();

    if (httpCode != HTTP_CODE_OK) {
        Serial.printf("!! HTTP loi: %d\n", httpCode);
        Serial.println("----- RESPONSE LOI -----");
        Serial.println(response.substring(0, 1800));
        https.end();
        return "";
    }

    https.end();

    return response;
}

// ham lay text tu json response cua gemini
static String extractFirstText(const String &response) {
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, response);

    if (err) {
        Serial.print("!! Loi parse JSON text: ");
        Serial.println(err.c_str());
        Serial.println(response.substring(0, 1000));
        return "";
    }

    String text = doc["candidates"][0]["content"]["parts"][0]["text"] | "";
    text.trim();

    return text;
}

// gui audio len gemini de nhan dang giong noi
static String transcribeAudioToText(uint8_t *wavBuffer, size_t wavSize) {
    if (!wavBuffer || wavSize <= 44) {
        Serial.println("!! Chua co audio de nhan dang.");
        return "";
    }

    String encodedAudio = base64::encode(wavBuffer, wavSize);

    if (encodedAudio.length() == 0) {
        Serial.println("!! Loi base64 audio.");
        return "";
    }

    String payload;
    payload.reserve(encodedAudio.length() + 1400);

    payload = "{";
    payload += "\"contents\":[{";
    payload += "\"parts\":[";
    payload += "{";
    payload += "\"text\":\"";
    payload += "Hay nhan dang giong noi trong audio. ";
    payload += "Chi tra ve dung noi dung nguoi dung noi. ";
    payload += "Tra ve chu thuong, khong dau tieng Viet de ESP32 de xu ly. ";
    payload += "Neu nguoi dung noi so mot thi tra ve so 1, so hai thi tra ve so 2. ";
    payload += "Vi du: bat dieu hoa so 1, tat den, giam nhiet do 1 xuong 2 do. ";
    payload += "Khong giai thich. Khong them dau ngoac kep.";
    payload += "\"";
    payload += "},";
    payload += "{";
    payload += "\"inlineData\":{";
    payload += "\"mimeType\":\"audio/wav\",";
    payload += "\"data\":\"";
    payload += encodedAudio;
    payload += "\"";
    payload += "}";
    payload += "}";
    payload += "]";
    payload += "}],";
    payload += "\"generationConfig\":{";
    payload += "\"temperature\":0";
    payload += "}";
    payload += "}";

    encodedAudio = "";

    String response = postGeminiJson(GEMINI_STT_MODEL, payload, 60000);

    if (response.length() == 0) {
        return "";
    }

    return extractFirstText(response);
}

// cho ESP32 dieu khien phan hoi lai
static String waitUartReply(uint32_t timeoutMs) {
    String reply = "";
    unsigned long startWait = millis();

    while (millis() - startWait < timeoutMs) {
        reply = uartReadLine();

        if (reply.length() > 0) {
            return reply;
        }

        delay(10);
    }

    return "";
}

// xu ly khi thu xong
void geminiProcessAudio(uint8_t *wavBuffer, size_t wavSize) {
    if (!wavBuffer || wavSize <= 44) {
        Serial.println("!! Chua co du lieu audio.");
        return;
    }

    writeWavHeader(wavBuffer, wavSize - 44);

    String saidText = transcribeAudioToText(wavBuffer, wavSize);

    if (saidText.length() == 0) {
        Serial.println("!! Khong nhan dang duoc loi noi.");
        loaMAX98357Speak("Tôi chưa nghe rõ, bạn nói lại giúp tôi nhé.");
        return;
    }

    Serial.println();
    Serial.print("BAN NOI: ");
    Serial.println(saidText);
    Serial.println();

    CommandResult result = dieukhienDetectCommand(saidText);

    String textToSpeak;

    if (result.cmd != CMD_NONE) {

        //clear buffer cũ
         clearUartReplyBuffer();
        
        //lưu kết quả thu được vào chuỗi
        String frame = dieukhienCommandName(result);

        Serial.print("LENH GUI UART: ");
        Serial.println(frame);

        // gui lenh sang ESP32 dieu khien
        dieukhienHandleCommand(result);

        // cau tra loi tam thoi neu ESP32 dieu khien khong phan hoi
        textToSpeak = dieukhienAnswerForCommand(result);

        // doi ESP32 dieu khien phan hoi that
        String reply = waitUartReply(500);

        if (reply.length() > 0) {
            textToSpeak = reply;

            Serial.print("ESP32 DIEU KHIEN PHAN HOI: ");
            Serial.println(reply);
        } else {
            Serial.println("!! Khong nhan duoc phan hoi tu ESP32 dieu khien.");
        }

        Serial.print("TRA LOI: ");
        Serial.println(textToSpeak);
        Serial.println();
    } else {
        textToSpeak = dieukhienUnknownAnswer();

        Serial.print("TRA LOI: ");
        Serial.println(textToSpeak);
        Serial.println();
    }

    loaMAX98357Speak(textToSpeak);
}