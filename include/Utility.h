#pragma once

#include <cstdint>

namespace ANNESE {
    using Byte = uint8_t;

    using SByte = int8_t;

    using ExtendedByte = uint16_t;

    using Address = uint16_t;

    using CycleLength = int;

    constexpr Address operator "" _a(unsigned long long v) {
        return static_cast<Address>(v);
    }

    constexpr Byte operator "" _b(unsigned long long v) {
        return static_cast<Byte>(v);
    }
}