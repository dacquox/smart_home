#define EIDSP_QUANTIZE_FILTERBANK 0
#include <Arduino.h>
#include <string.h>
#include <esp_heap_caps.h>
#include "Dieu_Khien_Giong_Noi_Offline_inferencing.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2s.h"
#include "edge_wake.h"
#include "gemini.h"
#include "loamax98357.h"

// ================== CHAN INMP441 ==================
const int INMP441_SCK = 5;   // SCK / BCLK
const int INMP441_WS  = 4;   // WS / LRCLK
const int INMP441_SD  = 6;   // SD / DOUT

#define INMP441_CHANNEL I2S_CHANNEL_FMT_ONLY_LEFT

// ================== BOT CONFIG ==================
// Label trong Edge Impulse phai dung y het la "White"
const char *WAKE_LABEL = "diệp ơi";

// Chi can white >= 0.05 la goi bot
const float WAKE_CONFIDENCE = 0.2f;

// Sau khi goi "white", bot noi "Vang toi day", doi 0.5 giay roi thu lenh
const uint32_t WAIT_AFTER_WAKE_MS = 10;

// Thu lenh 5 giay gui len Gemini/API
const uint32_t COMMAND_SAMPLE_RATE = 16000;
const uint32_t COMMAND_SECONDS = 4;

const size_t COMMAND_PCM_SAMPLES = COMMAND_SAMPLE_RATE * COMMAND_SECONDS;
const size_t COMMAND_PCM_BYTES = COMMAND_PCM_SAMPLES * 2;
const size_t COMMAND_WAV_BYTES = 44 + COMMAND_PCM_BYTES;

// ================== I2S CONFIG ==================
static const i2s_port_t MIC_I2S_PORT = I2S_NUM_1;

static const size_t RAW_READ_SAMPLES = 512;
static const size_t RAW_READ_BYTES = RAW_READ_SAMPLES * sizeof(int32_t);

static const int EDGE_MIC_GAIN = 2;
static const int COMMAND_MIC_GAIN = 4;

// ================== EDGE IMPULSE BUFFER ==================
typedef struct {
    int16_t *buffer;
    uint8_t buf_ready;
    uint32_t buf_count;
    uint32_t n_samples;
} inference_t;

static inference_t inference = {0};

static int32_t rawSampleBuffer[RAW_READ_SAMPLES];

static bool debug_nn = false;
static volatile bool record_status = false;

// ================== PROTOTYPE ==================
static bool microphone_inference_start(uint32_t n_samples);
static bool microphone_inference_record(void);
static void microphone_inference_end(void);
static int microphone_audio_signal_get_data(size_t offset, size_t length, float *out_ptr);

static void capture_samples(void *arg);
static void push_samples_to_edge(const int16_t *samples, size_t sampleCount);

static int i2s_init(uint32_t sampling_rate);
static int i2s_deinit(void);

static bool handleWakeWord(ei_impulse_result_t &result);
static bool recordCommandAudio(uint8_t **outBuffer, size_t *outSize);
static int16_t convertINMP441To16Bit(int32_t sample32, int gain);

// ================== EDGE BEGIN ==================
void edgeWakeBegin()
{
    ei_printf("Inferencing settings:\n");
    ei_printf("\tInterval: ");
    ei_printf_float((float)EI_CLASSIFIER_INTERVAL_MS);
    ei_printf(" ms.\n");

    ei_printf("\tFrame size: %d\n", EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE);
    ei_printf("\tSample length: %d ms.\n", EI_CLASSIFIER_RAW_SAMPLE_COUNT / 16);
    ei_printf("\tNo. of classes: %d\n",
              sizeof(ei_classifier_inferencing_categories) / sizeof(ei_classifier_inferencing_categories[0]));

    ei_printf("\nStarting continuous inference in 2 seconds...\n");
    ei_sleep(2000);

    if (!microphone_inference_start(EI_CLASSIFIER_RAW_SAMPLE_COUNT)) {
        return;
    }

}

// ================== EDGE LOOP ==================
void edgeWakeLoop()
{
    if (!microphone_inference_record()) {
        return;
    }

    signal_t signal;
    signal.total_length = EI_CLASSIFIER_RAW_SAMPLE_COUNT;
    signal.get_data = &microphone_audio_signal_get_data;

    ei_impulse_result_t result = {0};

    EI_IMPULSE_ERROR r = run_classifier(&signal, &result, debug_nn);

    if (r != EI_IMPULSE_OK) {
        Serial.printf("!! run_classifier loi: %d\n", r);
        return;
    }

    for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
        Serial.print(result.classification[ix].label);
        Serial.print(": ");
        Serial.println(result.classification[ix].value, 4);
    }

#if EI_CLASSIFIER_HAS_ANOMALY == 1
    Serial.print("anomaly: ");
    Serial.println(result.anomaly, 4);
#endif

    handleWakeWord(result);
}

