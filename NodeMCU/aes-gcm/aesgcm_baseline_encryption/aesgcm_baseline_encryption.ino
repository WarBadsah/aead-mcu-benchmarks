#include <Arduino.h>

// =========================================================================
#define MAX_PAYLOAD_SIZE 512 // 16, 32, 64, 128, 256, 512: rebuild per size
// =========================================================================

#define TAG_SIZE 16
#define ITERATIONS 10000

uint8_t plaintext_buffer[MAX_PAYLOAD_SIZE];
uint8_t ciphertext_buffer[MAX_PAYLOAD_SIZE + TAG_SIZE];

void setup() {
    Serial.begin(115200);
    delay(2000); // Give the serial monitor time to connect

    for (uint16_t i = 0; i < MAX_PAYLOAD_SIZE; i++) {
        plaintext_buffer[i] = (uint8_t)(i % 256);
    }
}

void loop() {
    Serial.print(F("--- NODEMCU AESGCM ENCRYPT BASELINE | Payload: "));
    Serial.print(MAX_PAYLOAD_SIZE);
    Serial.println(F(" Bytes ---"));
    Serial.flush();

    volatile uint8_t checksum = 0;
    unsigned long long clen = 0;

    uint32_t start_time = micros();

    for (uint16_t i = 0; i < ITERATIONS; i++) {
        plaintext_buffer[0] = (uint8_t)i;

        // Stands in for the cipher call: same memory the real call touches
        asm volatile("" : "+m"(plaintext_buffer), "+m"(ciphertext_buffer), "+m"(clen));

        checksum += ciphertext_buffer[0];
        yield(); // feeds the ESP8266 watchdog; identical in the real sketch
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