#include "../include/MapperNROM.h"
#include "../include/TeeLog.hpp"

#include <utility>

namespace ANNESE {
    MapperNROM::MapperNROM(std::unique_ptr<Cartridge> cartridge)
            : Mapper(std::move(cartridge)) {
        mOneBank = mCartridge->ROM().size() == 0x4000;
        mUsesCHRRAM = mCartridge->VROM().empty();
        if (mUsesCHRRAM) {
            mCHRRAM.resize(0x2000);
            Log(Debug) << "Uses character RAM" << std::endl;
        }
    }

    Byte MapperNROM::readPRG(Address addr) const {
        Address address = static_cast<Address>(mOneBank ? (addr - 0x8000) & 0x3fff : (addr - 0x8000));
        return mCartridge->ROM().at(address);
    }

    void MapperNROM::writePRG(Address addr, Byte value) {
        Log(Debug) << "ROM memory write attempt at " << addr << " to set " << +value << std::endl;
    }

    Byte MapperNROM::readCHR(Address addr) const {
        return mUsesCHRRAM ? mCHRRAM.at(addr) : mCartridge->VROM().at(addr);
    }

    void MapperNROM::writeCHR(Address addr, Byte value) {
        if (mUsesCHRRAM) {
            mCHRRAM.at(addr) = value;
        } else {
            Log(Debug) << "Read-only CHR memory write attempt at " << std::hex << addr << std::endl;
        }
    }

    const Byte *MapperNROM::getPagePtr(Address addr) const {
        Address address = mOneBank ? (addr - 0x8000_a) & 0x3fff_a : (addr - 0x8000_a);
        return &mCartridge->ROM().at(address);
    }


}