// ================== HANDLE WAKE WORD ==================
static bool handleWakeWord(ei_impulse_result_t &result)
{
    float whiteScore = 0.0f;

    // Tim rieng diem cua label "White"
    for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
        if (strcmp(result.classification[ix].label, WAKE_LABEL) == 0) {
            whiteScore = result.classification[ix].value;
            break;
        }
    }

    Serial.print("WAKE LABEL: ");
    Serial.print(WAKE_LABEL);
    Serial.print(" | SCORE: ");
    Serial.println(whiteScore, 4);

    // Chi can white >= 0.005 la kich hoat
    if (whiteScore < WAKE_CONFIDENCE) {
        return false;
    }

    Serial.println();
    Serial.println("===== DA GOI DUNG BOT: WHITE =====");

    // Dung Edge Impulse de giai phong I2S mic
    microphone_inference_end();

    Serial.println("BOT: Da em diep day");
    loaMAX98357Speak("Dạ, em Diệp đây.");

    delay(WAIT_AFTER_WAKE_MS);

    uint8_t *wavBuffer = nullptr;
    size_t wavSize = 0;

    Serial.println();
    Serial.print("===== BAT DAU THU LENH ");

    if (i2s_init(COMMAND_SAMPLE_RATE) != 0) {
        microphone_inference_start(EI_CLASSIFIER_RAW_SAMPLE_COUNT);
        return true;
    }

    bool ok = recordCommandAudio(&wavBuffer, &wavSize);

    i2s_deinit();

    if (!ok || wavBuffer == nullptr || wavSize <= 44) {
        Serial.println("!! Thu lenh that bai.");

        if (wavBuffer) {
            free(wavBuffer);
            wavBuffer = nullptr;
        }

        microphone_inference_start(EI_CLASSIFIER_RAW_SAMPLE_COUNT);
        return true;
    }

    Serial.println();
    Serial.println("===== THU XONG, GUI AUDIO LEN GEMINI/API =====");

    geminiProcessAudio(wavBuffer, wavSize);

    free(wavBuffer);
    wavBuffer = nullptr;

    Serial.println();
    Serial.println("===== QUAY LAI CHE DO NGHE WHITE =====");

    microphone_inference_start(EI_CLASSIFIER_RAW_SAMPLE_COUNT);

    return true;
}

// ================== THU LENH COMMAND_SECONDS GIAY ==================
static bool recordCommandAudio(uint8_t **outBuffer, size_t *outSize)
{
    if (!outBuffer || !outSize) {
        return false;
    }

    *outBuffer = nullptr;
    *outSize = 0;

    uint8_t *wavBuffer = (uint8_t *)ps_malloc(COMMAND_WAV_BYTES);

    if (!wavBuffer) {
        wavBuffer = (uint8_t *)malloc(COMMAND_WAV_BYTES);
    }

    if (!wavBuffer) {
        Serial.println("!! Khong du RAM/PSRAM de cap phat audio.");
        Serial.printf("Can khoang: %u bytes\n", (unsigned int)COMMAND_WAV_BYTES);
        return false;
    }

    // 44 byte dau de geminiProcessAudio() ghi WAV header
    memset(wavBuffer, 0, 44);

    size_t totalSamples = 0;

    while (totalSamples < COMMAND_PCM_SAMPLES) {
        size_t bytesRead = 0;

        esp_err_t ret = i2s_read(
            MIC_I2S_PORT,
            (void *)rawSampleBuffer,
            RAW_READ_BYTES,
            &bytesRead,
            portMAX_DELAY
        );

        if (ret != ESP_OK) {
            Serial.println("!! Loi i2s_read khi thu lenh.");
            free(wavBuffer);
            return false;
        }

        size_t samplesRead = bytesRead / sizeof(int32_t);

        for (size_t i = 0; i < samplesRead && totalSamples < COMMAND_PCM_SAMPLES; i++) {
            int16_t out = convertINMP441To16Bit(rawSampleBuffer[i], COMMAND_MIC_GAIN);

            size_t byteIndex = 44 + totalSamples * 2;

            wavBuffer[byteIndex] = (uint8_t)(out & 0xFF);
            wavBuffer[byteIndex + 1] = (uint8_t)((out >> 8) & 0xFF);

            totalSamples++;
        }

        Serial.print(".");
        delay(1);
    }

    Serial.println();

    *outBuffer = wavBuffer;
    *outSize = COMMAND_WAV_BYTES;

    return true;
}

// ================== CONVERT INMP441 32BIT -> 16BIT ==================
static int16_t convertINMP441To16Bit(int32_t sample32, int gain)
{
    // INMP441 thuong tra 24-bit nam trong int32.
    // Dich ve 16-bit PCM.
    int32_t sample16 = sample32 >> 14;

    sample16 *= gain;

    if (sample16 > 32767) sample16 = 32767;
    if (sample16 < -32768) sample16 = -32768;

    return (int16_t)sample16;
}

