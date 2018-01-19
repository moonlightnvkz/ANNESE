#pragma once

#include "Mapper.h"

namespace ANNESE {
    class MapperUxROM : public Mapper {
    public:
        explicit MapperUxROM(std::unique_ptr<Cartridge> &&cartridge);

        virtual ~MapperUxROM() = default;

        void writePRG(Address addr, Byte value) override;

        Byte readPRG(Address addr) const override;

        void writeCHR(Address addr, Byte value) override;

        Byte readCHR(Address addr) const override;

        const Byte *getPagePtr(Address addr) const override;

    protected:
        bool mUseCharacterRAM;

        const Byte *mLastBankPtr;

        Address mSelectPRG = 0  ;

        std::vector<Byte> mCharacterRAM;
    };
}