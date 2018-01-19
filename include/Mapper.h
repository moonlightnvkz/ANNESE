#pragma once

#include <memory>
#include <utility>
#include <functional>
#include "Utility.h"
#include "Cartridge.h"

namespace ANNESE {
    class Mapper {
    public:
        enum class NameTableMirroring : Byte{
            Horizontal = 0,
            Vertical = 1,
            FourScreen = 8,
            OneScreenLower,
            OneScreenHigher,
        };

        enum class Type {
            NROM = 0,
            SxROM = 1,
            UxROM = 2,
            CNROM = 3,
        };

        explicit Mapper(std::unique_ptr<Cartridge> &&cartridge)
                : mCartridge(std::move(cartridge)){
        }

        virtual ~Mapper() = default;

        virtual void writePRG(Address addr, Byte value) = 0;

        virtual Byte readPRG(Address addr) const = 0;

        virtual void writeCHR(Address addr, Byte value) = 0;

        virtual Byte readCHR(Address addr) const = 0;

        /// DMA
        virtual const Byte *getPagePtr(Address addr) const = 0;

        virtual NameTableMirroring nameTableMirroring() const {
            return static_cast<NameTableMirroring>(mCartridge->nameTableMirroring());
        }

        virtual bool hasExtendedRAM() const {
            return mCartridge->hasExtendedRAM();
        }

        static std::shared_ptr<Mapper> Create(std::unique_ptr<Cartridge> &&cartridge,
                                              std::function<void(void)> mirroringCallback);
    protected:
        std::unique_ptr<Cartridge> mCartridge;
    };
}