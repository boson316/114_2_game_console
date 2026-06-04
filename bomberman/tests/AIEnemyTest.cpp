#include "AI_Enemy.hpp"
#include "GameConfig.hpp"
#include "Grid.hpp"

#include <gtest/gtest.h>

TEST(AIEnemyTest, PlacesBombWhenPlayerInBlastLine) {
    Grid grid(11, 11, 0.0f);
    grid.initialize({{1, 1}});
    for (int x = 1; x <= 8; ++x) {
        for (int y = 3; y <= 7; ++y) {
            if (!((x % 2 == 0) && (y % 2 == 0))) {
                grid.setCell({x, y}, CellType::EMPTY);
            }
        }
    }
    AI_Enemy enemy({3, 5}, 0.1f);
    GameConfig cfg;
    cfg.enemyAggressive = true;
    cfg.enemyBlastRadius = 2;
    const EnemyDecision decision = enemy.decide(grid, {5, 5}, {}, cfg, nullptr);
    EXPECT_TRUE(decision.placeBomb);
    EXPECT_TRUE(decision.moveStep.x != 0 || decision.moveStep.y != 0);
}

TEST(AIEnemyTest, PlacesBombWhenWallBlocksLine) {
    Grid grid(11, 11, 0.0f);
    grid.initialize({{1, 1}});
    grid.setCell({4, 5}, CellType::DESTRUCTIBLE);
    AI_Enemy enemy({3, 5}, 0.1f);
    GameConfig cfg;
    cfg.enemyAggressive = true;
    cfg.enemyUseRL = false;
    const EnemyDecision decision = enemy.decide(grid, {9, 5}, {}, cfg, nullptr);
    EXPECT_TRUE(decision.placeBomb);
    EXPECT_TRUE(decision.breakWall);
    EXPECT_NE(0, decision.moveStep.x + decision.moveStep.y);
}

TEST(AIEnemyTest, WallBreakRespectsFiveSecondCooldown) {
    Grid grid(11, 11, 0.0f);
    grid.initialize({{1, 1}});
    grid.setCell({4, 5}, CellType::DESTRUCTIBLE);
    AI_Enemy enemy({3, 5}, 0.1f);
    GameConfig cfg;
    cfg.enemyAggressive = false;
    cfg.enemyUseRL = false;
    cfg.enemyWallBreakInterval = 5.0f;
    enemy.onWallBreakBombPlaced(cfg.enemyWallBreakInterval);
    const EnemyDecision decision = enemy.decide(grid, {9, 9}, {}, cfg, nullptr);
    EXPECT_FALSE(decision.placeBomb);
}

TEST(AIEnemyTest, SkipsBombWhenNoEscape) {
    Grid grid(7, 7, 0.0f);
    grid.initialize({{1, 1}});
    const Position trap{3, 3};
    grid.setCell({trap.x, trap.y - 1}, CellType::INDESTRUCTIBLE);
    grid.setCell({trap.x, trap.y + 1}, CellType::INDESTRUCTIBLE);
    grid.setCell({trap.x - 1, trap.y}, CellType::INDESTRUCTIBLE);
    grid.setCell({trap.x + 1, trap.y}, CellType::INDESTRUCTIBLE);
    AI_Enemy enemy(trap, 0.1f);
    GameConfig cfg;
    cfg.enemyAggressive = true;
    const EnemyDecision decision = enemy.decide(grid, {5, 3}, {}, cfg, nullptr);
    EXPECT_FALSE(decision.placeBomb);
}

TEST(AIEnemyTest, UrgentDodgeMovesOffDanger) {
    Grid grid(11, 11, 0.0f);
    grid.initialize({{1, 1}});
    for (int x = 1; x <= 8; ++x) {
        for (int y = 3; y <= 7; ++y) {
            if (!((x % 2 == 0) && (y % 2 == 0))) {
                grid.setCell({x, y}, CellType::EMPTY);
            }
        }
    }
    AI_Enemy enemy({5, 5}, 1.0f);
    std::vector<Bomb> bombs;
    bombs.emplace_back(Position{5, 5}, 2.0f, 2, BombOwner::ENEMY);
    EXPECT_TRUE(enemy.tryUrgentDodge(grid, bombs, 0.0f));
    const Position start{5, 5};
    EXPECT_FALSE(enemy.getPosition() == start);
}

TEST(AIEnemyTest, MovesWhenTimerElapses) {
    Grid grid(11, 11, 0.0f);
    grid.initialize({{1, 1}});
    AI_Enemy enemy({9, 9}, 0.1f);
    GameConfig cfg;
    EXPECT_TRUE(enemy.tickMove(0.2f));
    const EnemyDecision decision = enemy.decide(grid, {1, 1}, {}, cfg, nullptr);
    enemy.applyMove(decision.moveStep, grid);
    EXPECT_TRUE(grid.isInBounds(enemy.getPosition()));
}
