#include <iostream>
#include "cpu/cpu.h"

int main() {
    std::shared_ptr<bus> pBus = std::make_shared<bus>();
    std::shared_ptr<cpu> pCpu = std::make_shared<cpu>(pBus);
    
    while (true) {
        pCpu->step();
    }
    return 0;
}