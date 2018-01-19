#define CPPTOML_USE_MAP
#include <cpptoml.h>
#include "../include/ConfigManager.h"
#include "../include/TeeLog.hpp"

namespace ANNESE {

    std::pair<sf::Keyboard::Key, const char *> ConfigManager::Keys[] = {
            {sf::Keyboard::A, "A"},
            {sf::Keyboard::B, "B"},
            {sf::Keyboard::C, "C"},
            {sf::Keyboard::D, "D"},
            {sf::Keyboard::E, "E"},
            {sf::Keyboard::F, "F"},
            {sf::Keyboard::G, "G"},
            {sf::Keyboard::H, "H"},
            {sf::Keyboard::I, "I"},
            {sf::Keyboard::J, "J"},
            {sf::Keyboard::K, "K"},
            {sf::Keyboard::L, "L"},
            {sf::Keyboard::M, "M"},
            {sf::Keyboard::N, "N"},
            {sf::Keyboard::O, "O"},
            {sf::Keyboard::P, "P"},
            {sf::Keyboard::Q, "Q"},
            {sf::Keyboard::R, "R"},
            {sf::Keyboard::S, "S"},
            {sf::Keyboard::T, "T"},
            {sf::Keyboard::U, "U"},
            {sf::Keyboard::V, "V"},
            {sf::Keyboard::W, "W"},
            {sf::Keyboard::X, "X"},
            {sf::Keyboard::Y, "Y"},
            {sf::Keyboard::Z, "Z"},
            {sf::Keyboard::Num0, "Num0"},
            {sf::Keyboard::Num1, "Num1"},
            {sf::Keyboard::Num2, "Num2"},
            {sf::Keyboard::Num3, "Num3"},
            {sf::Keyboard::Num4, "Num4"},
            {sf::Keyboard::Num5, "Num5"},
            {sf::Keyboard::Num6, "Num6"},
            {sf::Keyboard::Num7, "Num7"},
            {sf::Keyboard::Num8, "Num8"},
            {sf::Keyboard::Num9, "Num9"},
            {sf::Keyboard::Escape, "Escape"},
            {sf::Keyboard::LControl, "LControl"},
            {sf::Keyboard::LShift, "LShift"},
            {sf::Keyboard::LAlt, "LAlt"},
            {sf::Keyboard::LSystem, "LSystem"},
            {sf::Keyboard::RControl, "RControl"},
            {sf::Keyboard::RShift, "RShift"},
            {sf::Keyboard::RAlt, "RAlt"},
            {sf::Keyboard::RSystem, "RSystem"},
            {sf::Keyboard::Menu, "Menu"},
            {sf::Keyboard::LBracket, "LBracket"},
            {sf::Keyboard::RBracket, "RBracket"},
            {sf::Keyboard::SemiColon, "SemiColon"},
            {sf::Keyboard::Comma, "Comma"},
            {sf::Keyboard::Period, "Period"},
            {sf::Keyboard::Quote, "Quote"},
            {sf::Keyboard::Slash, "Slash"},
            {sf::Keyboard::BackSlash, "BackSlash"},
            {sf::Keyboard::Tilde, "Tilde"},
            {sf::Keyboard::Equal, "Equal"},
            {sf::Keyboard::Dash, "Dash"},
            {sf::Keyboard::Space, "Space"},
            {sf::Keyboard::Return, "Return"},
            {sf::Keyboard::BackSpace, "BackSpace"},
            {sf::Keyboard::Tab, "Tab"},
            {sf::Keyboard::PageUp, "PageUp"},
            {sf::Keyboard::PageDown, "PageDown"},
            {sf::Keyboard::End, "End"},
            {sf::Keyboard::Home, "Home"},
            {sf::Keyboard::Insert, "Insert"},
            {sf::Keyboard::Delete, "Delete"},
            {sf::Keyboard::Add, "Add"},
            {sf::Keyboard::Subtract, "Subtract"},
            {sf::Keyboard::Multiply, "Multiply"},
            {sf::Keyboard::Divide, "Divide"},
            {sf::Keyboard::Left, "Left"},
            {sf::Keyboard::Right, "Right"},
            {sf::Keyboard::Up, "Up"},
            {sf::Keyboard::Down, "Down"},
            {sf::Keyboard::Numpad0, "Numpad0"},
            {sf::Keyboard::Numpad1, "Numpad1"},
            {sf::Keyboard::Numpad2, "Numpad2"},
            {sf::Keyboard::Numpad3, "Numpad3"},
            {sf::Keyboard::Numpad4, "Numpad4"},
            {sf::Keyboard::Numpad5, "Numpad5"},
            {sf::Keyboard::Numpad6, "Numpad6"},
            {sf::Keyboard::Numpad7, "Numpad7"},
            {sf::Keyboard::Numpad8, "Numpad8"},
            {sf::Keyboard::Numpad9, "Numpad9"},
            {sf::Keyboard::F1, "F1"},
            {sf::Keyboard::F2, "F2"},
            {sf::Keyboard::F3, "F3"},
            {sf::Keyboard::F4, "F4"},
            {sf::Keyboard::F5, "F5"},
            {sf::Keyboard::F6, "F6"},
            {sf::Keyboard::F7, "F7"},
            {sf::Keyboard::F8, "F8"},
            {sf::Keyboard::F9, "F9"},
            {sf::Keyboard::F10, "F10"},
            {sf::Keyboard::F11, "F11"},
            {sf::Keyboard::F12, "F12"},
            {sf::Keyboard::F13, "F13"},
            {sf::Keyboard::F14, "F14"},
            {sf::Keyboard::F15, "F15"},
            {sf::Keyboard::Pause, "Pause"}
    };

