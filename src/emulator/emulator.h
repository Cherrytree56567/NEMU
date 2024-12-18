#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <chrono>
#include <memory>
#include "../ppu/ppu.h"
#include "../cpu/cpu.h"
#include "../cartridge/cartridge.h"
#include "../mapper/mapper.h"
#include "../bus/bus.h"
#include "../ppu/ppu.h"
#include "../picturebus/picturebus.h"
#include "../Screen/Screen.h"

using TimePoint = std::chrono::high_resolution_clock::time_point;

const int NESVideoWidth = ScanlineVisibleDots;
const int NESVideoHeight = VisibleScanlines;

class emulator {
public:
    emulator(std::string path);
    void setVideoWidth(int width);
    void setVideoHeight(int height);
    void setVideoScale(float scale);
    bool loop();
private:
    void DMA(uint8_t page);

    float screenScale;
    TimePoint cycleTimer;
    std::chrono::high_resolution_clock::duration elapsedTime;
    std::chrono::nanoseconds cpuCycleDuration;
    std::shared_ptr<Cartridge> pCartridge;
    std::shared_ptr<Mapper> pMapper;
    std::shared_ptr<Screen> pScreen;
    std::shared_ptr<picturebus> pPictureBus;
    std::shared_ptr<PPU> pPpu;
    std::shared_ptr<bus> pBus;
    std::shared_ptr<cpu> pCpu;
};