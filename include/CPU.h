#pragma once


#include <memory>
#include <bitset>
#include "MainBus.h"
#include "CPUOpcodes.h"

namespace ANNESE {
    class CPU {
    public:
        enum class Interruption {
            IRQ,
            NMI,
            BRK
        };

        explicit CPU(std::shared_ptr<MainBus> mainBus);

        virtual ~CPU() = default;

        void reset();

        void reset(Address startAddr);

        void interrupt(Interruption inter);

        void skipDMACycles() {
            mSkipCycles += 513; // 256 read + 256 write + 1 dummy read
            mSkipCycles += (mCycles & 1);   // +1 if on odd cycle
        }

        void step();

    protected:
        void execute(Operation op, AddressingMode mode);

        void executeBranch(Operation op);

        void execute1(Operation op, AddressingMode mode);

        Address readAddress(Address addr);

        void pushToStack(Byte value);

        Byte pullFromStack();

        void setPageCrossed(Address a, Address b, CycleLength inc = 1);

        void setZN(Byte value);

        std::shared_ptr<MainBus> mMainBus;

        CycleLength mSkipCycles;

        CycleLength mCycles;

        Address mRegPC;

        Byte mRegSP;

        Byte mRegA;

        Byte mRegX;

        Byte mRegY;

        std::bitset<8> mFlags;
    };

    class OpcodeDecoder {
    public:
        OpcodeDecoder() = delete;

        static std::tuple<Operation, AddressingMode, CycleLength> Decode(const Byte opcode);
    };
}