#pragma once

#include "AI_Enemy.hpp"
#include "Bomb.hpp"
#include "EnemyRLPolicy.hpp"
#include "GameConfig.hpp"
#include "Grid.hpp"
#include "Input.hpp"
#include "NetSession.hpp"
#include "PlayMode.hpp"
#include "Player.hpp"
#include "Position.hpp"
#include "PowerUpType.hpp"

#include <optional>
#include <unordered_map>
#include <vector>

enum class PlayerSlot { One, Two };

class Game {
public:
    explicit Game(GameConfig config);

    void setConfig(GameConfig config);
    void setPlayMode(PlayMode mode);
    PlayMode getPlayMode() const;

    void applyAction(InputAction action, PlayerSlot slot = PlayerSlot::One);
    void applyNetworkInput(InputAction action);
    void update(float deltaTime);

    void exportSnapshot(NetworkSnapshot& out) const;
    void importSnapshot(const NetworkSnapshot& in);

    GameState getState() const;
    const GameConfig& getConfig() const;
    const Grid& getGrid() const;
    const Player& getPlayer() const;
    const Player& getPlayer2() const;
    bool isDuoMode() const;
    const std::vector<AI_Enemy>& getEnemies() const;
    const std::vector<Bomb>& getActiveBombs() const;
    const std::vector<Position>& getExplosionCells() const;

    int getPlayerMaxBombs() const;
    int getPlayerBlastRadius() const;
    int getSpeedLevel() const;
    int getPlayer2MaxBombs() const;
    int getPlayer2BlastRadius() const;
    int getPlayer2SpeedLevel() const;
    int getAliveEnemyCount() const;
    std::optional<PowerUpType> getPowerUpAt(Position pos) const;

    void placePowerUpAt(Position pos, PowerUpType type);

private:
    GameConfig config_;
    PlayMode playMode_ = PlayMode::SOLO;
    GameState state_;
    Grid grid_;
    Player player_;
    Player player2_;
    std::vector<AI_Enemy> enemies_;
    std::vector<Bomb> activeBombs_;
    std::vector<Position> activeExplosionCells_;
    float explosionOverlayTimer_;

    int playerMaxBombs_ = 1;
    int playerBlastRadius_ = 2;
    int speedLevel_ = 0;
    float moveTimer_ = 0.0f;

    int player2MaxBombs_ = 1;
    int player2BlastRadius_ = 2;
    int speedLevel2_ = 0;
    float moveTimer2_ = 0.0f;

    std::unordered_map<Position, PowerUpType, PositionHash> powerupsOnGrid_;
    EnemyRLPolicy enemyPolicy_;

    void resetRound();
    void loadEnemyPolicy();
    void resetPlayerStats();
    int countActiveBombs() const;
    int countPlayerActiveBombs(PlayerSlot slot) const;
    bool hasLiveBombAt(Position pos) const;
    void tryEnemyPlaceBomb(AI_Enemy& enemy, bool forWallBreak);
    void handleBombExplosion(Bomb& bomb);
    void applyExplosionEffects(const std::vector<Position>& cells);
    void checkWinLoseConditions();
    bool isEnemyAt(Position pos) const;
    bool isPlayerAt(Position pos, PlayerSlot ignore) const;
    void tryMovePlayer(PlayerSlot slot, Position direction);
    void tryPickupPowerUp(PlayerSlot slot, Position pos);
    void applyPowerUp(PlayerSlot slot, PowerUpType type);
    PowerUpType rollPowerUpType();
    float currentMoveCooldown(PlayerSlot slot) const;
    Position findDuoSpawn(Position avoid) const;
};
