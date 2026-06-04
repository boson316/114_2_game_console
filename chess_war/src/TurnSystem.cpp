#include "TurnSystem.h"

#include "AssetManager.h"
#include "Board.h"
#include "DisplayConfig.h"
#include <string>
#include <algorithm> // 引入 std::min / std::max

TurnSystem::TurnSystem() {
    initialize();
}

void TurnSystem::initialize() {
    m_state = TurnState::PLAYER_TURN;
    m_turn_count = 1;
    m_regen_counter = 0;
    m_enemy_action_timer = 0.0f;
    m_banner_timer = 2.0f; // 初始開啟時顯示 2 秒提示
    m_banner_color = Color{ 139, 92, 246, 255 }; // 預設使用英雄 A 幻紫
}

void TurnSystem::update(Board& board) {
    float dt = GetFrameTime();
    
    // 更新提示橫幅顯示計時器 (使用 C++11 std::max)
    if (m_banner_timer > 0.0f) {
        m_banner_timer = std::max(0.0f, m_banner_timer - dt);
    }
    
    // 玩家回合邏輯
    if (m_state == TurnState::PLAYER_TURN) {
        auto hero_a = board.get_hero_a();
        auto hero_b = board.get_hero_b();
        
        // 自動結算結束回合：若兩位英雄 AP 皆耗盡 (已死亡英雄視為 0 AP)，則自動結束玩家回合
        if (hero_a && hero_b) {
            int ap_a = hero_a->is_dead() ? 0 : hero_a->get_ap();
            int ap_b = hero_b->is_dead() ? 0 : hero_b->get_ap();
            if (ap_a == 0 && ap_b == 0) {
                end_player_turn(board);
            }
        }
    }
    // 敵方回合邏輯
    else if (m_state == TurnState::ENEMY_TURN) {
        m_enemy_action_timer -= dt;
        if (m_enemy_action_timer <= 0.0f) {
            // 延遲計時器歸零後，執行怪物動作並自動切回玩家回合
            execute_enemy_turn(board);
            start_player_turn(board);
        }
    }
}

void TurnSystem::end_player_turn(Board& board) {
    m_state = TurnState::ENEMY_TURN;
    m_enemy_action_timer = 0.9f; // 給予 0.9 秒行動間隔延遲，提升操作反饋
    m_banner_timer = 0.9f;
    m_banner_color = Color{ 244, 63, 94, 255 }; // 敵方回合提示：怪物玫紅
    AssetManager::get_instance().play_sound_turn_enemy(); // 播放敵方回合音效
}

void TurnSystem::start_player_turn(Board& board) {
    m_state = TurnState::PLAYER_TURN;
    m_turn_count++;
    m_banner_timer = 1.5f; // 顯示新回合提示 1.5 秒
    m_banner_color = Color{ 6, 182, 212, 255 }; // 玩家回合提示：英雄 B 霓青
    AssetManager::get_instance().play_sound_turn_player(); // 播放玩家回合音效
    
    // 1. 重置兩位英雄的行動點 (AP = 2)、身份、位移狀態與重新抽 4 張牌
    auto hero_a = board.get_hero_a();
    auto hero_b = board.get_hero_b();
    if (hero_a) {
        hero_a->set_ap(2);
        hero_a->set_identity(ChessPiece::NONE);
        hero_a->set_has_moved_this_turn(false);
        hero_a->discard_hand();
        hero_a->draw_cards(4);
    }
    if (hero_b) {
        hero_b->set_ap(2);
        hero_b->set_identity(ChessPiece::NONE);
        hero_b->set_has_moved_this_turn(false);
        hero_b->discard_hand();
        hero_b->draw_cards(4);
    }
    
    // 2. 生命值自動回復邏輯：每 3 回合回復 1 點 HP (上限為 MaxHP)
    m_regen_counter++;
    if (m_regen_counter >= 3) {
        m_regen_counter = 0;
        if (hero_a && !hero_a->is_dead()) {
            hero_a->set_hp(std::min(hero_a->get_max_hp(), hero_a->get_hp() + 1));
        }
        if (hero_b && !hero_b->is_dead()) {
            hero_b->set_hp(std::min(hero_b->get_max_hp(), hero_b->get_hp() + 1));
        }
    }
    
    TraceLog(LOG_INFO, "TURN SYSTEM: Turn %d started. Hand discarded and 4 cards drawn for both heroes.", m_turn_count);
}

void TurnSystem::execute_enemy_turn(Board& board) {
    // 1. 執行怪物 AI 的移動與攻擊
    board.execute_enemy_actions();
    
    // 2. 隨機生成一隻怪物小兵 (場上限 5 隻)
    board.spawn_minion_randomly();
    
    // 3. 安全清理死亡的怪物實體
    board.remove_dead_entities();
}

void TurnSystem::draw() const {
    auto& assets = AssetManager::get_instance();
    Font font = assets.get_font();
    // 若横幅計時器仍大於 0，繪製質感半透明提示橫幅 (Wow UI 規範)
    if (m_banner_timer > 0.0f) {
        int banner_y = 280;
        int banner_h = 130;
        
        // 繪製深藍灰半透明底板
        DrawRectangle(0, banner_y, kScreenW, banner_h, Color{15, 23, 42, 210});
        DrawRectangleLines(0, banner_y, kScreenW, banner_h, Color{51, 65, 85, 100});
        
        std::string title_text = (m_state == TurnState::PLAYER_TURN) ? assets.loc("phase_player") : assets.loc("phase_enemy");
        std::string subtitle_text = (m_state == TurnState::PLAYER_TURN) 
            ? assets.loc("banner_sub_player") 
            : assets.loc("banner_sub_enemy");
            
        // 渲染主標題 (置中)
        float title_size = 38.0f;
        Vector2 title_w = MeasureTextEx(font, title_text.c_str(), title_size, 1.0f);
        DrawTextEx(font, title_text.c_str(),
                   Vector2{(static_cast<float>(kScreenW) - title_w.x) / 2.0f,
                            static_cast<float>(banner_y + 25)},
                   title_size, 1.0f, m_banner_color);
        
        // 渲染副標題 (置中)
        float sub_size = 16.0f;
        Vector2 sub_w = MeasureTextEx(font, subtitle_text.c_str(), sub_size, 1.0f);
        DrawTextEx(font, subtitle_text.c_str(),
                   Vector2{(static_cast<float>(kScreenW) - sub_w.x) / 2.0f,
                            static_cast<float>(banner_y + 75)},
                   sub_size, 1.0f, Color{203, 213, 225, 255});
    }
}
