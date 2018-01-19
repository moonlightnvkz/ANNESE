#pragma once

#include <iosfwd>
#include <optional>
#include <SFML/Window/Keyboard.hpp>

namespace ANNESE {
    struct Configuration {
        using Kb = sf::Keyboard;

        struct Application {
            float scale;
        };

        struct Joypad {
            sf::Keyboard::Key a;

            sf::Keyboard::Key b;

            sf::Keyboard::Key select;

            sf::Keyboard::Key start;

            sf::Keyboard::Key up;

            sf::Keyboard::Key down;

            sf::Keyboard::Key left;

            sf::Keyboard::Key right;
        };

        Application application;

        Joypad player1;

        Joypad player2;
    };

    class ConfigManager {
    public:
        ConfigManager() = default;

        virtual ~ConfigManager() = default;

        void loadDefaults();

        bool load(std::istream &is);

        bool store(std::ostream &os);

        Configuration configuration = {};

    protected:
        static std::optional<sf::Keyboard::Key> KeyFromStr(std::string_view name);

        static std::string KeyToStr(sf::Keyboard::Key key);

        static std::pair<sf::Keyboard::Key, const char *> Keys[];
    };
}