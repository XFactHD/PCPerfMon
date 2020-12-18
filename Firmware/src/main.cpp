#include <Arduino.h>

#include "display.hpp"
#include "data.hpp"

#define CMD_STARTUP 0x10
#define CMD_SHUTDOWN 0x11
#define CMD_DATA 0x20
#define CMD_CFG 0x30
#define CMD_ACK 0x99

void setup() {
    initDisplay();

    Serial.begin(1000000);
}

void loop() {
    if (Serial.available() >= 2) {
        uint8_t cmd = Serial.read();
        uint8_t length = Serial.read();
        uint8_t data[length];

        if (length > 0) {
            uint32_t start = millis();
            while (Serial.available() < length) {
                if (millis() - start > 100) {
                    return;
                }
                delay(10);
            }
            Serial.readBytes((char*)data, length);
        }

        switch (cmd) {
            case CMD_STARTUP: {
                enableDisplay();
                break;
            }
            case CMD_SHUTDOWN: {
                disableDisplay();
                break;
            }
            case CMD_DATA: {
                parseData(data, length);
                break;
            }
            case CMD_CFG: {
                break;
            }
            default: break;
        }
        Serial.write(CMD_ACK);
    }
}