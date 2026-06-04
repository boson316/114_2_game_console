#pragma once
#include "Common.h"
#include <string>

// ============================================================================
// Card: 西洋棋子卡牌結構
// 本專案卡牌皆代表「棋子身份」（如 Pawn, Knight, King），由玩家在出牌時動態決定
// 要作為「移動」還是「攻擊」用途，打出卡牌會消耗 1 AP。
// ============================================================================
struct Card {
    std::string name;          // 卡牌名稱
    ChessPiece piece_type;     // 對應西洋棋子身份
    int ap_cost;               // 行動點 AP 消耗量
    std::string description;   // 卡牌描述與說明
    bool is_temporary = false; // 是否為暫時性卡牌
};
