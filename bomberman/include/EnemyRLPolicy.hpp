#pragma once

#include "Grid.hpp"
#include "Position.hpp"

#include <string>
#include <vector>

class EnemyRLPolicy {
public:
    static constexpr int kDxBins = 7;
    static constexpr int kDyBins = 7;
    static constexpr int kWallMaskBins = 16;
    static constexpr int kStateCount = kDxBins * kDyBins * 2 * 2 * kWallMaskBins;
    static constexpr int kActionCount = 6;

    // 0..5 = up, down, left, right, wait, place_bomb
    bool tryLoad(const std::string& path);
    bool isLoaded() const { return loaded_; }

    static int encodeState(Position enemy, Position player, bool inDanger, bool bombAtFeet,
                           const Grid& grid);

    int bestAction(int state) const;
    static Position actionToStep(int action);

private:
    bool loaded_ = false;
    std::vector<float> q_;
};

struct EnemyDecision {
    Position moveStep{0, 0};
    bool placeBomb = false;
    /** 為炸開可破壞牆；冷卻用 enemyWallBreakInterval（預設 5 秒） */
    bool breakWall = false;
};
