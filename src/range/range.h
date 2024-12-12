#pragma once
#include <cstdint>

class range {
public:
    range(uint8_t start, uint8_t end, uint8_t size);
    uint8_t start;
    uint8_t end;
    uint8_t size;
};