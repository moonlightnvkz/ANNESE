#include <utility>

#include "../include/Mapper.h"
#include "../include/MapperNROM.h"
#include "../include/MapperSxROM.h"
#include "../include/MapperCNROM.h"
#include "../include/MapperUxROM.h"
#include "../include/TeeLog.hpp"

namespace ANNESE {
    std::shared_ptr<Mapper> Mapper::Create(std::unique_ptr<Cartridge> &&cartridge,
                                           std::function<void(void)> mirroringCallback) {
        if (cartridge->mapperNumber() == static_cast<Byte>(Type::NROM)) {
            return std::shared_ptr<Mapper>(new MapperNROM(std::move(cartridge)));
        }
        if (cartridge->mapperNumber() == static_cast<Byte>(Type::SxROM)) {
            return std::shared_ptr<Mapper>(new MapperSxROM(std::move(cartridge), std::move(mirroringCallback)));
        }
        if (cartridge->mapperNumber() == static_cast<Byte>(Type::UxROM)) {
            return std::shared_ptr<Mapper>(new MapperUxROM(std::move(cartridge)));
        }
        if (cartridge->mapperNumber() == static_cast<Byte>(Type::CNROM)) {
            return std::shared_ptr<Mapper>(new MapperCNROM(std::move(cartridge)));
        }
        Log(Error) << "Unsupported mapper type: " << cartridge->mapperNumber() << std::endl;
        throw std::invalid_argument("Unsupported mapper type");
    }
}