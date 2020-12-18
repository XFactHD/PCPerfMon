#include "data.hpp"

void parseData(uint8_t* data, uint8_t length) {
    if (length % sizeof(data_point_t) == 0) {
        ST7789* display = getDisplay();

        uint32_t start = millis();
        auto* points = (data_point_t*)data;
        unsigned int count = length / sizeof(data_point_t);

        for (unsigned int i = 0; i < count; ++i) {
            data_point_t dataPoint = points[i];
            switch (dataPoint.type) {
                case data_type_t::CPU_LOAD: {
                    display->setCursor(10, 45);
                    display->printf("Load: %3d%%", (int)dataPoint.data);
                    break;
                }
                case data_type_t::CPU_CLOCK: {
                    display->setCursor(10, 60);
                    display->print("Clock:");
                    display->setCursor(10, 70);
                    display->printf("%4dMHz", dataPoint.data);
                    break;
                }
                case data_type_t::CPU_TEMP: {
                    display->setCursor(10, 85);
                    display->printf("Temp: %3dC", (int)dataPoint.data);
                    break;
                }
                case data_type_t::CPU_POWER: {
                    display->setCursor(10, 100);
                    display->print("Power:");
                    display->setCursor(10, 110);
                    display->printf("%4dW", dataPoint.data);
                    break;
                }
                case data_type_t::RAM_LOAD: {
                    display->setCursor(90, 45);
                    display->printf("Load: %3d%%", (int)dataPoint.data);
                    break;
                }
                case data_type_t::RAM_TOTAL: {
                    display->setCursor(90, 60);
                    display->print("Total:");
                    display->setCursor(90, 70);
                    display->printf("%4.1fGB", dataPoint.data / 1073741824.0);
                    break;
                }
                case data_type_t::RAM_USED: {
                    display->setCursor(90, 85);
                    display->print("Used:");
                    display->setCursor(90, 95);
                    display->printf("%4.1fGB", dataPoint.data / 1073741824.0);
                    break;
                }
                case data_type_t::NET_BANDWIDTH: {
                    break;
                }
                case data_type_t::NET_IN: {
                    display->setCursor(170, 45);
                    display->print("Net In:");
                    display->setCursor(170, 55);
                    double netIn = dataPoint.data / 100.0;
                    if (netIn >= 1000.0) {
                        netIn /= 1000.0;
                        if (netIn >= 1000.0) {
                            netIn /= 1000.0;
                            display->printf("%5.1fGBit/s", netIn);
                        }
                        else {
                            display->printf("%5.1fMBit/s", netIn);
                        }
                    }
                    else {
                        display->printf("%5.1fKBit/s", netIn);
                    }
                    break;
                }
                case data_type_t::NET_OUT: {
                    display->setCursor(170, 70);
                    display->print("Net Out:");
                    display->setCursor(170, 80);
                    double netOut = dataPoint.data / 100.0;
                    if (netOut >= 1000.0) {
                        netOut /= 1000.0;
                        if (netOut >= 1000.0) {
                            netOut /= 1000.0;
                            display->printf("%5.1fGBit/s", netOut);
                        }
                        else {
                            display->printf("%5.1fMBit/s", netOut);
                        }
                    }
                    else {
                        display->printf("%5.1fKBit/s", netOut);
                    }
                    break;
                }
                case data_type_t::GPU_LOAD: {
                    display->setCursor(250, 45);
                    display->printf("Load: %3d%%", (int)dataPoint.data);
                    break;
                }
                case data_type_t::GPU_CLOCK: {
                    display->setCursor(250, 60);
                    display->print("Clock:");
                    display->setCursor(250, 70);
                    display->printf("%4dMHz", dataPoint.data);
                    break;
                }
                case data_type_t::GPU_TEMP: {
                    display->setCursor(250, 85);
                    display->printf("Temp: %3dC", (int)dataPoint.data);
                    break;
                }
                case data_type_t::GPU_VRAM_LOAD: {
                    display->setCursor(250, 100);
                    display->print("VRAM Load: ");
                    display->setCursor(250, 110);
                    display->printf("%3d%%", (int)dataPoint.data);
                    break;
                }
                case data_type_t::GPU_VRAM_TOTAL: {
                    display->setCursor(250, 125);
                    display->print("VRAM Total:");
                    display->setCursor(250, 135);
                    display->printf("%4.1fGB", dataPoint.data / 1048576.0);
                    break;
                }
                case data_type_t::GPU_VRAM_USED: {
                    display->setCursor(250, 150);
                    display->print("VRAM Used:");
                    display->setCursor(250, 160);
                    display->printf("%4.1fGB", dataPoint.data / 1048576.0);
                    break;
                }
                case data_type_t::GPU_POWER: {
                    display->setCursor(250, 175);
                    display->print("Power:");
                    display->setCursor(250, 185);
                    display->printf("%4dW", dataPoint.data);
                    break;
                }
            }
        }

        uint32_t end = millis();
        display->setCursor(2, 2);
        display->print(end - start);
        display->print(" ms");
    }
}