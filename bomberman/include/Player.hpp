#pragma once

#include "Grid.hpp"
#include "Position.hpp"

class Player {
public:
    explicit Player(Position startPos);

    void move(Position direction, const Grid& grid);
    bool tryPlaceBomb(int maxBombs, int activeBombs);

    Position getPosition() const;
    bool isAlive() const;
    void kill();
    int getScore() const;
    void addScore(int points);

private:
    Position pos_;
    bool alive_;
    int score_;
};
