#pragma once

#include <functional>
#include "Mapper.h"

namespace ANNESE {
    class MapperSxROM : public Mapper {
    public:
        MapperSxROM(std::unique_ptr<Cartridge> cartridge, std::function<void(void)> mirroringCallback);

        virtual ~MapperSxROM() = default;

        void writePRG(Address addr, Byte value) override;

        Byte readPRG(Address addr) const override;

        void writeCHR(Address addr, Byte value) override;

        Byte readCHR(Address addr) const override;

        const Byte *getPagePtr(Address addr) const override;

        NameTableMirroring nameTableMirroring() const override;

    protected:
        void updatePRGPointers();

        std::function<void(void)> mMirroringCallback;

        NameTableMirroring mMirroring = NameTableMirroring::Horizontal;

        Byte mModeCHR = 0;

        Byte mModePRG = 3;

        int mWriteCount = 0;

        Byte mRegTemp = 0;

        Byte mRegPRG = 0;

        Byte mRegCHR0 = 0;

        Byte mRegCHR1 = 0;

        const Byte *mBankPRG0 = nullptr;

        const Byte *mBankPRG1 = nullptr;

        const Byte *mBankCHR0 = nullptr;

        const Byte *mBankCHR1 = nullptr;

        bool mUseCharacterRAM;

        std::vector<Byte> mCharacterRAM;
    };
}