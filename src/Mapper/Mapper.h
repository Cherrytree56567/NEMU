#pragma once
#include "../cartridge/cartridge.h"
#include <functional>
#include <memory>

enum MapperType {
    NROM  = 0,
    SxROM = 1,
    UxROM = 2,
    CNROM = 3,
    MMC3  = 4,
    AxROM = 7,
    ColorDreams = 11,
    GxROM = 66,
};

/*
* Modified version from https://github.com/amhndu/SimpleNES/blob/master/src/Mapper.cpp
*/

class Mapper {
public: 
    Mapper(Cartridge cart, MapperType mtype) : cartridge(cart), type(mtype) {}
    virtual void writePRG (uint16_t addr, uint8_t value) = 0;
    virtual uint8_t readPRG (uint16_t addr) = 0;

    virtual uint8_t readCHR (uint16_t addr) = 0;
    virtual void writeCHR (uint16_t addr, uint8_t value) = 0;

    virtual NameTableMirroring getNameTableMirroring();

    bool inline hasExtendedRAM() {
        return cartridge.hasExtendedRAM();
    }

    virtual void scanlineIRQ(){}

    static std::shared_ptr<Mapper> createMapper(MapperType mapper, Cartridge& cart);

protected:
    Cartridge cartridge;
    MapperType type;
};