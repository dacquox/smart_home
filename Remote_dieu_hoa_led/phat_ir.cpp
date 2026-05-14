#include "phat_ir.h"

static IRsend* irsend = nullptr;
static uint8_t currentPin = DEFAULT_SEND_PIN;

//================ INIT =================
void setup_sendir(uint8_t pin) {
    currentPin = pin;

    if (irsend != nullptr) {
        delete irsend;
    }

    irsend = new IRsend(pin);
    irsend->begin();
}

//================ SET PIN =================
void setSendPin(uint8_t pin) {
    setup_sendir(pin);  // recreate lại object
}

//================ RAW =================
void sendRaw(const uint16_t buf[], uint16_t len) {
    if (irsend == nullptr || buf == nullptr || len == 0) return;

    irsend->sendRaw(buf, len, 38);
    delay(50);
}