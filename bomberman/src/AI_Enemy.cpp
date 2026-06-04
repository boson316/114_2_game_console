#include "AI_Enemy.hpp"

#include "Explosion.hpp"
#include "Pathfinder.hpp"

#include <algorithm>
#include <cmath>
#include <optional>

namespace {

int manhattan(Position a, Position b) {
    return std::abs(a.x - b.x) + std::abs(a.y - b.y);
}

bool bombAtHitsPosition(const Grid& grid, Position bombPos, Position target, int blastRadius) {
    const auto cells = computeExplosionCells(grid, bombPos, blastRadius);
    for (const Position& cell : cells) {
        if (cell == target) {
            return true;
        }
    }
    return false;
}

std::optional<Position> escapeAfterBombAt(const Grid& grid, Position bombPos, int blastRadius,
                                          const std::vector<Bomb>& activeBombs) {
    std::vector<Bomb> bombs = activeBombs;
    const bool alreadyListed =
        std::any_of(bombs.begin(), bombs.end(),
                    [&](const Bomb& b) { return b.getPosition() == bombPos; });
    if (!alreadyListed) {
        bombs.emplace_back(bombPos, 1.0f, blastRadius, BombOwner::ENEMY);
    }
    if (const auto bfsStep = Pathfinder::nextStepToSafety(grid, bombPos, bombs); bfsStep.has_value()) {
        return bfsStep;
    }

    const auto danger = Pathfinder::computeDangerZone(grid, bombs);

    std::optional<Position> best;
    int bestScore = -1;
    for (const Position& dir : Direction::ALL) {
        const Position next = bombPos + dir;
        if (!grid.isInBounds(next) || !grid.isPassable(next)) {
            continue;
        }
        if (danger.count(next) > 0) {
            continue;
        }
        int safeDepth = 0;
        for (const Position& dir2 : Direction::ALL) {
            const Position n2 = next + dir2;
            if (grid.isInBounds(n2) && grid.isPassable(n2) && danger.count(n2) == 0) {
                ++safeDepth;
            }
        }
        if (safeDepth > bestScore) {
            bestScore = safeDepth;
            best = dir;
        }
    }
    if (best.has_value()) {
        return best;
    }
    return Pathfinder::nextStepToSafety(grid, bombPos, bombs);
}

bool shouldBombDestructibleWall(const Grid& grid, Position enemy, Position player) {
    for (const Position& dir : Direction::ALL) {
        const Position wall = enemy + dir;
        if (grid.getCell(wall) != CellType::DESTRUCTIBLE) {
            continue;
        }
        if (enemy.x == player.x && wall.x == enemy.x) {
            const int lo = std::min(enemy.y, player.y);
            const int hi = std::max(enemy.y, player.y);
            if (wall.y > lo && wall.y < hi) {
                return true;
            }
        }
        if (enemy.y == player.y && wall.y == enemy.y) {
            const int lo = std::min(enemy.x, player.x);
            const int hi = std::max(enemy.x, player.x);
            if (wall.x > lo && wall.x < hi) {
                return true;
            }
        }
        const Position beyond = wall + dir;
        if (grid.isInBounds(beyond) && grid.isPassable(beyond)) {
            if (manhattan(beyond, player) < manhattan(enemy, player)) {
                return true;
            }
        }
    }
    return false;
}

bool hasAdequateEscapeSpace(const Grid& grid, Position bombPos, int blastRadius,
                            const std::vector<Bomb>& activeBombs) {
    const auto escape = escapeAfterBombAt(grid, bombPos, blastRadius, activeBombs);
    if (!escape.has_value()) {
        return false;
    }
    std::vector<Bomb> bombs = activeBombs;
    if (!std::any_of(bombs.begin(), bombs.end(),
                     [&](const Bomb& b) { return b.getPosition() == bombPos; })) {
        bombs.emplace_back(bombPos, 1.0f, blastRadius, BombOwner::ENEMY);
    }
    const Position landing = bombPos + *escape;
    if (Pathfinder::computeDangerZone(grid, bombs).count(landing) > 0) {
        return false;
    }
    constexpr int kMinSafeTiles = 3;
    return Pathfinder::countSafeReachable(grid, landing, bombs, 5) >= kMinSafeTiles;
}

bool canSafelyPlaceBomb(const Grid& grid, Position bombPos, int blastRadius,
                        const std::vector<Bomb>& activeBombs) {
    return hasAdequateEscapeSpace(grid, bombPos, blastRadius, activeBombs);
}

bool canBombDestroyDestructibleNow(const Grid& grid, Position bombPos, int blastRadius) {
    const auto cells = computeExplosionCells(grid, bombPos, blastRadius);
    for (const Position& cell : cells) {
        if (grid.getCell(cell) == CellType::DESTRUCTIBLE) {
            return true;
        }
    }
    return false;
}

}  // namespace

