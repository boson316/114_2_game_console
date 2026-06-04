#pragma once

#include "Bomb.hpp"
#include "Grid.hpp"
#include "Position.hpp"

#include <optional>
#include <unordered_set>
#include <vector>

class Pathfinder {
public:
    static std::optional<Position> nextStepToward(const Grid& grid, Position start, Position goal);

    static std::optional<Position> nextGreedyStepToward(const Grid& grid, Position start,
                                                        Position goal);

    /** 優先走非危險格繞路；無路時仍用一般 BFS */
    static std::optional<Position> nextStepTowardAvoidingDanger(
        const Grid& grid, Position start, Position goal, const std::vector<Bomb>& activeBombs);

    static std::optional<Position> nextStepToSafety(const Grid& grid, Position start,
                                                    const std::vector<Bomb>& activeBombs);

    /** 無安全格時，朝較安全鄰格移動（避免卡死） */
    static std::optional<Position> nextStepReduceDanger(const Grid& grid, Position start,
                                                        const std::vector<Bomb>& activeBombs);

    /** 從 start 起算，危險區外可到達格數（BFS 深度上限） */
    static int countSafeReachable(const Grid& grid, Position start,
                                  const std::vector<Bomb>& activeBombs, int maxDepth);

    static std::unordered_set<Position, PositionHash> computeDangerZone(
        const Grid& grid, const std::vector<Bomb>& activeBombs);
};
