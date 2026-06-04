#include "Grid.hpp"
#include "Pathfinder.hpp"

#include <gtest/gtest.h>

TEST(PathfinderTest, NextStepDoesNotEnterWall) {
    Grid grid(11, 11, 0.0f);
    grid.initialize({{1, 1}});
    const Position start{1, 1};
    const Position goal{9, 9};
    const auto step = Pathfinder::nextStepToward(grid, start, goal);
    ASSERT_TRUE(step.has_value());
    const Position next = start + *step;
    EXPECT_TRUE(grid.isPassable(next));
}

TEST(PathfinderTest, ReduceDangerMovesOffBlastLine) {
    Grid grid(11, 11, 0.0f);
    grid.initialize({{1, 1}});
    for (int x = 1; x <= 8; ++x) {
        for (int y = 3; y <= 7; ++y) {
            if (!((x % 2 == 0) && (y % 2 == 0))) {
                grid.setCell({x, y}, CellType::EMPTY);
            }
        }
    }
    std::vector<Bomb> bombs;
    bombs.emplace_back(Position{5, 5}, 3.0f, 2);
    const Position start{7, 5};
    const auto step = Pathfinder::nextStepReduceDanger(grid, start, bombs);
    ASSERT_TRUE(step.has_value());
    const Position next = start + *step;
    const auto danger = Pathfinder::computeDangerZone(grid, bombs);
    EXPECT_EQ(0u, danger.count(next));
}

TEST(PathfinderTest, DangerZoneContainsBombCenter) {
    Grid grid(11, 11, 0.0f);
    grid.initialize({{1, 1}});
    std::vector<Bomb> bombs;
    bombs.emplace_back(Position{5, 5}, 3.0f, 2);
    const auto danger = Pathfinder::computeDangerZone(grid, bombs);
    EXPECT_GT(danger.count(Position{5, 5}), 0u);
}
