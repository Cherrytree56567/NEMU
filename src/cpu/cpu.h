#pragma once
#include <cstdint>
#include <memory>
#include "../bus/bus.h"

#define STACK_STARTING_POINTER 0xFF
#define NMIVector 0xfffa
#define ResetVector 0xfffc
#define IRQVector 0xfffe

struct Status {
    bool C;
    bool Z;
    bool I;
    bool D;
    bool V;
    bool N;
    bool pendingNMI;
    bool pendingIRQ;
};

enum Interrupt {
    IRQ,
    NMI,
    BRK
};

/*
From https://github.com/amhndu/SimpleNES/blob/master/include/CPUOpcodes.h
*/
static const int OperationCycles[0x100] = {
    7, 6, 0, 0, 0, 3, 5, 0, 3, 2, 2, 0, 0, 4, 6, 0,
    2, 5, 0, 0, 0, 4, 6, 0, 2, 4, 0, 0, 0, 4, 7, 0,
    6, 6, 0, 0, 3, 3, 5, 0, 4, 2, 2, 0, 4, 4, 6, 0,
    2, 5, 0, 0, 0, 4, 6, 0, 2, 4, 0, 0, 0, 4, 7, 0,
    6, 6, 0, 0, 0, 3, 5, 0, 3, 2, 2, 0, 3, 4, 6, 0,
    2, 5, 0, 0, 0, 4, 6, 0, 2, 4, 0, 0, 0, 4, 7, 0,
    6, 6, 0, 0, 0, 3, 5, 0, 4, 2, 2, 0, 5, 4, 6, 0,
    2, 5, 0, 0, 0, 4, 6, 0, 2, 4, 0, 0, 0, 4, 7, 0,
    0, 6, 0, 0, 3, 3, 3, 0, 2, 0, 2, 0, 4, 4, 4, 0,
    2, 6, 0, 0, 4, 4, 4, 0, 2, 5, 2, 0, 0, 5, 0, 0,
    2, 6, 2, 0, 3, 3, 3, 0, 2, 2, 2, 0, 4, 4, 4, 0,
    2, 5, 0, 0, 4, 4, 4, 0, 2, 4, 2, 0, 4, 4, 4, 0,
    2, 6, 0, 0, 3, 3, 5, 0, 2, 2, 2, 0, 4, 4, 6, 0,
    2, 5, 0, 0, 0, 4, 6, 0, 2, 4, 0, 0, 0, 4, 7, 0,
    2, 6, 0, 0, 3, 3, 5, 0, 2, 2, 2, 2, 4, 4, 6, 0,
    2, 5, 0, 0, 0, 4, 6, 0, 2, 4, 0, 0, 0, 4, 7, 0,
};

class cpu {
public:
    cpu(std::shared_ptr<bus> pBus);

    void step();
    void reset();
    void reset(uint16_t start_addr);
    void interrupt(Interrupt type);

    bool execute(uint8_t opcode);
    
    uint16_t getProgramCounter() {
        return program_counter;
    }

private:
    void InterruptSeq(Interrupt type);
    void pushStack(uint8_t val);
    
    uint16_t readAddr(uint16_t addr);
    uint8_t pullStack();

    void setZN(uint8_t value);

    void NOP();
    void BRK();
    void JSR();
    void RTS();
    void RTI();
    void JMP();
    void JMPI();
    void PHP();
    void PLP();
    void PHA();
    void PLA();
    void DEY();
    void DEX();
    void TAY();
    void INY();
    void INX();
    void CLC();
    void SEC();
    void CLI();
    void SEI();
    void CLD();
    void SED();
    void TYA();
    void CLV();
    void TXA();
    void TXS();
    void TAX();
    void TSX();

    uint8_t accumulator;
    uint8_t x_reg;
    uint8_t y_reg;
    Status status;
    uint8_t stack_pointer;
    uint16_t program_counter;
    int skipCycles;
    int cycles;
    std::shared_ptr<bus> Bus;
};