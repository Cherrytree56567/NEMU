#include "ppu.h"

PPU::PPU(std::shared_ptr<picturebus> pictbus, std::shared_ptr<Screen> scr) : spriteMemory(64 * 4), pictureBuffer(ScanlineVisibleDots, std::vector<Color>(VisibleScanlines, Color(255, 0, 255, 0))), bus(pictbus), screen(scr) {
    
}

void PPU::reset() {
    longSprites = false;
    generateInterrupt = false;
    greyscaleMode = false;
    vblank = false;
    spriteOverflow = false;
    showBackground = true;
    showSprites = true;
    evenFrame = true;
    firstWrite = true;
    bgPage = Low;
    sprPage = Low;
    dataAddress = 0;
    cycle = 0;
    scanline = 0;
    spriteDataAddress = 0;
    fineXScroll = 0;
    tempAddress = 0;
    dataAddrIncrement = 1;
    pipelineState = PreRender;
    scanlineSprites.reserve(8);
    scanlineSprites.resize(0);
}

void PPU::setInterruptCallback(std::function<void(void)> cb) {
    vblankCallback = cb;
}

void PPU::step() {
    switch (pipelineState) {
    case PreRender:
        if (cycle == 1) {
            vblank = sprZeroHit = false;
        } else if (cycle == ScanlineVisibleDots + 2 && showBackground && showSprites) {
            dataAddress &= ~0x41f;
            dataAddress |= tempAddress & 0x41f;
        } else if (cycle > 280 && cycle <= 304 && showBackground && showSprites) {
            dataAddress &= ~0x7be0;
            dataAddress |= tempAddress & 0x7be0;
        }
        
        if (cycle >= ScanlineEndCycle - (!evenFrame && showBackground && showSprites)) {
            pipelineState = Render;
            cycle = 0;
            scanline = 0;
        }

        if (cycle == 260 && showBackground && showSprites){
            bus->scanlineIRQ();
        }
        break;
    case Render:
        if (cycle > 0 && cycle <= ScanlineVisibleDots) {
            uint8_t bgColor = 0;
            uint8_t sprColor = 0;
            bool bgOpaque = false;
            bool sprOpaque = true;
            bool spriteForeground = false;

            int x = cycle - 1;
            int y = scanline;

            if (showBackground) {
                auto x_fine = (fineXScroll + x) % 8;
                if (!hideEdgeBackground || x >= 8) {
                    auto addr = 0x2000 | (dataAddress & 0x0FFF);
                    uint8_t tile = read(addr);

                    addr = (tile * 16) + ((dataAddress >> 12) & 0x7);
                    addr |= bgPage << 12;
                    bgColor = (read(addr) >> (7 ^ x_fine)) & 1;
                    bgColor |= ((read(addr + 8) >> (7 ^ x_fine)) & 1) << 1;

                    bgOpaque = bgColor;
                    
                    addr = 0x23C0 | (dataAddress & 0x0C00) | ((dataAddress >> 4) & 0x38) | ((dataAddress >> 2) & 0x07);
                    auto attribute = read(addr);
                    int shift = ((dataAddress >> 4) & 4) | (dataAddress & 2);
                    bgColor |= ((attribute >> shift) & 0x3) << 2;
                }
                
                if (x_fine == 7) {
                    if ((dataAddress & 0x001F) == 31) {
                        dataAddress &= ~0x001F;
                        dataAddress ^= 0x0400;
                    } else {
                        dataAddress += 1;
                    }
                }
            }

            if (showSprites && (!hideEdgeSprites || x >= 8)) {
                for (auto i : scanlineSprites) {
                    uint8_t spr_x = spriteMemory[i * 4 + 3];

                    if (0 > x - spr_x || x - spr_x >= 8)
                        continue;

                    uint8_t spr_y = spriteMemory[i * 4 + 0] + 1;
                    uint8_t tile = spriteMemory[i * 4 + 1];
                    uint8_t attribute = spriteMemory[i * 4 + 2];

                    int length = (longSprites) ? 16 : 8;

                    int x_shift = (x - spr_x) % 8, y_offset = (y - spr_y) % length;

                    if ((attribute & 0x40) == 0) {
                        x_shift ^= 7;
                    }

                    if ((attribute & 0x80) != 0) {
                        y_offset ^= (length - 1);
                    }

                    uint16_t addr = 0;

                    if (!longSprites) {
                        addr = tile * 16 + y_offset;
                        if (sprPage == High) addr += 0x1000;
                    } else {
                        y_offset = (y_offset & 7) | ((y_offset & 8) << 1);
                        addr = (tile >> 1) * 32 + y_offset;
                        addr |= (tile & 1) << 12;
                    }

                    sprColor |= (read(addr) >> (x_shift)) & 1;
                    sprColor |= ((read(addr + 8) >> (x_shift)) & 1) << 1;

                    if (!(sprOpaque = sprColor)) {
                        sprColor = 0;
                        continue;
                    }

                    sprColor |= 0x10;
                    sprColor |= (attribute & 0x3) << 2;

                    spriteForeground = !(attribute & 0x20);

                    if (!sprZeroHit && showBackground && i == 0 && sprOpaque && bgOpaque) {
                        sprZeroHit = true;
                    }

                    break;
                }
            }

            uint8_t paletteAddr = bgColor;

            if ((!bgOpaque && sprOpaque) || (bgOpaque && sprOpaque && spriteForeground)) {
                paletteAddr = sprColor;
            } else if (!bgOpaque && !sprOpaque) {
                paletteAddr = 0;
            }

            pictureBuffer[x][y] = Color(colors[bus->readPalette(paletteAddr)]);
        } else if (cycle == ScanlineVisibleDots + 1 && showBackground) {
            /*
            * From NESDEV Wiki
            */
            if ((dataAddress & 0x7000) != 0x7000) {
                dataAddress += 0x1000;
            } else {
                dataAddress &= ~0x7000;
                int y = (dataAddress & 0x03E0) >> 5;
                if (y == 29) {
                    y = 0;
                    dataAddress ^= 0x0800;
                } else if (y == 31) {
                    y = 0;
                } else {
                    y += 1;
                }
                dataAddress = (dataAddress & ~0x03E0) | (y << 5);
            }
        } else if (cycle == ScanlineVisibleDots + 2 && showBackground && showSprites) {
            dataAddress &= ~0x41f;
            dataAddress |= tempAddress & 0x41f;
        }
        
        if(cycle == 260 && showBackground && showSprites){
            bus->scanlineIRQ();
        }

        if (cycle >= ScanlineEndCycle) {
            scanlineSprites.resize(0);

            int range = 8;
            if (longSprites) {
                range = 16;
            }

            std::size_t j = 0;
            for (std::size_t i = spriteDataAddress / 4; i < 64; ++i) {
                auto diff = (scanline - spriteMemory[i * 4]);
                if (0 <= diff && diff < range) {
                    if (j >= 8) {
                        spriteOverflow = true;
                        break;
                    }
                    scanlineSprites.push_back(i);
                    ++j;
                }
            }

            ++scanline;
            cycle = 0;
        }

        if (scanline >= VisibleScanlines) {
            pipelineState = PostRender;
        }

        break;
    case PostRender:
        if (cycle >= ScanlineEndCycle) {
            ++scanline;
            cycle = 0;
            pipelineState = VerticalBlank;

            for (std::size_t x = 0; x < pictureBuffer.size(); ++x) {
                for (std::size_t y = 0; y < pictureBuffer[0].size(); ++y) {
                    screen->setPixel(x, y, pictureBuffer[x][y]);
                }
            }

            screen->draw();

        }

        break;
    case VerticalBlank:
        if (cycle == 1 && scanline == VisibleScanlines + 1) {
            vblank = true;
            if (generateInterrupt) vblankCallback();
        }

        if (cycle >= ScanlineEndCycle) {
            ++scanline;
            cycle = 0;
        }

        if (scanline >= FrameEndScanline) {
            pipelineState = PreRender;
            scanline = 0;
            evenFrame = !evenFrame;
        }

        break;
    default:
        std::cout << "[NEMU] Error: Unknown Pipeline State.\n";
    }
    ++cycle;
}