AI_Enemy::AI_Enemy(Position startPos, float moveInterval)
    : pos_(startPos),
      alive_(true),
      moveInterval_(moveInterval),
      moveTimer_(0.0f),
      bombCooldown_(0.0f),
      wallBreakCooldown_(0.0f),
      dodgeCooldown_(0.0f) {}

void AI_Enemy::updateBombCooldown(float deltaTime) {
    if (bombCooldown_ > 0.0f) {
        bombCooldown_ -= deltaTime;
        if (bombCooldown_ < 0.0f) {
            bombCooldown_ = 0.0f;
        }
    }
}

void AI_Enemy::updateWallBreakCooldown(float deltaTime) {
    if (wallBreakCooldown_ > 0.0f) {
        wallBreakCooldown_ -= deltaTime;
        if (wallBreakCooldown_ < 0.0f) {
            wallBreakCooldown_ = 0.0f;
        }
    }
}

void AI_Enemy::updateDodgeCooldown(float deltaTime) {
    if (dodgeCooldown_ > 0.0f) {
        dodgeCooldown_ -= deltaTime;
        if (dodgeCooldown_ < 0.0f) {
            dodgeCooldown_ = 0.0f;
        }
    }
}

bool AI_Enemy::tickMove(float deltaTime) {
    if (!alive_) {
        return false;
    }
    moveTimer_ += deltaTime;
    if (moveTimer_ < moveInterval_) {
        return false;
    }
    moveTimer_ = 0.0f;
    return true;
}

void AI_Enemy::applyMove(Position step, const Grid& grid) {
    if (step.x == 0 && step.y == 0) {
        return;
    }
    const Position target = pos_ + step;
    if (!grid.isInBounds(target) || !grid.isPassable(target)) {
        return;
    }
    pos_ = target;
}

Position AI_Enemy::getPosition() const { return pos_; }

bool AI_Enemy::isAlive() const { return alive_; }

void AI_Enemy::kill() { alive_ = false; }

void AI_Enemy::onBombPlaced(float cooldownSeconds) { bombCooldown_ = cooldownSeconds; }

void AI_Enemy::onWallBreakBombPlaced(float intervalSeconds) {
    wallBreakCooldown_ = intervalSeconds;
}

bool AI_Enemy::isInDanger(const Grid& grid, const std::vector<Bomb>& activeBombs) const {
    const auto danger = Pathfinder::computeDangerZone(grid, activeBombs);
    return danger.count(pos_) > 0;
}

bool AI_Enemy::hasBombAtFeet(const std::vector<Bomb>& activeBombs) const {
    for (const Bomb& bomb : activeBombs) {
        if (!bomb.isExplosionFinished() && bomb.getPosition() == pos_) {
            return true;
        }
    }
    return false;
}

bool AI_Enemy::needsDodgeNow(const Grid& grid, const std::vector<Bomb>& activeBombs) const {
    (void)grid;
    return isInDanger(grid, activeBombs) || hasBombAtFeet(activeBombs);
}

bool AI_Enemy::tryUrgentDodge(const Grid& grid, const std::vector<Bomb>& activeBombs,
                              float dodgeIntervalSeconds) {
    if (!alive_ || dodgeCooldown_ > 0.0f) {
        return false;
    }
    if (!needsDodgeNow(grid, activeBombs)) {
        return false;
    }
    std::optional<Position> escape = Pathfinder::nextStepToSafety(grid, pos_, activeBombs);
    if (!escape.has_value()) {
        escape = Pathfinder::nextStepReduceDanger(grid, pos_, activeBombs);
    }
    if (!escape.has_value()) {
        return false;
    }
    applyMove(*escape, grid);
    dodgeCooldown_ = dodgeIntervalSeconds;
    return true;
}

