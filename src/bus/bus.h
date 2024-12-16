#pragma once
#include <vector>
#include <iostream>
#include <unordered_map>
#include <functional>
#include "../range/range.h"
#include "../Mapper/Mapper.h"

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
#define cartridge range(0x4020, 0xFFFF, 0xBFE0) // Cartridge space: PRG ROM, PRG RAM, and mapper registers

/*
* IO Registers and IO Registers Hasher found in https://github.com/amhndu/SimpleNES/blob/master/include/MainBus.h
*/

enum IORegisters {
    PPUCTRL = 0x2000,
    PPUMASK,
    PPUSTATUS,
    OAMADDR,
    OAMDATA,
    PPUSCROL,
    PPUADDR,
    PPUDATA,
    OAMDMA = 0x4014,
    JOY1 = 0x4016,
    JOY2 = 0x4017,
};

struct IORegistersHasher {
    std::size_t operator()(IORegisters const & reg) const noexcept {
        return std::hash<std::uint32_t>{}(reg);
    }
};

class bus {
public:
    bus(std::shared_ptr<Mapper> map);

    uint8_t read(uint16_t addr);
    void write(uint16_t addr, uint8_t val);
    bool setWriteCallback(IORegisters reg, std::function<void(uint8_t)> callback);
    bool setReadCallback(IORegisters reg, std::function<uint8_t(void)> callback);
    const uint8_t* getPagePtr(uint8_t page);

private:
    std::shared_ptr<Mapper> mapper;
    std::vector<uint8_t> ram;
    std::vector<uint8_t> extendedRAM;

    std::unordered_map<IORegisters, std::function<void(uint8_t)>, IORegistersHasher> writeCallbacks;
    std::unordered_map<IORegisters, std::function<uint8_t(void)>, IORegistersHasher> readCallbacks;
};