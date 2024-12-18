#include "Screen.h"
/*#include <emscripten.h>
#include <emscripten/bind.h>

Screen::Screen() : ScreenWidth(400), ScreenHeight(400), pixelSize(1.0f) {

}

void Screen::create(unsigned int width, unsigned int height, float pixel_size, Color color) {
    ScreenWidth = width;
    ScreenHeight = height;
    pixelSize = pixel_size;
    buffer.resize(width * height, color);
}

void Screen::setPixel(size_t x, size_t y, Color color) {
    if (x >= ScreenWidth || y >= ScreenHeight) return;
    buffer[y * ScreenWidth + x] = color;
}

void Screen::draw() const {
    EM_ASM_ARGS({
        const len = $0;
        const bufferPtr = $1;
        const memoryBuffer = new Uint8Array(Module.HEAPU8.buffer, bufferPtr, len);

        let pixelBuffer = Array.from(memoryBuffer);

        Module.renderScreen(JSON.stringify(pixelBuffer));
    }, buffer.size() * sizeof(Color), reinterpret_cast<int>(buffer.data()));
}*/

Screen::Screen() : ScreenWidth(400), ScreenHeight(400), pixelSize(1.0f) {

}

void Screen::create(unsigned int width, unsigned int height, float pixel_size, Color color) {
}

void Screen::setPixel(size_t x, size_t y, Color color) {
}

void Screen::draw() const {
    
}