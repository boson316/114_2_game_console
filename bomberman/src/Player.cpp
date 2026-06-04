#include "Player.hpp"

Player::Player(Position startPos) : pos_(startPos), alive_(true), score_(0) {}

void Player::move(Position direction, const Grid& grid) {
    if (!alive_) {
        return;
    }
    const Position target = pos_ + direction;
    if (!grid.isInBounds(target) || !grid.isPassable(target)) {
        return;
    }
    pos_ = target;
}

bool Player::tryPlaceBomb(int maxBombs, int activeBombs) {
    if (!alive_ || activeBombs >= maxBombs) {
        return false;
    }
    return true;
}

Position Player::getPosition() const { return pos_; }

bool Player::isAlive() const { return alive_; }

void Player::kill() { alive_ = false; }

int Player::getScore() const { return score_; }

void Player::addScore(int points) { score_ += points; }
