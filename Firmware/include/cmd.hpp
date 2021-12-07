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
#define CFG_TIMEOUT 0x03
#define CFG_HYBRID_CPU 0x04

void handleConfig(const uint8_t* data, uint8_t length);

void commandReceived();
void checkConnection();
bool isConnected();

#endif //CMD_HPP
