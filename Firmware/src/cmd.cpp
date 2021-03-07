#include "cmd.hpp"

void handleConfig(const uint8_t* data, uint8_t length) {
    uint8_t cfg = data[0];
    switch (cfg) {
        case CFG_DARK_MODE: {
            if (length == 2) {
                switchDarkMode(data[1]);
            }
            break;
        }
    }
}