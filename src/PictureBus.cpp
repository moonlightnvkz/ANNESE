#include <cassert>
#include "../include/PictureBus.h"
#include "../include/TeeLog.hpp"

/*
   CHRROM = 0x0,
   VRAM0 = 0x2000,
   VRAM1 = 0x2400,
   VRAM2 = 0x2800,
   VRAM3 = 0x2C00,
   VRAMMirror = 0x3000,
   PaletteBG = 0x3f00,
   PaletteSP = 0x3f10,
   NotUsed = 0x3f20,
 */

#define IN_CHRROM(address)         (address) < static_cast<Address>(MemoryMap::VRAM0)
#define IN_VRAM0(address)          (address) < static_cast<Address>(MemoryMap::VRAM1)
#define IN_VRAM1(address)          (address) < static_cast<Address>(MemoryMap::VRAM2)
#define IN_VRAM2(address)          (address) < static_cast<Address>(MemoryMap::VRAM3)
#define IN_VRAM3(address)          (address) < static_cast<Address>(MemoryMap::VRAMMirror)
#define IN_VRAMMirror(address)     (address) < static_cast<Address>(MemoryMap::PaletteBG)
#define IN_PaletteBG(address)      (address) < static_cast<Address>(MemoryMap::PaletteSP)
#define IN_PaletteSP(address)      (address) < static_cast<Address>(MemoryMap::NotUsed)


namespace ANNESE {
    PictureBus::PictureBus()
            : mRAM(0x800), mPalette(0x20){
    }

    Byte PictureBus::read(Address addr) const {
        if (IN_CHRROM(addr)) {
            return mMapper->readCHR(addr);
        }
        Address rel = addr & Address(0x3ff);
        if (IN_VRAM0(addr)) {
            return mRAM[mVRAM0 + rel];
        }
        if (IN_VRAM1(addr)) {
            return mRAM[mVRAM1 + rel];
        }
        if (IN_VRAM2(addr)) {
            return mRAM[mVRAM2 + rel];
        }
        if (IN_VRAM3(addr)) {
            return mRAM[mVRAM3 + rel];
        }
        assert(!IN_VRAMMirror(addr));
        if (IN_PaletteSP(addr) | IN_PaletteBG(addr)) {
            return mPalette.at(addr & Address(0x1f));
        }
        return 0;
    }

    void PictureBus::write(Address addr, Byte value) {
        Address rel = addr & Address(0x3ff);
        if (IN_CHRROM(addr)) {
            mMapper->writeCHR(addr, value);
        } else if (IN_VRAM0(addr)) {
            mRAM[mVRAM0 + rel] = value;
        } else if (IN_VRAM1(addr)) {
            mRAM[mVRAM1 + rel] = value;
        } else if (IN_VRAM2(addr)) {
            mRAM[mVRAM2 + rel] = value;
        } else if (IN_VRAM3(addr)) {
            mRAM[mVRAM3 + rel] = value;
        } else if (IN_VRAMMirror(addr)) {
            assert(false);
        } else if (IN_PaletteSP(addr) | IN_PaletteBG(addr)) {
            if (addr == static_cast<Address>(MemoryMap::PaletteSP)) {
                mPalette[0] = value;
            } else {
                mPalette.at(addr & Address(0x1f)) = value;
            }
        } else {
            assert(false);
        }
    }

    bool PictureBus::setMapper(std::shared_ptr<Mapper> mapper) {
        if (!mapper) {
            return false;
        }
        mMapper = mapper;
        updateMirroring();
        return true;
    }

    Byte PictureBus::readPalette(Byte paletteAddr) {
        return mPalette.at(paletteAddr);
    }

    void PictureBus::updateMirroring() {
        switch (mMapper->nameTableMirroring()) {
            case Mapper::NameTableMirroring::Horizontal:
                mVRAM0 = mVRAM1 = 0;
                mVRAM2 = mVRAM3 = 0x400;
                Log(Debug) << "Horizontal mirroring set" << std::endl;
                break;
            case Mapper::NameTableMirroring::Vertical:
                mVRAM0 = mVRAM2 = 0;
                mVRAM1 = mVRAM3 = 0x400;
                Log(Debug) << "Vertical mirroring set" << std::endl;
                break;
            case Mapper::NameTableMirroring::OneScreenLower:
                mVRAM0 = mVRAM1 = mVRAM2 = mVRAM3 = 0;
                Log(Debug) << "One Screen Lower mirroring set" << std::endl;
                break;
            case Mapper::NameTableMirroring::OneScreenHigher:
                mVRAM0 = mVRAM1 = mVRAM2 = mVRAM3 = 0x400;
                Log(Debug) << "One Screen Higher mirroring set" << std::endl;
                break;
            default:
                mVRAM0 = mVRAM1 = mVRAM2 = mVRAM3 = 0;
                Log(Error) << "Unsupported Name Table mirroring: "
                           << static_cast<Byte>(mMapper->nameTableMirroring()) << std::endl;
        }
    }
}