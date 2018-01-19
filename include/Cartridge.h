#pragma once

#include <vector>
#include "Utility.h"

namespace ANNESE {
    class Cartridge {
    public:
        Cartridge(const std::vector<Byte> &prgROM, const std::vector<Byte> &chrROM,
                  Byte tableNameMirroring, Byte mapperNumber,
                  bool extendedRAM);

        Cartridge(std::vector<Byte> &&prgROM, std::vector<Byte> &&chrROM,
                  Byte tableNameMirroring, Byte mapperNumber,
                  bool extendedRAM);

        virtual ~Cartridge() = default;

        const std::vector<Byte> &ROM() const {
            return mPRGROM;
        }

        const std::vector<Byte> &VROM() const {
            return mCHRROM;
        }

        Byte mapperNumber() const {
            return mMapperNumber;
        }

        Byte nameTableMirroring() const {
            return mNameTableMirroring;
        }

        bool hasExtendedRAM() const {
            return mExtendedRAM;
        }

    protected:
        std::vector<Byte> mPRGROM;

        std::vector<Byte> mCHRROM;

        Byte mNameTableMirroring;

        Byte mMapperNumber;

        bool mExtendedRAM;
    };
}