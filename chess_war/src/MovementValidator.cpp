#include "MovementValidator.h"
#include "Board.h"
#include <cmath>
#include <algorithm>

bool MovementValidator::is_in_board(Position pos) {
    return pos.x >= 0 && pos.x < 8 && pos.y >= 0 && pos.y < 8;
}

bool MovementValidator::is_path_clear(Position from, Position to, const Board& board) {
    int dx = to.x - from.x;
    int dy = to.y - from.y;

    if (dy == 0) { // 水平移動
        int step_x = (dx > 0) ? 1 : -1;
        int x = from.x + step_x;
        while (x != to.x) {
            if (board.get_entity_at(Position{x, from.y}) != nullptr) {
                return false;
            }
            x += step_x;
        }
    } else if (dx == 0) { // 垂直移動
        int step_y = (dy > 0) ? 1 : -1;
        int y = from.y + step_y;
        while (y != to.y) {
            if (board.get_entity_at(Position{from.x, y}) != nullptr) {
                return false;
            }
            y += step_y;
        }
    } else if (std::abs(dx) == std::abs(dy)) { // 對角線移動
        int step_x = (dx > 0) ? 1 : -1;
        int step_y = (dy > 0) ? 1 : -1;
        int steps = std::abs(dx);
        for (int i = 1; i < steps; ++i) {
            if (board.get_entity_at(Position{from.x + i * step_x, from.y + i * step_y}) != nullptr) {
                return false;
            }
        }
    }
    return true;
}

std::vector<Position> MovementValidator::get_valid_moves(
    ChessPiece piece,
    Position from,
    const Board& board,
    const RuleBreakers& rules,
    bool is_hero_b
) {
    std::vector<Position> valid_moves;

    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            Position to{ c, r };
            if (to == from) continue;

            // 處理王車易位特例 (僅限 Hero B 使用城堡卡且目標是國王身份的 Hero A)
            bool is_castling_target = false;
            if (is_hero_b && piece == ChessPiece::ROOK) {
                auto hero_a = board.get_hero_a();
                auto hero_b = board.get_hero_b();
                if (hero_a && to == hero_a->get_pos() && hero_b) {
                    if (!hero_b->has_moved_this_turn() && 
                        hero_a->get_identity() == ChessPiece::KING && 
                        hero_a->has_moved_this_turn()) {
                        
                        // 檢查是否在水平或垂直直線上且路徑無阻礙
                        if ((from.x == to.x || from.y == to.y) && is_path_clear(from, to, board)) {
                            is_castling_target = true;
                        }
                    }
                }
            }

            // 若目標格已被佔用且非王車易位目標，則為非法移動
            if (board.is_cell_occupied(to) && !is_castling_target) {
                continue;
            }

            // 檢查是否符合該棋子的移動規則
            bool is_pattern_valid = false;
            int dx = std::abs(to.x - from.x);
            int dy = std::abs(to.y - from.y);

            switch (piece) {
                case ChessPiece::NONE:
                    break;
                case ChessPiece::PAWN:
                    // 士兵移動：只能向上 1 格 (這裡定義為 y - 1)
                    if (to.x == from.x && to.y == from.y - 1) {
                        is_pattern_valid = true;
                    }
                    // 解鎖後退 (向下 1 格)
                    if (rules.pawn_backwards && to.x == from.x && to.y == from.y + 1) {
                        is_pattern_valid = true;
                    }
                    break;
                case ChessPiece::KNIGHT:
                    // 騎士移動：L 型 (2x1 或 1x2) 且可越子 (不需檢查 clear path)
                    if ((dx == 2 && dy == 1) || (dx == 1 && dy == 2)) {
                        is_pattern_valid = true;
                    }
                    // 解鎖加長跳躍 (3x1 或 1x3)
                    if (rules.knight_extended_jump && ((dx == 3 && dy == 1) || (dx == 1 && dy == 3))) {
                        is_pattern_valid = true;
                    }
                    break;
                case ChessPiece::BISHOP:
                    // 主教移動：斜對角線且路徑淨空
                    if (dx == dy && dx > 0 && is_path_clear(from, to, board)) {
                        is_pattern_valid = true;
                    }
                    break;
                case ChessPiece::ROOK:
                    // 城堡移動：十字直線且路徑淨空 (若是王車易位也需路徑淨空)
                    if (((to.x == from.x) != (to.y == from.y)) && is_path_clear(from, to, board)) {
                        is_pattern_valid = true;
                    }
                    break;
                case ChessPiece::QUEEN:
                    // 皇后移動：城堡 + 主教且路徑淨空
                    if ((dx == dy || to.x == from.x || to.y == from.y) && is_path_clear(from, to, board)) {
                        is_pattern_valid = true;
                    }
                    break;
                case ChessPiece::KING:
                    // 國王移動：周圍 8 方向 1 格
                    if (dx <= 1 && dy <= 1) {
                        is_pattern_valid = true;
                    }
                    break;
            }

            if (is_pattern_valid || is_castling_target) {
                valid_moves.push_back(to);
            }
        }
    }

    return valid_moves;
}

