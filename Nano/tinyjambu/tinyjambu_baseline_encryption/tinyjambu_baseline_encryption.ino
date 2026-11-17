#include <Arduino.h>

#define MAX_PAYLOAD_SIZE 512 // 16, 32, 64, 128, 256, 512: rebuild per size
#define TAG_SIZE 8
#define ITERATIONS 10000

uint8_t plaintext_buffer[MAX_PAYLOAD_SIZE];
uint8_t ciphertext_buffer[MAX_PAYLOAD_SIZE + TAG_SIZE];

void setup() {
    Serial.begin(115200);
    delay(2000);
    for (uint16_t i = 0; i < MAX_PAYLOAD_SIZE; i++) plaintext_buffer[i] = (uint8_t)(i % 256);
}

void loop() {
    Serial.print(F("--- NANO TINYJAMBU ENCRYPT BASELINE | Payload: "));
    Serial.print(MAX_PAYLOAD_SIZE);
    Serial.println(F(" Bytes ---"));
    Serial.flush();

    volatile uint8_t checksum = 0;
    unsigned long long clen = 0;
    uint32_t start_time = micros();

    for (uint16_t i = 0; i < ITERATIONS; i++) {
        plaintext_buffer[0] = (uint8_t)i;
        asm volatile("" : "+m"(plaintext_buffer), "+m"(ciphertext_buffer), "+m"(clen));
        checksum += ciphertext_buffer[0];
    }

    uint32_t total_time_us = micros() - start_time;
    uint32_t cycles_per_op = (uint32_t)(((uint64_t)total_time_us * (F_CPU / 1000000UL)) / ITERATIONS);
    float latency_us = (float)cycles_per_op / (F_CPU / 1000000.0);

    Serial.print(F("Gross Cycles: ")); Serial.println(cycles_per_op);
    Serial.print(F("Latency (us): ")); Serial.println(latency_us, 2);
    Serial.print(F("Checksum: "));     Serial.println(checksum);
    while (1);
}