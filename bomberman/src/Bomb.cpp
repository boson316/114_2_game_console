#include "Bomb.hpp"

#include <algorithm>

int Bomb::nextId_ = 1;

Bomb::Bomb(Position pos, float fuseTime, int blastRadius, BombOwner owner)
    : id_(nextId_++),
      pos_(pos),
      fuseTime_(fuseTime),
      elapsed_(0.0f),
      blastRadius_(blastRadius),
      owner_(owner),
      exploded_(false),
      explosionDuration_(0.5f),
      explosionElapsed_(0.0f) {}

void Bomb::update(float deltaTime) {
    if (!exploded_) {
        elapsed_ += deltaTime;
        return;
    }
    explosionElapsed_ += deltaTime;
}

bool Bomb::isExploded() const { return exploded_; }

bool Bomb::isExplosionFinished() const {
    return exploded_ && explosionElapsed_ >= explosionDuration_;
}

bool Bomb::isFuseExpired() const { return !exploded_ && elapsed_ >= fuseTime_; }

Position Bomb::getPosition() const { return pos_; }

int Bomb::getBlastRadius() const { return blastRadius_; }

float Bomb::getRemainingFuse() const {
    if (exploded_) {
        return 0.0f;
    }
    return std::max(0.0f, fuseTime_ - elapsed_);
}

int Bomb::getId() const { return id_; }

BombOwner Bomb::getOwner() const { return owner_; }

void Bomb::startExplosion(float duration) {
    exploded_ = true;
    explosionDuration_ = duration;
    explosionElapsed_ = 0.0f;
}

void Bomb::markExploded() { startExplosion(explosionDuration_); }
