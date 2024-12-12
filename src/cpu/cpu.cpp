#include "cpu.h"

cpu::cpu(std::shared_ptr<bus> pBus) : Bus(pBus) {
    status.pendingIRQ = false;
    status.pendingNMI = false;
}

void cpu::reset(uint16_t start_addr) {
    skipCycles = 0;
    cycles = 0;
    accumulator = 0;
    x_reg = 0;
    y_reg = 0;
    status.I = true;
    status.C = false;
    status.D = false;
    status.N = false;
    status.V = false;
    status.Z = false;
    program_counter = start_addr;
    stack_pointer = 0xfd;
}

void cpu::reset() {
    reset(readAddr(ResetVector));
}

uint16_t cpu::readAddr(uint16_t addr) {
    return Bus->read(addr) | Bus->read(addr + 1) << 8;
}

void cpu::pushStack(uint8_t val) {
    Bus->write(0x100 | stack_pointer, val);
    stack_pointer -= 1;
}

uint8_t cpu::pullStack() {
    return Bus->read(0x100 | ++stack_pointer);
}

void cpu::InterruptSeq(Interrupt type) {
    if (status.I && type != NMI && type != Interrupt::BRK){
        return;
    }

    if (type == Interrupt::BRK) {
        program_counter += 1;
    }

    pushStack(program_counter >> 8);
    pushStack(program_counter);

    uint8_t flags = status.N << 7 | status.V << 6 | 1 << 5 | (type == Interrupt::BRK) << 4 | status.D << 3 | status.I << 2 | status.Z << 1 | status.C;
    pushStack(flags);

    status.I = true;

    switch (type) {
        case Interrupt::IRQ:
        case Interrupt::BRK:
            program_counter = readAddr(IRQVector);
            break;
        case Interrupt::NMI:
            program_counter = readAddr(NMIVector);
            break;
    }

    skipCycles += 6;
}

void cpu::setZN(uint8_t value) {
    status.Z = !value;
    status.N = value & 0x80;
}

void cpu::step() {
    cycles += 1;

    if (skipCycles-- > 1){
        return;
    }

    skipCycles = 0;

    if (status.pendingNMI) {
        InterruptSeq(NMI);
        status.pendingNMI = false;
        status.pendingIRQ = false;
        return;
    } else if (status.pendingIRQ) {
        InterruptSeq(IRQ);
        status.pendingNMI = false;
        status.pendingIRQ = false;
        return;
    }

    uint16_t opcode = Bus->read(program_counter++);

    auto CycleLength = OperationCycles[opcode];

    if (CycleLength && (execute(opcode) || executeBranch(opcode) || executeType1(opcode) || executeType2(opcode) || executeType0(opcode))) {
        skipCycles += CycleLength;
    } else {
        std::cout << "[NEMU] ERROR: Unknown Opcode : " << std::hex << opcode << std::endl;
    }   
}

void cpu::interrupt(Interrupt type) {
    switch (type) {
    case Interrupt::NMI:
        status.pendingNMI = true;
        break;

    case Interrupt::IRQ:
        status.pendingIRQ = true;
        break;

    default:
        break;
    }
}

bool cpu::execute(uint8_t opcode) {
    switch (opcode) {
            case 0xEA: // NOP
                NOP();
                break;
            case 0x00: // BRK
                BRK();
                break;
            case 0x20: // JSR
                JSR();
                break;
            case 0x60: // RTS
                RTS();
                break;
            case 0x40: // RTI
                RTI();
                break;
            case 0x4C: // JMP
                JMP();
                break;
            case 0x6C: // JMPI
                JMPI();
                break;
            case 0x08: // PHP
                PHP();
                break;
            case 0x28: // PLP
                PLP();
                break;
            case 0x48: // PHA
                PHA();
                break;
            case 0x68: // PLA
                PLA();
                break;
            case 0x88: // DEY
                DEY();
                break;
            case 0xCA: // DEX
                DEX();
                break;
            case 0xa8: // TAY
                TAY();
                break;
            case 0xc8: // INY
                INY();
                break;
            case 0xE8: // INX
                INX();
                break;
            case 0x18: // CLC
                CLC();
                break;
            case 0x38: // SEC
                SEC();
                break;
            case 0x58: // CLI
                CLI();
                break;
            case 0x78: // SEI
                SEI();
                break;
            case 0xD8: // CLD
                CLD();
                break;
            case 0xF8: // SED
                SED();
                break;
            case 0x98: // TYA
                TYA();
                break;
            case 0xB8: // CLV
                CLV();
                break;
            case 0x8A: // TXA
                TXA();
                break;
            case 0x9A: // TXS
                TXS();
                break;
            case 0xAA: // TAX
                TAX();
                break;
            case 0xBA: // TSX
                TSX();
                break;
            default:
                return false;
        };
        return true;
    }