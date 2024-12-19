#include <iostream>
#include <memory>
#include "emulator/emulator.h"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cout << "Please provide rom args.\n";
    }
    emulator emu("../power_up_palette.nes");//argv[1]);
    while (emu.loop()) {
        
    }
    return 0;
}