#include "emulator.h"

emulator::emulator(std::string path) : screenScale(3.f) {
    pCartridge = std::make_shared<Cartridge>(path);
    pMapper = Mapper::createMapper(static_cast<MapperType>(pCartridge->getMapper()), *pCartridge.get());
    pScreen = std::make_shared<Screen>();
    pPictureBus = std::make_shared<picturebus>(pMapper);
    pPpu = std::make_shared<PPU>(pPictureBus, pScreen);
    pBus = std::make_shared<bus>(pMapper, pPpu);
    pCpu = std::make_shared<cpu>(pBus);

    if(!pBus->setReadCallback(PPUSTATUS, [&]() -> uint8_t { return pPpu->getStatus(); }) ||
        !pBus->setReadCallback(PPUDATA, [&]() -> uint8_t { return pPpu->getData(); }) ||
        !pBus->setReadCallback(JOY1, [&]() -> uint8_t { return 0; }) ||
        !pBus->setReadCallback(JOY2, [&]() -> uint8_t { return 0; }) ||
        !pBus->setReadCallback(OAMDATA, [&]() -> uint8_t { return pPpu->getOAMData(); })) {
        std::cout << "[NEMU] Error: Failed to set I/O callbacks.\n";
    } 

    if(!pBus->setWriteCallback(PPUCTRL, [&](uint8_t b) { pPpu->control(b); }) ||
        !pBus->setWriteCallback(PPUMASK, [&](uint8_t b) { pPpu->setMask(b); }) ||
        !pBus->setWriteCallback(OAMADDR, [&](uint8_t b) { pPpu->setOAMAddress(b); }) ||
        !pBus->setWriteCallback(PPUADDR, [&](uint8_t b) { pPpu->setDataAddress(b); }) ||
        !pBus->setWriteCallback(PPUSCROL, [&](uint8_t b) { pPpu->setScroll(b); }) ||
        !pBus->setWriteCallback(PPUDATA, [&](uint8_t b) { pPpu->setData(b); }) ||
        !pBus->setWriteCallback(OAMDMA, [&](uint8_t b) { DMA(b); }) ||
        !pBus->setWriteCallback(JOY1, [&](uint8_t b) {  }) ||
        !pBus->setWriteCallback(OAMDATA, [&](uint8_t b) { pPpu->setOAMData(b); })) {
        std::cout << "[NEMU] Error: Failed to set I/O callbacks.\n";
    }

    pPpu->setInterruptCallback([&](){ pCpu->interrupt(Interrupt::NMI); });
    
    pCpu->reset();
    pPpu->reset();
    pScreen->create(NESVideoWidth, NESVideoHeight, screenScale, Color(255, 255, 255, 255));
    cycleTimer = std::chrono::high_resolution_clock::now();
    elapsedTime = cycleTimer - cycleTimer;
}

void emulator::setVideoWidth(int width) {
    screenScale = width / float(NESVideoWidth);
}

void emulator::setVideoHeight(int height) {
    screenScale = height / float(NESVideoHeight);
}

void emulator::setVideoScale(float scale) {
    screenScale = scale;
}

bool emulator::loop() {
    pPpu->step();
    pPpu->step();
    pPpu->step();
    pCpu->step();
    pScreen->setPixel(9, 9, Color(24, 47, 31, 255));

    //pScreen->draw();
    
    /*
    * TODO: Draw here using HTML canvas' and JS via WASM
    */

    return true;
}

void emulator::DMA(uint8_t page) {
    pCpu->skipDMACycles();
    auto page_ptr = pBus->getPagePtr(page);
    if (page_ptr != nullptr) {
        pPpu->doDMA(page_ptr);
    } else {
        std::cout << "[NEMU] Error: Cannot get pageptr for DMA.\n";
    }
}