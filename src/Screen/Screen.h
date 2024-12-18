#pragma once
#include <vector>
#include <cstdint>
#include "../color/color.h"

class Screen {
public:
    Screen();
    void create(unsigned int width, unsigned int height, float pixel_size, Color color);
    void setPixel(size_t x, size_t y, Color color);
    void draw() const;
private:
    int ScreenWidth;
    int ScreenHeight;
    float pixelSize;

    std::vector<Color> buffer;
};