#pragma once

#include "Position.hpp"

enum class BombOwner { PLAYER, PLAYER2, ENEMY };

class Bomb {
public:
    Bomb(Position pos, float fuseTime = 3.0f, int blastRadius = 2,
         BombOwner owner = BombOwner::PLAYER);

    void update(float deltaTime);
    bool isExploded() const;
    bool isExplosionFinished() const;
    bool isFuseExpired() const;

    Position getPosition() const;
    int getBlastRadius() const;
    float getRemainingFuse() const;
    int getId() const;
    BombOwner getOwner() const;

    void startExplosion(float duration = 0.5f);
    void markExploded();

private:
    static int nextId_;

    int id_;
    Position pos_;
    float fuseTime_;
    float elapsed_;
    int blastRadius_;
    BombOwner owner_;
    bool exploded_;
    float explosionDuration_;
    float explosionElapsed_;
};
