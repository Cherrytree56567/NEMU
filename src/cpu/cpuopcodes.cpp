#include "cpu.h"

void cpu::NOP() {
    std::cout << "[NEMU] INFO: NOP" << std::endl;
}

void cpu::BRK() {
    InterruptSeq(Interrupt::BRK);
    std::cout << "[NEMU] INFO: BRK" << std::endl;
}

void cpu::JSR() {
    pushStack(static_cast<uint8_t>((program_counter + 1) >> 8));
    pushStack(static_cast<uint8_t>(program_counter + 1));
    program_counter = readAddr(program_counter);
    std::cout << "[NEMU] INFO: JSR" << std::endl;
}

void cpu::RTS() {
    program_counter = pullStack();
    program_counter |= pullStack() << 8;
    program_counter++;
    std::cout << "[NEMU] INFO: RTS" << std::endl;
}

void cpu::RTI() {
    uint8_t flags = pullStack();
    status.N = flags & 0x80;
    status.V = flags & 0x40;
    status.D = flags & 0x8;
    status.I = flags & 0x4;
    status.Z = flags & 0x2;
    status.C = flags & 0x1;
    program_counter = pullStack();
    program_counter |= pullStack() << 8;
    std::cout << "[NEMU] INFO: RTI" << std::endl;
}

void cpu::JMP() {
    program_counter = readAddr(program_counter);
    std::cout << "[NEMU] INFO: JMP" << std::endl;
}

void cpu::JMPI() {
    uint16_t location = readAddr(program_counter);
    uint16_t Page = location & 0xff00;
    program_counter = Bus->read(location) | Bus->read(Page | ((location + 1) & 0xff)) << 8;
    std::cout << "[NEMU] INFO: JMPI" << std::endl;
}

void cpu::PHP() {
    uint8_t flags = status.N << 7 | status.V << 6 | 1 << 5 | 1 << 4 | status.D << 3 | status.I << 2 | status.Z << 1 | status.C;
    pushStack(flags);
    std::cout << "[NEMU] INFO: PHP" << std::endl;
}

void cpu::PLP() {
    uint8_t flags = pullStack();
    status.N = flags & 0x80;
    status.V = flags & 0x40;
    status.D = flags & 0x8;
    status.I = flags & 0x4;
    status.Z = flags & 0x2;
    status.C = flags & 0x1;
    std::cout << "[NEMU] INFO: PLP" << std::endl;
}

void cpu::PHA() {
    pushStack(accumulator);
    std::cout << "[NEMU] INFO: PHA" << std::endl;
}

void cpu::PLA() {
    accumulator = pullStack();
    setZN(accumulator);
    std::cout << "[NEMU] INFO: PLA" << std::endl;
}

void cpu::DEY() {
    --y_reg;
    setZN(y_reg);
    std::cout << "[NEMU] INFO: DEY" << std::endl;
}

void cpu::DEX() {
    --x_reg;
    setZN(x_reg);
    std::cout << "[NEMU] INFO: DEX" << std::endl;
}

void cpu::TAY() {
    y_reg = accumulator;
    setZN(y_reg);
    std::cout << "[NEMU] INFO: TAY" << std::endl;
}

void cpu::INY() {
    ++y_reg;
    setZN(y_reg);
    std::cout << "[NEMU] INFO: INY" << std::endl;
}

void cpu::INX() {
    ++x_reg;
    setZN(x_reg);
    std::cout << "[NEMU] INFO: INX" << std::endl;
}

void cpu::CLC() {
    status.C = false;
    std::cout << "[NEMU] INFO: CLC" << std::endl;
}

void cpu::SEC() {
    status.C = true;
    std::cout << "[NEMU] INFO: SEC" << std::endl;
}

void cpu::CLI() {
    status.I = false;
    std::cout << "[NEMU] INFO: CLI" << std::endl;
}

void cpu::SEI() {
    status.I = true;
    std::cout << "[NEMU] INFO: SEI" << std::endl;
}

void cpu::CLD() {
    status.D = false;
    std::cout << "[NEMU] INFO: CLD" << std::endl;
}

void cpu::SED() {
    status.D = true;
    std::cout << "[NEMU] INFO: SED" << std::endl;
}

void cpu::TYA() {
    std::cout << "[NEMU] INFO: TYA" << std::endl;
}

void cpu::CLV() {
    std::cout << "[NEMU] INFO: CLV" << std::endl;
}

void cpu::TXA() {
    std::cout << "[NEMU] INFO: TXA" << std::endl;
}

void cpu::TXS() {
    std::cout << "[NEMU] INFO: TXS" << std::endl;
}

void cpu::TAX() {
    std::cout << "[NEMU] INFO: TAX" << std::endl;
}

void cpu::TSX() {
    std::cout << "[NEMU] INFO: TSX" << std::endl;
}
