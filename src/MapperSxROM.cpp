#include "../include/MapperSxROM.h"
#include "../include/TeeLog.hpp"

#include <utility>
#include <cassert>

namespace ANNESE {

    MapperSxROM::MapperSxROM(std::unique_ptr<Cartridge> cartridge, std::function<void(void)> mirroringCallback)
            : Mapper(std::move(cartridge)), mMirroringCallback(std::move(mirroringCallback)) {
        auto &vrom = mCartridge->VROM();
        if (vrom.empty()) {
            mUseCharacterRAM = true;
            mCharacterRAM.resize(0x2000);
            Log(Debug) << "Use character RAM" << std::endl;
        } else {
            mUseCharacterRAM = false;
            mBankCHR0 = mBankCHR1 = vrom.data();
            Log(Debug) << "Use character ROM" << std::endl;
        }

        auto &rom = mCartridge->ROM();
        mBankPRG0 = rom.data();
        mBankPRG1 = rom.data() + rom.size() - 0x4000;
    }

    void MapperSxROM::writePRG(Address addr, Byte value) {
        if (!(value & 0x80)) {
            mRegTemp = (mRegTemp >> 1_b) | ((value & 1_b) << 4_b);
            ++mWriteCount;

            if (mWriteCount == 5) {
                auto *vromData = mCartridge->VROM().data();
                if (addr < 0xa000) {
                    switch (mRegTemp & 0x3) {
                        case 0:
                            mMirroring = NameTableMirroring::OneScreenLower;
                            break;
                        case 1:
                            mMirroring = NameTableMirroring::OneScreenHigher;
                            break;
                        case 2:
                            mMirroring = NameTableMirroring::Vertical;
                            break;
                        case 3:
                            mMirroring = NameTableMirroring::Horizontal;
                            break;
                        default:
                            assert(false);
                    }
                    mMirroringCallback();

                    mModeCHR = (mRegTemp & 0x10_b) >> 4_b;
                    mModePRG = (mRegTemp & 0xc_b) >> 2_b;
                    updatePRGPointers();

                    if (mModeCHR == 0) {
                        mBankCHR0 = vromData + 0x1000 * (mRegCHR0| 1); //ignore last bit
                        mBankCHR1 = mBankCHR0 + 0x1000;
                    } else {
                        mBankCHR0 = vromData + 0x1000 * mRegCHR0;
                        mBankCHR1 = vromData + 0x1000 * mRegCHR1;
                    }
                } else if (addr < 0xc000) {
                    mRegCHR0 = mRegTemp;
                    mBankCHR0 = vromData + 0x1000 * (mRegTemp | (1 - mModeCHR));
                    if (mModeCHR == 0) {
                        mBankCHR1 = mBankCHR0 + 0x1000;
                    }
                } else if (addr < 0xe000) {
                    mRegCHR1 = mRegTemp;
                    if (mModeCHR == 1) {
                        mBankCHR1 = vromData + 0x1000 * mRegTemp;
                    }
                } else {
                    if ((mRegTemp & 0x10) == 0x10) {
                        Log(Debug) << "PRG-RAM activated" << std::endl;
                    }

                    mRegTemp &= 0xf;
                    mRegPRG = mRegTemp;
                    updatePRGPointers();
                }
                mRegTemp = 0;
                mWriteCount = 0;
            }
        } else {
            mRegTemp = 0;
            mWriteCount = 0;
            mModePRG = 3;
            updatePRGPointers();
        }
    }

    Byte MapperSxROM::readPRG(Address addr) const {
        return addr < 0xc000
               ? mBankPRG0[addr & 0x3fff]
               : mBankPRG1[addr & 0x3fff];
    }

    void MapperSxROM::writeCHR(Address addr, Byte value) {
        if (mUseCharacterRAM) {
            mCharacterRAM[addr] = value;
        } else {
            Log(Debug) << "Read-only CHR memory write attempt at " << std::hex << addr << std::endl;
        }
    }

    Byte MapperSxROM::readCHR(Address addr) const {
        if (mUseCharacterRAM) {
            return mCharacterRAM[addr];
        } else {
            return addr < 0x1000 ? mBankCHR0[addr] : mBankCHR1[addr & 0xfff];
        }
    }

    const Byte *MapperSxROM::getPagePtr(Address addr) const {
        return addr < 0xc000
               ? mBankPRG0 + (addr & 0x3fff)
               : mBankPRG1 + (addr & 0x3fff);
    }

    Mapper::NameTableMirroring MapperSxROM::nameTableMirroring() const {
        return mMirroring;
    }

    void MapperSxROM::updatePRGPointers() {
        auto *data = mCartridge->ROM().data();
        assert(mModePRG >= 0);
        switch (mModePRG) {
            case 0:
            case 1:
                // Equivalent to multiplying 0x8000 * (m_regPRG >> 1)
                mBankPRG0 = data + 0x4000 * (mRegPRG & ~1);
                mBankPRG1 = mBankPRG0 + 0x4000;   //add 16KB
                break;
            case 2:
                mBankPRG0 = data;
                mBankPRG1 = mBankPRG0 + 0x4000 * mRegPRG;
                break;
            case 3:
            default:
                mBankPRG0 = data + 0x4000 * mRegPRG;
                mBankPRG1 = data + mCartridge->ROM().size() - 0x4000;
        }
    }
}
