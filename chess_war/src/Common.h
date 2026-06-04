#pragma once
#include <string>

// 西洋棋棋子身份列舉
enum class ChessPiece {
    NONE,   // 無狀態
    PAWN,   // 兵 (Pawn)
    ROOK,   // 車 (Rook)
    KNIGHT, // 馬 (Knight)
    BISHOP, // 象 (Bishop)
    QUEEN,  // 后 (Queen)
    KING    // 王 (King)
};

// 規則破壞者解鎖狀態 (Roguelike 成長解鎖)
struct RuleBreakers {
    bool pawn_backwards = false;       // 士兵解鎖後退移動與攻擊
    bool knight_extended_jump = false;  // 騎士解鎖跳躍加長 (3x1)
    bool bishop_piercing = false;       // 主教解鎖穿透攻擊
};

// 升變強化類型
enum class PromotionType {
    INCREASE_DAMAGE,
    INCREASE_MAX_HP,
    UNLOCK_PAWN_BACKWARDS,
    UNLOCK_KNIGHT_EXTENDED,
    UNLOCK_BISHOP_PIERCING
};

// 升變選項結構
struct PromotionOption {
    PromotionType type;
    std::string title;
    std::string description;
};
