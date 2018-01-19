//
// Created by akarpovskii on 06.10.17.
//

#include <cassert>
#include <iomanip>
#include "../include/CPU.h"
#include "../include/TeeLog.hpp"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCDFAInspection"

#define FlagC mFlags[0]

#define FlagZ mFlags[1]

#define FlagI mFlags[2]

#define FlagD mFlags[3]

#define FlagB mFlags[4]

#define FlagUnused mFlags[5]

#define FlagV mFlags[6]

#define FlagN mFlags[7]

namespace ANNESE {
    CPU::CPU(std::shared_ptr<MainBus> mainBus)
            : mMainBus(std::move(mainBus)) {
    }

    void CPU::reset() {
        reset(readAddress(ResetVector));
    }

    void CPU::reset(Address startAddr) {
        mSkipCycles = mCycles = 0;
        mRegA = mRegX = mRegY = 0;
        FlagI = true;
        FlagC = FlagD = FlagN = FlagV = FlagZ = false;
        FlagUnused = true;
        mRegPC = startAddr;
        mRegSP = 0xfd;  // Documented startup state;
    }

    void CPU::interrupt(CPU::Interruption inter) {
        if (FlagI && inter != Interruption::NMI && inter != Interruption::BRK) {
            return;
        }

        if (inter == Interruption::BRK) {
            ++mRegPC;
        }

        pushToStack(static_cast<Byte>(mRegPC >> 8));
        pushToStack(static_cast<Byte>(mRegPC));
        FlagB = inter == Interruption::BRK;

        pushToStack(static_cast<Byte>(mFlags.to_ulong()));

        FlagI = true;
        switch (inter) {
            case Interruption::IRQ:
                [[fallthrough]]
            case Interruption::BRK:
                mRegPC = readAddress(IRQVector);
                break;
            case Interruption::NMI:
                mRegPC = readAddress(NMIVector);
        }
        mSkipCycles += 7;
    }

    void CPU::step() {
        ++mCycles;

        if (--mSkipCycles > 0) {
            return;
        }

        Byte opcode = mMainBus->read(mRegPC++);

        Operation operation;
        AddressingMode addressingMode;
        CycleLength cycleLength;
        std::tie(operation, addressingMode, cycleLength) = OpcodeDecoder::Decode(opcode);
        execute(operation, addressingMode);
        mSkipCycles += cycleLength;
    }

    Address CPU::readAddress(Address addr) {
        return mMainBus->read(addr) | mMainBus->read(addr + 1_a) << 8;
    }

    void CPU::pushToStack(Byte value) {
        mMainBus->write(0x100_a | mRegSP, value);
        --mRegSP;
    }

    Byte CPU::pullFromStack() {
        return mMainBus->read(0x100_a | ++mRegSP);
    }

    void CPU::setPageCrossed(Address a, Address b, CycleLength inc) {
        if ((a & 0xff00) != (b & 0xff00)) {
            mSkipCycles += inc;
        }
    }

    void CPU::setZN(Byte value) {
        FlagZ = !value;
        FlagN = (value & 0x80) != 0;
    }

