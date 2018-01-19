#pragma once

#include <memory>
#include <chrono>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Text.hpp>

namespace ANNESE {

    class Cartridge;

    class Mapper;

    class MainBus;

    class PictureBus;

    class Screen;

    class PPU;

    class CPU;

    class Configuration;

    class Joypad;

    class Emulator {
    public:
        explicit Emulator(const Configuration &conf);

        virtual ~Emulator() = default;

        void run(std::istream &rom);

    protected:
        bool initLogoText(sf::Text &acronym, sf::Text &fullName, sf::Font &font) const;

        std::shared_ptr<Mapper> mMapper;

        std::shared_ptr<MainBus> mMainBus;

        std::shared_ptr<PictureBus> mPictureBus;

        std::shared_ptr<Screen> mScreen;

        std::shared_ptr<CPU> mCPU;

        std::shared_ptr<PPU> mPPU;

        std::shared_ptr<Joypad> mJoypad1;

        std::shared_ptr<Joypad> mJoypad2;

        static constexpr const auto CPUCycleDuration = std::chrono::nanoseconds(559); // NOLINT nanoseconds doesn't throw

        static constexpr const auto LogoDuration = std::chrono::seconds(4); // NOLINT

        static constexpr const char *FontName = "/usr/share/fonts/TTF/Inconsolata-Regular.ttf";

        static constexpr const float LogoLinesSpacing = 20.f;

        sf::RenderWindow mWindow;
    };
}