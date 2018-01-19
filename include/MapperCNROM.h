#pragma once

#include "Mapper.h"

namespace ANNESE {
    class MapperCNROM : public Mapper {
    public:
        explicit MapperCNROM(std::unique_ptr<Cartridge> &&cartridge);

        virtual ~MapperCNROM() = default;

        void writePRG(Address addr, Byte value) override;

        Byte readPRG(Address addr) const override;

        void writeCHR(Address addr, Byte value) override;

        Byte readCHR(Address addr) const override;

        const Byte *getPagePtr(Address addr) const override;

    protected:
        bool mOneBank;

        Address mSelectCHR = 0;
    };

}