#include "Game.hpp"

#include "Explosion.hpp"

#include <algorithm>
#include <cmath>
#include <queue>
#include <random>
#include <unordered_set>

namespace {
constexpr Position kPlayerSpawn{1, 1};

bool cellInList(const std::vector<Position>& cells, Position pos) {
    return std::find(cells.begin(), cells.end(), pos) != cells.end();
}

Position pickChaseTarget(Position enemyPos, Position livePlayer, int engageRadius) {
    const int toPlayer = std::abs(livePlayer.x - enemyPos.x) + std::abs(livePlayer.y - enemyPos.y);
    const int toSpawn =
        std::abs(kPlayerSpawn.x - enemyPos.x) + std::abs(kPlayerSpawn.y - enemyPos.y);
    if (toSpawn <= engageRadius && toSpawn < toPlayer) {
        return kPlayerSpawn;
    }
    if (toPlayer <= engageRadius) {
        return livePlayer;
    }
    return toSpawn <= toPlayer ? kPlayerSpawn : livePlayer;
}

std::mt19937& gameRng() {
    static std::mt19937 rng{std::random_device{}()};
    return rng;
}
}  // namespace

Game::Game(GameConfig config)
    : config_(GameConfig::clamped(config)),
      state_(GameState::INIT),
      grid_(config_.gridWidth, config_.gridHeight, config_.destructibleDensity),
      player_({1, 1}),
      player2_({0, 0}),
      explosionOverlayTimer_(0.0f) {
    loadEnemyPolicy();
    resetRound();
}

void Game::loadEnemyPolicy() {
    const char* paths[] = {"assets/ai/enemy_qtable.json", "../assets/ai/enemy_qtable.json",
                           "../../assets/ai/enemy_qtable.json"};
    for (const char* path : paths) {
        if (enemyPolicy_.tryLoad(path)) {
            return;
        }
    }
}

void Game::setConfig(GameConfig config) {
    config_ = GameConfig::clamped(config);
}

void Game::setPlayMode(PlayMode mode) { playMode_ = mode; }

PlayMode Game::getPlayMode() const { return playMode_; }

bool Game::isDuoMode() const {
    return playMode_ == PlayMode::LOCAL_DUO || playMode_ == PlayMode::ONLINE_HOST ||
           playMode_ == PlayMode::ONLINE_CLIENT;
}

void Game::resetPlayerStats() {
    playerMaxBombs_ = config_.maxBombs;
    playerBlastRadius_ = config_.blastRadius;
    speedLevel_ = 0;
    moveTimer_ = 0.0f;
    player2MaxBombs_ = config_.maxBombs;
    player2BlastRadius_ = config_.blastRadius;
    speedLevel2_ = 0;
    moveTimer2_ = 0.0f;
    powerupsOnGrid_.clear();
}

Position Game::findDuoSpawn(Position avoid) const {
    Position best{grid_.getWidth() - 2, grid_.getHeight() - 2};
    int bestDist = -1;
    for (int y = 0; y < grid_.getHeight(); ++y) {
        for (int x = 0; x < grid_.getWidth(); ++x) {
            Position p{x, y};
            if (!grid_.isPassable(p)) {
                continue;
            }
            const int d = std::abs(p.x - avoid.x) + std::abs(p.y - avoid.y);
            if (d > bestDist) {
                bestDist = d;
                best = p;
            }
        }
    }
    return best;
}

void Game::resetRound() {
    resetPlayerStats();
    const Position playerStart{1, 1};
    player_ = Player(playerStart);
    const bool duo = isDuoMode();
    grid_ = Grid(config_.gridWidth, config_.gridHeight, config_.destructibleDensity);
    if (duo) {
        const Position p2Start = findDuoSpawn(playerStart);
        player2_ = Player(p2Start);
        grid_.initialize({playerStart, p2Start});
    } else {
        grid_.initialize({playerStart});
    }

    enemies_.clear();
    std::vector<Position> enemyCandidates;
    for (int y = 0; y < grid_.getHeight(); ++y) {
        for (int x = 0; x < grid_.getWidth(); ++x) {
            Position p{x, y};
            if (!grid_.isPassable(p)) {
                continue;
            }
            if (p.x <= playerStart.x + 1 && p.y <= playerStart.y + 1) {
                continue;
            }
            enemyCandidates.push_back(p);
        }
    }
    std::sort(enemyCandidates.begin(), enemyCandidates.end(),
              [playerStart](const Position& a, const Position& b) {
                  const int da = std::abs(a.x - playerStart.x) + std::abs(a.y - playerStart.y);
                  const int db = std::abs(b.x - playerStart.x) + std::abs(b.y - playerStart.y);
                  return da > db;
              });
    for (int i = 0; i < config_.enemyCount && i < static_cast<int>(enemyCandidates.size()); ++i) {
        enemies_.emplace_back(enemyCandidates[static_cast<std::size_t>(i)],
                              config_.enemyMoveInterval);
    }

    activeBombs_.clear();
    activeExplosionCells_.clear();
    explosionOverlayTimer_ = 0.0f;
    state_ = GameState::PLAYING;
}

