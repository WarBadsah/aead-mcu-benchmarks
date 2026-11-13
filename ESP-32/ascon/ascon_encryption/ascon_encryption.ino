#include <Arduino.h>

extern "C" {
#include "src/esp32/crypto_aead.h"
}

// =========================================================================
#define MAX_PAYLOAD_SIZE 512 // 16, 32, 64, 128, 256, 512: rebuild per size
// =========================================================================

#define TAG_SIZE 16
#define ITERATIONS 10000

uint8_t plaintext_buffer[MAX_PAYLOAD_SIZE];
uint8_t ciphertext_buffer[MAX_PAYLOAD_SIZE + TAG_SIZE];

const uint8_t dummy_key[16]   = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};
const uint8_t dummy_nonce[16] = {0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f};

void setup() {
    setCpuFrequencyMhz(240);
    Serial.begin(115200);
    delay(2000);

    for (uint16_t i = 0; i < MAX_PAYLOAD_SIZE; i++) {
        plaintext_buffer[i] = (uint8_t)(i % 256);
    }
}

void loop() {
    Serial.print(F("--- ESP32 ASCON-AEAD128 ENCRYPT | Payload: "));
    Serial.print(MAX_PAYLOAD_SIZE);
    Serial.println(F(" Bytes ---"));
    Serial.flush();

    volatile uint8_t checksum = 0;
    unsigned long long clen = 0;

    uint32_t start_time = micros();

    for (uint16_t i = 0; i < ITERATIONS; i++) {
        plaintext_buffer[0] = (uint8_t)i;

        crypto_aead_encrypt(ciphertext_buffer, &clen,
                            plaintext_buffer, MAX_PAYLOAD_SIZE,
                            NULL, 0, NULL,
                            dummy_nonce, dummy_key);

        checksum += ciphertext_buffer[0];
    }

    uint32_t total_time_us = micros() - start_time;

    uint32_t cpu_freq_mhz = ESP.getCpuFreqMHz();
    uint32_t cycles_per_op = (uint32_t)(((uint64_t)total_time_us * cpu_freq_mhz) / ITERATIONS);
    float latency_us = (float)cycles_per_op / (float)cpu_freq_mhz;

    Serial.print(F("Gross Cycles: ")); Serial.println(cycles_per_op);
    Serial.print(F("Latency (us): ")); Serial.println(latency_us, 2);
    Serial.print(F("Checksum: "));     Serial.println(checksum);

  
    while (1) {
        delay(10);
    }
}