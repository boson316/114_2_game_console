#pragma once

#include <array>
#include <cstddef>
#include <functional>

struct Position {
    int x = 0;
    int y = 0;

    bool operator==(const Position& other) const { return x == other.x && y == other.y; }
    bool operator!=(const Position& other) const { return !(*this == other); }
    Position operator+(const Position& other) const { return {x + other.x, y + other.y}; }
};

namespace Direction {
inline constexpr Position UP{0, -1};
inline constexpr Position DOWN{0, 1};
inline constexpr Position LEFT{-1, 0};
inline constexpr Position RIGHT{1, 0};
inline const std::array<Position, 4> ALL = {UP, DOWN, LEFT, RIGHT};
}  // namespace Direction

struct PositionHash {
    std::size_t operator()(const Position& p) const noexcept {
        const std::size_t hx = std::hash<int>{}(p.x);
        const std::size_t hy = std::hash<int>{}(p.y);
        return hx ^ (hy + 0x9e3779b9 + (hx << 6) + (hx >> 2));
    }
};
