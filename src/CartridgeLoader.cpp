#include <vector>
#include <optional>
#include "../include/CartridgeLoader.h"
#include "../include/TeeLog.hpp"

namespace ANNESE {
    std::unique_ptr<Cartridge> CartridgeLoader::Load(std::istream &rom) {
        std::vector<Byte> header(0x10);
        Log(Debug) << "Reading ROM" << std::endl;
        if (!rom.read(reinterpret_cast<char*>(&header[0]), 0x10)) {
            Log(Error) << "Reading ROM header failed" << std::endl;
            return {};
        }

        if (std::string{&header[0], &header[4]} != "NES\x1A") {
            Log(Error) << "Not a valid iNES image. Magic number: "
                       << std::hex << header[0] << " "
                       << header[1] << " " << header[2] << " " << int(header[3]) << std::endl
                       << "Valid magic number : N E S 1a" << std::endl;
            return {};
        }

        Log(Debug) << "Reading header" << std::endl;
        Byte banks = header[4];
        Log(Debug) << "16KB PRG-ROM Banks: " << +banks << std::endl;
        if (!banks) {
            Log(Error) << "ROM has no PRG-ROM banks. Loading ROM failed." << std::endl;
            return {};
        }
        Byte vbanks = header[5];
        Log(Debug) << "8KB CHR-ROM Banks: " << +vbanks << std::endl;

        Byte nameTableMirroring = header[6] & Byte(0xB);
        Log(Debug) << "Name Table Mirroring: " << +nameTableMirroring << std::endl;

        Byte mapperNumber = ((header[6] >> 4) & 0xf) | (header[7] & Byte(0xf0));
        Log(Debug) << "Mapper #: " << +mapperNumber << std::endl;

        bool extendedRAM = header[6] & Byte(0x2);
        Log(Debug) << "Extended (CPU) RAM: " << std::boolalpha << extendedRAM << std::endl;

        if (header[6] & 0x4) {
            Log(Error) << "Trainer is not supported." << std::endl;
            return {};
        }

        if ((header[0xA] & 0x3) == 0x2 || (header[0xA] & 0x1)) {
            Log(Error) << "PAL ROM not supported." << std::endl;
            return {};
        } else {
            Log(Debug) << "ROM is NTSC compatible.\n";
        }

        //PRG-ROM 16KB banks
        std::vector<Byte> prgROM(0x4000u * banks);
        if (!rom.read(reinterpret_cast<char*>(&prgROM[0]), 0x4000 * banks)) {
            Log(Error) << "Reading PRG-ROM from image file failed." << std::endl;
            return {};
        }

        //CHR-ROM 8KB banks
        std::vector<Byte> chrROM;
        if (vbanks) {
            chrROM.resize(0x2000u * vbanks);
            if (!rom.read(reinterpret_cast<char*>(&chrROM[0]), 0x2000 * vbanks)) {
                Log(Error) << "Reading CHR-ROM from image file failed." << std::endl;
                return {};
            }
        } else {
            Log(Debug) << "Cartridge with CHR-RAM." << std::endl;
        }
        return {std::make_unique<Cartridge>(std::move(prgROM), std::move(chrROM),
                                            nameTableMirroring, mapperNumber, extendedRAM)};
    }
}