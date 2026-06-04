#pragma once

#include <cstdint>

enum class CellType : std::uint8_t {
    EMPTY = 0,
    INDESTRUCTIBLE = 1,
    DESTRUCTIBLE = 2,
    EXPLOSION = 3,
    POWERUP = 4
};