GameState Game::getState() const { return state_; }

const GameConfig& Game::getConfig() const { return config_; }

const Grid& Game::getGrid() const { return grid_; }

const Player& Game::getPlayer() const { return player_; }

const Player& Game::getPlayer2() const { return player2_; }

const std::vector<AI_Enemy>& Game::getEnemies() const { return enemies_; }

const std::vector<Bomb>& Game::getActiveBombs() const { return activeBombs_; }

const std::vector<Position>& Game::getExplosionCells() const { return activeExplosionCells_; }

int Game::getPlayerMaxBombs() const { return playerMaxBombs_; }

int Game::getPlayerBlastRadius() const { return playerBlastRadius_; }

int Game::getSpeedLevel() const { return speedLevel_; }

int Game::getPlayer2MaxBombs() const { return player2MaxBombs_; }

int Game::getPlayer2BlastRadius() const { return player2BlastRadius_; }

int Game::getPlayer2SpeedLevel() const { return speedLevel2_; }

std::optional<PowerUpType> Game::getPowerUpAt(Position pos) const {
    const auto it = powerupsOnGrid_.find(pos);
    if (it == powerupsOnGrid_.end()) {
        return std::nullopt;
    }
    return it->second;
}

void Game::placePowerUpAt(Position pos, PowerUpType type) {
    if (!grid_.isInBounds(pos)) {
        return;
    }
    grid_.setCell(pos, CellType::POWERUP);
    powerupsOnGrid_[pos] = type;
}

PowerUpType Game::rollPowerUpType() {
    std::uniform_int_distribution<int> dist(0, 2);
    const int v = dist(gameRng());
    if (v == 0) {
        return PowerUpType::BOMB_UP;
    }
    if (v == 1) {
        return PowerUpType::FIRE_UP;
    }
    return PowerUpType::SPEED_UP;
}

float Game::currentMoveCooldown(PlayerSlot slot) const {
    const int level = slot == PlayerSlot::One ? speedLevel_ : speedLevel2_;
    if (level <= 0) {
        return 0.0f;
    }
    if (level == 1) {
        return 0.10f;
    }
    if (level == 2) {
        return 0.07f;
    }
    return 0.04f;
}

void Game::applyPowerUp(PlayerSlot slot, PowerUpType type) {
    if (slot == PlayerSlot::One) {
        switch (type) {
            case PowerUpType::BOMB_UP:
                if (playerMaxBombs_ < kMaxBombUp) {
                    ++playerMaxBombs_;
                }
                break;
            case PowerUpType::FIRE_UP:
                if (playerBlastRadius_ < kMaxFireUp) {
                    ++playerBlastRadius_;
                }
                break;
            case PowerUpType::SPEED_UP:
                if (playerMaxBombs_ < kMaxBombUp) {
                    ++playerMaxBombs_;
                }
                break;
        }
        return;
    }
    switch (type) {
        case PowerUpType::BOMB_UP:
            if (player2MaxBombs_ < kMaxBombUp) {
                ++player2MaxBombs_;
            }
            break;
        case PowerUpType::FIRE_UP:
            if (player2BlastRadius_ < kMaxFireUp) {
                ++player2BlastRadius_;
            }
            break;
        case PowerUpType::SPEED_UP:
            if (player2MaxBombs_ < kMaxBombUp) {
                ++player2MaxBombs_;
            }
            break;
    }
}

void Game::tryPickupPowerUp(PlayerSlot slot, Position pos) {
    const auto it = powerupsOnGrid_.find(pos);
    if (it == powerupsOnGrid_.end()) {
        return;
    }
    applyPowerUp(slot, it->second);
    powerupsOnGrid_.erase(it);
    if (grid_.getCell(pos) == CellType::POWERUP) {
        grid_.setCell(pos, CellType::EMPTY);
    }
}

int Game::countActiveBombs() const {
    int count = 0;
    for (const Bomb& bomb : activeBombs_) {
        if (!bomb.isExplosionFinished()) {
            ++count;
        }
    }
    return count;
}