// ================== PUSH SAMPLE CHO EDGE ==================
static void push_samples_to_edge(const int16_t *samples, size_t sampleCount)
{
    if (!inference.buffer) {
        return;
    }

    for (size_t i = 0; i < sampleCount; i++) {
        inference.buffer[inference.buf_count++] = samples[i];

        if (inference.buf_count >= inference.n_samples) {
            inference.buf_count = 0;
            inference.buf_ready = 1;
        }
    }
}

// ================== TASK DOC MIC CHO EDGE ==================
static void capture_samples(void *arg)
{
    (void)arg;

    int16_t converted[RAW_READ_SAMPLES];

    while (record_status) {
        size_t bytesRead = 0;

        esp_err_t ret = i2s_read(
            MIC_I2S_PORT,
            (void *)rawSampleBuffer,
            RAW_READ_BYTES,
            &bytesRead,
            100
        );

        if (ret != ESP_OK) {
            Serial.println("!! Edge i2s_read loi.");
            delay(10);
            continue;
        }

        if (bytesRead > 0) {
            size_t samplesRead = bytesRead / sizeof(int32_t);

            if (samplesRead > RAW_READ_SAMPLES) {
                samplesRead = RAW_READ_SAMPLES;
            }

            for (size_t i = 0; i < samplesRead; i++) {
                converted[i] = convertINMP441To16Bit(rawSampleBuffer[i], EDGE_MIC_GAIN);
            }

            if (record_status) {
                push_samples_to_edge(converted, samplesRead);
            }
        }
    }

    vTaskDelete(NULL);
}

// ================== START EDGE MIC ==================
static bool microphone_inference_start(uint32_t n_samples)
{
    record_status = false;
    delay(150);

    if (inference.buffer != NULL) {
        ei_free(inference.buffer);
        inference.buffer = NULL;
    }

    inference.buffer = (int16_t *)malloc(n_samples * sizeof(int16_t));

    if (inference.buffer == NULL) {
        return false;
    }

    inference.buf_count = 0;
    inference.n_samples = n_samples;
    inference.buf_ready = 0;

    if (i2s_init(EI_CLASSIFIER_FREQUENCY) != 0) {
        ei_free(inference.buffer);
        inference.buffer = NULL;

        return false;
    }

    delay(100);

    record_status = true;

    BaseType_t taskOk = xTaskCreate(
        capture_samples,
        "CaptureSamples",
        1024 * 32,
        NULL,
        10,
        NULL
    );

    if (taskOk != pdPASS) {
        record_status = false;
        i2s_deinit();

        ei_free(inference.buffer);
        inference.buffer = NULL;

        return false;
    }

    return true;
}

// ================== DOI DU AUDIO CHO EDGE ==================
static bool microphone_inference_record(void)
{
    if (!inference.buffer) {
        return false;
    }

    while (inference.buf_ready == 0) {
        delay(10);
    }

    inference.buf_ready = 0;

    return true;
}

// ================== LAY DATA CHO EDGE ==================
static int microphone_audio_signal_get_data(size_t offset, size_t length, float *out_ptr)
{
    if (!inference.buffer) {
        return -1;
    }

    numpy::int16_to_float(&inference.buffer[offset], out_ptr, length);

    return 0;
}

// ================== STOP EDGE MIC ==================
static void microphone_inference_end(void)
{
    record_status = false;
    delay(200);

    i2s_deinit();

    if (inference.buffer != NULL) {
        ei_free(inference.buffer);
        inference.buffer = NULL;
    }

    inference.buf_ready = 0;
    inference.buf_count = 0;
    inference.n_samples = 0;
}

// ================== INIT I2S MIC ==================
static int i2s_init(uint32_t sampling_rate)
{
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = sampling_rate,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
        .channel_format = INMP441_CHANNEL,
        .communication_format = I2S_COMM_FORMAT_I2S,
        .intr_alloc_flags = 0,
        .dma_buf_count = 8,
        .dma_buf_len = 512,
        .use_apll = false,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0,
    };

    i2s_pin_config_t pin_config = {
        .bck_io_num = INMP441_SCK,
        .ws_io_num = INMP441_WS,
        .data_out_num = -1,
        .data_in_num = INMP441_SD,
    };

    esp_err_t ret = i2s_driver_install(MIC_I2S_PORT, &i2s_config, 0, NULL);

    if (ret != ESP_OK) {
        return int(ret);
    }

    ret = i2s_set_pin(MIC_I2S_PORT, &pin_config);

    if (ret != ESP_OK) {
        i2s_driver_uninstall(MIC_I2S_PORT);
        return int(ret);
    }

    ret = i2s_zero_dma_buffer(MIC_I2S_PORT);

    if (ret != ESP_OK) {
        i2s_driver_uninstall(MIC_I2S_PORT);
        return int(ret);
    }

    return 0;
}

// ================== DEINIT I2S MIC ==================
static int i2s_deinit(void)
{
    i2s_driver_uninstall(MIC_I2S_PORT);
    return 0;
}

#if !defined(EI_CLASSIFIER_SENSOR) || EI_CLASSIFIER_SENSOR != EI_CLASSIFIER_SENSOR_MICROPHONE
#error "Invalid model for current sensor."
#endif