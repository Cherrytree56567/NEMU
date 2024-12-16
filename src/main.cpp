#include <iostream>
#include <memory>
#include "cpu/cpu.h"
#include "Mapper/NROM/NROM.h"

int main() {
    std::shared_ptr<Cartridge> pCartridge = std::make_shared<Cartridge>("Super Mario Bros. (Japan, USA).nes");
    std::shared_ptr<Mapper> pMapper = Mapper::createMapper(static_cast<MapperType>(pCartridge->getMapper()), *pCartridge.get());
    std::shared_ptr<bus> pBus = std::make_shared<bus>(pMapper);
    std::shared_ptr<cpu> pCpu = std::make_shared<cpu>(pBus);
    
    pCpu->reset();
    while (true) {
        pCpu->step();
    }
    return 0;
}