int Game::countPlayerActiveBombs(PlayerSlot slot) const {
    const BombOwner owner = slot == PlayerSlot::One ? BombOwner::PLAYER : BombOwner::PLAYER2;
    int count = 0;
    for (const Bomb& bomb : activeBombs_) {
        if (!bomb.isExplosionFinished() && bomb.getOwner() == owner) {
            ++count;
        }
    }
    return count;
}

bool Game::hasLiveBombAt(Position pos) const {
    for (const Bomb& bomb : activeBombs_) {
        if (!bomb.isExplosionFinished() && bomb.getPosition() == pos) {
            return true;
        }
    }
    return false;
}

int Game::getAliveEnemyCount() const {
    int n = 0;
    for (const AI_Enemy& enemy : enemies_) {
        if (enemy.isAlive()) {
            ++n;
        }
    }
    return n;
}

void Game::tryEnemyPlaceBomb(AI_Enemy& enemy, bool forWallBreak) {
    if (!enemy.isAlive() || state_ != GameState::PLAYING) {
        return;
    }
    if (hasLiveBombAt(enemy.getPosition())) {
        return;
    }
    activeBombs_.emplace_back(enemy.getPosition(), config_.enemyBombFuseTime,
                              config_.enemyBlastRadius, BombOwner::ENEMY);
    enemy.onBombPlaced(config_.enemyBombCooldown);
    if (forWallBreak) {
        enemy.onWallBreakBombPlaced(config_.enemyWallBreakInterval);
    }
}

bool Game::isEnemyAt(Position pos) const {
    for (const AI_Enemy& enemy : enemies_) {
        if (enemy.isAlive() && enemy.getPosition() == pos) {
            return true;
        }
    }
    return false;
}

bool Game::isPlayerAt(Position pos, PlayerSlot ignore) const {
    if (ignore != PlayerSlot::One && player_.isAlive() && player_.getPosition() == pos) {
        return true;
    }
    if (isDuoMode() && ignore != PlayerSlot::Two && player2_.isAlive() &&
        player2_.getPosition() == pos) {
        return true;
    }
    return false;
}

void Game::tryMovePlayer(PlayerSlot slot, Position direction) {
    Player* pl = slot == PlayerSlot::One ? &player_ : &player2_;
    float* timer = slot == PlayerSlot::One ? &moveTimer_ : &moveTimer2_;
    if (state_ != GameState::PLAYING || !pl->isAlive()) {
        return;
    }
    if (*timer > 0.0f) {
        return;
    }
    const Position before = pl->getPosition();
    const Position target = before + direction;
    if (isEnemyAt(target)) {
        pl->kill();
        checkWinLoseConditions();
        return;
    }
    if (isPlayerAt(target, slot)) {
        return;
    }
    pl->move(direction, grid_);
    if (pl->getPosition() != before) {
        tryPickupPowerUp(slot, pl->getPosition());
        *timer = currentMoveCooldown(slot);
    }
}

void Game::applyAction(InputAction action, PlayerSlot slot) {
    if (action == InputAction::QUIT) {
        state_ = GameState::QUIT;
        return;
    }
    if (action == InputAction::RESTART &&
        (state_ == GameState::GAME_OVER || state_ == GameState::VICTORY)) {
        resetRound();
        return;
    }
    if (state_ != GameState::PLAYING) {
        return;
    }
    Player* pl = slot == PlayerSlot::One ? &player_ : &player2_;
    const int maxBombs = slot == PlayerSlot::One ? playerMaxBombs_ : player2MaxBombs_;
    const int blast = slot == PlayerSlot::One ? playerBlastRadius_ : player2BlastRadius_;
    const BombOwner owner = slot == PlayerSlot::One ? BombOwner::PLAYER : BombOwner::PLAYER2;
    switch (action) {
        case InputAction::MOVE_UP:
            tryMovePlayer(slot, Direction::UP);
            break;
        case InputAction::MOVE_DOWN:
            tryMovePlayer(slot, Direction::DOWN);
            break;
        case InputAction::MOVE_LEFT:
            tryMovePlayer(slot, Direction::LEFT);
            break;
        case InputAction::MOVE_RIGHT:
            tryMovePlayer(slot, Direction::RIGHT);
            break;
        case InputAction::PLACE_BOMB:
            if (pl->tryPlaceBomb(maxBombs, countPlayerActiveBombs(slot))) {
                activeBombs_.emplace_back(pl->getPosition(), config_.bombFuseTime, blast, owner);
            }
            break;
        default:
            break;
    }
}

void Game::applyNetworkInput(InputAction action) {
    applyAction(action, PlayerSlot::Two);
}

