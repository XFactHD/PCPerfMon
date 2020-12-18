#ifndef DATA_HPP
#define DATA_HPP

#include <Arduino.h>

#include "display.hpp"

enum class data_type_t {
    CPU_LOAD,
    CPU_CLOCK,
    CPU_TEMP,
    CPU_POWER,
    RAM_LOAD,
    RAM_TOTAL,
    RAM_USED,
    NET_BANDWIDTH,
    NET_IN,
    NET_OUT,
    GPU_LOAD,
    GPU_CLOCK,
    GPU_TEMP,
    GPU_VRAM_LOAD,
    GPU_VRAM_TOTAL,
    GPU_VRAM_USED,
    GPU_POWER
};

typedef struct __attribute__((packed)) {
    data_type_t type;
    uint64_t data;
} data_point_t;

void parseData(uint8_t* data, uint8_t length);

#endif //DATA_HPP
