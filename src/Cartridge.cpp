#include "../include/Cartridge.h"

namespace ANNESE {
    Cartridge::Cartridge(const std::vector<Byte> &prgROM, const std::vector<Byte> &chrROM,
                         Byte tableNameMirroring, Byte mapperNumber,
                         bool extendedRAM)
            : mPRGROM(prgROM), mCHRROM(chrROM), mNameTableMirroring(tableNameMirroring),
              mMapperNumber(mapperNumber), mExtendedRAM(extendedRAM) {
    }

    Cartridge::Cartridge(std::vector<Byte> &&prgROM, std::vector<Byte> &&chrROM,
                         Byte tableNameMirroring, Byte mapperNumber,
                         bool extendedRAM)
            : mPRGROM(std::move(prgROM)), mCHRROM(std::move(chrROM)), mNameTableMirroring(tableNameMirroring),
              mMapperNumber(mapperNumber), mExtendedRAM(extendedRAM) {
    }
}