void Game::applyExplosionEffects(const std::vector<Position>& cells) {
    for (const Position& cell : cells) {
        if (grid_.getCell(cell) == CellType::DESTRUCTIBLE) {
            grid_.destroyDestructible(cell, config_.powerupDropChance);
            if (grid_.getCell(cell) == CellType::POWERUP) {
                placePowerUpAt(cell, rollPowerUpType());
            }
        }
        if (player_.isAlive() && player_.getPosition() == cell) {
            player_.kill();
        }
        if (isDuoMode() && player2_.isAlive() && player2_.getPosition() == cell) {
            player2_.kill();
        }
        for (AI_Enemy& enemy : enemies_) {
            if (enemy.isAlive() && enemy.getPosition() == cell) {
                enemy.kill();
                player_.addScore(100);
            }
        }
    }
    enemies_.erase(std::remove_if(enemies_.begin(), enemies_.end(),
                                  [](const AI_Enemy& e) { return !e.isAlive(); }),
                   enemies_.end());
    checkWinLoseConditions();
}

void Game::handleBombExplosion(Bomb& bomb) {
    std::unordered_set<int> processed;
    std::queue<Bomb*> toProcess;
    toProcess.push(&bomb);

    activeExplosionCells_.clear();

    while (!toProcess.empty()) {
        Bomb* current = toProcess.front();
        toProcess.pop();
        if (processed.count(current->getId()) > 0) {
            continue;
        }
        processed.insert(current->getId());
        current->startExplosion(config_.explosionDuration);

        const auto cells =
            computeExplosionCells(grid_, current->getPosition(), current->getBlastRadius());
        for (const Position& cell : cells) {
            if (!cellInList(activeExplosionCells_, cell)) {
                activeExplosionCells_.push_back(cell);
            }
        }
        applyExplosionEffects(cells);

        for (Bomb& other : activeBombs_) {
            if (processed.count(other.getId()) > 0 || other.isExplosionFinished()) {
                continue;
            }
            for (const Position& cell : cells) {
                if (cell == other.getPosition()) {
                    toProcess.push(&other);
                    break;
                }
            }
        }
    }
    explosionOverlayTimer_ = config_.explosionDuration;
}

void Game::checkWinLoseConditions() {
    const bool p1ok = player_.isAlive();
    const bool p2ok = !isDuoMode() || player2_.isAlive();
    if (!p1ok || !p2ok) {
        state_ = GameState::GAME_OVER;
        return;
    }
    const bool anyAlive =
        std::any_of(enemies_.begin(), enemies_.end(), [](const AI_Enemy& e) { return e.isAlive(); });
    if (!anyAlive) {
        state_ = GameState::VICTORY;
    }
}

void Game::update(float deltaTime) {
    if (state_ != GameState::PLAYING) {
        return;
    }

    if (moveTimer_ > 0.0f) {
        moveTimer_ -= deltaTime;
        if (moveTimer_ < 0.0f) {
            moveTimer_ = 0.0f;
        }
    }
    if (moveTimer2_ > 0.0f) {
        moveTimer2_ -= deltaTime;
        if (moveTimer2_ < 0.0f) {
            moveTimer2_ = 0.0f;
        }
    }

    const EnemyRLPolicy* policy =
        (config_.enemyUseRL && enemyPolicy_.isLoaded()) ? &enemyPolicy_ : nullptr;
    for (AI_Enemy& enemy : enemies_) {
        if (!enemy.isAlive()) {
            continue;
        }
        enemy.updateBombCooldown(deltaTime);
        enemy.updateWallBreakCooldown(deltaTime);
        enemy.updateDodgeCooldown(deltaTime);
        enemy.tryUrgentDodge(grid_, activeBombs_, config_.enemyDodgeInterval);
        if (!enemy.tickMove(deltaTime)) {
            continue;
        }
        Position liveTarget = player_.isAlive() ? player_.getPosition() : kPlayerSpawn;
        if (isDuoMode() && player2_.isAlive()) {
            const int d1 = std::abs(liveTarget.x - enemy.getPosition().x) +
                           std::abs(liveTarget.y - enemy.getPosition().y);
            const Position p2 = player2_.getPosition();
            const int d2 =
                std::abs(p2.x - enemy.getPosition().x) + std::abs(p2.y - enemy.getPosition().y);
            if (!player_.isAlive() || d2 < d1) {
                liveTarget = p2;
            }
        }
        const Position chaseTarget =
            pickChaseTarget(enemy.getPosition(), liveTarget, config_.enemyEngageRadius);
        const EnemyDecision decision =
            enemy.decide(grid_, chaseTarget, activeBombs_, config_, policy);
        if (decision.placeBomb) {
            tryEnemyPlaceBomb(enemy, decision.breakWall);
            enemy.fleeAfterBomb(grid_, decision.moveStep, config_.enemyBlastRadius, activeBombs_);
        } else {
            enemy.applyMove(decision.moveStep, grid_);
        }
        if (enemy.isAlive() && player_.isAlive() && enemy.getPosition() == player_.getPosition()) {
            player_.kill();
            checkWinLoseConditions();
            return;
        }
        if (enemy.isAlive() && isDuoMode() && player2_.isAlive() &&
            enemy.getPosition() == player2_.getPosition()) {
            player2_.kill();
            checkWinLoseConditions();
            return;
        }
    }

    std::vector<Bomb*> pendingExplosions;
    for (Bomb& bomb : activeBombs_) {
        bomb.update(deltaTime);
        if (!bomb.isExploded() && bomb.isFuseExpired()) {
            pendingExplosions.push_back(&bomb);
        }
    }
    for (Bomb* bomb : pendingExplosions) {
        handleBombExplosion(*bomb);
    }

    if (explosionOverlayTimer_ > 0.0f) {
        explosionOverlayTimer_ -= deltaTime;
        if (explosionOverlayTimer_ <= 0.0f) {
            activeExplosionCells_.clear();
        }
    }

    activeBombs_.erase(std::remove_if(activeBombs_.begin(), activeBombs_.end(),
                                      [](Bomb& b) { return b.isExplosionFinished(); }),
                        activeBombs_.end());

    checkWinLoseConditions();
}

