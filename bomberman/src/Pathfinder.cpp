#include "Pathfinder.hpp"

#include "Explosion.hpp"

#include <cmath>
#include <queue>
#include <unordered_map>

namespace {
std::optional<Position> bfsFirstStep(const Grid& grid, Position start, Position goal,
                                     const std::unordered_set<Position, PositionHash>* forbidden) {
    if (start == goal) {
        return std::nullopt;
    }
    if (!grid.isInBounds(goal) || !grid.isPassable(goal)) {
        return std::nullopt;
    }

    std::queue<Position> q;
    std::unordered_map<Position, Position, PositionHash> parent;
    std::unordered_set<Position, PositionHash> visited;

    q.push(start);
    visited.insert(start);
    parent[start] = start;

    while (!q.empty()) {
        const Position cur = q.front();
        q.pop();
        if (cur == goal) {
            Position node = goal;
            while (parent[node] != start) {
                node = parent[node];
            }
            return node;
        }
        for (const Position& dir : Direction::ALL) {
            const Position next = cur + dir;
            if (!grid.isInBounds(next) || !grid.isPassable(next)) {
                continue;
            }
            if (forbidden && forbidden->count(next) > 0) {
                continue;
            }
            if (visited.count(next) > 0) {
                continue;
            }
            visited.insert(next);
            parent[next] = cur;
            q.push(next);
        }
    }
    return std::nullopt;
}
}  // namespace

std::optional<Position> Pathfinder::nextStepToward(const Grid& grid, Position start,
                                                   Position goal) {
    return bfsFirstStep(grid, start, goal, nullptr);
}

std::optional<Position> Pathfinder::nextGreedyStepToward(const Grid& grid, Position start,
                                                          Position goal) {
    if (start == goal) {
        return std::nullopt;
    }
    const int startDist = std::abs(goal.x - start.x) + std::abs(goal.y - start.y);
    std::optional<Position> best;
    int bestDist = startDist;
    for (const Position& dir : Direction::ALL) {
        const Position next = start + dir;
        if (!grid.isInBounds(next) || !grid.isPassable(next)) {
            continue;
        }
        const int d = std::abs(goal.x - next.x) + std::abs(goal.y - next.y);
        if (d < bestDist) {
            bestDist = d;
            best = dir;
        }
    }
    return best;
}

std::optional<Position> Pathfinder::nextStepTowardAvoidingDanger(
    const Grid& grid, Position start, Position goal, const std::vector<Bomb>& activeBombs) {
    const auto danger = computeDangerZone(grid, activeBombs);
    if (const auto safeRoute = bfsFirstStep(grid, start, goal, &danger); safeRoute.has_value()) {
        return safeRoute;
    }
    return nextStepToward(grid, start, goal);
}

std::unordered_set<Position, PositionHash> Pathfinder::computeDangerZone(
    const Grid& grid, const std::vector<Bomb>& activeBombs) {
    std::unordered_set<Position, PositionHash> danger;
    for (const Bomb& bomb : activeBombs) {
        const auto cells = computeExplosionCells(grid, bomb.getPosition(), bomb.getBlastRadius());
        for (const Position& cell : cells) {
            danger.insert(cell);
        }
    }
    return danger;
}

std::optional<Position> Pathfinder::nextStepReduceDanger(const Grid& grid, Position start,
                                                         const std::vector<Bomb>& activeBombs) {
    const auto danger = computeDangerZone(grid, activeBombs);
    std::optional<Position> best;
    int bestScore = -1;
    for (const Position& dir : Direction::ALL) {
        const Position next = start + dir;
        if (!grid.isInBounds(next) || !grid.isPassable(next)) {
            continue;
        }
        int score = danger.count(next) == 0 ? 200 : 0;
        for (const Bomb& bomb : activeBombs) {
            if (bomb.isExplosionFinished()) {
                continue;
            }
            score += std::abs(next.x - bomb.getPosition().x) +
                     std::abs(next.y - bomb.getPosition().y);
        }
        if (score > bestScore) {
            bestScore = score;
            best = dir;
        }
    }
    return best;
}

int Pathfinder::countSafeReachable(const Grid& grid, Position start,
                                   const std::vector<Bomb>& activeBombs, int maxDepth) {
    const auto danger = computeDangerZone(grid, activeBombs);
    if (danger.count(start) > 0) {
        return 0;
    }

    std::queue<Position> q;
    std::unordered_set<Position, PositionHash> visited;
    std::unordered_map<Position, int, PositionHash> depth;
    q.push(start);
    visited.insert(start);
    depth[start] = 0;
    int count = 1;

    while (!q.empty()) {
        const Position cur = q.front();
        q.pop();
        if (depth[cur] >= maxDepth) {
            continue;
        }
        for (const Position& dir : Direction::ALL) {
            const Position next = cur + dir;
            if (!grid.isInBounds(next) || !grid.isPassable(next)) {
                continue;
            }
            if (danger.count(next) > 0 || visited.count(next) > 0) {
                continue;
            }
            visited.insert(next);
            depth[next] = depth[cur] + 1;
            ++count;
            q.push(next);
        }
    }
    return count;
}

std::optional<Position> Pathfinder::nextStepToSafety(const Grid& grid, Position start,
                                                     const std::vector<Bomb>& activeBombs) {
    const auto danger = computeDangerZone(grid, activeBombs);
    const bool needsDodge = danger.count(start) > 0;
    if (!needsDodge) {
        for (const Position& dir : Direction::ALL) {
            if (danger.count(start + dir) > 0) {
                if (const auto step = nextStepReduceDanger(grid, start, activeBombs);
                    step.has_value()) {
                    return step;
                }
                break;
            }
        }
        return std::nullopt;
    }

    std::queue<Position> q;
    std::unordered_map<Position, Position, PositionHash> parent;
    std::unordered_set<Position, PositionHash> visited;

    q.push(start);
    visited.insert(start);
    parent[start] = start;

    while (!q.empty()) {
        const Position cur = q.front();
        q.pop();
        if (danger.count(cur) == 0) {
            Position node = cur;
            while (parent[node] != start) {
                node = parent[node];
            }
            return node;
        }
        for (const Position& dir : Direction::ALL) {
            const Position next = cur + dir;
            if (!grid.isInBounds(next) || !grid.isPassable(next)) {
                continue;
            }
            if (visited.count(next) > 0) {
                continue;
            }
            visited.insert(next);
            parent[next] = cur;
            q.push(next);
        }
    }
    return nextStepReduceDanger(grid, start, activeBombs);
}
