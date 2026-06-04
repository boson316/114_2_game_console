#pragma once

#include "Grid.hpp"
#include "Position.hpp"

#include <vector>

std::vector<Position> computeExplosionCells(const Grid& grid, Position bombPos, int blastRadius);
