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

uint16_t cpu::readAddr(uint16_t addr) {
    return Bus->read(addr) | Bus->read(addr + 1) << 8;
}

void cpu::pushStack(uint8_t val) {
    Bus->write(0x100 | stack_pointer, val);
    --stack_pointer;
}

uint8_t cpu::pullStack() {
    uint8_t ss = Bus->read(0x100 | ++stack_pointer); 
    return ss;
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

void cpu::reset() {
    reset(readAddr(ResetVector));
}

void cpu::step() {
    ++cycles;

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

    uint8_t opcode = Bus->read(program_counter++);
    auto CycleLength = OperationCycles[opcode];

    if (CycleLength && (execute(opcode) || executeBranch(opcode) || executeType1(opcode) || executeType2(opcode) || executeType0(opcode))) {
        skipCycles += CycleLength;
    } else {
        currentINSTR = "UNKN";
        //std::cout << "[NEMU] ERROR: Unknown Opcode : " << std::hex << opcode << std::endl;
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

void cpu::setPageCrossed(uint16_t a, uint16_t b, int inc) {
    if ((a & 0xff00) != (b & 0xff00)) {
        skipCycles += inc;
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

bool cpu::executeBranch(uint8_t opcode) {
    if ((opcode & 0x1F) == 0x10) {
        bool branch = opcode & 0x20;

        switch (opcode >> 6) {
            case Negative:
                branch = !(branch ^ status.N);
                break;
            case Overflow:
                branch = !(branch ^ status.V);
                break;
            case Carry:
                branch = !(branch ^ status.C);
                break;
            case Zero:
                branch = !(branch ^ status.Z);
                break;
            default:
                return false;
        }

        if (branch) {
            BNH();
        } else {
            ++program_counter;
        }
        return true;
    }
    return false;
}

bool cpu::executeType1(uint8_t opcode) {
    if ((opcode & 0x3) == 0x1) {
        uint16_t location = 0;
        auto op = (opcode & 0xE0) >> 5;
        switch ((opcode & 0x1C) >> 2) {
            case 0: { // Indexed Indirect X
                uint8_t zero_addr = x_reg + Bus->read(program_counter++);
                location = Bus->read(zero_addr & 0xff) | Bus->read((zero_addr + 1) & 0xff) << 8;
            }
                break;
            case 1: // ZeroPage
                location = Bus->read(program_counter++);
                break;
            case 2: // Immediate
                location = program_counter++;
                break;
            case 3: // Absolute
                location = readAddr(program_counter);
                program_counter += 2;
                break;
            case 4: { // Indirect Y
                uint8_t zero_addr = Bus->read(program_counter++);
                location = Bus->read(zero_addr & 0xff) | Bus->read((zero_addr + 1) & 0xff) << 8;
                if (op != 4)
                    setPageCrossed(location, location + y_reg);
                location += y_reg;
                }
                break;
            case 5: // Indexed X
                location = (Bus->read(program_counter++) + x_reg) & 0xff;
                break;
            case 6: // AbsoluteY
                location = readAddr(program_counter);
                program_counter += 2;
                if (op != 4)
                    setPageCrossed(location, location + y_reg);
                location += y_reg;
                break;
            case 7: // AbsoluteX
                location = readAddr(program_counter);
                program_counter += 2;
                if (op != 4)
                    setPageCrossed(location, location + x_reg);
                location += x_reg;
                break;
            default:
                return false;
        }

        switch (op) {
            case 0x0: // ORA
                ORA(location);
                break;
            case 0x1: // AND
                AND(location);
                break;
            case 0x2: // EOR
                EOR(location);
                break;
            case 0x3: // ADC
                ADC(location);
                break;
            case 0x4: // STA
                STA(location);
                break;
            case 0x5: // LDA
                LDA(location);
                break;
            case 0x7: // SBC
                SBC(location);
                break;
            case 0x6: // CMP
                CMP(location);
                break;
            default:
                return false;
        }
        return true;
    }
    return false;
}

bool cpu::executeType2(uint8_t opcode) {
    if ((opcode & 0x3) == 2) {
        uint16_t location = 0;
        auto op = (opcode & 0xE0) >> 5;
        auto addr_mode = (opcode & 0x1C) >> 2;
        switch (addr_mode) {
            case 0x0: // Immediate
                location = program_counter++;
                break;
            case 0x1: // ZeroPage
                location = Bus->read(program_counter++);
                break;
            case 0x2: // Accumulator
                break;
            case 0x3: // Absolute
                location = readAddr(program_counter);
                program_counter += 2;
                break;
            case 0x5: // Indexed
            {
                location = Bus->read(program_counter++);
                uint8_t index;
                if (op == 0x5 || op == 0x4) {
                    index = y_reg;
                } else {
                    index = x_reg;
                }
                location = (location + index) & 0xff;
            }
                break;
            case 0x7: // Absolute Indexed
            {
                location = readAddr(program_counter);
                program_counter += 2;
                uint8_t index;
                if (op == 0x5 || op == 0x4) {
                    index = y_reg;
                } else {
                    index = x_reg;
                }
                setPageCrossed(location, location + index);
                location += index;
            }
                break;
            default:
                return false;
        }

        std::uint16_t operand = 0;
        switch (op) {
            case 0x0: // ASL
            case 0x1: // ROL
                ROL(location, op, addr_mode, operand);
                break;
            case 0x2: // LSR
            case 0x3: // ROR
                ROR(location, op, addr_mode, operand);
                break;
            case 0x4: // STX
                STX(location, op, addr_mode, operand);
                break;
            case 0x5: // LDX
                LDX(location, op, addr_mode, operand);
                break;
            case 0x6: // DEC
                DEC(location, op, addr_mode, operand);
                break;
            case 0x7: // INC
                INC(location, op, addr_mode, operand);
                break;
            default:
                return false;
        }
        return true;
    }
    return false;
}

bool cpu::executeType0(uint8_t opcode) {
    if ((opcode & 3) == 0x0) {
        uint16_t location = 0;
        switch ((opcode & 0x1C) >> 2) {
            case 0x0: // Immediate
                location = program_counter++;
                break;
            case 0x1: // ZeroPage
                location = Bus->read(program_counter++);
                break;
            case 0x3: // Absolute
                location = readAddr(program_counter);
                program_counter += 2;
                break;
            case 0x5: // Indexed
                location = (Bus->read(program_counter++) + x_reg) & 0xff;
                break;
            case 0x7: // Absolute Indexed
                location = readAddr(program_counter);
                program_counter += 2;
                setPageCrossed(location, location + x_reg);
                location += x_reg;
                break;
            default:
                return false;
        }
        std::uint16_t operand = 0;
        switch ((opcode & 0xE0) >> 5) {
            case 0x1: // BIT
                BIT(location, operand);
                break;
            case 0x4: // STY
                STY(location, operand);
                break;
            case 0x5: // LDY
                LDY(location, operand);
                break;
            case 0x6: // CPY
                CPY(location, operand);
                break;
            case 0x7: // CPX
                CPX(location, operand);
                break;
            default:
                return false;
        }

        return true;
    }
    return false;
}