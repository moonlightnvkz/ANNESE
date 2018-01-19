#pragma once


#include "Utility.h"
#include "Mapper.h"

namespace ANNESE {
    class PictureBus {
    public:
        PictureBus();

        virtual ~PictureBus() = default;

        Byte read(Address addr) const;

        void write(Address addr, Byte value);

        bool setMapper(std::shared_ptr<Mapper> mapper);

        Byte readPalette(Byte paletteAddr);

        void updateMirroring();

    protected:
        std::vector<Byte> mRAM;

        // indices
        size_t mVRAM0;

        size_t mVRAM1;

        size_t mVRAM2;

        size_t mVRAM3;

        std::vector<Byte> mPalette;

        std::shared_ptr<Mapper> mMapper;

        enum class MemoryMap : Address {
            CHRROM = 0x0,
            VRAM0 = 0x2000,
            VRAM1 = 0x2400,
            VRAM2 = 0x2800,
            VRAM3 = 0x2C00,
            VRAMMirror = 0x3000,
            PaletteBG = 0x3f00,
            PaletteSP = 0x3f10,
            NotUsed = 0x3f20,
        };
    };
}