void Game::exportSnapshot(NetworkSnapshot& out) const {
    out = {};
    out.gameState = static_cast<uint8_t>(state_);
    out.gridW = static_cast<uint16_t>(grid_.getWidth());
    out.gridH = static_cast<uint16_t>(grid_.getHeight());
    out.p1x = static_cast<int16_t>(player_.getPosition().x);
    out.p1y = static_cast<int16_t>(player_.getPosition().y);
    out.p2x = static_cast<int16_t>(player2_.getPosition().x);
    out.p2y = static_cast<int16_t>(player2_.getPosition().y);
    out.p1alive = player_.isAlive() ? 1 : 0;
    out.p2alive = player2_.isAlive() ? 1 : 0;
    out.duo = isDuoMode() ? 1 : 0;
    out.score = player_.getScore();
    out.enemyCount = static_cast<uint8_t>(std::min<std::size_t>(
        enemies_.size(), static_cast<std::size_t>(NetworkSnapshot::kMaxEnemies)));
    for (std::size_t i = 0; i < out.enemyCount; ++i) {
        out.enemyX[i] = static_cast<int16_t>(enemies_[i].getPosition().x);
        out.enemyY[i] = static_cast<int16_t>(enemies_[i].getPosition().y);
        out.enemyAlive[i] = enemies_[i].isAlive() ? 1 : 0;
    }
    out.cellCount =
        static_cast<uint16_t>(out.gridW * out.gridH);
    if (out.cellCount > NetworkSnapshot::kMaxCells) {
        out.cellCount = NetworkSnapshot::kMaxCells;
    }
    int idx = 0;
    for (int y = 0; y < grid_.getHeight(); ++y) {
        for (int x = 0; x < grid_.getWidth(); ++x) {
            if (idx >= out.cellCount) {
                break;
            }
            out.cells[idx++] = static_cast<uint8_t>(grid_.getCell({x, y}));
        }
    }
}

void Game::importSnapshot(const NetworkSnapshot& in) {
    if (in.gridW == 0 || in.gridH == 0) {
        return;
    }
    state_ = static_cast<GameState>(in.gameState);
    if (static_cast<int>(in.gridW) != grid_.getWidth() ||
        static_cast<int>(in.gridH) != grid_.getHeight()) {
        grid_ = Grid(static_cast<int>(in.gridW), static_cast<int>(in.gridH),
                     config_.destructibleDensity);
    }
    int idx = 0;
    for (int y = 0; y < grid_.getHeight(); ++y) {
        for (int x = 0; x < grid_.getWidth(); ++x) {
            if (idx >= in.cellCount) {
                break;
            }
            grid_.setCell({x, y}, static_cast<CellType>(in.cells[idx++]));
        }
    }
    player_ = Player({in.p1x, in.p1y});
    if (!in.p1alive) {
        player_.kill();
    }
    player2_ = Player({in.p2x, in.p2y});
    if (!in.p2alive) {
        player2_.kill();
    }
    playMode_ = in.duo ? PlayMode::ONLINE_CLIENT : PlayMode::SOLO;
}
