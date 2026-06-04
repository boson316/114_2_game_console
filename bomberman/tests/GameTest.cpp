#include "Game.hpp"
#include "GameConfig.hpp"
#include "Input.hpp"

#include <gtest/gtest.h>

TEST(GameTest, ConfigClamp) {
    GameConfig raw;
    raw.gridWidth = 5;
    raw.gridHeight = 5;
    raw.destructibleDensity = 2.0f;
    raw.enemyCount = 10;
    const GameConfig cfg = GameConfig::clamped(raw);
    EXPECT_GE(cfg.gridWidth, 11);
    EXPECT_GE(cfg.gridHeight, 11);
    EXPECT_LE(cfg.destructibleDensity, 0.8f);
    EXPECT_LE(cfg.enemyCount, 4);
}

TEST(GameTest, PickupBombUpIncreasesMaxBombs) {
    GameConfig cfg;
    cfg.gridWidth = 11;
    cfg.gridHeight = 11;
    cfg.destructibleDensity = 0.0f;
    cfg.enemyCount = 0;
    Game game(cfg);
    EXPECT_EQ(game.getPlayerMaxBombs(), 1);
    game.placePowerUpAt({2, 1}, PowerUpType::BOMB_UP);
    game.applyAction(InputAction::MOVE_RIGHT);
    EXPECT_EQ(game.getPlayerMaxBombs(), 2);
}

TEST(GameTest, PickupFireUpIncreasesBlastRadius) {
    GameConfig cfg;
    cfg.gridWidth = 11;
    cfg.gridHeight = 11;
    cfg.destructibleDensity = 0.0f;
    cfg.enemyCount = 0;
    Game game(cfg);
    EXPECT_EQ(game.getPlayerBlastRadius(), 2);
    game.placePowerUpAt({1, 2}, PowerUpType::FIRE_UP);
    game.applyAction(InputAction::MOVE_DOWN);
    EXPECT_EQ(game.getPlayerBlastRadius(), 3);
}