    void CPU::execute(Operation op, AddressingMode mode) {
        switch (op) {
            case Operation::BIT: [[fallthrough]]
            case Operation::STY: [[fallthrough]]
            case Operation::LDY: [[fallthrough]]
            case Operation::CPY: [[fallthrough]]
            case Operation::CPX: [[fallthrough]]
            case Operation::ORA: [[fallthrough]]
            case Operation::AND: [[fallthrough]]
            case Operation::EOR: [[fallthrough]]
            case Operation::ADC: [[fallthrough]]
            case Operation::STA: [[fallthrough]]
            case Operation::LDA: [[fallthrough]]
            case Operation::CMP: [[fallthrough]]
            case Operation::SBC: [[fallthrough]]
            case Operation::ASL: [[fallthrough]]
            case Operation::ROL: [[fallthrough]]
            case Operation::LSR: [[fallthrough]]
            case Operation::ROR: [[fallthrough]]
            case Operation::STX: [[fallthrough]]
            case Operation::LDX: [[fallthrough]]
            case Operation::DEC: [[fallthrough]]
            case Operation::INC:
                execute1(op, mode);
                break;
            case Operation::NOP:
                break;
            case Operation::BRK:
                interrupt(Interruption::BRK);
                break;
            case Operation::JSR:
                pushToStack(static_cast<Byte>((mRegPC + 1) >> 8));
                pushToStack(static_cast<Byte>((mRegPC + 1)));
                mRegPC = readAddress(mRegPC);
                break;
            case Operation::RTI: {
                mFlags = pullFromStack();
                mRegPC = pullFromStack();
                mRegPC |= pullFromStack() << 8;
                break;
            }
            case Operation::RTS:
                mRegPC = pullFromStack();
                mRegPC |= pullFromStack() << 8;
                ++mRegPC;
                break;
            case Operation::JMP:
                mRegPC = readAddress(mRegPC);
                break;
            case Operation::JMPI: {
                Address location = readAddress(mRegPC);
                //6502 has a bug such that the when the vector of an indirect address begins at the last byte of a page,
                //the second byte is fetched from the beginning of that page rather than the beginning of the next
                //Recreating here:
                Address page = location & 0xff00_a;
                mRegPC = mMainBus->read(location) |
                         mMainBus->read(page | ((location + 1_a) & 0xff_a)) << 8;
                break;
            }
            case Operation::PHP: {
                FlagB = true;
                pushToStack(static_cast<Byte>(mFlags.to_ulong()));
                break;
            }
            case Operation::PLP:{
                mFlags = pullFromStack();
                break;
            }
            case Operation::PHA:
                pushToStack(mRegA);
                break;
            case Operation::PLA:
                mRegA = pullFromStack();
                setZN(mRegA);
                break;
            case Operation::DEY:
                --mRegY;
                setZN(mRegY);
                break;
            case Operation::DEX:
                --mRegX;
                setZN(mRegX);
                break;
            case Operation::TAY:
                mRegY = mRegA;
                setZN(mRegY);
                break;
            case Operation::INY:
                ++mRegY;
                setZN(mRegY);
                break;
            case Operation::INX:
                ++mRegX;
                setZN(mRegX);
                break;
            case Operation::CLC:
                FlagC = false;
                break;
            case Operation::SEC:
                FlagC = true;
                break;
            case Operation::CLI:
                FlagI = false;
                break;
            case Operation::SEI:
                FlagI = true;
                break;
            case Operation::CLD:
                FlagD = false;
                break;
            case Operation::SED:
                FlagD = true;
                break;
            case Operation::TYA:
                mRegA = mRegY;
                setZN(mRegA);
                break;
            case Operation::CLV:
                FlagV = false;
                break;
            case Operation::TXA:
                mRegA = mRegX;
                setZN(mRegA);
                break;
            case Operation::TXS:
                mRegSP = mRegX;
                break;
            case Operation::TAX:
                mRegX = mRegA;
                setZN(mRegX);
                break;
            case Operation::TSX:
                mRegX = mRegSP;
                setZN(mRegX);
                break;

            case Operation::BCC: [[fallthrough]]
            case Operation::BCS: [[fallthrough]]
            case Operation::BEQ: [[fallthrough]]
            case Operation::BMI: [[fallthrough]]
            case Operation::BPL: [[fallthrough]]
            case Operation::BNE: [[fallthrough]]
            case Operation::BVC: [[fallthrough]]
            case Operation::BVS:
                executeBranch(op);
        }
    }

