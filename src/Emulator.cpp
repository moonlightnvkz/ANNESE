#include <SFML/Window/Event.hpp>
#include "../include/Emulator.h"
#include "../include/Cartridge.h"
#include "../include/CartridgeLoader.h"
#include "../include/Mapper.h"
#include "../include/MainBus.h"
#include "../include/CPU.h"
#include "../include/PPU.h"
#include "../include/TeeLog.hpp"
#include "../include/Joypad.h"

namespace ANNESE {
    Emulator::Emulator(const Configuration &conf)
            : mWindow(sf::VideoMode(static_cast<unsigned int>(PPU::ScanlineVisibleDots * conf.application.scale),
                                    static_cast<unsigned int>(PPU::VisibleScanlines * conf.application.scale)),
                      "ANNESE", sf::Style::Titlebar | sf::Style::Close) {
        mMainBus = std::make_shared<MainBus>();
        mPictureBus = std::make_shared<PictureBus>();
        mScreen = std::make_shared<Screen>(sf::Vector2i{PPU::ScanlineVisibleDots,
                                                        PPU::VisibleScanlines},
                                           conf.application.scale);
        mPPU = std::make_shared<PPU>(mPictureBus, mScreen);
        mCPU = std::make_shared<CPU>(mMainBus);
        mJoypad1 = std::make_shared<Joypad>(conf.player1);
        mJoypad2 = std::make_shared<Joypad>(conf.player2);

        (*mMainBus.get())
                .setReadCallback(IORegisters::PPUStatus, [=]() {
                    return mPPU->status();
                })
                .setReadCallback(IORegisters::PPUData, [=]() {
                    return mPPU->data();
                })
                .setReadCallback(IORegisters::OAMData, [=]() {
                    return mPPU->OAMData();
                })
                .setReadCallback(IORegisters::Joy1, [=]() {
                    return mJoypad1->read();
                })
                .setReadCallback(IORegisters::Joy2, [=]() {
                    return mJoypad2->read();
                })
                .setWriteCallback(IORegisters::PPUCtrl, [=](Byte value) {
                    mPPU->control(value);
                })
                .setWriteCallback(IORegisters::PPUMask, [=](Byte value) {
                    mPPU->mask(value);
                })
                .setWriteCallback(IORegisters::OAMAddr, [=](Byte value) {
                    mPPU->OAMAddress(value);
                })
                .setWriteCallback(IORegisters::PPUAddr, [=](Byte value) {
                    mPPU->dataAddress(value);
                })
                .setWriteCallback(IORegisters::PPUScroll, [=](Byte value) {
                    mPPU->scroll(value);
                })
                .setWriteCallback(IORegisters::PPUData, [=](Byte value) {
                    mPPU->data(value);
                })
                .setWriteCallback(IORegisters::OAMData, [=](Byte value) {
                    mPPU->OAMData(value);
                })
                .setWriteCallback(IORegisters::OAMDMA, [=](Byte value) {
                    mCPU->skipDMACycles();
                    mPPU->doDMA(mMainBus->getPagePtr(value));
                })
                .setWriteCallback(IORegisters::Joy1, [=](Byte value) {
                    mJoypad1->strobe(value);
                    mJoypad2->strobe(value);
                });
//                .setWriteCallback(IORegisters::Joy2, [=](Byte value) {
//                    mJoypad1->strobe(value);
//                    mJoypad2->strobe(value);
//                });
    mPPU->setInterruptCallback([=]() {
        mCPU->interrupt(CPU::Interruption::NMI);
    });
        mWindow.setVerticalSyncEnabled(true);
    }

    void ANNESE::Emulator::run(std::istream &rom) {
        std::unique_ptr<Cartridge> cartridge = CartridgeLoader::Load(rom);
        if (!cartridge) {
            Log(Error) << "Failed to load the cartridge" << std::endl;
            exit(1);
        }
        mMapper = Mapper::Create(std::move(cartridge), [&]() {
                                     mPictureBus->updateMirroring();
                                 });
        mMainBus->setMapper(mMapper);
        mPictureBus->setMapper(mMapper);
        mCPU->reset();
        mPPU->reset();

        sf::Event event{};
        bool keep = true;

        sf::Text acronym{}, fullName{};
        sf::Font font{};
        bool drawLogo = initLogoText(acronym, fullName, font);

        auto elapsed = std::chrono::high_resolution_clock::duration(0);
        auto timer = std::chrono::high_resolution_clock::now();

        while (drawLogo && mWindow.isOpen() && elapsed < LogoDuration) {
            while (mWindow.pollEvent(event)) {
                if (event.type == sf::Event::Closed ||
                    (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)) {
                    mWindow.close();
                    keep = false;
                    break;
                }
            }
            if (!keep) {
                break;
            }
            auto newTimer = std::chrono::high_resolution_clock::now();
            elapsed += newTimer - timer;
            timer = newTimer;

            mWindow.clear(sf::Color::Black);
            mWindow.draw(acronym);
            mWindow.draw(fullName);
            mWindow.display();
        }

        elapsed = std::chrono::high_resolution_clock::duration(0);
        timer = std::chrono::high_resolution_clock::now();

        while (mWindow.isOpen()) {
            while (mWindow.pollEvent(event)) {
                if (event.type == sf::Event::Closed ||
                   (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)) {
                    mWindow.close();
                    keep = false;
                    break;
                }
            }
            if (!keep) {
                break;
            }
            auto newTimer = std::chrono::high_resolution_clock::now();
            elapsed += newTimer - timer;
            timer = newTimer;

            while (elapsed > CPUCycleDuration) {
                mPPU->step();
                mPPU->step();
                mPPU->step();

                mCPU->step();

                elapsed -= CPUCycleDuration;
            }
            mWindow.draw(*mScreen);
            mWindow.display();
        }
    }

    bool Emulator::initLogoText(sf::Text &acronym, sf::Text &fullName, sf::Font &font) const {
        if (!font.loadFromFile(FontName)) {
            Log(Error) << "Failed to load font for logo: " << FontName << std::endl;
            return false;
        }

        acronym.setString("ANNESE");
        acronym.setCharacterSize(40);
        acronym.setStyle(sf::Text::Bold);
        acronym.setFillColor(sf::Color::White);
        acronym.setFont(font);

        fullName.setString("ANother NES Emulator");
        fullName.setCharacterSize(40);
        fullName.setStyle(sf::Text::Bold);
        fullName.setFillColor(sf::Color::White);
        fullName.setFont(font);

        sf::Rect acronymRect = acronym.getLocalBounds();
        sf::Rect fullNameRect = fullName.getLocalBounds();

        sf::Vector2u windowSize = mWindow.getSize();
        acronym.setPosition((windowSize.x - acronymRect.width) / 2.f,
                            windowSize.y / 2.f - acronymRect.height - LogoLinesSpacing / 2.f);
        fullName.setPosition((windowSize.x - fullNameRect.width) / 2.f,
                             windowSize.y / 2.f + fullNameRect.height - LogoLinesSpacing / 2.f);
        return true;
    }
}