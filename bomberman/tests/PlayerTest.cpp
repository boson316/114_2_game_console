#include "Grid.hpp"
#include "Player.hpp"

#include <gtest/gtest.h>

TEST(PlayerTest, BlockedByWall) {
    Grid grid(11, 11, 0.0f);
    grid.initialize({{1, 1}});
    Player player({1, 1});
    grid.setCell({1, 0}, CellType::INDESTRUCTIBLE);
    const Position before = player.getPosition();
    player.move(Direction::UP, grid);
    EXPECT_EQ(player.getPosition(), before);
}

TEST(PlayerTest, MovesToPassableCell) {
    Grid grid(11, 11, 0.0f);
    grid.initialize({{1, 1}});
    Player player({1, 1});
    player.move(Direction::RIGHT, grid);
    EXPECT_EQ(player.getPosition(), (Position{2, 1}));
}

TEST(PlayerTest, BombPlacementLimit) {
    Player player({1, 1});
    EXPECT_TRUE(player.tryPlaceBomb(1, 0));
    EXPECT_FALSE(player.tryPlaceBomb(1, 1));
}
