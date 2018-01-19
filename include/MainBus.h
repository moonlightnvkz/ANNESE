#pragma once

#include <memory>
#include <functional>
#include "Utility.h"
#include "Mapper.h"

namespace ANNESE {
    enum class IORegisters : Address {
        PPUCtrl = 0x2000,
        PPUMask,
        PPUStatus,
        OAMAddr,
        OAMData,
        PPUScroll,
        PPUAddr,
        PPUData,
        OAMDMA = 0x4014,
        Joy1 = 0x4016,
        Joy2 = 0x4017,
    };

    class MainBus {
    public:
        MainBus();

        virtual ~MainBus() = default;

        Byte read(Address addr);

        void write(Address addr, Byte value);

        bool setMapper(std::shared_ptr<Mapper> mapper);

        MainBus &setWriteCallback(IORegisters reg, std::function<void(Byte)> cb) {
            mWriteCallbacks[reg] = std::move(cb);
            return *this;
        }

        MainBus &setReadCallback(IORegisters reg, std::function<Byte(void)> cb) {
            mReadCallbacks[reg] = std::move(cb);
            return *this;
        }

        const Byte *getPagePtr(Byte page);

    protected:
        std::vector<Byte> mRAM;

        std::vector<Byte> mExtRAM;

        std::shared_ptr<Mapper> mMapper;

        std::unordered_map<IORegisters, std::function<void(Byte)>> mWriteCallbacks;

        std::unordered_map<IORegisters, std::function<Byte(void)>> mReadCallbacks;

        enum class MemoryMap : Address{
            RAM = 0x0,
            PPU = 0x2000,
            APU = 0x4000,
            NotUsed = 0x4018,
            ExpansionROM = 0x5000,
            SRAM = 0x6000,
            PRG = 0x8000,
        };
    };
}