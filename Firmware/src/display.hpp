#ifndef DISPLAY_HPP
#define DISPLAY_HPP

#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_I2CDevice.h>
#include "ST7789_16bit.hpp"

#include "icons.h"
#include "data.hpp"

#define TFT_RST 0
#define TFT_RD  17
#define TFT_WR  18
#define TFT_CS  20
#define TFT_DC  19
#define TFT_BL  255 //TODO: set properly on production hardware

void initDisplay();

void configureColors();
void drawBackground();
void printLabels();

void printData(uint8_t* data, uint8_t length);
void printScientific(double value, double divider, int len, int decimals, const char** units, int steps);

void enableDisplay();
void disableDisplay();
void switchDarkMode(bool on);

#endif //DISPLAY_HPP
