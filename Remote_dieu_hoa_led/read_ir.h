#ifndef READ_IR_H
#define READ_IR_H

#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>
#include <IRac.h>       // Thêm: decode điều hòa
#include <IRtext.h>

// ESP32: dùng pin 15, có thể đổi tuỳ sơ đồ mạch
#define RECV_PIN 15

// Kích thước buffer lớn hơn để bắt tín hiệu điều hòa (thường 200-500 marks)
#define CAPTURE_BUFFER_SIZE 1024
#define IR_TIMEOUT_MS 50   // Đổi tên tránh trùng thư viện

void setupReadIR();
void loopReadIR();

#endif