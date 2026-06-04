#pragma once
#include "raylib.h"

// 前置宣告 Board 類別，避免與 TurnSystem 發生標頭檔循環引入
class Board;

// 回合狀態機列舉
enum class TurnState {
    PLAYER_TURN, // 玩家回合
    ENEMY_TURN   // 敵方回合
};

// ============================================================================
// TurnSystem: 管理回合狀態機、AP 結算、怪物回合 AI 行動延遲及 HP 自動回復邏輯
// ============================================================================
class TurnSystem {
public:
    TurnSystem();
    ~TurnSystem() = default;

    // 初始化回合狀態
    void initialize();

    // 更新狀態機 (處理玩家 AP 檢查與敵方 AI 自動延遲執行)
    void update(Board& board);

    // 在畫面中央繪製動態回合切換提示橫幅
    void draw() const;

    // 觸發玩家回合結束
    void end_player_turn(Board& board);

    // Getters
    TurnState get_state() const { return m_state; }
    int get_turn_count() const { return m_turn_count; }

private:
    // 開始玩家回合 (執行 AP 重置、HP 3回合自動回復與抽牌重置)
    void start_player_turn(Board& board);

    // 執行敵方回合的 AI 行動與怪物生成
    void execute_enemy_turn(Board& board);

    TurnState m_state;             // 當前狀態 (玩家/敵方)
    int m_turn_count;             // 遊戲總回合數 (1-based)
    int m_regen_counter;          // 生命值自動回復計數器 (每3回合觸發)

    float m_enemy_action_timer;   // 敵方行動延遲計時器 (秒)，提升動畫流暢感
    float m_banner_timer;         // 回合橫幅提示字眼顯示計時器 (秒)
    Color m_banner_color;         // 當前回合橫幅的繪製色彩
};
