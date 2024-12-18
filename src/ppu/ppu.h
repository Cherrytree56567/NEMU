#pragma once
#include <functional>
#include <vector>
#include <memory>
#include "../picturebus/picturebus.h"
#include "../color/color.h"
#include "../Screen/Screen.h"

#define ScanlineCycleLength 341
#define ScanlineEndCycle 340
#define VisibleScanlines 240
#define ScanlineVisibleDots 256
#define FrameEndScanline 261

#define AttributeOffset 0x3C0

enum PPUState {
    PreRender,
    Render,
    PostRender,
    VerticalBlank
};

enum CharacterPage {
    Low,
    High,
};

class PPU {
public:
    PPU(std::shared_ptr<picturebus> pictbus, std::shared_ptr<Screen> scr);
    void step();
    void reset();
 
    void setInterruptCallback(std::function<void(void)> cb);

    void doDMA(uint8_t* page_ptr);

    void control(uint8_t ctrl);
    void setMask(uint8_t mask);
    void setOAMAddress(uint8_t addr);
    void setDataAddress(uint8_t addr);
    void setScroll(uint8_t scroll);
    void setData(uint8_t data);
    
    uint8_t getStatus();
    uint8_t getData();
    uint8_t getOAMData();
    void setOAMData(uint8_t value);
private:
    uint8_t readOAM(uint8_t addr);
    void writeOAM(uint8_t addr, uint8_t value);
    uint8_t read(uint16_t addr);
    std::shared_ptr<picturebus> bus;
    std::shared_ptr<Screen> screen;

    std::function<void(void)> vblankCallback;
    std::vector<uint8_t> spriteMemory;
    std::vector<uint8_t> scanlineSprites;

    PPUState pipelineState;
    int cycle;
    int scanline;
    bool evenFrame;
    bool vblank;
    bool sprZeroHit;
    bool spriteOverflow;

    uint16_t dataAddress;
    uint16_t tempAddress;
    uint8_t fineXScroll;
    bool firstWrite;
    uint8_t dataBuffer;
    uint8_t spriteDataAddress;

    bool longSprites;
    bool generateInterrupt;
    bool greyscaleMode;
    bool showSprites;
    bool showBackground;
    bool hideEdgeSprites;
    bool hideEdgeBackground;

    CharacterPage bgPage;
    CharacterPage sprPage;

    uint16_t dataAddrIncrement;

    std::vector<std::vector<Color>> pictureBuffer;
};