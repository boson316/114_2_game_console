#include "GameConfig.hpp"

#include <algorithm>

GameConfig GameConfig::clamped(GameConfig cfg) {
    cfg.gridWidth = std::max(11, cfg.gridWidth);
    cfg.gridHeight = std::max(11, cfg.gridHeight);
    cfg.destructibleDensity = std::clamp(cfg.destructibleDensity, 0.0f, 0.8f);
    cfg.maxBombs = std::clamp(cfg.maxBombs, 1, 5);
    cfg.blastRadius = std::clamp(cfg.blastRadius, 1, 6);
    cfg.bombFuseTime = std::max(0.5f, cfg.bombFuseTime);
    cfg.explosionDuration = std::max(0.1f, cfg.explosionDuration);
    cfg.enemyCount = std::clamp(cfg.enemyCount, 1, 4);
    cfg.enemyMoveInterval = std::max(0.1f, cfg.enemyMoveInterval);
    cfg.enemyBombFuseTime = std::max(0.8f, cfg.enemyBombFuseTime);
    cfg.enemyBlastRadius = std::clamp(cfg.enemyBlastRadius, 1, 4);
    cfg.enemyBombCooldown = std::max(0.8f, cfg.enemyBombCooldown);
    cfg.enemyWallBreakInterval = std::max(1.0f, cfg.enemyWallBreakInterval);
    cfg.enemyBombPlaceChance = std::clamp(cfg.enemyBombPlaceChance, 0.0f, 1.0f);
    cfg.enemyEngageRadius = std::clamp(cfg.enemyEngageRadius, 3, 99);
    cfg.enemyDodgeInterval = std::max(0.15f, cfg.enemyDodgeInterval);
    cfg.powerupDropChance = std::clamp(cfg.powerupDropChance, 0.0f, 1.0f);
    cfg.targetFPS = std::clamp(cfg.targetFPS, 15, 120);
    return cfg;
}
