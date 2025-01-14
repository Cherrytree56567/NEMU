#include "bus.h"

bus::bus(std::shared_ptr<Mapper> map, std::shared_ptr<PPU> ppunit) : ram(0x800, 0), mapper(map), ppu(ppunit) {
    if (!mapper) {
        std::cout << "[NEMU] Error: Mapper pointer is null.\n";
    }

    if (!ppunit) {
        std::cout << "[NEMU] Error: PPU pointer is null.\n";
    }

    if (mapper->hasExtendedRAM()) {
        extendedRAM.resize(0x2000);
    }
}

uint8_t bus::read(uint16_t addr) {
    if (addr < 0x2000) { // RAM
        return ram[addr & 0x7ff];
    } else if (addr < 0x4020) { // PPU and IO 
        if (addr < 0x4000) { // PPU and Mirrored PPU
            auto it = readCallbacks.find(static_cast<IORegisters>(addr & 0x2007));
                if (it != readCallbacks.end()) {
                    return (it -> second)();
                } else {
                    std::cout << "[NEMU] Error: No read callback registered for I/O register at: " << std::hex << +addr << std::endl;
                }
        } else if (addr < 0x4018 && addr >= 0x4014) { // IO
            auto it = readCallbacks.find(static_cast<IORegisters>(addr));
            if (it != readCallbacks.end()) {
                return (it -> second)();
            } else {
                std::cout << "[NEMU] Error: No read callback registered for I/O register at: " << std::hex << +addr << std::endl;
            }
        } else {
            std::cout << "[NEMU] WARNING: UNKNOWN ACCESS to PPU and IO : " << addr << ".\n";
            return 0;
        }
    } else if (addr < 0x6000) { // Expansion ROM
        std::cout << "[NEMU] WARNING: Expansion ROM is not implemented yet : " << addr << ".\n";
    } else if (addr < 0x8000) { // Extended RAM
        if (mapper->hasExtendedRAM()) {
            return extendedRAM[addr - 0x6000];
        }
    } else { // PRG
        return mapper->readPRG(addr);
    }
    return 0;
}

void bus::write(uint16_t addr, uint8_t val) {
    if (addr == 0x6000) {
        std::cout << "[NEMU] INFO: Test Results = " << std::hex << val << std::endl;
    }
    if (addr == 0x6001) {
        std::cout << "[NEMU] INFO: Test Results = " << std::hex << val << std::endl;
    }
    if (addr < 0x2000) { // RAM
        ram[addr & Ram.end] = val;
    } else if (addr < 0x4020) { // PPU and IO 
        if (addr < 0x4000) { // PPU and Mirrored PPU
            auto it = writeCallbacks.find(static_cast<IORegisters>(addr & 0x2007));
                if (it != writeCallbacks.end()) {
                    (it -> second)(val);
                } else {
                    std::cout << "[NEMU] Error: No write callback registered for I/O register at: " << std::hex << +addr << std::endl;
                }
        } else if (addr < 0x4017 && addr >= 0x4014) { // IO
            auto it = writeCallbacks.find(static_cast<IORegisters>(addr));
            if (it != writeCallbacks.end()) {
                (it -> second)(val);
            } else {
                std::cout << "[NEMU] Error: No write callback registered for I/O register at: " << std::hex << +addr << std::endl;
            }
        } else {
            std::cout << "[NEMU] WARNING: UNKNOWN ACCESS to PPU and IO : " << addr << ".\n";
        }
    } else if (addr < 0x6000) { // Expansion ROM
        std::cout << "[NEMU] WARNING: Expansion ROM is not implemented yet : " << addr << ".\n";
    } else if (addr < 0x8000) { // Extended RAM
        if (mapper->hasExtendedRAM()) {
            extendedRAM[addr - 0x6000] = val;
        }
    } else { // PRG
        mapper->writePRG(addr, val);
    }
}

bool bus::setWriteCallback(IORegisters reg, std::function<void(uint8_t)> callback) {
    if (!callback) {
        std::cout << "[NEMU] Error: Callback argument is null\n";
        return false;
    }
    return writeCallbacks.emplace(reg, callback).second;
}

bool bus::setReadCallback(IORegisters reg, std::function<uint8_t(void)> callback) {
    if (!callback) {
        std::cout << "[NEMU] Error: Callback argument is null\n";
        return false;
    }
    return readCallbacks.emplace(reg, callback).second;
}

const uint8_t* bus::getPagePtr(uint8_t page) {
    uint16_t addr = page << 8;
    if (addr < 0x2000) {
        return &ram[addr & 0x7ff];
    } else if (addr < 0x4020) {
        std::cout << "[NEMU] Warning: Unsupported Register address memory pointer has been accessed\n";
    } else if (addr < 0x6000) {
        std::cout << "[NEMU] Warning: Unsupported Expansion ROM has been accessed.\n";
    } else if (addr < 0x8000) {
        if (mapper->hasExtendedRAM()) {
            return &extendedRAM[addr - 0x6000];
        }
    } else {
        std::cout << "[NEMU] Error: Unknown DMA request: " << std::hex << "0x" << +addr << " (" << +page << ")" << std::dec << std::endl;
    }
    return nullptr;
}