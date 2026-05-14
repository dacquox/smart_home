#include "read_ir.h"

// Buffer lớn hơn để bắt tín hiệu điều hòa
IRrecv irrecv(RECV_PIN, CAPTURE_BUFFER_SIZE, IR_TIMEOUT_MS, true);
decode_results results;

void setupReadIR() {
    Serial.begin(115200);
    irrecv.enableIRIn();
    Serial.println("===========================================");
    Serial.println("  IR Receiver Ready (ESP32)");
    Serial.println("  Chia remote vao PIN 15, bam nut bat ky");
    Serial.println("===========================================");
}

void loopReadIR() {
    if (!irrecv.decode(&results)) return;

    Serial.println("\n==================== TIN HIEU IR ====================");

    // ===== Thông tin điều hòa (dùng API chuẩn của thư viện) =====
    if (results.decode_type != UNKNOWN) {
        Serial.println("------ THONG TIN ------");
        Serial.println(resultToHumanReadableBasic(&results));
        Serial.println("-----------------------");
    }

    // ===== RAW Data - dùng để copy vào control.cpp =====
    Serial.println("------ RAW DATA (copy vao control.cpp) ------");
    Serial.print("uint16_t rawData[] = {");
    for (uint16_t i = 1; i < results.rawlen; i++) {
        Serial.print(results.rawbuf[i] * kRawTick);
        if (i < results.rawlen - 1) Serial.print(", ");
    }
    Serial.println("};");
    Serial.print(" Length = ");
    Serial.println(results.rawlen - 1);
    Serial.println("=====================================================\n");

    irrecv.resume();
}