#include "cpu.h"

void cpu::NOP() {
    currentINSTR = "NOP";
}

void cpu::BRK() {
    InterruptSeq(Interrupt::BRK);
    currentINSTR = "BRK";
}

void cpu::JSR() {
    pushStack(static_cast<uint8_t>((program_counter + 1) >> 8));
    pushStack(static_cast<uint8_t>(program_counter + 1));
    program_counter = readAddr(program_counter);
    currentINSTR = "JSR";
}

void cpu::RTS() {
    program_counter = pullStack();
    program_counter |= pullStack() << 8;
    ++program_counter;
    currentINSTR = "RTS";
}

void cpu::RTI() {
    {
        uint8_t flags = pullStack();
        status.N = flags & 0x80;
        status.V = flags & 0x40;
        status.D = flags & 0x8;
        status.I = flags & 0x4;
        status.Z = flags & 0x2;
        status.C = flags & 0x1;
    }
    program_counter = pullStack();
    program_counter |= pullStack() << 8;
    currentINSTR = "RTI";
}

void cpu::JMP() {
    program_counter = readAddr(program_counter);
    currentINSTR = "JMP";
}

void cpu::JMPI() {
    uint16_t location = readAddr(program_counter);
    uint16_t Page = location & 0xff00;
    program_counter = Bus->read(location) | Bus->read(Page | ((location + 1) & 0xff)) << 8;
    currentINSTR = "JMPI";
}

void cpu::PHP() {
    uint8_t flags = status.N << 7 | status.V << 6 | 1 << 5 | 1 << 4 | status.D << 3 | status.I << 2 | status.Z << 1 | status.C;
    pushStack(flags);
    currentINSTR = "PHP";
}

void cpu::PLP() {
    uint8_t flags = pullStack();
    status.N = flags & 0x80;
    status.V = flags & 0x40;
    status.D = flags & 0x8;
    status.I = flags & 0x4;
    status.Z = flags & 0x2;
    status.C = flags & 0x1;
    currentINSTR = "PLP";
}

void cpu::PHA() {
    pushStack(accumulator);
    currentINSTR = "PHA";
}

void cpu::PLA() {
    accumulator = pullStack();
    setZN(accumulator);
    currentINSTR = "PLA";
}

void cpu::DEY() {
    --y_reg;
    setZN(y_reg);
    currentINSTR = "DEY";
}

void cpu::DEX() {
    --x_reg;
    setZN(x_reg);
    currentINSTR = "DEX";
}

void cpu::TAY() {
    y_reg = accumulator;
    setZN(y_reg);
    currentINSTR = "TAY";
}

void cpu::INY() {
    ++y_reg;
    setZN(y_reg);
    currentINSTR = "INY";
}

void cpu::INX() {
    ++x_reg;
    setZN(x_reg);
    currentINSTR = "INX";
}

void cpu::CLC() {
    status.C = false;
    currentINSTR = "CLC";
}

void cpu::SEC() {
    status.C = true;
    currentINSTR = "SEC";
}

void cpu::CLI() {
    status.I = false;
    currentINSTR = "CLI";
}

void cpu::SEI() {
    status.I = true;
    currentINSTR = "SEI";
}

void cpu::CLD() {
    status.D = false;
    currentINSTR = "CLD";
}

void cpu::SED() {
    status.D = true;
    currentINSTR = "SED";
}

void cpu::TYA() {
    accumulator = y_reg;
    setZN(accumulator);
    currentINSTR = "TYA";
}

void cpu::CLV() {
    status.V = false;
    currentINSTR = "CLV";
}

void cpu::TXA() {
    accumulator = x_reg;
    setZN(accumulator);
    currentINSTR = "TXA";
}

void cpu::TXS() {
    stack_pointer = x_reg;
    currentINSTR = "TXS";
}

void cpu::TAX() {
    x_reg = accumulator;
    setZN(x_reg);
    currentINSTR = "TAX";
}

void cpu::TSX() {
    x_reg = stack_pointer;
    setZN(x_reg);
    currentINSTR = "TSX";
}

void cpu::BNH() {
    int8_t offset = Bus->read(program_counter++);
    ++skipCycles;
    auto newPC = static_cast<uint16_t>(program_counter + offset);
    setPageCrossed(program_counter, newPC, 2);
    program_counter = newPC;
}

void cpu::ORA(uint16_t loc) {
    accumulator |= Bus->read(loc);
    setZN(accumulator);
    currentINSTR = "ORA";
}

void cpu::AND(uint16_t loc) {
    accumulator &= Bus->read(loc);
    setZN(accumulator);
    currentINSTR = "AND";
}

