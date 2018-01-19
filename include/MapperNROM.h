#pragma once

#include <utility>

#include "Mapper.h"

namespace ANNESE {
    class MapperNROM : public Mapper {
    public:
        explicit MapperNROM(std::unique_ptr<Cartridge> cartridge);

        virtual ~MapperNROM() = default;

        Byte readPRG(Address addr) const override;

        void writePRG(Address addr, Byte value) override;

        Byte readCHR(Address addr) const override;

        void writeCHR(Address addr, Byte value) override;

        const Byte *getPagePtr(Address addr) const override;

    protected:
        bool mOneBank;

        bool mUsesCHRRAM;

        std::vector<Byte> mCHRRAM;
    };
}