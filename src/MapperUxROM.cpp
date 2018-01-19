#include "../include/MapperUxROM.h"
#include "../include/TeeLog.hpp"

namespace ANNESE {

    MapperUxROM::MapperUxROM(std::unique_ptr<Cartridge> &&cartridge)
            : Mapper(std::move(cartridge)) {
        if (mCartridge->VROM().empty()) {
            mUseCharacterRAM = true;
            mCharacterRAM.resize(0x2000);
            Log(Debug) << "Using character RAM" << std::endl;
        } else {
            mUseCharacterRAM = false;
        }

        mLastBankPtr = &mCartridge->ROM()[mCartridge->ROM().size() - 0x4000]; //last - 16KB
    }

    void MapperUxROM::writePRG(Address addr, Byte value) {
        mSelectPRG = value;
    }

    Byte MapperUxROM::readPRG(Address addr) const {
        return *getPagePtr(addr);
    }

    void MapperUxROM::writeCHR(Address addr, Byte value) {
        if (mUseCharacterRAM) {
            mCharacterRAM.at(addr) = value;
        } else {
            Log(Debug) << "Read-only CHR memory write attempt at " << std::hex << addr << std::endl;
        }
    }

    Byte MapperUxROM::readCHR(Address addr) const {
        if (mUseCharacterRAM)
            return mCharacterRAM.at(addr);
        else
            return mCartridge->VROM().at(addr);
    }

    const Byte *MapperUxROM::getPagePtr(Address addr) const {
        if (addr < 0xc000) {
            return &mCartridge->ROM().at((addr - 0x8000u) & 0x3fffu | (mSelectPRG << 14));
        } else {
            return &mLastBankPtr[addr & 0x3fff];
        }
    }
}