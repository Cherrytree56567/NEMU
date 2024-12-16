#include <iostream>
#include <memory>
#include "emulator/emulator.h"

int main() {
    emulator emu("Super Mario Bros. (Japan, USA).nes");
    while (emu.loop()) {
        
    }
    return 0;
}