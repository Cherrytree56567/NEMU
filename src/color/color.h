#pragma once
#include <cstdint>

struct Color {
    Color(int rr, int bb, int gg, int aa) : r(rr), b(bb), g(gg), a(aa) {}
    Color(const uint32_t rgba) {
        r = (rgba >> 24) & 0xFF; // Extract red component
        g = (rgba >> 16) & 0xFF; // Extract green component
        b = (rgba >> 8)  & 0xFF; // Extract blue component
        a = rgba & 0xFF;         // Extract alpha component
    }

    int r;
    int g;
    int b;
    int a;
};