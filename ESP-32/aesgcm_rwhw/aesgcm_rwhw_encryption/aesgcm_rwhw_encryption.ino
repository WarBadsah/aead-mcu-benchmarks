// the baseline stays the same as Ascon’s, because the AES-GCM tag is also 16 B. The two baseline sketches are unchanged (only the title text differs)


#include <Arduino.h>
#include "src/AES.h"
#include "src/GCM.h"

// =========================================================================
#define MAX_PAYLOAD_SIZE 512 // 16, 32, 64, 128, 256, 512: rebuild per size
// =========================================================================

#define TAG_SIZE 32
#define ITERATIONS 10000

uint8_t plaintext_buffer[MAX_PAYLOAD_SIZE];
uint8_t ciphertext_buffer[MAX_PAYLOAD_SIZE + TAG_SIZE]; // tag goes at the end
GCM<AES128> gcm;                                        // AES128 maps to the ESP32 hardware backend

const uint8_t dummy_key[16]   = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};
const uint8_t dummy_nonce[12] = {0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b};

void setup() {
    setCpuFrequencyMhz(240);
    Serial.begin(115200);
    delay(2000); // Give the serial monitor time to connect

    for (uint16_t i = 0; i < MAX_PAYLOAD_SIZE; i++) {
        plaintext_buffer[i] = (uint8_t)(i % 256);
    }
}

void loop() {
    Serial.print(F("--- ESP32 AES-128-GCM (rweather, HW AES backend) ENCRYPT | Payload: "));
    Serial.print(MAX_PAYLOAD_SIZE);
    Serial.println(F(" Bytes ---"));
    Serial.flush();

    volatile uint8_t checksum = 0;

    uint32_t start_time = micros();

    for (uint16_t i = 0; i < ITERATIONS; i++) {
        plaintext_buffer[0] = (uint8_t)i;

        // Full standalone message: fresh state, key set, IV set, encrypt + tag
        gcm.clear();
        gcm.setKey(dummy_key, sizeof(dummy_key));
        gcm.setIV(dummy_nonce, sizeof(dummy_nonce));
        gcm.encrypt(ciphertext_buffer, plaintext_buffer, MAX_PAYLOAD_SIZE);
        gcm.computeTag(ciphertext_buffer + MAX_PAYLOAD_SIZE, TAG_SIZE);

        checksum += ciphertext_buffer[0];
    }

    uint32_t total_time_us = micros() - start_time;

    uint32_t cpu_freq_mhz = ESP.getCpuFreqMHz();
    uint32_t cycles_per_op = (uint32_t)(((uint64_t)total_time_us * cpu_freq_mhz) / ITERATIONS);
    float latency_us = (float)cycles_per_op / (float)cpu_freq_mhz;

    Serial.print(F("Gross Cycles: ")); Serial.println(cycles_per_op);
    Serial.print(F("Latency (us): ")); Serial.println(latency_us, 2);
    Serial.print(F("Checksum: "));     Serial.println(checksum);

    // Yield to FreeRTOS to prevent Task Watchdog Timer (TWDT) panic
    while (1) {
        delay(10);
    }
}