#include "loamax98357.h"
#include <WiFi.h>
#include "Audio.h"

// Tao object audio de dieu khien module MAX98357A
// Thu vien Audio.h se phat audio qua I2S
static Audio audio;


// Ham ma hoa chuoi tieng Viet sang dang URL Encode
// Vi Google Translate TTS nhan text qua URL, nen dau cach, dau tieng Viet,
// ky tu dac biet phai doi sang dang %xx
static String urlEncodeUtf8(const String &s) {
    const char *hex = "0123456789ABCDEF";
    String out;
    out.reserve(s.length() * 3);

    for (size_t i = 0; i < s.length(); i++) {
        uint8_t c = (uint8_t)s[i];

        // Neu la chu cai, so hoac ky tu hop le trong URL thi giu nguyen
        if ((c >= 'A' && c <= 'Z') ||
            (c >= 'a' && c <= 'z') ||
            (c >= '0' && c <= '9') ||
            c == '-' || c == '_' || c == '.' || c == '~') {
            out += (char)c;
        }

        // Neu la dau cach thi doi thanh %20
        else if (c == ' ') {
            out += "%20";
        }

        // Cac ky tu con lai, bao gom tieng Viet co dau,
        // se duoc doi thanh dang %xx
        else {
            out += '%';
            out += hex[(c >> 4) & 0x0F];
            out += hex[c & 0x0F];
        }
    }

    return out;
}


// Ham khoi tao loa MAX98357A
// bclkPin: chan BCLK cua MAX98357A
// lrcPin : chan LRC / WS cua MAX98357A
// dinPin : chan DIN cua MAX98357A
// volume : am luong, thuong tu 0 den 100
void loaMAX98357Init(int bclkPin, int lrcPin, int dinPin, int volume) {
    // Gan chan I2S cho loa
    audio.setPinout(bclkPin, lrcPin, dinPin);

    // Dat am luong loa
    audio.setVolume(volume);

    Serial.println("-> [AUDIO] Loa MAX98357A san sang.");
}


// Ham dung phat am thanh
// Goi ham nay khi muon loa ngung noi ngay
void loaMAX98357Stop() {
    audio.stopSong();
}


// Ham doc text ra loa bang Google Translate TTS
// text: cau can phat ra loa
void loaMAX98357Speak(const String &text) {
    // Neu chuoi rong thi khong phat
    if (text.length() == 0) {
        Serial.println("!! Text rong, khong phat loa.");
        return;
    }

    // Google TTS can WiFi, neu chua co WiFi thi khong phat duoc
    if (WiFi.status() != WL_CONNECTED) {
        return;
    }

    // Tao ban copy cua text de xu ly
    String shortText = text;

    // Google Translate TTS URL khong nen gui text qua dai
    // Gioi han tam 180 ky tu cho on dinh
    if (shortText.length() > 180) {
        shortText = shortText.substring(0, 180);
    }

    // Tao link Google Translate TTS
    // tl=vi la ngon ngu tieng Viet
    String url = "https://translate.google.com/translate_tts?ie=UTF-8&tl=vi&client=tw-ob&q=";

    // Ma hoa text truoc khi ghep vao URL
    url += urlEncodeUtf8(shortText);

    Serial.print("NOI: ");
    Serial.println(shortText);

    // Dung bai cu neu dang phat
    audio.stopSong();
    delay(100);

    // Ket noi den link TTS va bat dau phat audio MP3 tu internet
    audio.connecttohost(url.c_str());

    uint32_t t0 = millis();

    // Bien nay luu thoi diem gan nhat audio con dang chay
    uint32_t lastRunning = millis();

    // Vong lap phat audio, toi da 20 giay
    while (millis() - t0 < 20000) {
        // Ham nay bat buoc phai goi lien tuc de audio duoc phat ra loa
        audio.loop();

        // Neu audio dang chay thi cap nhat moc thoi gian
        if (audio.isRunning()) {
            lastRunning = millis();
        }

        // Dieu kien thoat:
        // Sau it nhat 3 giay,
        // neu audio khong con chay va da dung hon 1.2 giay thi thoat vong lap
        if (!audio.isRunning() && millis() - t0 > 3000 && millis() - lastRunning > 1200) {
            break;
        }

        // Nhuong CPU cho FreeRTOS, tranh treo task
        vTaskDelay(pdMS_TO_TICKS(1));
    }

    // Dung audio sau khi noi xong
    audio.stopSong();
    delay(100);
}