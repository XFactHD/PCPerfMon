#include "cmd.hpp"

uint16_t timeout = 2000;
uint64_t lastCommand = 0;
bool connected = false;

void handleConfig(const uint8_t* data, uint8_t length) {
    uint8_t cfg = data[0];
    switch (cfg) {
        case CFG_DARK_MODE: {
            if (length == 2) {
                switchDarkMode(data[1]);
            }
            break;
        }
        case CFG_BRIGHTNESS: {
            if (length == 2) {
                setBrightness(data[1]);
            }
            break;
        }
        case CFG_TIMEOUT: {
            if (length == 3) {
                timeout = (data[1] << 8) | data[2];
            }
            break;
        }
        case CFG_HYBRID_CPU: {
            if (length == 2) {
                switchHybridCpu(data[1]);
            }
            break;
        }
        default: break;
    }
}

void commandReceived() {
    lastCommand = millis();
}

void checkConnection() {
    uint64_t diff = millis() - lastCommand;
    if (diff > timeout && connected) {
        connected = false;
        drawConnection(false);
    }
    else if (diff <= timeout && !connected) {
        connected = true;
        drawConnection(true);
    }
}

bool isConnected() {
    return connected;
}
