#pragma once

#include "GameLaunch.hpp"
#include "raylib.h"
#include "Board.h"
#include "TurnSystem.h"
#include "GameUI.h"
#include "Common.h"
#include <memory>
#include <vector>

// ============================================================================
// Game: 遊戲主控制器，控制視窗生命週期、輸入處理與主要遊戲狀態機 (C++20)
// ============================================================================
class Game {
public:
    Game();
    ~Game();

    GameLaunchResult run();

private:
    // 輸入處理
    void handle_input();
    // 遊戲狀態邏輯更新
    void update_game();
    // 畫面渲染
    void render_game();

    // 升變成長邏輯 (Promotion)
    void check_promotions();
    void start_promotion(EntityType hero_type);
    void apply_promotion(int option_idx);

    std::unique_ptr<Board> m_board;
    std::unique_ptr<TurnSystem> m_turn_system;
    std::unique_ptr<GameUI> m_game_ui;

    // 輸入狀態機
    enum class InputState {
        IDLE,       // 閒置狀態 (無選取手牌)
        AIMING,     // 選取手牌中 (顯示移動與攻擊高亮)
        PROMOTION,  // 升變三選一對話框狀態 (暫停其他行為)
        VICTORY,    // 勝利狀態
        DEFEAT      // 失敗狀態
    };
    InputState m_input_state;
    
    EntityType m_active_hero_type;              // 當前控制的英雄 (HERO_A 或 HERO_B)
    int m_selected_card_idx;                    // 當前選取的手牌索引
    std::vector<Position> m_highlighted_moves;   // 移動高亮座標集
    std::vector<Position> m_highlighted_attacks; // 攻擊高亮座標集

    Position m_selected_minion_pos;              // 當前選取的怪物位置
    std::vector<Position> m_minion_moves;        // 怪物的移動範圍
    std::vector<Position> m_minion_attacks;      // 怪物的移動後攻擊範圍

    // 升變專屬屬性
    EntityType m_promoting_hero_type;            // 當前正在進行升變的英雄
    std::vector<PromotionOption> m_promotion_options; // 3 個隨機升變選項
    int m_hovered_promotion_idx;                 // 當前滑鼠懸停的升變選項索引 (-1 代表無)
};
