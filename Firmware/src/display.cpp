#include "display.hpp"

#ifdef __IMXRT1062__
uint8_t dataPins[16] = { 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1 };
ST7789 display = ST7789(TFT_RST, TFT_RD, TFT_WR, TFT_CS, TFT_DC, dataPins, TFT_BL);
#elif defined(__SAMD21G18A__) || defined(__SAMD51__)
#ifdef PCB_V1
ST7789 display = ST7789(TFT_RST, TFT_RD, TFT_WR, TFT_CS, TFT_DC, &PORT->Group[PORTA], 6, false, TFT_BL); //Initial PCB design
#else
ST7789 display = ST7789(TFT_RST, TFT_RD, TFT_WR, TFT_CS, TFT_DC, &PORT->Group[PORTA], 4, true, TFT_BL); //New PCB design
#endif
#endif

constexpr char unitMagPrefixes[] = {'K', 'M', 'G', 'T', 'E' };

bool darkMode = false;
bool hybridCpu = false;
uint32_t lastData = 0;
uint8_t brightness = 255;
uint16_t colorLine = 0;
uint16_t colorHeader = 0;
uint16_t colorText = 0;
uint16_t colorBackground = 0;

void initDisplay() {
    display.begin(false);
#ifdef PCB_V1
    display.setRotation(1);
#else
    display.setRotation(3);
#endif

    configureColors();
    drawBackground();
    printLabels();
    drawConnection(false);
}

void configureColors() {
    if (darkMode) {
        colorLine = TFT_LIGHTGREY;
        colorHeader = TFT_NAVY;
        colorText = TFT_LIGHTGREY;
        colorBackground = TFT_BLACK;
    }
    else {
        colorLine = TFT_BLACK;
        colorHeader = TFT_CYAN;
        colorText = TFT_BLACK;
        colorBackground = TFT_WHITE;
    }
}

void drawBackground() {
    display.fillScreen(colorBackground);

    display.fillRect(0, 0, 320, 36, colorHeader);
    display.drawRect(0, 0, 320, 36, colorLine);
    display.drawRect(  0, 0, 80, 240, colorLine);
    display.drawRect( 80, 0, 80, 240, colorLine);
    display.drawRect(160, 0, 80, 240, colorLine);
    display.drawRect(240, 0, 80, 240, colorLine);
    display.fillRect(0, 225, 320, 15, colorBackground);
    display.drawRect(0, 225, 320, 15, colorLine);

    display.setTextSize(2);
    display.setTextColor(colorText, colorHeader);

    display.setCursor(22, 10);
    display.print("CPU");

    display.setCursor(102, 10);
    display.print("RAM");

    display.setCursor(182, 10);
    display.print("NET");

    display.setCursor(262, 10);
    display.print("GPU");

    display.setTextSize(1);
    display.setTextColor(colorText, colorBackground);

    display.setCursor(20, 229);
    display.print("| Interval:    - ms | Draw:   - ms");
}

