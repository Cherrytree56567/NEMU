#include "bus.h"

bus::bus() : ram(Ram.size) {

}

uint8_t bus::read(uint16_t addr) {
    if (addr < 0x2000) { // RAM
        return ram[addr & Ram.end];
    } else if (addr < 0x4020) { // PPU and IO 
        if (addr < 0x4000) { // PPU and Mirrored PPU
            std::cout << "[NEMU] WARNING: PPU is not implemented yet : " << addr << ".\n";
            return 0;
        } else if (addr < 0x4018 && addr >= 0x4014) { // IO
            std::cout << "[NEMU] WARNING: IO is not implemented yet : " << addr << ".\n";
            return 0;
        } else {
            std::cout << "[NEMU] WARNING: UNKNOWN ACCESS to PPU and IO : " << addr << ".\n";
            return 0;
        }
    } else if (addr < 0x6000) { // Expansion ROM
        std::cout << "[NEMU] WARNING: Expansion ROM is not implemented yet : " << addr << ".\n";
    } else if (addr < 0x8000) { // Extended RAM
        std::cout << "[NEMU] WARNING: Extended RAM is not implemented yet : " << addr << ".\n";
    } else { // PRG
        std::cout << "[NEMU] WARNING: PRG is not implemented yet : " << addr << ".\n";
    }
}

void bus::write(uint16_t addr, uint8_t val) {
    if (addr < 0x2000) {
        ram[addr & Ram.end] = val;
    } else if ((addr > (PPU.start - 1)) || (addr < (PPU.end + 1))) {
        std::cout << "[NEMU] WARNING: PPU is not implemented yet : " << addr << ".\n";
    } else if ((addr > (PPU1.start - 1)) || (addr < (PPU1.end + 1))) {
        std::cout << "[NEMU] WARNING: PPU Mirror is not implemented yet : " << addr << ".\n";
    } else if ((addr > (APU.start - 1)) || (addr < (APU.end + 1))) {
        std::cout << "[NEMU] WARNING: APU is not implemented yet : " << addr << ".\n";
    } else if ((addr > (APUDISB.start - 1)) || (addr < (APUDISB.end + 1))) {
        std::cout << "[NEMU] WARNING: APU Disabled is not implemented yet : " << addr << ".\n";
    } else if ((addr > (Cartridge.start - 1)) || (addr < (Cartridge.end + 1))) {
        std::cout << "[NEMU] WARNING: Cartridge Disabled is not implemented yet : " << addr << ".\n";
    }
}