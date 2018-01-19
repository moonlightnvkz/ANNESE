#pragma once

#include <SFML/Window/Keyboard.hpp>
#include <vector>
#include "Utility.h"
#include "ConfigManager.h"

namespace ANNESE {
    class Joypad {
    public:
        explicit Joypad(const Configuration::Joypad &conf);

        virtual ~Joypad() = default;

        void strobe(Byte value);

        Byte read();

    protected:
        enum class Button : Byte {
            A,
            B,
            Select,
            Start,
            Up,
            Down,
            Left,
            Right,
        };

        bool mStrobe;

        Byte mKeyStates;

        std::vector<sf::Keyboard::Key> mKeyBindings;
    };
}