#include "cartridge.h"

Cartridge::Cartridge(std::string path) {
    std::ifstream rom(path, std::ios_base::binary | std::ios_base::in);
    if (!rom) {
        std::cout << "[NEMU] Error: Couldn't open " << path << " rom file.\n";
    }

    std::vector<uint8_t> header;

    /*
    * NES Header
    */
    header.resize(0x10);
    if (!rom.read(reinterpret_cast<char*>(&header[0]), 0x10)) {
        std::cout << "[NEMU] Error: Couldn't Read iNES header.\n";
    }

    if (std::string{&header[0], &header[4]} != "NES\x1A") {
        std::cout << "[NEMU] Error: Not a valid iNES image. Magic number: "<< std::hex << header[0] << " " << header[1] << " " << header[2] << " " << int(header[3]) << std::endl;
    }

    uint8_t banks = header[4];
    if (!banks) {
        std::cout << "[NEMU] Error: ROM has no PRG-ROM banks. \n";
    }

    uint8_t vbanks = header[5];

    if (header[6] & 0x8) {
        nameTableMirroring = NameTableMirroring::FourScreen;
    } else {
        nameTableMirroring = header[6] & 0x1;
    }

    mapperNumber = ((header[6] >> 4) & 0xf) | (header[7] & 0xf0);

    extendedRAM = header[6] & 0x2;

    if (header[6] & 0x4) {
        std::cout << "[NEMU] Error: Trainer is not supported. \n";
    }

    if ((header[0xA] & 0x3) == 0x2 || (header[0xA] & 0x1)) {
        std::cout << "[NEMU] Error: PAL ROM's are not supported. \n";
    }
    
    PRG.resize(0x4000 * banks);
    if (!rom.read(reinterpret_cast<char*>(&PRG[0]), 0x4000 * banks)) {
        std::cout << "[NEMU] Error: Failed reading PRG-ROM.\n";
    }

    if (vbanks) {
        CHR.resize(0x2000 * vbanks);
        if (!rom.read(reinterpret_cast<char*>(&CHR[0]), 0x2000 * vbanks)) {
            std::cout << "[NEMU] Error: Failed reading CHR-ROM.\n";
        }
    }
}

std::vector<uint8_t> Cartridge::getROM() {
    return PRG;
}

std::vector<uint8_t> Cartridge::getVROM() {
    return CHR;
}

uint8_t Cartridge::getMapper() {
    return mapperNumber;
}

uint8_t Cartridge::getNameTableMirroring() {
    return nameTableMirroring;
}

bool Cartridge::hasExtendedRAM() {
    return extendedRAM;
}