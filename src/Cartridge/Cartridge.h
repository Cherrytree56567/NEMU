#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <iostream>

/*
* From https://github.com/amhndu/SimpleNES/blob/master/include/Cartridge.h
*/
enum NameTableMirroring {
    Horizontal  = 0,
    Vertical    = 1,
    FourScreen  = 8,
    OneScreenLower,
    OneScreenHigher,
};

class Cartridge {
public:
    Cartridge(std::string path);
    std::vector<uint8_t> getROM();
    std::vector<uint8_t> getVROM();
    uint8_t getMapper();
    uint8_t getNameTableMirroring();
    bool hasExtendedRAM();
private:
    std::vector<uint8_t> PRG;
    std::vector<uint8_t> CHR;
    uint8_t nameTableMirroring;
    uint8_t mapperNumber;
    bool extendedRAM;
    bool chrRAM;
};