#pragma once

#include "CellType.hpp"
#include "Position.hpp"

#include <vector>

class Grid {
public:
    Grid(int width, int height, float destructibleDensity = 0.5f);

    CellType getCell(Position pos) const;
    bool isInBounds(Position pos) const;
    bool isPassable(Position pos) const;
    bool isWall(Position pos) const;
    int getWidth() const;
    int getHeight() const;

    void setCell(Position pos, CellType type);
    void destroyDestructible(Position pos, float powerupDropChance);

    void initialize(const std::vector<Position>& playerStartPositions);

private:
    int width_;
    int height_;
    std::vector<std::vector<CellType>> cells_;
    float destructibleDensity_;

    void placeIndestructibleWalls();
    void placeDestructibleWalls(const std::vector<Position>& safeZones);
    bool isSafeZone(Position pos, const std::vector<Position>& starts) const;
};
