#include "../include/MapperCNROM.h"
#include "../include/TeeLog.hpp"

namespace ANNESE {

    MapperCNROM::MapperCNROM(std::unique_ptr<Cartridge> &&cartridge)
            : Mapper(std::move(cartridge)) {
        mOneBank = mCartridge->ROM().size() == 0x4000;
    }

    void MapperCNROM::writePRG(Address addr, Byte value) {
        mSelectCHR = value & 0x3_a;
    }

    Byte MapperCNROM::readPRG(Address addr) const {
        return *getPagePtr(addr);
    }

    void MapperCNROM::writeCHR(Address addr, Byte value) {
        Log(Debug) << "Read only CHR memory attempt at" << std::hex << addr << "with value " << value << std::endl;
    }

    Byte MapperCNROM::readCHR(Address addr) const {
        return mCartridge->VROM().at(addr | mSelectCHR << 13);
    }

    const Byte *MapperCNROM::getPagePtr(Address addr) const {
        if (!mOneBank) {
            return &mCartridge->ROM().at(addr - 0x8000_a);
        } else {
            return &mCartridge->ROM().at((addr - 0x8000_a) & 0x3fff_a);
        }
    }
}