uint8_t PPU::readOAM(uint8_t addr) {
    return spriteMemory[addr];
}

void PPU::writeOAM(uint8_t addr, uint8_t value) {
    spriteMemory[addr] = value;
}

void PPU::doDMA(const uint8_t* page_ptr) {
    std::memcpy(spriteMemory.data() + spriteDataAddress, page_ptr, 256 - spriteDataAddress);
    if (spriteDataAddress) {
        std::memcpy(spriteMemory.data(), page_ptr + (256 - spriteDataAddress), spriteDataAddress);
    }
}

void PPU::control(uint8_t ctrl) {
    generateInterrupt = ctrl & 0x80;
    longSprites = ctrl & 0x20;
    bgPage = static_cast<CharacterPage>(!!(ctrl & 0x10));
    sprPage = static_cast<CharacterPage>(!!(ctrl & 0x8));
    if (ctrl & 0x4) {
        dataAddrIncrement = 0x20;
    } else {
        dataAddrIncrement = 1;
    }
    
    tempAddress &= ~0xc00;
    tempAddress |= (ctrl & 0x3) << 10;
}

void PPU::setMask(uint8_t mask) {
    greyscaleMode = mask & 0x1;
    hideEdgeBackground = !(mask & 0x2);
    hideEdgeSprites = !(mask & 0x4);
    showBackground = mask & 0x8;
    showSprites = mask & 0x10;
}

uint8_t PPU::getStatus() {
    uint8_t status = spriteOverflow << 5 | sprZeroHit << 6 | vblank << 7;
    
    vblank = false;
    firstWrite = true;
    return status;
}

void PPU::setDataAddress(uint8_t addr) {
    if (firstWrite) {
        tempAddress &= ~0xff00;
        tempAddress |= (addr & 0x3f) << 8;
        firstWrite = false;
    } else {
        tempAddress &= ~0xff;
        tempAddress |= addr;
        dataAddress = tempAddress;
        firstWrite = true;
    }
}

uint8_t PPU::getData() {
    auto data = bus->read(dataAddress);
    dataAddress += dataAddrIncrement;

    if (dataAddress < 0x3f00) {
        std::swap(data, dataBuffer);
    }

    return data;
}

uint8_t PPU::getOAMData() {
    return readOAM(spriteDataAddress);
}

void PPU::setData(uint8_t data) {
    bus->write(dataAddress, data);
    dataAddress += dataAddrIncrement;
}

void PPU::setOAMAddress(uint8_t addr) {
    spriteDataAddress = addr;
}

void PPU::setOAMData(uint8_t value) {
    writeOAM(spriteDataAddress++, value);
}

void PPU::setScroll(uint8_t scroll) {
    if (firstWrite) {
        tempAddress &= ~0x1f;
        tempAddress |= (scroll >> 3) & 0x1f;
        fineXScroll = scroll & 0x7;
        firstWrite = false;
    } else {
        tempAddress &= ~0x73e0;
        tempAddress |= ((scroll & 0x7) << 12) | ((scroll & 0xf8) << 2);
        firstWrite = true;
    }
}

uint8_t PPU::read(uint16_t addr) {
    return bus->read(addr);
}