#include "nrom.h"

MapperNROM::MapperNROM(Cartridge cart) : Mapper(cart, MapperType::NROM) {
    if (cart.getROM().size() == 0x4000) {
        oneBank = true;
    } else {
        oneBank = false;
    }

    if (cart.getVROM().size() == 0) {
        usesCharacterRAM = true;
        characterRAM.resize(0x2000);
    } else {
        usesCharacterRAM = false;
    }
}

uint8_t MapperNROM::readPRG(uint16_t addr) {
    if (!oneBank)
        return cartridge.getROM()[addr - 0x8000];
    else //mirrored
        return cartridge.getROM()[(addr - 0x8000) & 0x3fff];
}

void MapperNROM::writePRG(uint16_t addr, uint8_t value) {
    std::cout << "[NEMU] Error: ROM memory write attempt at " << +addr << " to set " << +value << std::endl;
}

uint8_t MapperNROM::readCHR(uint16_t addr) {
    if (usesCharacterRAM) {
        return characterRAM[addr];
    } else {
        return cartridge.getVROM()[addr];
    }
}

void MapperNROM::writeCHR(uint16_t addr, uint8_t value) {
    if (usesCharacterRAM) {
        characterRAM[addr] = value;
    } else {
        std::cout << "[NEMU] Error: Read-only CHR memory write attempt at " << std::hex << addr << std::endl;
    }
}