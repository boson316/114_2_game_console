#include "Renderer.hpp"

#include <cstdlib>
#include <iostream>
#include <unordered_set>

namespace {
bool contains(const std::vector<Position>& cells, Position pos) {
    for (const Position& p : cells) {
        if (p == pos) {
            return true;
        }
    }
    return false;
}
}  // namespace

void Renderer::clearScreen() {
#ifdef _WIN32
    std::system("cls");
#else
    std::system("clear");
#endif
}

void Renderer::drawGrid(const Grid& grid, const Player& player, const std::vector<AI_Enemy>& enemies,
                        const std::vector<Bomb>& bombs, const std::vector<Position>& explosionCells) {
    const Position playerPos = player.getPosition();
    std::unordered_set<Position, PositionHash> bombCells;
    for (const Bomb& bomb : bombs) {
        if (!bomb.isExplosionFinished()) {
            bombCells.insert(bomb.getPosition());
        }
    }

    for (int y = 0; y < grid.getHeight(); ++y) {
        for (int x = 0; x < grid.getWidth(); ++x) {
            const Position pos{x, y};
            char ch = CHAR_EMPTY;
            if (player.isAlive() && pos == playerPos) {
                ch = CHAR_PLAYER;
            } else {
                bool drawn = false;
                for (const AI_Enemy& enemy : enemies) {
                    if (enemy.isAlive() && enemy.getPosition() == pos) {
                        ch = CHAR_ENEMY;
                        drawn = true;
                        break;
                    }
                }
                if (!drawn && contains(explosionCells, pos)) {
                    ch = CHAR_EXPLOSION;
                } else if (!drawn && bombCells.count(pos) > 0) {
                    ch = CHAR_BOMB;
                } else if (!drawn) {
                    switch (grid.getCell(pos)) {
                        case CellType::INDESTRUCTIBLE:
                            ch = CHAR_INDESTRUCTIBLE;
                            break;
                        case CellType::DESTRUCTIBLE:
                            ch = CHAR_DESTRUCTIBLE;
                            break;
                        case CellType::POWERUP:
                            ch = CHAR_POWERUP;
                            break;
                        default:
                            ch = CHAR_EMPTY;
                            break;
                    }
                }
            }
            std::cout << ch;
        }
        std::cout << '\n';
    }
}

void Renderer::drawHUD(int score, GameState state) {
    std::cout << "Score: " << score << "  ";
    switch (state) {
        case GameState::GAME_OVER:
            std::cout << "[GAME OVER] Press R to restart, Q to quit";
            break;
        case GameState::VICTORY:
            std::cout << "[VICTORY] Press R to restart, Q to quit";
            break;
        case GameState::PLAYING:
            std::cout << "WASD/Arrows move, Space bomb, Q quit (click window first)";
            break;
        default:
            break;
    }
    std::cout << '\n';
}

void Renderer::render(const Grid& grid, const Player& player, const std::vector<AI_Enemy>& enemies,
                      const std::vector<Bomb>& bombs, const std::vector<Position>& explosionCells,
                      GameState state, int score) {
    clearScreen();
    drawGrid(grid, player, enemies, bombs, explosionCells);
    drawHUD(score, state);
}
