#include <fstream>
#include <chrono>
#include <iostream>
#include "../include/ConfigManager.h"
#include "../include/Emulator.h"
#include "../include/TeeLog.hpp"

static constexpr const char *Zelda = "cartridges/Legend of Zelda, The (U) (PRG1) [!].nes";
static constexpr const char *Galaxian = "cartridges/Galaxian (Japan).nes";
static constexpr const char *DevilWorld = "cartridges/Devil World (Japan).nes";
static constexpr const char *ArkanoidUSA = "cartridges/Arkanoid (USA).nes";
static constexpr const char *ArkanoidJapan = "cartridges/Arkanoid (Japan).nes";
static constexpr const char *Fight = "cartridges/10-Yard Fight (Japan).nes";
static constexpr const char *Contra = "cartridges/Contra (USA).nes";

static void printHelp(char *name) {
    std::cout << "Usage: \n> " << name << " <path_to_cartridge>";
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printHelp(argv[0]);
        return 0;
    }

    ANNESE::TeeLog::Instance().setLogFile(std::make_unique<std::ofstream>("log.log"));

    std::ifstream confIn("config.toml");
    ANNESE::ConfigManager configManager;
    if (!configManager.load(confIn)) {
        configManager.loadDefaults();
    }
    confIn.close();
    
    std::ifstream rom(Contra);
//    std::ifstream rom(argv[1]);
    ANNESE::Emulator emulator(configManager.configuration);
    emulator.run(rom);

    std::ofstream confOut("config.toml");
    configManager.store(confOut);
    return 0;
}