void cpu::EOR(uint16_t loc) {
    accumulator ^= Bus->read(loc);
    setZN(accumulator);
    currentINSTR = "EOR";
}

void cpu::ADC(uint16_t loc) {
    uint8_t operand = Bus->read(loc);
    uint16_t sum = accumulator + operand + status.C;
    status.C = sum & 0x100;
    status.V = (accumulator ^ sum) & (operand ^ sum) & 0x80;
    accumulator = static_cast<uint8_t>(sum);
    setZN(accumulator);
    currentINSTR = "ADC";
}

void cpu::STA(uint16_t loc) {
    Bus->write(loc, accumulator);
    currentINSTR = "STA";
}

void cpu::LDA(uint16_t loc) {
    accumulator = Bus->read(loc);
    setZN(accumulator);
    currentINSTR = "LDA";
}

void cpu::SBC(uint16_t loc) {
    uint16_t subtrahend = Bus->read(loc), diff = accumulator - subtrahend - !status.C;
    status.C = !(diff & 0x100);
    status.V = (accumulator ^ diff) & (~subtrahend ^ diff) & 0x80;
    accumulator = diff;
    setZN(diff);
    currentINSTR = "SBC";
}

void cpu::CMP(uint16_t loc) {
    std::uint16_t diff = accumulator - Bus->read(loc);
    status.C = !(diff & 0x100);
    setZN(diff);
    currentINSTR = "CMP";
}

void cpu::ROL(uint16_t loc, int op, int addr_mode, uint16_t operand) {
    if (addr_mode == 0x2) {
        auto prev_C = status.C;
        status.C = accumulator & 0x80;
        accumulator <<= 1;
        accumulator = accumulator | (prev_C && (op == 0x1));
        setZN(accumulator);
    } else {
        auto prev_C = status.C;
        operand = Bus->read(loc);
        status.C = operand & 0x80;
        operand = operand << 1 | (prev_C && (op == 0x1));
        setZN(operand);
        Bus->write(loc, operand);
    }
    currentINSTR = "ROL";
}

void cpu::ROR(uint16_t loc, int op, int addr_mode, uint16_t operand) {
    if (addr_mode == 0x2) {
        auto prev_C = status.C;
        status.C = accumulator & 1;
        accumulator >>= 1;
        accumulator = accumulator | (prev_C && (op == 0x3)) << 7;
        setZN(accumulator);
    } else {
        auto prev_C = status.C;
        operand = Bus->read(loc);
        status.C = operand & 1;
        operand = operand >> 1 | (prev_C && (op == 0x3)) << 7;
        setZN(operand);
        Bus->write(loc, operand);
    }
    currentINSTR = "ROR";
}

void cpu::STX(uint16_t loc, int op, int addr_mode, uint16_t operand) {
    Bus->write(loc, x_reg);
    currentINSTR = "STX";
}

void cpu::LDX(uint16_t loc, int op, int addr_mode, uint16_t operand) {
    x_reg = Bus->read(loc);
    setZN(x_reg);
    currentINSTR = "LDX";
}

void cpu::DEC(uint16_t loc, int op, int addr_mode, uint16_t operand) {
    auto tmp = Bus->read(loc) - 1;
    setZN(tmp);
    Bus->write(loc, tmp);
    currentINSTR = "DEC";
}

void cpu::INC(uint16_t loc, int op, int addr_mode, uint16_t operand) {
    auto tmp = Bus->read(loc) + 1;
    setZN(tmp);
    Bus->write(loc, tmp);
    currentINSTR = "INC";
}

void cpu::BIT(uint16_t loc, uint16_t) {
    uint8_t operand = Bus->read(loc);
    status.Z = !(accumulator & operand);
    status.V = operand & 0x40;
    status.N = operand & 0x80;
    currentINSTR = "BIT";
}

void cpu::STY(uint16_t loc, uint16_t operand) {
    Bus->write(loc, y_reg);
    currentINSTR = "STY";
}

void cpu::LDY(uint16_t loc, uint16_t operand) {
    y_reg = Bus->read(loc);
    setZN(y_reg);
    currentINSTR = "LDY";
}

void cpu::CPY(uint16_t loc, uint16_t operand) {
    std::uint16_t diff = y_reg - Bus->read(loc);
    status.C = !(diff & 0x100);
    setZN(diff);
    currentINSTR = "CPY";
}

void cpu::CPX(uint16_t loc, uint16_t operand) {
    std::uint16_t diff = x_reg - Bus->read(loc);
    status.C = !(diff & 0x100);
    setZN(diff);
    currentINSTR = "CPX";
}
