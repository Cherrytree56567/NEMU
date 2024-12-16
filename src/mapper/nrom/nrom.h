#pragma once
#include "../mapper.h"

class MapperNROM : public Mapper {
public:
    MapperNROM(Cartridge cart);
    void writePRG (uint16_t addr, uint8_t value);
    virtual uint8_t readPRG(uint16_t addr) override;

    uint8_t readCHR (uint16_t addr);
    void writeCHR (uint16_t addr, uint8_t value);
private:
    bool oneBank;
    bool usesCharacterRAM;

    std::vector<uint8_t> characterRAM;
};