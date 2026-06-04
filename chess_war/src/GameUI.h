#pragma once
#include "raylib.h"
#include "Board.h"
#include "Hero.h"
#include "Common.h"
#include "TurnSystem.h"
#include <vector>
#include <string>

// ============================================================================
// GameUI: 專責繪製遊戲 HUD 狀態面板與升變 (Promotion) 三選一遮罩對話框
// ============================================================================
class GameUI {
public:
    GameUI() = default;
    ~GameUI() = default;

    // 繪製右側 HUD 面板與說明文字
    void draw_hud(
        const Board& board, 
        EntityType active_hero_type, 
        int selected_card_idx, 
        int turn_count, 
        TurnState turn_state, 
        bool is_aiming
    );

    // 繪製升變 (Promotion) 三選一覆蓋面板
    void draw_promotion_overlay(
        EntityType promoting_hero_type, 
        const std::vector<PromotionOption>& options, 
        int hovered_option_idx
    );

    // 繪製遊戲勝負結算覆蓋面板
    void draw_game_over_overlay(bool victory, int kill_count);

    /** 0=無, 1=再玩, 2=返回主選單 */
    int poll_game_over_action() const;

private:
    Rectangle m_btn_restart_{};
    Rectangle m_btn_menu_{};
};
