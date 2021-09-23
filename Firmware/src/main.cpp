#include <Arduino.h>

#if defined(__SAMD21G18A__) || defined(__SAMD51__)
#include <avr/dtostrf.h> //Necessary to format floating point numbers with printf()
#endif

#include "display.hpp"
#include "cmd.hpp"

void setup() {
#if defined(__SAMD21G18A__) || defined(__SAMD51__)
    //Force linking
    char buf1[10];
    dtostrf(0.0, 6, 1, buf1);
#endif

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
                printData(data, length);
                break;
            }
            case CMD_CFG: {
                handleConfig(data, length);
                break;
            }
            default: break;
        }
        Serial.write(CMD_ACK);
    }
}