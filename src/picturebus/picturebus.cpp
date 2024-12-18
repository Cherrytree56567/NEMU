#include "picturebus.h"

picturebus::picturebus(std::shared_ptr<Mapper> map) : palette(0x20), vram(0x800), mapper(map) {
    if (!mapper) {
        std::cout << "[NEMU] Error: Mapper argument is nullptr.\n";
    }
    mapper = map;
    updateMirroring();
}

uint8_t picturebus::read(uint16_t addr) {
    if (addr < 0x2000) {
        return mapper->readCHR(addr);
    } else if (addr <= 0x3eff) {
        const auto index = addr & 0x3ff;
        auto normalizedAddr = addr;
        if (addr >= 0x3000) {
            normalizedAddr -= 0x1000;
        }

        if (NameTable0 >= vram.size()) {
            return mapper->readCHR(normalizedAddr);
        } else if (normalizedAddr < 0x2400) {
            return vram[NameTable0 + index];
        } else if (normalizedAddr < 0x2800) {
            return vram[NameTable1 + index];
        } else if (normalizedAddr < 0x2c00) {
            return vram[NameTable2 + index];
        } else {
            return vram[NameTable3 + index];
        }
    } else if (addr <= 0x3fff) {
        auto paletteAddr = addr & 0x1f;
        return readPalette(paletteAddr);
    }
    return 0;
}

uint8_t picturebus::readPalette(uint8_t paletteAddr) {
    if (paletteAddr >= 0x10 && paletteAddr % 4 == 0) {
        paletteAddr = paletteAddr & 0xf;
    }
    return palette[paletteAddr];
}

void picturebus::write(uint16_t addr, uint8_t value) {
    if (addr < 0x2000) {
        mapper->writeCHR(addr, value);
    } else if (addr <= 0x3eff) {
        const auto index = addr & 0x3ff;
        auto normalizedAddr = addr;
        if (addr >= 0x3000) {
            normalizedAddr -= 0x1000;
        }
        
        if (NameTable0 >= vram.size()) {
            mapper->writeCHR(normalizedAddr, value);
        } else if (normalizedAddr < 0x2400) {
            vram[NameTable0 + index] = value;
        } else if (normalizedAddr < 0x2800) {
            vram[NameTable1 + index] = value;
        } else if (normalizedAddr < 0x2c00) {
            vram[NameTable2 + index] = value;
        } else {
            vram[NameTable3 + index] = value;
        }
    } else if (addr <= 0x3fff) {
        auto palette_ = addr & 0x1f;
        if (palette_ >= 0x10 && addr % 4 == 0) {
            palette_ = palette_ & 0xf;
        }
        palette[palette_] = value;
    }
}

void picturebus::updateMirroring() {
    switch (mapper->getNameTableMirroring()) {
    case Horizontal:
        NameTable0 = NameTable1 = 0;
        NameTable2 = NameTable3 = 0x400;
        break;
    case Vertical:
        NameTable0 = NameTable2 = 0;
        NameTable1 = NameTable3 = 0x400;
        break;
    case OneScreenLower:
        NameTable0 = NameTable1 = NameTable2 = NameTable3 = 0;
        break;
    case OneScreenHigher:
        NameTable0 = NameTable1 = NameTable2 = NameTable3 = 0x400;
        break;
    case FourScreen:
        NameTable0 = vram.size();
        break;
    default:
        NameTable0 = 0;
        NameTable1 = 0;
        NameTable2 = 0;
        NameTable3 = 0;
        std::cout << "[NEMU] Error: Unsupported Name Table mirroring : " << mapper->getNameTableMirroring() << std::endl;
    }
}

void picturebus::scanlineIRQ(){
    mapper->scanlineIRQ();
}