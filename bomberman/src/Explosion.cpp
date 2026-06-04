#include "Explosion.hpp"

#include <algorithm>

namespace {
void raycast(const Grid& grid, Position origin, Position dir, int blastRadius,
             std::vector<Position>& out) {
    for (int step = 1; step <= blastRadius; ++step) {
        const Position cell{origin.x + dir.x * step, origin.y + dir.y * step};
        if (!grid.isInBounds(cell)) {
            break;
        }
        const CellType type = grid.getCell(cell);
        if (type == CellType::INDESTRUCTIBLE) {
            break;
        }
        out.push_back(cell);
        if (type == CellType::DESTRUCTIBLE) {
            break;
        }
    }
}
}  // namespace

std::vector<Position> computeExplosionCells(const Grid& grid, Position bombPos, int blastRadius) {
    std::vector<Position> cells;
    if (!grid.isInBounds(bombPos)) {
        return cells;
    }
    cells.push_back(bombPos);
    raycast(grid, bombPos, Direction::UP, blastRadius, cells);
    raycast(grid, bombPos, Direction::DOWN, blastRadius, cells);
    raycast(grid, bombPos, Direction::LEFT, blastRadius, cells);
    raycast(grid, bombPos, Direction::RIGHT, blastRadius, cells);
    std::sort(cells.begin(), cells.end(), [](const Position& a, const Position& b) {
        if (a.y != b.y) {
            return a.y < b.y;
        }
        return a.x < b.x;
    });
    cells.erase(std::unique(cells.begin(), cells.end()), cells.end());
    return cells;
}
