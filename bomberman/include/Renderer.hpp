#pragma once

#include "AI_Enemy.hpp"
#include "Bomb.hpp"
#include "GameConfig.hpp"
#include "Grid.hpp"
#include "Player.hpp"
#include "Position.hpp"

#include <vector>

class Renderer {
public:
    static constexpr char CHAR_EMPTY = '.';
    static constexpr char CHAR_INDESTRUCTIBLE = '#';
    static constexpr char CHAR_DESTRUCTIBLE = 'X';
    static constexpr char CHAR_PLAYER = 'P';
    static constexpr char CHAR_ENEMY = 'E';
    static constexpr char CHAR_BOMB = 'B';
    static constexpr char CHAR_EXPLOSION = '*';
    static constexpr char CHAR_POWERUP = 'o';

    void render(const Grid& grid, const Player& player, const std::vector<AI_Enemy>& enemies,
                const std::vector<Bomb>& bombs, const std::vector<Position>& explosionCells,
                GameState state, int score);

private:
    void clearScreen();
    void drawGrid(const Grid& grid, const Player& player, const std::vector<AI_Enemy>& enemies,
                  const std::vector<Bomb>& bombs, const std::vector<Position>& explosionCells);
    void drawHUD(int score, GameState state);
};
