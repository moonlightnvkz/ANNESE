#include "../include/Joypad.h"

namespace ANNESE {
    Joypad::Joypad(const Configuration::Joypad &conf) {
        mKeyBindings.push_back(conf.a);
        mKeyBindings.push_back(conf.b);
        mKeyBindings.push_back(conf.select);
        mKeyBindings.push_back(conf.start);
        mKeyBindings.push_back(conf.up);
        mKeyBindings.push_back(conf.down);
        mKeyBindings.push_back(conf.left);
        mKeyBindings.push_back(conf.right);
    }

    void Joypad::strobe(Byte value) {
        mStrobe = (value & 1) != 0;
        if (!mStrobe) {
            mKeyStates = 0;
            int shift = 0;
            for (auto key : mKeyBindings) {
                mKeyStates |= (sf::Keyboard::isKeyPressed(key) << shift);
                ++shift;
            }
        }
    }

    Byte Joypad::read() {
        Byte ret;
        if (mStrobe) {
            ret = static_cast<Byte>(sf::Keyboard::isKeyPressed(mKeyBindings[static_cast<Byte>(Button::A)]));
        } else {
            ret = mKeyStates & Byte(1);
            mKeyStates >>= 1;
        }
        return ret | Byte(0x40);
    }
}