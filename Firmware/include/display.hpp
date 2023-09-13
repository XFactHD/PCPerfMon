#ifndef DISPLAY_HPP
#define DISPLAY_HPP

#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_I2CDevice.h>
#include "ST7789_16bit.hpp"

#include "icons.h"
#include "data.hpp"
#include "cmd.hpp"

// Undefine Arduino round macro to use cmath round function instead
#undef round

#define PCB_V1

#ifdef __IMXRT1062__
#define TFT_RST 0
#define TFT_RD  17
#define TFT_WR  18
#define TFT_CS  20
#define TFT_DC  19
#define TFT_BL  255
#elif defined(PCB_V1)
#define TFT_RST 31 //PB23
#define TFT_RD  255
#define TFT_WR  18 //PA05
#define TFT_CS  15 //PB08
#define TFT_DC  16 //PB09
#define TFT_BL  30 //PB22
#else
#define TFT_RST 15 //PB08
#define TFT_RD  255
#define TFT_WR  11 //PA20
#define TFT_CS  20 //PA22
#define TFT_DC  12 //PA21
#define TFT_BL  15 //PB08
#endif

void initDisplay();

void configureColors();
void drawBackground();
void printLabels();
void drawConnection(bool connected);

void printData(uint8_t* data, uint8_t length);
void printScientific(float value, float divider, int len, int decimals, int steps);
void printClampedOrDefault(uint32_t value, uint32_t max, int width, char placeholder, bool condition);

void enableDisplay();
void disableDisplay();
void switchDarkMode(bool on);
void switchHybridCpu(bool on);
void setBrightness(uint8_t percent);

#endif //DISPLAY_HPP
