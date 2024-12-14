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
    accumulator = y_reg;
    setZN(accumulator);
    std::cout << "[NEMU] INFO: TYA" << std::endl;
}

void cpu::CLV() {
    status.V = false;
    std::cout << "[NEMU] INFO: CLV" << std::endl;
}

void cpu::TXA() {
    accumulator = x_reg;
    setZN(accumulator);
    std::cout << "[NEMU] INFO: TXA" << std::endl;
}

void cpu::TXS() {
    stack_pointer = x_reg;
    std::cout << "[NEMU] INFO: TXS" << std::endl;
}

void cpu::TAX() {
    x_reg = accumulator;
    setZN(x_reg);
    std::cout << "[NEMU] INFO: TAX" << std::endl;
}

void cpu::TSX() {
    x_reg = stack_pointer;
    setZN(x_reg);
    std::cout << "[NEMU] INFO: TSX" << std::endl;
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
}

void cpu::AND(uint16_t loc) {
    accumulator &= Bus->read(loc);
    setZN(accumulator);
}

void cpu::EOR(uint16_t loc) {
    accumulator ^= Bus->read(loc);
    setZN(accumulator);
}

void cpu::ADC(uint16_t loc) {
    uint8_t operand = Bus->read(loc);
    uint16_t sum = accumulator + operand + status.C;
    status.C = sum & 0x100;
    status.V = (accumulator ^ sum) & (operand ^ sum) & 0x80;
    accumulator = static_cast<uint8_t>(sum);
    setZN(accumulator);
}

void cpu::STA(uint16_t loc) {
    Bus->write(loc, accumulator);
}

void cpu::LDA(uint16_t loc) {
    accumulator = Bus->read(loc);
    setZN(accumulator);
}

void cpu::SBC(uint16_t loc) {
    uint16_t subtrahend = Bus->read(loc), diff = accumulator - subtrahend - !status.C;
    status.C = !(diff & 0x100);
    status.V = (accumulator ^ diff) & (~subtrahend ^ diff) & 0x80;
    accumulator = diff;
    setZN(diff);
}

void cpu::CMP(uint16_t loc) {
    std::uint16_t diff = accumulator - Bus->read(loc);
    status.C = !(diff & 0x100);
    setZN(diff);
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
}

void cpu::STX(uint16_t loc, int op, int addr_mode, uint16_t operand) {
    Bus->write(loc, x_reg);
}

void cpu::LDX(uint16_t loc, int op, int addr_mode, uint16_t operand) {
    x_reg = Bus->read(loc);
    setZN(x_reg);
}

void cpu::DEC(uint16_t loc, int op, int addr_mode, uint16_t operand) {
    auto tmp = Bus->read(loc) - 1;
    setZN(tmp);
    Bus->write(loc, tmp);
}

void cpu::INC(uint16_t loc, int op, int addr_mode, uint16_t operand) {
    auto tmp = Bus->read(loc) + 1;
    setZN(tmp);
    Bus->write(loc, tmp);
}

void cpu::BIT(uint16_t loc, uint16_t operand) {
    operand = Bus->read(loc);
    status.Z = !(accumulator & operand);
    status.V = operand & 0x40;
    status.N = operand & 0x80;
}

void cpu::STY(uint16_t loc, uint16_t operand) {
    Bus->write(loc, y_reg);
}

void cpu::LDY(uint16_t loc, uint16_t operand) {
    y_reg = Bus->read(loc);
    setZN(y_reg);
}

void cpu::CPY(uint16_t loc, uint16_t operand) {
    std::uint16_t diff = y_reg - Bus->read(loc);
    status.C = !(diff & 0x100);
    setZN(diff);
}

void cpu::CPX(uint16_t loc, uint16_t operand) {
    std::uint16_t diff = x_reg - Bus->read(loc);
    status.C = !(diff & 0x100);
    setZN(diff);
}
