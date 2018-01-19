#pragma once


#include <istream>
#include <memory>
#include "Cartridge.h"

namespace ANNESE {
    class CartridgeLoader {
    public:
        CartridgeLoader() = delete;

        static std::unique_ptr<Cartridge> Load(std::istream &rom);
    };
}