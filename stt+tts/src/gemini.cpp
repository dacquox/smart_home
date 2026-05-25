#include "gemini.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <base64.h>

#include "dieukhien.h"
#include "loamax98357.h"
#include "uart.h"

// ================== CAU HINH GEMINI ==================

// Dien API key vao day
const char *GEMINI_API_KEY = "AIzaSyC4FHfxfHdeY9QRZX1G9C7Fjk2xTbdmwb8";

// Model nhanh hơn cho tác vụ STT/lệnh ngắn.
// const char *GEMINI_STT_MODEL = "gemini-3-flash-preview";
const char *GEMINI_STT_MODEL = "gemini-2.5-flash-lite";

// Timeout gọi Gemini
static const uint32_t GEMINI_HTTP_TIMEOUT_MS = 30000;

// Timeout đợi ESP32 điều khiển phản hồi UART
static const uint32_t UART_REPLY_TIMEOUT_MS = 500;

// Nếu không cần chờ phản hồi thật từ ESP32 điều khiển, đặt false để phản hồi nhanh hơn.
static const bool WAIT_UART_REPLY = true;
 
// ================== WAV HEADER ==================

// Ghi 44 byte header WAV vào đầu buffer audio.
// Buffer của bạn phải chừa sẵn 44 byte đầu cho header.
static void writeWavHeader(uint8_t *buffer, uint32_t pcmDataSize) {
    uint32_t fileSize = 36 + pcmDataSize;
    uint32_t sampleRate = 16000;
    uint32_t byteRate = sampleRate * 2; // mono, 16-bit

    uint8_t header[44] = {
        'R','I','F','F',
        (uint8_t)(fileSize & 0xFF),
        (uint8_t)((fileSize >> 8) & 0xFF),
        (uint8_t)((fileSize >> 16) & 0xFF),
        (uint8_t)((fileSize >> 24) & 0xFF),

        'W','A','V','E',

        'f','m','t',' ',
        16, 0, 0, 0,     // Subchunk1Size = 16
        1, 0,            // AudioFormat = PCM
        1, 0,            // NumChannels = 1 mono

        (uint8_t)(sampleRate & 0xFF),
        (uint8_t)((sampleRate >> 8) & 0xFF),
        (uint8_t)((sampleRate >> 16) & 0xFF),
        (uint8_t)((sampleRate >> 24) & 0xFF),

        (uint8_t)(byteRate & 0xFF),
        (uint8_t)((byteRate >> 8) & 0xFF),
        (uint8_t)((byteRate >> 16) & 0xFF),
        (uint8_t)((byteRate >> 24) & 0xFF),

        2, 0,            // BlockAlign = 2 bytes
        16, 0,           // BitsPerSample = 16

        'd','a','t','a',
        (uint8_t)(pcmDataSize & 0xFF),
        (uint8_t)((pcmDataSize >> 8) & 0xFF),
        (uint8_t)((pcmDataSize >> 16) & 0xFF),
        (uint8_t)((pcmDataSize >> 24) & 0xFF)
    };

    memcpy(buffer, header, 44);
}


// ================== HTTP GEMINI ==================

static String postGeminiJson(const char *model, const String &payload, uint32_t timeoutMs) {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("!! WiFi chua ket noi.");
        return "";
    }

    if (strlen(GEMINI_API_KEY) == 0) {
        Serial.println("!! Chua dien GEMINI_API_KEY.");
        return "";
    }

    WiFiClientSecure client;
    client.setInsecure();

    HTTPClient https;
    https.setTimeout(timeoutMs);
    https.setReuse(false);

    String url = "https://generativelanguage.googleapis.com/v1beta/models/";
    url += model;
    url += ":generateContent";

    Serial.print("Dang goi Gemini model: ");
    Serial.println(model);

    if (!https.begin(client, url)) {
        Serial.println("!! Khong begin HTTPS duoc.");
        return "";
    }

    https.addHeader("Content-Type", "application/json");
    https.addHeader("x-goog-api-key", GEMINI_API_KEY);

    uint32_t t0 = millis();
    int httpCode = https.POST(payload);
    uint32_t t1 = millis();

    String response = https.getString();
    https.end();

    Serial.printf("[TIME] HTTPS POST = %lu ms\n", t1 - t0);
    Serial.printf("[HTTP] Code = %d, response length = %d\n", httpCode, response.length());

    if (httpCode != HTTP_CODE_OK) {
        Serial.printf("!! HTTP loi: %d\n", httpCode);
        Serial.println("----- RESPONSE LOI -----");
        Serial.println(response.substring(0, 1800));
        return "";
    }

    return response;
}


// ================== DOC TEXT TU JSON ==================

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

    // Xoa dau ngoac kep neu model lo tra ve "bat den"
    if (text.length() >= 2) {
        if (text[0] == '"' && text[text.length() - 1] == '"') {
            text = text.substring(1, text.length() - 1);
            text.trim();
        }
    }

    return text;
}

