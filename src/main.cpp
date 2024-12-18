#include <iostream>
#include <memory>
#include "emulator/emulator.h"

int main() {
    emulator emu("D:/NEMU/build/Debug/file.nes");
    while (emu.loop()) {
        
    }
    return 0;
}