    void CPU::executeBranch(Operation op) {
        bool branch;
        switch (op) {
            case Operation::BCC:
                branch = FlagC == 0;
                break;
            case Operation::BCS:
                branch = FlagC == 1;
                break;
            case Operation::BNE:
                branch = FlagZ == 0;
                break;
            case Operation::BEQ:
                branch = FlagZ == 1;
                break;
            case Operation::BPL:
                branch = FlagN == 0;
                break;
            case Operation::BMI:
                branch = FlagN == 1;
                break;
            case Operation::BVC:
                branch = FlagV == 0;
                break;
            case Operation::BVS:
                branch = FlagV == 1;
                break;
            default:
                assert(false);
        }
        if (branch) {
            SByte offset = mMainBus->read(mRegPC++);
            ++mSkipCycles;
            Address newPC = mRegPC + offset;
            setPageCrossed(mRegPC, newPC, 2);
            mRegPC = newPC;
        } else {
            ++mRegPC;
        }
    }

    void CPU::execute1(Operation op, AddressingMode mode) {
        Address location = 0;
        switch (mode) {
            case AddressingMode::IndexedIndirectX: {
                Byte zeroAddr = mRegX + mMainBus->read(mRegPC++);
                location = mMainBus->read(zeroAddr & 0xff_a) | mMainBus->read((zeroAddr + 1_a) & 0xff_a) << 8;
                break;
            }
            case AddressingMode::ZeroPage:
                location = mMainBus->read(mRegPC++);
                break;
            case AddressingMode::Immediate:
                location = mRegPC++;
                break;
            case AddressingMode::Absolute:
                location = readAddress(mRegPC);
                mRegPC += 2;
                break;
            case AddressingMode::IndirectY: {
                Byte zeroAddr = mMainBus->read(mRegPC++);
                location = mMainBus->read(zeroAddr & 0xff_a) | mMainBus->read((zeroAddr + 1_a) & 0xff_a) << 8;
                if (op != Operation::STA) {
                    setPageCrossed(location, location + mRegY);
                }
                location += mRegY;
                break;
            }
            case AddressingMode::IndexedX:
                location = (mMainBus->read(mRegPC++) + mRegX) & 0xff_a;
                break;
            case AddressingMode::AbsoluteY:
                location = readAddress(mRegPC);
                mRegPC += 2;
                if (op != Operation::STA) {
                    setPageCrossed(location, location + mRegY);
                }
                location += mRegY;
                break;
            case AddressingMode::AbsoluteX:
                location = readAddress(mRegPC);
                mRegPC += 2;
                if (op != Operation::STA) {
                    setPageCrossed(location, location + mRegX);
                }
                location += mRegX;
                break;
            case AddressingMode::Accumulator:
                break;
            case AddressingMode::Indexed: {
                Byte index = (op == Operation::LDX || op == Operation::STX) ? mRegY : mRegX;
                location = (mMainBus->read(mRegPC++) + index) & Address(0xff);
                break;
            }
            case AddressingMode::AbsoluteIndexed: {
                location = readAddress(mRegPC);
                mRegPC += 2;
                Byte index = (op == Operation::LDX || op == Operation::STX) ? mRegY : mRegX;
                setPageCrossed(location, location + index);
                location += index;
                break;
            }
            default:
                assert(false);
        }

        switch (op) {
            case Operation::ORA:
                mRegA |= mMainBus->read(location);
                setZN(mRegA);
                break;
            case Operation::AND:
                mRegA &= mMainBus->read(location);
                setZN(mRegA);
                break;
            case Operation::EOR:
                mRegA ^= mMainBus->read(location);
                setZN(mRegA);
                break;
            case Operation::ADC: {
                Byte operand = mMainBus->read(location);
                ExtendedByte sum = mRegA + operand + FlagC;
                // Unsigned overflow
                FlagC = (sum & 0x100) != 0;
                // Signed overflow, would only happen if the sign of sum is
                // different from both the operands
                FlagV = ((mRegA ^ sum) & (operand ^ sum) & 0x80) != 0;
                mRegA = static_cast<Byte>(sum);
                setZN(mRegA);
                break;
            }
            case Operation::STA:
                mMainBus->write(location, mRegA);
                break;
            case Operation::LDA:
                mRegA = mMainBus->read(location);
                setZN(mRegA);
                break;
            case Operation::CMP: {
                ExtendedByte diff = mRegA - mMainBus->read(location);
                FlagC = !(diff & 0x100);
                setZN(static_cast<Byte>(diff));
                break;
            }
            case Operation::SBC: {
                ExtendedByte subtrahend = mMainBus->read(location);
                //High carry means "no borrow", thus negate and subtract
                ExtendedByte diff = mRegA - subtrahend - !FlagC;
                // If the ninth bit is 1, the resulting number is negative => borrow => low carry
                FlagC = !(diff & 0x100);
                // Same as ADC, except instead of the subtrahend,
                // substitute with it's one complement
                FlagV = ((mRegA ^ diff) & (~subtrahend ^ diff) & 0x80) != 0;
                mRegA = static_cast<Byte>(diff);
                setZN(static_cast<Byte>(diff));
                break;
            }
            case Operation::ASL:
                [[fallthrough]]
            case Operation::ROL: {
                bool oldC = FlagC;
                bool bit0 = (oldC && (op == Operation::ROL));
                if (mode == AddressingMode::Accumulator) {
                    FlagC = (mRegA & 0x80) != 0;
                    mRegA <<= 1;
                    // If Rotating, set the bit-0 to the the previous carry
                    mRegA = mRegA | bit0;
                    setZN(mRegA);
                } else {
                    ExtendedByte operand = mMainBus->read(location);
                    FlagC = (operand & 0x80) != 0;
                    operand = operand << 1 | bit0;
                    setZN(static_cast<Byte>(operand));
                    mMainBus->write(location, static_cast<Byte>(operand));
                }
                break;
            }
            case Operation::LSR:
                [[fallthrough]]
            case Operation::ROR: {
                bool oldC = FlagC;
                bool bit7 = (oldC && (op == Operation::ROR));
                if (mode == AddressingMode::Accumulator) {
                    FlagC = (mRegA & 1) != 0;
                    mRegA >>= 1;
                    // If Rotating, set the bit-0 to the the previous carry
                    mRegA = mRegA | bit7 << 7;
                    setZN(mRegA);
                } else {
                    ExtendedByte operand = mMainBus->read(location);
                    FlagC = (operand & 1) != 0;
                    operand = operand >> 1 | bit7 << 7;
                    setZN(static_cast<Byte>(operand));
                    mMainBus->write(location, static_cast<Byte>(operand));
                }
                break;
            }
            case Operation::STX:
                mMainBus->write(location, mRegX);
                break;
            case Operation::LDX:
                mRegX = mMainBus->read(location);
                setZN(mRegX);
                break;
            case Operation::DEC: {
                Byte value = mMainBus->read(location) - 1_b;
                setZN(value);
                mMainBus->write(location, value);
                break;
            }
            case Operation::INC: {
                Byte value = mMainBus->read(location) + 1_b;
                setZN(value);
                mMainBus->write(location, value);
                break;
            }
            case Operation::BIT: {
                ExtendedByte operand = mMainBus->read(location);
                FlagZ = !(mRegA & operand);
                FlagV = (operand & 0x40) != 0;
                FlagN = (operand & 0x80) != 0;
                break;
            }
            case Operation::STY:
                mMainBus->write(location, mRegY);
                break;
            case Operation::LDY:
                mRegY = mMainBus->read(location);
                setZN(mRegY);
                break;
            case Operation::CPY: {
                ExtendedByte diff = mRegY - mMainBus->read(location);
                FlagC = !(diff & 0x100);
                setZN(static_cast<Byte>(diff));
                break;
            }
            case Operation::CPX: {
                ExtendedByte diff = mRegX - mMainBus->read(location);
                FlagC = !(diff & 0x100);
                setZN(static_cast<Byte>(diff));
                break;
            }
            default:
                assert(false);
        }
    }

