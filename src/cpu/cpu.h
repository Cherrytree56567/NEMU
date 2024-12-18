#pragma once
#include <cstdint>
#include <memory>
#include <string>
#include <iomanip>
#include "../bus/bus.h"

#define STACK_STARTING_POINTER 0xFF
#define NMIVector 0xfffa
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

enum BranchOnFlag {
    Negative,
    Overflow,
    Carry,
    Zero
};

const auto ResetVector = 0xfffc;

class cpu {
public:
    cpu(std::shared_ptr<bus> pBus);

    void step();
    void reset(uint16_t start_addr);
    void reset();
    void interrupt(Interrupt type);

    void skipDMACycles() {
        skipCycles += 513;
        skipCycles += (cycles & 1);
    }
    
    uint16_t getProgramCounter() {
        return program_counter;
    }

private:
    void InterruptSeq(Interrupt type);
    void pushStack(uint8_t val);
    
    uint16_t readAddr(uint16_t addr);
    uint8_t pullStack();

    void setZN(uint8_t value);
    void setPageCrossed(uint16_t a, uint16_t b, int inc = 1);

    bool execute(uint8_t opcode);
    bool executeBranch(uint8_t opcode);
    bool executeType1(uint8_t opcode);
    bool executeType2(uint8_t opcode);
    bool executeType0(uint8_t opcode);

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
    void BNH();
    void ORA(uint16_t loc);
    void AND(uint16_t loc);
    void EOR(uint16_t loc);
    void ADC(uint16_t loc);
    void STA(uint16_t loc);
    void LDA(uint16_t loc);
    void SBC(uint16_t loc);
    void CMP(uint16_t loc);
    void ROL(uint16_t loc, int op, int addr_mode, uint16_t operand);
    void ROR(uint16_t loc, int op, int addr_mode, uint16_t operand);
    void STX(uint16_t loc, int op, int addr_mode, uint16_t operand);
    void LDX(uint16_t loc, int op, int addr_mode, uint16_t operand);
    void DEC(uint16_t loc, int op, int addr_mode, uint16_t operand);
    void INC(uint16_t loc, int op, int addr_mode, uint16_t operand);
    void BIT(uint16_t loc, uint16_t operand);
    void STY(uint16_t loc, uint16_t operand);
    void LDY(uint16_t loc, uint16_t operand);
    void CPY(uint16_t loc, uint16_t operand);
    void CPX(uint16_t loc, uint16_t operand);

    uint8_t accumulator;
    uint8_t x_reg;
    uint8_t y_reg;
    Status status;
    uint8_t stack_pointer;
    uint16_t program_counter;
    int skipCycles;
    int cycles;
    std::shared_ptr<bus> Bus;
    std::string currentINSTR;
};