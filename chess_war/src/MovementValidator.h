#pragma once
#include "Common.h"
#include "Entity.h"
#include <vector>
#include <memory>

class Board;

class MovementValidator {
public:
    // 取得合法移動位置列表
    static std::vector<Position> get_valid_moves(
        ChessPiece piece,
        Position from,
        const Board& board,
        const RuleBreakers& rules,
        bool is_hero_b = false
    );

    // 取得合法攻擊位置列表 (目標格必須有怪物小兵)
    static std::vector<Position> get_valid_attacks(
        ChessPiece piece,
        Position from,
        const Board& board,
        const RuleBreakers& rules
    );

    // 檢查兩點之間的路徑是否被任何實體阻擋 (適用於車、象、后)
    static bool is_path_clear(Position from, Position to, const Board& board);

    // 檢查座標是否在棋盤內 (0-7, 0-7)
    static bool is_in_board(Position pos);
};
