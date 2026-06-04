#include "Grid.hpp"

#include <gtest/gtest.h>
#include <rapidcheck.h>
#include <rapidcheck/gtest.h>

TEST(GridTest, IndestructibleAtEvenIntersections) {
    Grid grid(15, 13, 0.5f);
    grid.initialize({});
    for (int y = 0; y < grid.getHeight(); ++y) {
        for (int x = 0; x < grid.getWidth(); ++x) {
            if (x % 2 == 0 && y % 2 == 0) {
                EXPECT_EQ(grid.getCell({x, y}), CellType::INDESTRUCTIBLE);
            }
        }
    }
}

TEST(GridTest, SafeZoneIsPassable) {
    Grid grid(15, 13, 0.8f);
    const Position start{1, 1};
    grid.initialize({start});
    for (int dy = 0; dy < 2; ++dy) {
        for (int dx = 0; dx < 2; ++dx) {
            EXPECT_TRUE(grid.isPassable({start.x + dx, start.y + dy}));
        }
    }
}

RC_GTEST_PROP(GridTest, IndestructibleWallInvariant, ()) {
    const auto width = *rc::gen::inRange(11, 25);
    const auto height = *rc::gen::inRange(11, 25);
    Grid grid(width, height, 0.5f);
    grid.initialize({});

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (x % 2 == 0 && y % 2 == 0) {
                RC_ASSERT(grid.getCell({x, y}) == CellType::INDESTRUCTIBLE);
            }
        }
    }
}
