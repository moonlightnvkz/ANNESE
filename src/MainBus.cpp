#include <cassert>
#include <utility>
#include "../include/MainBus.h"
#include "../include/TeeLog.hpp"

/*
   RAM = 0x0,
   PPU = 0x2000,
   APU = 0x4000,
   NotUsed = 0x4018,
   ExpansionROM = 0x5000,
   SRAM = 0x6000,
   PRG = 0x8000,
 */

#define IN_RAM(address)          (address) < static_cast<Address>(MemoryMap::PPU)
#define IN_PPU(address)          (address) < static_cast<Address>(MemoryMap::APU)
#define IN_APU(address)          (address) < static_cast<Address>(MemoryMap::NotUsed)
#define IN_NotUsed(address)      (address) < static_cast<Address>(MemoryMap::ExpansionROM)
#define IN_ExpansionROM(address) (address) < static_cast<Address>(MemoryMap::SRAM)
#define IN_SRAM(address)         (address) < static_cast<Address>(MemoryMap::PRG)


namespace ANNESE {
    MainBus::MainBus()
            : mRAM(0x800, 0) {
    }

    Byte MainBus::read(Address addr) {
        using Mem = MemoryMap;
        if (IN_RAM(addr)) {
            return mRAM.at(addr & 0x7ffu);   // User area varies from 0x200 to 0x7ff
        } else if (IN_PPU(addr)) {
            auto it = mReadCallbacks.find(static_cast<IORegisters>(addr & 0x2007));
            if (it != mReadCallbacks.end()) {
                return it->second();
            } else {
                Log(Debug) << "No read callback registered for I/O register at: " << +addr << std::endl;
            }
        } else if (IN_APU(addr)) {    // >0x4014
            auto it = mReadCallbacks.find(static_cast<IORegisters>(addr));
            if (it != mReadCallbacks.end()) {
                return it->second();
            } else {
                static bool once = false;
                if (!once) {
                    Log(Info) << "APU read attempt. APU is unsupported" << std::endl;
                    once = true;
                }
            }
        } else if (IN_NotUsed(addr)) {
//            assert(false);
        } else if (IN_ExpansionROM(addr)) {
            static bool once = false;
            if (!once) {
                Log(Info) << "Expansion ROM read attempt. The ROM is unsupported" << std::endl;
                once = true;
            }
        } else if (IN_SRAM(addr)) {
            if (mMapper->hasExtendedRAM()) {
                return mExtRAM[addr - static_cast<Address>(Mem::SRAM)];
            }
        } else {
            return mMapper->readPRG(addr);
        }
        return 0_b;
    }

    void MainBus::write(Address addr, Byte value) {
        using Mem = MemoryMap;
        if (IN_RAM(addr)) {
            mRAM.at(addr & 0x7ffu) = value;   // User area varies from 0x200 to 0x7ff
        } else if (IN_PPU(addr)) {
            auto it = mWriteCallbacks.find(static_cast<IORegisters>(addr & 0x2007));
            if (it != mWriteCallbacks.end()) {
                it->second(value);
            } else {
                Log(Debug) << "No write callback registered for I/O register at: "
                          << std::hex << addr << std::dec << std::endl;
            }
        } else if (IN_APU(addr)) {  // Unsupported. Maybe temporarily...
            auto it = mWriteCallbacks.find(static_cast<IORegisters>(addr));
            if (it != mWriteCallbacks.end()) {
                it->second(value);
            } else {
                static bool once = false;
                if (!once) {
                    Log(Info) << "APU write attempt. APU is unsupported" << std::endl;
                    once = true;
                }
            }
        } else if (IN_NotUsed(addr)) {
//            assert(false);
        } else if (IN_ExpansionROM(addr)) {
            static bool once = false;
            if (!once) {
                Log(Info) << "Expansion ROM write attempt. The ROM is unsupported" << std::endl;
                once = true;
            }
        } else if (IN_SRAM(addr)) {
            if (mMapper->hasExtendedRAM()) {
                mExtRAM[addr - static_cast<Address>(Mem::SRAM)] = value;
            }
        } else {
            mMapper->writePRG(addr, value);
        }
    }

    bool MainBus::setMapper(std::shared_ptr<Mapper> mapper) {
        if (!mapper) {
            return false;
        }
        mMapper = mapper;
        if (mMapper->hasExtendedRAM()) {
            mExtRAM.resize(0x2000);
        }
        return true;
    }

    const Byte *MainBus::getPagePtr(Byte page) {
        using Mem = MemoryMap;
        Address addr = page << 8;
        if (IN_RAM(addr)) {
            return &mRAM[addr & 0x7ff];
        } else if (IN_SRAM(addr)) {
            if (mMapper->hasExtendedRAM()) {
                return &mExtRAM[addr - static_cast<Address>(Mem::SRAM)];
            }
        }
        Log(Error) << "Attempt to access the page: " << page << std::endl;
        return nullptr;
    }
}