    std::tuple<Operation, AddressingMode, CycleLength> OpcodeDecoder::Decode(const Byte opcode) {
        // Implied operation
        CycleLength cycleLength = OperationCyclesAmount_[opcode];
        using AMode = AddressingMode;
        using Op = Operation;
        using OpIm = OperationImplied_;
        switch (static_cast<OpIm>(opcode)) {
            case OpIm::NOP:  return {Op::NOP,  AMode::None, cycleLength};
            case OpIm::BRK:  return {Op::BRK,  AMode::None, cycleLength};
            case OpIm::JSR:  return {Op::JSR,  AMode::None, cycleLength};
            case OpIm::RTI:  return {Op::RTI,  AMode::None, cycleLength};
            case OpIm::RTS:  return {Op::RTS,  AMode::None, cycleLength};
            case OpIm::JMP:  return {Op::JMP,  AMode::None, cycleLength};
            case OpIm::JMPI: return {Op::JMPI, AMode::None, cycleLength};
            case OpIm::PHP:  return {Op::PHP,  AMode::None, cycleLength};
            case OpIm::PLP:  return {Op::PLP,  AMode::None, cycleLength};
            case OpIm::PHA:  return {Op::PHA,  AMode::None, cycleLength};
            case OpIm::PLA:  return {Op::PLA,  AMode::None, cycleLength};
            case OpIm::DEY:  return {Op::DEY,  AMode::None, cycleLength};
            case OpIm::DEX:  return {Op::DEX,  AMode::None, cycleLength};
            case OpIm::TAY:  return {Op::TAY,  AMode::None, cycleLength};
            case OpIm::INY:  return {Op::INY,  AMode::None, cycleLength};
            case OpIm::INX:  return {Op::INX,  AMode::None, cycleLength};
            case OpIm::CLC:  return {Op::CLC,  AMode::None, cycleLength};
            case OpIm::SEC:  return {Op::SEC,  AMode::None, cycleLength};
            case OpIm::CLI:  return {Op::CLI,  AMode::None, cycleLength};
            case OpIm::SEI:  return {Op::SEI,  AMode::None, cycleLength};
            case OpIm::TYA:  return {Op::TYA,  AMode::None, cycleLength};
            case OpIm::CLV:  return {Op::CLV,  AMode::None, cycleLength};
            case OpIm::CLD:  return {Op::CLD,  AMode::None, cycleLength};
            case OpIm::SED:  return {Op::SED,  AMode::None, cycleLength};
            case OpIm::TXA:  return {Op::TXA,  AMode::None, cycleLength};
            case OpIm::TXS:  return {Op::TXS,  AMode::None, cycleLength};
            case OpIm::TAX:  return {Op::TAX,  AMode::None, cycleLength};
            case OpIm::TSX:  return {Op::TSX,  AMode::None, cycleLength};
        }

        if ((opcode & BranchInstructionMask) == BranchInstructionMaskResult) {
            bool condition = (opcode & (1 << BranchConditionBit)) != 0;
            switch(static_cast<BranchOnFlag_>(opcode >> BranchOnFlagShift)) {
                case BranchOnFlag_::Negative:
                    return {condition ? Op::BMI : Op::BPL, AMode::Relative, cycleLength};
                case BranchOnFlag_::Overflow:
                    return {condition ? Op::BVS : Op::BVC, AMode::Relative, cycleLength};
                case BranchOnFlag_::Carry:
                    return {condition ? Op::BCS : Op::BCC, AMode::Relative, cycleLength};
                case BranchOnFlag_::Zero:
                    return {condition ? Op::BEQ : Op::BNE, AMode::Relative, cycleLength};
            }
        }

        if ((opcode & InstructionModeMask) == 0x1) {
            AddressingMode addressingMode;
            switch(static_cast<AddressingMode1_>((opcode & AddressingModeMask) >> AddressingModeShift)) {
                case AddressingMode1_::IndexedIndirectX: addressingMode = AMode::IndexedIndirectX; break;
                case AddressingMode1_::ZeroPage: addressingMode = AMode::ZeroPage; break;
                case AddressingMode1_::Immediate: addressingMode = AMode::Immediate; break;
                case AddressingMode1_::Absolute: addressingMode = AMode::Absolute; break;
                case AddressingMode1_::IndirectY: addressingMode = AMode::IndirectY; break;
                case AddressingMode1_::IndexedX: addressingMode = AMode::IndexedX; break;
                case AddressingMode1_::AbsoluteY: addressingMode = AMode::AbsoluteY; break;
                case AddressingMode1_::AbsoluteX: addressingMode = AMode::AbsoluteX; break;
            }
            switch(static_cast<Operation1_>((opcode & OperationMask) >> OperationShift)) {
                case Operation1_::ORA: return {Op::ORA, addressingMode, cycleLength};
                case Operation1_::AND: return {Op::AND, addressingMode, cycleLength};
                case Operation1_::EOR: return {Op::EOR, addressingMode, cycleLength};
                case Operation1_::ADC: return {Op::ADC, addressingMode, cycleLength};
                case Operation1_::STA: return {Op::STA, addressingMode, cycleLength};
                case Operation1_::LDA: return {Op::LDA, addressingMode, cycleLength};
                case Operation1_::CMP: return {Op::CMP, addressingMode, cycleLength};
                case Operation1_::SBC: return {Op::SBC, addressingMode, cycleLength};
            }
        }

        if ((opcode & InstructionModeMask) == 0x2) {
            AddressingMode addressingMode;
            switch(static_cast<AddressingMode2_>((opcode & AddressingModeMask) >> AddressingModeShift)) {
                case AddressingMode2_::Immediate: addressingMode = AMode::Immediate; break;
                case AddressingMode2_::ZeroPage: addressingMode = AMode::ZeroPage; break;
                case AddressingMode2_::Accumulator: addressingMode = AMode::Accumulator; break;
                case AddressingMode2_::Absolute: addressingMode = AMode::Absolute; break;
                case AddressingMode2_::Indexed: addressingMode = AMode::Indexed; break;
                case AddressingMode2_::AbsoluteIndexed: addressingMode = AMode::AbsoluteIndexed; break;
            }
            switch(static_cast<Operation2_>((opcode & OperationMask) >> OperationShift)) {
                case Operation2_::ASL: return {Op::ASL, addressingMode, cycleLength};
                case Operation2_::ROL: return {Op::ROL, addressingMode, cycleLength};
                case Operation2_::LSR: return {Op::LSR, addressingMode, cycleLength};
                case Operation2_::ROR: return {Op::ROR, addressingMode, cycleLength};
                case Operation2_::STX: return {Op::STX, addressingMode, cycleLength};
                case Operation2_::LDX: return {Op::LDX, addressingMode, cycleLength};
                case Operation2_::DEC: return {Op::DEC, addressingMode, cycleLength};
                case Operation2_::INC: return {Op::INC, addressingMode, cycleLength};
            }
        }

        if ((opcode & InstructionModeMask) == 0x0) {
            AddressingMode addressingMode;
            switch(static_cast<AddressingMode2_>((opcode & AddressingModeMask) >> AddressingModeShift)) {
                case AddressingMode2_::Immediate: addressingMode = AMode::Immediate; break;
                case AddressingMode2_::ZeroPage: addressingMode = AMode::ZeroPage; break;
                case AddressingMode2_::Accumulator: addressingMode = AMode::Accumulator; break;
                case AddressingMode2_::Absolute: addressingMode = AMode::Absolute; break;
                case AddressingMode2_::Indexed: addressingMode = AMode::Indexed; break;
                case AddressingMode2_::AbsoluteIndexed: addressingMode = AMode::AbsoluteIndexed; break;
            }
            switch(static_cast<Operation0_>((opcode & OperationMask) >> OperationShift)) {
                case Operation0_::BIT: return {Op::BIT, addressingMode, cycleLength};
                case Operation0_::STY: return {Op::STY, addressingMode, cycleLength};
                case Operation0_::LDY: return {Op::LDY, addressingMode, cycleLength};
                case Operation0_::CPY: return {Op::CPY, addressingMode, cycleLength};
                case Operation0_::CPX: return {Op::CPX, addressingMode, cycleLength};
            }
        }
        assert(false);
    }
}
#pragma clang diagnostic pop