void AI_Enemy::fleeAfterBomb(const Grid& grid, Position preferredStep, int blastRadius,
                             const std::vector<Bomb>& activeBombs) {
    if (!alive_) {
        return;
    }
    for (int attempt = 0; attempt < 1 && alive_; ++attempt) {
        if (!needsDodgeNow(grid, activeBombs) && !hasBombAtFeet(activeBombs)) {
            break;
        }

        std::vector<Position> tries;
        if (preferredStep.x != 0 || preferredStep.y != 0) {
            tries.push_back(preferredStep);
            preferredStep = {0, 0};
        }
        if (const auto computed = escapeAfterBombAt(grid, pos_, blastRadius, activeBombs);
            computed.has_value()) {
            tries.push_back(*computed);
        }
        if (const auto reduce = Pathfinder::nextStepReduceDanger(grid, pos_, activeBombs);
            reduce.has_value()) {
            tries.push_back(*reduce);
        }
        for (const Position& dir : Direction::ALL) {
            tries.push_back(dir);
        }

        std::vector<Bomb> bombs = activeBombs;
        if (!std::any_of(bombs.begin(), bombs.end(),
                         [&](const Bomb& b) { return b.getPosition() == pos_; })) {
            bombs.emplace_back(pos_, 1.0f, blastRadius, BombOwner::ENEMY);
        }
        const auto danger = Pathfinder::computeDangerZone(grid, bombs);

        bool moved = false;
        for (const Position& step : tries) {
            if (step.x == 0 && step.y == 0) {
                continue;
            }
            const Position target = pos_ + step;
            if (!grid.isInBounds(target) || !grid.isPassable(target)) {
                continue;
            }
            if (danger.count(target) > 0) {
                continue;
            }
            applyMove(step, grid);
            moved = true;
            break;
        }
        if (!moved) {
            break;
        }
    }
}

void AI_Enemy::assignChaseStep(EnemyDecision& out, const Grid& grid, Position playerPos,
                               const std::vector<Bomb>& activeBombs) const {
    if (const auto detour =
            Pathfinder::nextStepTowardAvoidingDanger(grid, pos_, playerPos, activeBombs);
        detour.has_value()) {
        out.moveStep = *detour;
        return;
    }
    if (const auto toward = Pathfinder::nextStepToward(grid, pos_, playerPos); toward.has_value()) {
        out.moveStep = *toward;
        return;
    }
    if (const auto greedy = Pathfinder::nextGreedyStepToward(grid, pos_, playerPos);
        greedy.has_value()) {
        out.moveStep = *greedy;
        return;
    }
    const auto danger = Pathfinder::computeDangerZone(grid, activeBombs);
    std::optional<Position> best;
    int bestDist = manhattan(pos_, playerPos);
    for (const Position& dir : Direction::ALL) {
        const Position next = pos_ + dir;
        if (!grid.isInBounds(next) || !grid.isPassable(next)) {
            continue;
        }
        if (danger.count(next) > 0) {
            continue;
        }
        const int d = manhattan(next, playerPos);
        if (d < bestDist) {
            bestDist = d;
            best = dir;
        }
    }
    if (best.has_value()) {
        out.moveStep = *best;
    }
}

bool AI_Enemy::tryAssignSafeBomb(EnemyDecision& out, const Grid& grid,
                                 const std::vector<Bomb>& activeBombs, int blastRadius) const {
    const auto escape = escapeAfterBombAt(grid, pos_, blastRadius, activeBombs);
    if (!escape.has_value()) {
        return false;
    }
    out.placeBomb = true;
    out.moveStep = *escape;
    return true;
}

bool AI_Enemy::tryAssignWallBreakBomb(EnemyDecision& out, const Grid& grid,
                                      const std::vector<Bomb>& activeBombs,
                                      int blastRadius) const {
    if (!tryAssignSafeBomb(out, grid, activeBombs, blastRadius)) {
        return false;
    }
    out.breakWall = true;
    return true;
}

bool AI_Enemy::stepReducesDistance(Position step, Position playerPos) const {
    if (step.x == 0 && step.y == 0) {
        return false;
    }
    const int before = manhattan(pos_, playerPos);
    const int after = manhattan(pos_ + step, playerPos);
    return after < before;
}

void AI_Enemy::enforceChaseStep(EnemyDecision& out, const Grid& grid, Position playerPos,
                                const std::vector<Bomb>& activeBombs) const {
    if (out.placeBomb) {
        return;
    }
    const int dist = manhattan(pos_, playerPos);
    if (dist == 0) {
        return;
    }
    if (stepReducesDistance(out.moveStep, playerPos)) {
        return;
    }
    EnemyDecision chase{};
    assignChaseStep(chase, grid, playerPos, activeBombs);
    if (chase.moveStep.x != 0 || chase.moveStep.y != 0) {
        out.moveStep = chase.moveStep;
    }
}

