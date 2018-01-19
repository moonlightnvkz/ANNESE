#pragma once

#include "MainBus.h"

namespace ANNESE
{
    constexpr const int InstructionModeMask = 0x3;

    constexpr const int OperationMask = 0xe0;

    constexpr const int OperationShift = 5;

    constexpr const int AddressingModeMask = 0x1c;

    constexpr const int AddressingModeShift = 2;

    constexpr const int BranchInstructionMask = 0x1f;

    constexpr const int BranchInstructionMaskResult = 0x10;

    constexpr const int BranchConditionBit = 5;

    constexpr const int BranchOnFlagShift = 6;

    constexpr const int NMIVector = 0xfffa;

    constexpr const int ResetVector = 0xfffc;

    constexpr const int IRQVector = 0xfffe;

    enum class Operation {
        BIT,
        STY,
        LDY,
        CPY,
        CPX,

        ORA,
        AND,
        EOR,
        ADC,
        STA,
        LDA,
        CMP,
        SBC,

        ASL,
        ROL,
        LSR,
        ROR,
        STX,
        LDX,
        DEC,
        INC,

        NOP,
        BRK,
        JSR,
        RTI,
        RTS,

        JMP ,
        JMPI,

        PHP,
        PLP,
        PHA,
        PLA,

        DEY,
        DEX,
        TAY,
        INY,
        INX,

        CLC,
        SEC,
        CLI,
        SEI,
        TYA,
        CLV,
        CLD,
        SED,

        TXA,
        TXS,
        TAX,
        TSX,

        BCC,
        BCS,
        BEQ,
        BMI,
        BNE,
        BPL,
        BVC,
        BVS
    };

    /// Addressing mode or branch flag
    enum class AddressingMode {
        None,
        IndexedIndirectX,
        ZeroPage,
        Immediate,
        Absolute,
        IndirectY,
        IndexedX,
        AbsoluteY,
        AbsoluteX,
        Accumulator,
        Indexed,
        AbsoluteIndexed,
        Relative
    };

    enum class BranchOnFlag_ : Byte {
        Negative,
        Overflow,
        Carry,
        Zero
    };

    enum class Operation0_ : Byte {
        BIT  = 1,
        STY  = 4,
        LDY,
        CPY,
        CPX,
    };

    enum class Operation1_ : Byte {
        ORA,
        AND,
        EOR,
        ADC,
        STA,
        LDA,
        CMP,
        SBC,
    };

    enum class AddressingMode1_ : Byte {
        IndexedIndirectX,
        ZeroPage,
        Immediate,
        Absolute,
        IndirectY,
        IndexedX,
        AbsoluteY,
        AbsoluteX,
    };

    enum class Operation2_ : Byte {
        ASL,
        ROL,
        LSR,
        ROR,
        STX,
        LDX,
        DEC,
        INC,
    };

    enum class AddressingMode2_ : Byte{
        Immediate,
        ZeroPage,
        Accumulator,
        Absolute,
        Indexed         = 5,
        AbsoluteIndexed = 7,
    };

    enum class OperationImplied_ : Byte {
        NOP = 0xea,
        BRK = 0x00,
        JSR = 0x20,
        RTI = 0x40,
        RTS = 0x60,

        JMP  = 0x4C,
        JMPI = 0x6C, //JMP Indirect

        PHP = 0x08,
        PLP = 0x28,
        PHA = 0x48,
        PLA = 0x68,

        DEY = 0x88,
        DEX = 0xca,
        TAY = 0xa8,
        INY = 0xc8,
        INX = 0xe8,

        CLC = 0x18,
        SEC = 0x38,
        CLI = 0x58,
        SEI = 0x78,
        TYA = 0x98,
        CLV = 0xb8,
        CLD = 0xd8,
        SED = 0xf8,

        TXA = 0x8a,
        TXS = 0x9a,
        TAX = 0xaa,
        TSX = 0xba,
    };

    //0 implies unused opcode
    constexpr const CycleLength OperationCyclesAmount_[0x100] {
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
};