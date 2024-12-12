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
    if (status.I && type != NMI && type != BRK){
        return;
    }

    if (type == BRK) {
        program_counter += 1;
    }

    pushStack(program_counter >> 8);
    pushStack(program_counter);

    uint8_t flags = status.N << 7 | status.V << 6 | 1 << 5 | (type == BRK) << 4 | status.D << 3 | status.I << 2 | status.Z << 1 | status.C;
    pushStack(flags);

    status.I = true;

    switch (type) {
        case IRQ:
        case BRK:
            program_counter = readAddr(IRQVector);
            break;
        case NMI:
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
            case NOP:
                break;
            case BRK:
                interruptSequence(BRK_);
                break;
            case JSR:
                //Push address of next instruction - 1, thus r_PC + 1 instead of r_PC + 2
                //since r_PC and r_PC + 1 are address of subroutine
                pushStack(static_cast<Byte>((r_PC + 1) >> 8));
                pushStack(static_cast<Byte>(r_PC + 1));
                r_PC = readAddress(r_PC);
                break;
            case RTS:
                r_PC = pullStack();
                r_PC |= pullStack() << 8;
                ++r_PC;
                break;
            case RTI:
                {
                    Byte flags = pullStack();
                    f_N = flags & 0x80;
                    f_V = flags & 0x40;
                    f_D = flags & 0x8;
                    f_I = flags & 0x4;
                    f_Z = flags & 0x2;
                    f_C = flags & 0x1;
                }
                r_PC = pullStack();
                r_PC |= pullStack() << 8;
                break;
            case JMP:
                r_PC = readAddress(r_PC);
                break;
            case JMPI:
                {
                    Address location = readAddress(r_PC);
                    //6502 has a bug such that the when the vector of anindirect address begins at the last byte of a page,
                    //the second byte is fetched from the beginning of that page rather than the beginning of the next
                    //Recreating here:
                    Address Page = location & 0xff00;
                    r_PC = m_bus.read(location) |
                           m_bus.read(Page | ((location + 1) & 0xff)) << 8;
                }
                break;
            case PHP:
                {
                    Byte flags = f_N << 7 |
                                 f_V << 6 |
                                   1 << 5 | //supposed to always be 1
                                   1 << 4 | //PHP pushes with the B flag as 1, no matter what
                                 f_D << 3 |
                                 f_I << 2 |
                                 f_Z << 1 |
                                 f_C;
                    pushStack(flags);
                }
                break;
            case PLP:
                {
                    Byte flags = pullStack();
                    f_N = flags & 0x80;
                    f_V = flags & 0x40;
                    f_D = flags & 0x8;
                    f_I = flags & 0x4;
                    f_Z = flags & 0x2;
                    f_C = flags & 0x1;
                }
                break;
            case PHA:
                pushStack(r_A);
                break;
            case PLA:
                r_A = pullStack();
                setZN(r_A);
                break;
            case DEY:
                --r_Y;
                setZN(r_Y);
                break;
            case DEX:
                --r_X;
                setZN(r_X);
                break;
            case TAY:
                r_Y = r_A;
                setZN(r_Y);
                break;
            case INY:
                ++r_Y;
                setZN(r_Y);
                break;
            case INX:
                ++r_X;
                setZN(r_X);
                break;
            case CLC:
                f_C = false;
                break;
            case SEC:
                f_C = true;
                break;
            case CLI:
                f_I = false;
                break;
            case SEI:
                f_I = true;
                break;
            case CLD:
                f_D = false;
                break;
            case SED:
                f_D = true;
                break;
            case TYA:
                r_A = r_Y;
                setZN(r_A);
                break;
            case CLV:
                f_V = false;
                break;
            case TXA:
                r_A = r_X;
                setZN(r_A);
                break;
            case TXS:
                r_SP = r_X;
                break;
            case TAX:
                r_X = r_A;
                setZN(r_X);
                break;
            case TSX:
                r_X = r_SP;
                setZN(r_X);
                break;
            default:
                return false;
        };
        return true;
    }