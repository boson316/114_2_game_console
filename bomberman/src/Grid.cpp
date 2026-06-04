#include "Grid.hpp"

#include <algorithm>
#include <random>

namespace {
std::mt19937& rng() {
    static std::mt19937 gen{std::random_device{}()};
    return gen;
}
}  // namespace

Grid::Grid(int width, int height, float destructibleDensity)
    : width_(width), height_(height), destructibleDensity_(destructibleDensity) {
    cells_.assign(static_cast<std::size_t>(height_),
                  std::vector<CellType>(static_cast<std::size_t>(width_), CellType::EMPTY));
}

CellType Grid::getCell(Position pos) const {
    if (!isInBounds(pos)) {
        return CellType::INDESTRUCTIBLE;
    }
    return cells_[static_cast<std::size_t>(pos.y)][static_cast<std::size_t>(pos.x)];
}

bool Grid::isInBounds(Position pos) const {
    return pos.x >= 0 && pos.x < width_ && pos.y >= 0 && pos.y < height_;
}

bool Grid::isPassable(Position pos) const {
    const CellType c = getCell(pos);
    return c == CellType::EMPTY || c == CellType::POWERUP;
}

bool Grid::isWall(Position pos) const {
    const CellType c = getCell(pos);
    return c == CellType::INDESTRUCTIBLE || c == CellType::DESTRUCTIBLE;
}

int Grid::getWidth() const { return width_; }

int Grid::getHeight() const { return height_; }

void Grid::setCell(Position pos, CellType type) {
    if (!isInBounds(pos)) {
        return;
    }
    cells_[static_cast<std::size_t>(pos.y)][static_cast<std::size_t>(pos.x)] = type;
}

void Grid::destroyDestructible(Position pos, float powerupDropChance) {
    if (!isInBounds(pos) || getCell(pos) != CellType::DESTRUCTIBLE) {
        return;
    }
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    if (dist(rng()) < powerupDropChance) {
        setCell(pos, CellType::POWERUP);
    } else {
        setCell(pos, CellType::EMPTY);
    }
}

bool Grid::isSafeZone(Position pos, const std::vector<Position>& starts) const {
    for (const Position& start : starts) {
        if (pos.x >= start.x && pos.x <= start.x + 1 && pos.y >= start.y && pos.y <= start.y + 1) {
            return true;
        }
    }
    return false;
}

void Grid::placeIndestructibleWalls() {
    for (int y = 0; y < height_; ++y) {
        for (int x = 0; x < width_; ++x) {
            if (x % 2 == 0 && y % 2 == 0) {
                setCell({x, y}, CellType::INDESTRUCTIBLE);
            }
        }
    }
}

void Grid::placeDestructibleWalls(const std::vector<Position>& safeZones) {
    std::vector<Position> candidates;
    for (int y = 0; y < height_; ++y) {
        for (int x = 0; x < width_; ++x) {
            Position p{x, y};
            if (getCell(p) != CellType::EMPTY) {
                continue;
            }
            if (isSafeZone(p, safeZones)) {
                continue;
            }
            candidates.push_back(p);
        }
    }

    std::shuffle(candidates.begin(), candidates.end(), rng());
    const int target =
        static_cast<int>(static_cast<float>(candidates.size()) * destructibleDensity_);
    for (int i = 0; i < target && i < static_cast<int>(candidates.size()); ++i) {
        setCell(candidates[static_cast<std::size_t>(i)], CellType::DESTRUCTIBLE);
    }
}

void Grid::initialize(const std::vector<Position>& playerStartPositions) {
    for (auto& row : cells_) {
        std::fill(row.begin(), row.end(), CellType::EMPTY);
    }
    placeIndestructibleWalls();
    for (const Position& start : playerStartPositions) {
        for (int dy = 0; dy < 2; ++dy) {
            for (int dx = 0; dx < 2; ++dx) {
                setCell({start.x + dx, start.y + dy}, CellType::EMPTY);
            }
        }
    }
    placeDestructibleWalls(playerStartPositions);
}
