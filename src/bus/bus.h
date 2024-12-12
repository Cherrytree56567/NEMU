#pragma once
#include <vector>
#include <iostream>
#include "../range/range.h"

/*
* Memory Map found on https://yizhang82.dev/nes-emu-cpu
*/
#define Ram range(0x0000, 0x07FF, 0x0800) // 2KB internal RAM
#define Ram1 range(0x800, 0x0FFF, 0x0800) // Mirrors of $0000-$07FF
#define Ram2 range(0x1000, 0x17FF, 0x0800) // Mirrors of $0000-$07FF
#define Ram3 range(0x1800, 0x1FFF, 0x0800) // Mirrors of $0000-$07FF
#define PPU range(0x2000, 0x2007, 0x0008) // NES PPU registers
#define PPU1 range(0x2008, 0x3FFF, 0x0008) // Mirrors of $2000-2007 (repeats every 8 bytes)
#define APU range(0x4000, 0x4017, 0x0018) // NES APU and I/O registers
#define APUDISB range(0x4000, 0x4017, 0x0018) // APU and I/O functionality that is normally disabled. For CPU Test Mode.
#define Cartridge range(0x4020, 0xFFFF, 0xBFE0) // Cartridge space: PRG ROM, PRG RAM, and mapper registers

class bus {
public:
    bus();

    uint8_t read(uint16_t addr);
    void write(uint16_t addr, uint8_t val);

private:
    std::vector<uint8_t> ram;
};