void AI_Enemy::overlayHeuristicBomb(EnemyDecision& out, const Grid& grid, Position playerPos,
                                    const std::vector<Bomb>& activeBombs,
                                    const GameConfig& config) const {
    if (bombCooldown_ > 0.0f || needsDodgeNow(grid, activeBombs)) {
        return;
    }
    const int blast = config.enemyBlastRadius;
    const bool hitsPlayer = bombAtHitsPosition(grid, pos_, playerPos, blast);
    if (config.enemyAggressive && hitsPlayer &&
        canSafelyPlaceBomb(grid, pos_, blast, activeBombs)) {
        tryAssignSafeBomb(out, grid, activeBombs, blast);
        return;
    }
    if (wallBreakCooldown_ <= 0.0f &&
        canBombDestroyDestructibleNow(grid, pos_, blast) &&
        canSafelyPlaceBomb(grid, pos_, blast, activeBombs)) {
        tryAssignWallBreakBomb(out, grid, activeBombs, blast);
    }
}

EnemyDecision AI_Enemy::decideHeuristic(const Grid& grid, Position playerPos,
                                        const std::vector<Bomb>& activeBombs,
                                        const GameConfig& config) const {
    EnemyDecision out{};
    const int dist = manhattan(pos_, playerPos);
    const int blast = config.enemyBlastRadius;

    if (needsDodgeNow(grid, activeBombs)) {
        std::optional<Position> escape = Pathfinder::nextStepToSafety(grid, pos_, activeBombs);
        if (!escape.has_value()) {
            escape = Pathfinder::nextStepReduceDanger(grid, pos_, activeBombs);
        }
        if (escape.has_value()) {
            out.moveStep = *escape;
        }
        return out;
    }

    if (bombCooldown_ <= 0.0f) {
        const bool hitsPlayer = bombAtHitsPosition(grid, pos_, playerPos, blast);

        if (config.enemyAggressive && hitsPlayer &&
            canSafelyPlaceBomb(grid, pos_, blast, activeBombs)) {
            tryAssignSafeBomb(out, grid, activeBombs, blast);
            return out;
        }
        if (wallBreakCooldown_ <= 0.0f &&
            canBombDestroyDestructibleNow(grid, pos_, blast) &&
            canSafelyPlaceBomb(grid, pos_, blast, activeBombs)) {
            tryAssignWallBreakBomb(out, grid, activeBombs, blast);
            return out;
        }
        if (!config.enemyAggressive && dist >= 1 && dist <= blast + 2) {
            const float roll =
                static_cast<float>((pos_.x * 31 + pos_.y * 17 + dist) % 100) / 100.0f;
            if (roll < config.enemyBombPlaceChance &&
                bombAtHitsPosition(grid, pos_, playerPos, blast) &&
                canSafelyPlaceBomb(grid, pos_, blast, activeBombs)) {
                tryAssignSafeBomb(out, grid, activeBombs, blast);
                return out;
            }
        }
    }

    assignChaseStep(out, grid, playerPos, activeBombs);

    if (config.enemyAggressive && dist == 1) {
        for (const Position& dir : Direction::ALL) {
            const Position next = pos_ + dir;
            if (next == playerPos) {
                out.moveStep = dir;
                break;
            }
        }
    }

    return out;
}

EnemyDecision AI_Enemy::decide(const Grid& grid, Position playerPos,
                               const std::vector<Bomb>& activeBombs, const GameConfig& config,
                               const EnemyRLPolicy* policy) const {
    if (!alive_) {
        return {};
    }

    if (needsDodgeNow(grid, activeBombs)) {
        return decideHeuristic(grid, playerPos, activeBombs, config);
    }

    if (policy != nullptr && policy->isLoaded() && config.enemyUseRL) {
        const bool danger = isInDanger(grid, activeBombs);
        const bool bombFeet = hasBombAtFeet(activeBombs);
        const int state = EnemyRLPolicy::encodeState(pos_, playerPos, danger, bombFeet, grid);
        const int action = policy->bestAction(state);
        EnemyDecision out{};
        out.moveStep = EnemyRLPolicy::actionToStep(action);
        out.placeBomb = (action == 5);
        if (out.placeBomb && (danger || bombFeet || bombCooldown_ > 0.0f)) {
            out.placeBomb = false;
        }
        if (out.placeBomb && !canSafelyPlaceBomb(grid, pos_, config.enemyBlastRadius, activeBombs)) {
            out.placeBomb = false;
        }
        overlayHeuristicBomb(out, grid, playerPos, activeBombs, config);
        enforceChaseStep(out, grid, playerPos, activeBombs);
        if (config.enemyAggressive && manhattan(pos_, playerPos) == 1) {
            for (const Position& dir : Direction::ALL) {
                if (pos_ + dir == playerPos) {
                    out.moveStep = dir;
                    break;
                }
            }
        }
        return out;
    }

    return decideHeuristic(grid, playerPos, activeBombs, config);
}
