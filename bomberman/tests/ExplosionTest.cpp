#include "Explosion.hpp"
#include "Grid.hpp"

#include <algorithm>
#include <gtest/gtest.h>
#include <rapidcheck.h>
#include <rapidcheck/gtest.h>

TEST(ExplosionTest, IncludesCenter) {
    Grid grid(15, 13, 0.0f);
    grid.initialize({{1, 1}});
    const auto cells = computeExplosionCells(grid, {5, 5}, 2);
    EXPECT_NE(std::find(cells.begin(), cells.end(), Position{5, 5}), cells.end());
}

TEST(ExplosionTest, StopsAtIndestructible) {
    Grid grid(11, 11, 0.0f);
    grid.initialize({{1, 1}});
    const auto cells = computeExplosionCells(grid, {2, 1}, 3);
    for (const Position& cell : cells) {
        EXPECT_TRUE(grid.isInBounds(cell));
        EXPECT_NE(grid.getCell(cell), CellType::INDESTRUCTIBLE);
    }
}

RC_GTEST_PROP(ExplosionTest, CrossShapeInBounds, ()) {
    const auto width = *rc::gen::inRange(11, 21);
    const auto height = *rc::gen::inRange(11, 21);
    const auto cx = *rc::gen::inRange(1, width - 2);
    const auto cy = *rc::gen::inRange(1, height - 2);
    const auto radius = *rc::gen::inRange(1, 4);

    Grid grid(width, height, 0.0f);
    grid.initialize({});

    const auto cells = computeExplosionCells(grid, {cx, cy}, radius);
    RC_ASSERT(std::find(cells.begin(), cells.end(), Position{cx, cy}) != cells.end());
    for (const Position& cell : cells) {
        RC_ASSERT(grid.isInBounds(cell));
    }
}