void printLabels() {
    for (data_type_t type = data_type_t::CPU_LOAD; type != data_type_t::DATA_TYPE_END; ++type) {
        switch (type) {
            case data_type_t::CPU_LOAD: {
                display.setCursor(10, 45);
                display.print("Load:    %");
                break;
            }
            case data_type_t::CPU_CLOCK: {
                display.setCursor(10, 60);
                display.print("Frequency:");
                display.setCursor(52, 70);
                display.print("MHz");
                break;
            }
            case data_type_t::CPU_CLOCK_SEC: {
                display.setCursor(52, 80);
                display.print("MHz");
                break;
            }
            case data_type_t::CPU_TEMP: {
                display.setCursor(10, 95);
                display.print("Temp:    C");
                break;
            }
            case data_type_t::CPU_CORE_VOLTAGE: {
                display.setCursor(10, 110);
                display.print("Voltage:");
                display.setCursor(64, 120);
                display.print("V");
                break;
            }
            case data_type_t::CPU_POWER: {
                display.setCursor(10, 135);
                display.print("Power:");
                display.setCursor(64, 145);
                display.print("W");
                break;
            }
            case data_type_t::RAM_LOAD: {
                display.setCursor(90, 45);
                display.print("Load:    %");
                break;
            }
            case data_type_t::RAM_TOTAL: {
                display.setCursor(90, 60);
                display.print("Total:");
                display.setCursor(138, 70);
                display.print("GB");
                break;
            }
            case data_type_t::RAM_USED: {
                display.setCursor(90, 85);
                display.print("Used:");
                display.setCursor(138, 95);
                display.print("GB");
                break;
            }
            case data_type_t::NET_BANDWIDTH: {
                break;
            }
            case data_type_t::NET_IN: {
                display.setCursor(170, 45);
                display.print("Download:");
                display.setCursor(206, 55);
                display.print("Bit/s");
                break;
            }
            case data_type_t::NET_OUT: {
                display.setCursor(170, 70);
                display.print("Upload:");
                display.setCursor(206, 80);
                display.print("Bit/s");
                break;
            }
            case data_type_t::GPU_LOAD: {
                display.setCursor(250, 45);
                display.print("Load:    %");
                break;
            }
            case data_type_t::GPU_CLOCK: {
                display.setCursor(250, 60);
                display.print("Frequency:");
                display.setCursor(292, 70);
                display.print("MHz");
                break;
            }
            case data_type_t::GPU_TEMP: {
                display.setCursor(250, 85);
                display.print("Temp:    C");
                break;
            }
            case data_type_t::GPU_VRAM_LOAD: {
                display.setCursor(250, 100);
                display.print("VRAM Load:");
                display.setCursor(304, 110);
                display.print("%");
                break;
            }
            case data_type_t::GPU_VRAM_TOTAL: {
                display.setCursor(250, 125);
                display.print("VRAM Total:");
                display.setCursor(298, 135);
                display.print("GB");
                break;
            }
            case data_type_t::GPU_VRAM_USED: {
                display.setCursor(250, 150);
                display.print("VRAM Used:");
                display.setCursor(298, 160);
                display.print("GB");
                break;
            }
            case data_type_t::GPU_POWER: {
                display.setCursor(250, 175);
                display.print("Power:");
                display.setCursor(304, 185);
                display.print("W");
                break;
            }
            case data_type_t::DATA_TYPE_END: break;
        }
    }
}

void drawConnection(bool connected) {
    display.fillCircle(7, 232, 4, connected ? TFT_GREEN : TFT_RED);
    if (!connected) {
        lastData = 0;
    }
}