std::vector<Position> MovementValidator::get_valid_attacks(
    ChessPiece piece,
    Position from,
    const Board& board,
    const RuleBreakers& rules
) {
    std::vector<Position> valid_attacks;

    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            Position to{ c, r };
            if (to == from) continue;

            // 攻擊的目標格必須有怪物小兵存在
            auto target_entity = board.get_entity_at(to);
            if (!target_entity || target_entity->get_type() != EntityType::MINION) {
                continue;
            }

            bool is_pattern_valid = false;
            int dx = std::abs(to.x - from.x);
            int dy = std::abs(to.y - from.y);

            switch (piece) {
                case ChessPiece::NONE:
                    // 無狀態身份無法發動攻擊
                    break;
                case ChessPiece::PAWN:
                    // 士兵攻擊：斜上方 1 格 (X 差 1, Y 差 1 且朝向 y-1)
                    if (dx == 1 && to.y == from.y - 1) {
                        is_pattern_valid = true;
                    }
                    // 解鎖後退攻擊 (向下)
                    if (rules.pawn_backwards && dx == 1 && to.y == from.y + 1) {
                        is_pattern_valid = true;
                    }
                    break;
                case ChessPiece::KNIGHT:
                    // 騎士攻擊：L 型 (2x1 或 1x2，解鎖後 3x1)
                    if ((dx == 2 && dy == 1) || (dx == 1 && dy == 2)) {
                        is_pattern_valid = true;
                    }
                    if (rules.knight_extended_jump && ((dx == 3 && dy == 1) || (dx == 1 && dy == 3))) {
                        is_pattern_valid = true;
                    }
                    break;
                case ChessPiece::BISHOP:
                    // 主教攻擊：斜對角線，若解鎖穿透 (bishop_piercing) 則忽略路徑阻礙，否則需淨空
                    if (dx == dy && dx > 0) {
                        if (rules.bishop_piercing || is_path_clear(from, to, board)) {
                            is_pattern_valid = true;
                        }
                    }
                    break;
                case ChessPiece::ROOK:
                    // 城堡攻擊：十字直線且路徑淨空
                    if (((to.x == from.x) != (to.y == from.y)) && is_path_clear(from, to, board)) {
                        is_pattern_valid = true;
                    }
                    break;
                case ChessPiece::QUEEN:
                    // 皇后攻擊：十字或斜對角線且路徑淨空
                    if ((dx == dy || to.x == from.x || to.y == from.y) && is_path_clear(from, to, board)) {
                        is_pattern_valid = true;
                    }
                    break;
                case ChessPiece::KING:
                    // 國王攻擊：周圍 8 方向 1 格
                    if (dx <= 1 && dy <= 1) {
                        is_pattern_valid = true;
                    }
                    break;
            }

            if (is_pattern_valid) {
                valid_attacks.push_back(to);
            }
        }
    }

    return valid_attacks;
}