// ================== GUI AUDIO LEN GEMINI DE STT ==================

static String transcribeAudioToText(uint8_t *wavBuffer, size_t wavSize) {
    if (!wavBuffer || wavSize <= 44) {
        Serial.println("!! Chua co audio de nhan dang.");
        return "";
    }

    Serial.println();
    Serial.println("===== BAT DAU NHAN DANG GIONG NOI =====");
    Serial.printf("WAV size: %u bytes\n", (unsigned int)wavSize);

    uint32_t t0 = millis();

    // Base64 la buoc kha nang. Audio cang dai thi buoc nay va upload cang cham.
    String encodedAudio = base64::encode(wavBuffer, wavSize);

    uint32_t t1 = millis();

    if (encodedAudio.length() == 0) {
        Serial.println("!! Loi base64 audio.");
        return "";
    }

    Serial.printf("Base64 size: %d bytes\n", encodedAudio.length());

    String payload;
    payload.reserve(encodedAudio.length() + 800);

    // Prompt rut gon de giam token va giam thoi gian xu ly.
    payload = "{";
    payload += "\"contents\":[{";
    payload += "\"parts\":[";
    payload += "{";
    payload += "\"text\":\"";
    payload += "Transcribe audio to lowercase Vietnamese without accents. ";
    payload += "Return only the spoken command. Convert spoken numbers to digits.";
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
    payload += "\"temperature\":0,";
    payload += "\"maxOutputTokens\":32";
    payload += "}";
    payload += "}";

    // Giai phong chuoi base64 som de nhe RAM hon
    encodedAudio = "";

    uint32_t t2 = millis();

    String response = postGeminiJson(GEMINI_STT_MODEL, payload, GEMINI_HTTP_TIMEOUT_MS);

    uint32_t t3 = millis();

    // Giai phong payload
    payload = "";


    if (response.length() == 0) {
        Serial.println("!! Gemini khong tra ve response hop le.");
        return "";
    }

    String text = extractFirstText(response);

    uint32_t t4 = millis();

    return text;
}


// ================== DOI PHAN HOI UART ==================

static String waitUartReply(uint32_t timeoutMs) {
    String reply = "";
    unsigned long startWait = millis();

    while (millis() - startWait < timeoutMs) {
        reply = uartReadLine();

        if (reply.length() > 0) {
            reply.trim();
            return reply;
        }

        delay(5);
    }

    return "";
}


// ================== HAM CHINH XU LY AUDIO ==================

void geminiProcessAudio(uint8_t *wavBuffer, size_t wavSize) {
    uint32_t totalStart = millis();

    if (!wavBuffer || wavSize <= 44) {
        Serial.println("!! Chua co du lieu audio.");
        return;
    }

    // Ghi WAV header vao 44 byte dau.
    writeWavHeader(wavBuffer, wavSize - 44);

    // Goi Gemini STT
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

    uint32_t detectStart = millis();

    // Detect lenh tu text
    CommandResult result = dieukhienDetectCommand(saidText);

    uint32_t detectEnd = millis();
    Serial.printf("[TIME] detect command = %lu ms\n", detectEnd - detectStart);

    String textToSpeak;

    if (result.cmd != CMD_NONE) {
        // Clear buffer UART cu de tranh doc nham phan hoi lan truoc
        clearUartReplyBuffer();

        // Luu ket qua thu duoc vao chuoi de debug
        String frame = dieukhienCommandName(result);

        Serial.print("LENH GUI UART: ");
        Serial.println(frame);

        uint32_t sendStart = millis();

        // Gui lenh sang ESP32 dieu khien
        dieukhienHandleCommand(result);

        uint32_t sendEnd = millis();
        Serial.printf("[TIME] send command UART = %lu ms\n", sendEnd - sendStart);

        // Cau tra loi mac dinh neu ESP32 dieu khien khong phan hoi
        textToSpeak = dieukhienAnswerForCommand(result);

        if (WAIT_UART_REPLY) {
            String reply = waitUartReply(UART_REPLY_TIMEOUT_MS);


            if (reply.length() > 0) {
                textToSpeak = reply;

                Serial.print("ESP32 DIEU KHIEN PHAN HOI: ");
                Serial.println(reply);
            } else {
                Serial.println("!! Khong nhan duoc phan hoi tu ESP32 dieu khien.");
            }
        } else {
            Serial.println("Bo qua cho UART reply de phan hoi nhanh hon.");
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

    uint32_t speakStart = millis();

    // Phat cau tra loi qua MAX98357
    loaMAX98357Speak(textToSpeak);

    uint32_t speakEnd = millis();
    uint32_t totalEnd = millis();

    Serial.println();
}