void printData(uint8_t* data, uint8_t length) {
    if (length % sizeof(data_point_t) == 0) {
        uint32_t prevLastData = lastData;
        lastData = millis();

        uint32_t start = millis();
        auto* points = (data_point_t*)data;
        unsigned int count = length / sizeof(data_point_t);

        for (unsigned int i = 0; i < count; ++i) {
            data_point_t dataPoint = points[i];
            switch (dataPoint.type) {
                case data_type_t::CPU_LOAD: {
                    if (dataPoint.data >= 0 && dataPoint.data <= 100) {
                        display.setCursor(46, 45);
                        display.printf("%3d", (int) dataPoint.data);
                    }
                    break;
                }
                case data_type_t::CPU_CLOCK: {
                    if (dataPoint.data >= 0 && dataPoint.data <= 9999) {
                        display.setCursor(28, 70);
                        display.printf("%4d", dataPoint.data);
                    }
                    break;
                }
                case data_type_t::CPU_CLOCK_SEC: {
                    if (dataPoint.data >= 0 && dataPoint.data <= 9999) {
                        display.setCursor(28, 80);
                        display.printf("%4d", dataPoint.data);
                    }
                    break;
                }
                case data_type_t::CPU_TEMP: {
                    if (dataPoint.data >= 0 && dataPoint.data <= 999) {
                        display.setCursor(46, 95);
                        display.printf("%3d", (int) dataPoint.data);
                    }
                    break;
                }
                case data_type_t::CPU_CORE_VOLTAGE: {
                    if (dataPoint.data >= 0 && dataPoint.data <= 9999) {
                        float value = (float) dataPoint.data / 1000.0F;
                        display.setCursor(40, 120);
                        display.printf("%4.2f", value);
                    }
                    break;
                }
                case data_type_t::CPU_POWER: {
                    if (dataPoint.data >= 0 && dataPoint.data <= 9999) {
                        display.setCursor(40, 145);
                        display.printf("%4d", dataPoint.data);
                    }
                    break;
                }
                case data_type_t::RAM_LOAD: {
                    if (dataPoint.data >= 0 && dataPoint.data <= 100) {
                        display.setCursor(126, 45);
                        display.printf("%3d", (int) dataPoint.data);
                    }
                    break;
                }
                case data_type_t::RAM_TOTAL: {
                    float value = (float) dataPoint.data / 1073741824.0F;
                    if (value >= 0.0F && value <= 9999.9F) {
                        display.setCursor(108, 70);
                        display.printf("%5.1f", value);
                    }
                    break;
                }
                case data_type_t::RAM_USED: {
                    float value = (float) dataPoint.data / 1073741824.0F;
                    if (value >= 0.0F && value <= 999.9F) {
                        display.setCursor(108, 95);
                        display.printf("%5.1f", value);
                    }
                    break;
                }
                case data_type_t::NET_IN: {
                    if (dataPoint.data >= 0 && dataPoint.data <= 999999999) {
                        display.setCursor(170, 55);
                        float netIn = (float) dataPoint.data / 100.0F;
                        printScientific(netIn, 1000.0F, 5, 1, 2);
                    }
                    break;
                }
                case data_type_t::NET_OUT: {
                    if (dataPoint.data >= 0 && dataPoint.data <= 999999999) {
                        display.setCursor(170, 80);
                        float netOut = (float) dataPoint.data / 100.0F;
                        printScientific(netOut, 1000.0F, 5, 1, 2);
                    }
                    break;
                }
                case data_type_t::GPU_LOAD: {
                    if (dataPoint.data >= 0 && dataPoint.data <= 100) {
                        display.setCursor(286, 45);
                        display.printf("%3d", (int) dataPoint.data);
                    }
                    break;
                }
                case data_type_t::GPU_CLOCK: {
                    if (dataPoint.data >= 0 && dataPoint.data <= 9999) {
                        display.setCursor(268, 70);
                        display.printf("%4d", dataPoint.data);
                    }
                    break;
                }
                case data_type_t::GPU_TEMP: {
                    if (dataPoint.data >= 0 && dataPoint.data <= 999) {
                        display.setCursor(286, 85);
                        display.printf("%3d", (int) dataPoint.data);
                    }
                    break;
                }
                case data_type_t::GPU_VRAM_LOAD: {
                    if (dataPoint.data >= 0 && dataPoint.data <= 100) {
                        display.setCursor(286, 110);
                        display.printf("%3d", (int) dataPoint.data);
                    }
                    break;
                }
                case data_type_t::GPU_VRAM_TOTAL: {
                    float value = (float) dataPoint.data / 1073741824.0F;
                    if (value >= 0.0F && value <= 999.9F) {
                        display.setCursor(268, 135);
                        display.printf("%5.1f", value);
                    }
                    break;
                }
                case data_type_t::GPU_VRAM_USED: {
                    float value = (float) dataPoint.data / 1073741824.0F;
                    if (value >= 0.0F && value <= 999.9F) {
                        display.setCursor(268, 160);
                        display.printf("%5.1f", value);
                    }
                    break;
                }
                case data_type_t::GPU_POWER: {
                    if (dataPoint.data >= 0 && dataPoint.data <= 999999) {
                        display.setCursor(280, 185);
                        display.printf("%4d", dataPoint.data / 1000);
                    }
                    break;
                }
                default: break;
            }
        }

        uint32_t end = millis();
        display.setCursor(92, 229);
        printClampedOrDefault(lastData - prevLastData, 9999, 4, '-', prevLastData != 0);
        display.setCursor(188, 229);
        printClampedOrDefault(end - start, 999, 3, '-', true);
    }
}

void printScientific(float value, float divider, int len, int decimals, int steps) {
    float fracFactor = 10.0F * (float) decimals;
    for (int i = 0; i < steps; ++i) {
        float dispValue = round(value * fracFactor) / fracFactor;
        if (dispValue < divider) {
            display.printf("%*.*f%c", len, decimals, dispValue, unitMagPrefixes[i]);
            break;
        }
        value /= divider;
    }
}

void printClampedOrDefault(uint32_t value, uint32_t max, int width, char placeholder, bool condition)
{
    if (condition)
    {
        display.printf("%*u", width, min(value, max));
    }
    else
    {
        display.printf("%*c", width, placeholder);
    }
}

void enableDisplay() {
    display.setDisplayOn(true);
    display.setBrightness(brightness);
}

void disableDisplay() {
    display.setDisplayOn(false);
    display.setBrightness(0);
}

void switchDarkMode(bool on) {
    if (darkMode != on) {
        darkMode = on;

        configureColors();
        drawBackground();
        printLabels();
        drawConnection(isConnected());
    }
}

void switchHybridCpu(bool on) {
    if (hybridCpu != on) {
        hybridCpu = on;

        display.setTextColor(hybridCpu ? colorText : colorBackground, colorBackground);

        display.setCursor(10, 70);
        display.print("P:");
        display.setCursor(10, 80);
        display.print("E:");

        display.setTextColor(colorText, colorBackground);
    }
}

void setBrightness(uint8_t percent) {
    auto factor = (float)percent;
    brightness = (uint32_t)(factor * 255.0);
}
