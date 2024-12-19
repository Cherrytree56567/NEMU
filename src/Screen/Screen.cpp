#include "Screen.h"

/*
#include <emscripten.h>
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

float normalizeColor(int value) {
    return static_cast<float>(value) / 255.0f;
}

Screen::Screen() : ScreenWidth(400), ScreenHeight(400), pixelSize(1.0f), window(nullptr) {}

void Screen::create(unsigned int width, unsigned int height, float pixel_size, Color color) {
    ScreenWidth = width;
    ScreenHeight = height;
    pixelSize = pixel_size;
    buffer.resize(ScreenWidth * ScreenHeight, color);

    if (!glfwInit()) {
        std::cout << "Failed to initialize GLFW\n";
        exit(-1);
        return;
    }

    window = glfwCreateWindow(ScreenWidth * pixelSize, ScreenHeight * pixelSize, "Screen", nullptr, nullptr);
    if (!window) {
        std::cout << "Failed to create GLFW window\n";
        glfwTerminate();
        exit(-1);
        return;
    }

    glfwMakeContextCurrent(window);
    glOrtho(0, ScreenWidth, ScreenHeight, 0, -1, 1); // Set up an orthographic projection
}

void Screen::setPixel(size_t x, size_t y, Color color) {
    if (x < ScreenWidth && y < ScreenHeight) {
        buffer[y * ScreenWidth + x] = color;
    }
}

void Screen::draw() const {
    if (!window) return;

    glClear(GL_COLOR_BUFFER_BIT);

    glBegin(GL_QUADS);
    for (size_t y = 0; y < ScreenHeight; ++y) {
        for (size_t x = 0; x < ScreenWidth; ++x) {
            const Color& color = buffer[y * ScreenWidth + x];
            glColor3f(normalizeColor(color.r), normalizeColor(color.g), normalizeColor(color.b));

            float xPos = x * pixelSize;
            float yPos = y * pixelSize;

            glVertex2f(xPos, yPos);
            glVertex2f(xPos + pixelSize, yPos);
            glVertex2f(xPos + pixelSize, yPos + pixelSize);
            glVertex2f(xPos, yPos + pixelSize);
        }
    }
    glEnd();

    glfwSwapBuffers(window);
    glfwPollEvents();
}