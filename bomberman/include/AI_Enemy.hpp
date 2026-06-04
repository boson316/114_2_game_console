#pragma once

#include "Bomb.hpp"
#include "EnemyRLPolicy.hpp"
#include "GameConfig.hpp"
#include "Grid.hpp"
#include "Position.hpp"

#include <vector>

class AI_Enemy {
public:
    explicit AI_Enemy(Position startPos, float moveInterval = 0.5f);

    void updateBombCooldown(float deltaTime);
    void updateWallBreakCooldown(float deltaTime);
    void updateDodgeCooldown(float deltaTime);
    bool tickMove(float deltaTime);
    EnemyDecision decide(const Grid& grid, Position playerPos, const std::vector<Bomb>& activeBombs,
                         const GameConfig& config, const EnemyRLPolicy* policy) const;
    void applyMove(Position step, const Grid& grid);
    void onBombPlaced(float cooldownSeconds);
    void onWallBreakBombPlaced(float intervalSeconds);
    /** 每幀呼叫：在爆炸／腳下炸彈時立刻閃躲（不等移動冷卻） */
    bool tryUrgentDodge(const Grid& grid, const std::vector<Bomb>& activeBombs,
                        float dodgeIntervalSeconds);
    /** 放彈後強制離開危險格（多方向嘗試） */
    void fleeAfterBomb(const Grid& grid, Position preferredStep, int blastRadius,
                       const std::vector<Bomb>& activeBombs);

    Position getPosition() const;
    bool isAlive() const;
    void kill();

private:
    Position pos_;
    bool alive_;
    float moveInterval_;
    float moveTimer_;
    float bombCooldown_;
    float wallBreakCooldown_;
    float dodgeCooldown_;

    bool tryAssignWallBreakBomb(EnemyDecision& out, const Grid& grid,
                                const std::vector<Bomb>& activeBombs, int blastRadius) const;

    bool isInDanger(const Grid& grid, const std::vector<Bomb>& activeBombs) const;
    bool hasBombAtFeet(const std::vector<Bomb>& activeBombs) const;
    bool needsDodgeNow(const Grid& grid, const std::vector<Bomb>& activeBombs) const;
    void assignChaseStep(EnemyDecision& out, const Grid& grid, Position playerPos,
                         const std::vector<Bomb>& activeBombs) const;
    bool tryAssignSafeBomb(EnemyDecision& out, const Grid& grid,
                           const std::vector<Bomb>& activeBombs, int blastRadius) const;
    EnemyDecision decideHeuristic(const Grid& grid, Position playerPos,
                                  const std::vector<Bomb>& activeBombs,
                                  const GameConfig& config) const;
    void overlayHeuristicBomb(EnemyDecision& out, const Grid& grid, Position playerPos,
                              const std::vector<Bomb>& activeBombs,
                              const GameConfig& config) const;
    void enforceChaseStep(EnemyDecision& out, const Grid& grid, Position playerPos,
                          const std::vector<Bomb>& activeBombs) const;
    bool stepReducesDistance(Position step, Position playerPos) const;
};
