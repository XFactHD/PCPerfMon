#ifndef CMD_HPP
#define CMD_HPP

#include <Arduino.h>

#include "display.hpp"

#define CMD_STARTUP 0x10
#define CMD_SHUTDOWN 0x11
#define CMD_DATA 0x20
#define CMD_CFG 0x30
#define CMD_ACK 0x99

#define CFG_DARK_MODE 0x01
#define CFG_BRIGHTNESS 0x02

void handleConfig(const uint8_t* data, uint8_t length);

#endif //CMD_HPP