    void ConfigManager::loadDefaults() {
        using Kb = sf::Keyboard;
        configuration.application.scale = 2.0f;
        configuration.player1 = {Kb::T, Kb::Y, Kb::E, Kb::R, Kb::W, Kb::S, Kb::A, Kb::D};
        configuration.player2 = {Kb::LBracket, Kb::RBracket, Kb::O, Kb::P, Kb::I, Kb::K, Kb::J, Kb::L};
    }

    bool ConfigManager::load(std::istream &is) {
        std::shared_ptr<::cpptoml::table> root;
        try {
            root = ::cpptoml::parser(is).parse();
        } catch (const ::cpptoml::parse_exception &e) {
            Log(Error) << "Failed to parse the configuration file" << std::endl;
            return false;
        }
        auto app = root->get_table("application");
        if (!app) {
            return false;
        } else {
            ::cpptoml::option<double> opt = app->get_as<double>("scale");
            if (!opt) {
                return false;
            }
            configuration.application.scale = static_cast<float>(*opt);
        }

        auto pConf = root->get_table("player 1");
        auto *player = &configuration.player1;
        for (int i = 0; i < 2; ++i, pConf = root->get_table("player 2"), player = &configuration.player2) {
            ::cpptoml::option<std::string> opt;
            if (!pConf) {
                return false;
            }
            if ((opt = pConf->get_as<std::string>("a"))) player->a = KeyFromStr(*opt).value();
            else return false;
            if ((opt = pConf->get_as<std::string>("b"))) player->b = KeyFromStr(*opt).value();
            else return false;
            if ((opt = pConf->get_as<std::string>("select"))) player->select = KeyFromStr(*opt).value();
            else return false;
            if ((opt = pConf->get_as<std::string>("start"))) player->start = KeyFromStr(*opt).value();
            else return false;
            if ((opt = pConf->get_as<std::string>("up"))) player->up = KeyFromStr(*opt).value();
            else return false;
            if ((opt = pConf->get_as<std::string>("down"))) player->down = KeyFromStr(*opt).value();
            else return false;
            if ((opt = pConf->get_as<std::string>("left"))) player->left = KeyFromStr(*opt).value();
            else return false;
            if ((opt = pConf->get_as<std::string>("right"))) player->right = KeyFromStr(*opt).value();
            else return false;
        }

        return true;
    }

    bool ConfigManager::store(std::ostream &os) {
        auto root = ::cpptoml::make_table();

        auto app = ::cpptoml::make_table();
        app->insert("scale", static_cast<double>(configuration.application.scale));
        root->insert("application", app);

        auto *player = &configuration.player1;
        const char *name = "player 1";
        for (int i = 0; i < 2; ++i, player = &configuration.player2, name = "player 2") {
            auto pConf = ::cpptoml::make_table();
            pConf->insert("a", KeyToStr(player->a));
            pConf->insert("b", KeyToStr(player->b));
            pConf->insert("select", KeyToStr(player->select));
            pConf->insert("start", KeyToStr(player->start));
            pConf->insert("up", KeyToStr(player->up));
            pConf->insert("down", KeyToStr(player->down));
            pConf->insert("left", KeyToStr(player->left));
            pConf->insert("right", KeyToStr(player->right));
            root->insert(name, pConf);
        }
        return (bool)(os << *root);
    }

    std::optional<sf::Keyboard::Key> ConfigManager::KeyFromStr(std::string_view name) {
        auto it = std::find_if(std::begin(Keys), std::end(Keys),
                               [&](auto &p) {
                                   return p.second == name;
                               });
        if (it == std::end(Keys)) {
            return {};
        } else {
            return {it->first};
        }
    }

    std::string ConfigManager::KeyToStr(sf::Keyboard::Key key) {
        auto it = std::find_if(std::begin(Keys), std::end(Keys),
                               [&](auto &p) {
                                   return p.first == key;
                               });
        assert(it != std::end(Keys));
        return {it->second};
    }
}