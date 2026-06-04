#pragma once
#include "raylib.h"
#include "Entity.h"
#include "Hero.h"
#include "Minion.h"
#include <vector>
#include <memory>

// ============================================================================
// FloatingText: 用於網格上方漂浮文字特效的結構
// ============================================================================
struct FloatingText {
    std::string text;
    float x;       // 螢幕實際 X 座標
    float y;       // 螢幕實際 Y 座標
    Color color;   // 顏色
    float alpha;   // 透明度 (1.0 -> 0.0)
    float lifetime; // 剩餘生存時間 (秒)
};

// ============================================================================
// Board: 棋盤管理類別，負責 8x8 網格繪製與所有實體 (Entity) 的位置生命週期管理
// ============================================================================
class Board {
public:
    Board(int start_x, int start_y, int cell_size);
    ~Board() = default;

    // 初始化棋盤，配置英雄初始位置與小兵
    void initialize();
    
    // 渲染棋盤與所有實體
    void draw();
    
    // 更新棋盤狀態與實體
    void update();

    // 實體獲取與生成管理
    std::shared_ptr<Hero> get_hero_a() const { return m_hero_a; }
    std::shared_ptr<Hero> get_hero_b() const { return m_hero_b; }
    const std::vector<std::shared_ptr<Minion>>& get_minions() const { return m_minions; }

    int get_start_x() const { return m_start_x; }
    int get_start_y() const { return m_start_y; }
    int get_cell_size() const { return m_cell_size; }

    void spawn_minion(Position pos);
    void spawn_minion_randomly();
    void remove_dead_entities();
    
    // 執行敵方所有小兵動作 (AI 移動與攻擊)
    void execute_enemy_actions();

    // 執行英雄移動與攻擊 (出牌結算)
    bool execute_hero_move(std::shared_ptr<Hero> hero, Position to, Card card);
    bool execute_hero_attack(std::shared_ptr<Hero> hero, Position to, Card card);

    // 查詢特定座標是否被佔用
    bool is_cell_occupied(Position pos) const;

    // 查詢特定座標上的實體，若無則返回 nullptr
    std::shared_ptr<Entity> get_entity_at(Position pos) const;

    // 浮動文字特效輔助方法
    void add_floating_text(const std::string& text, Position grid_pos, Color color);
    void update_floating_texts(float dt);
    void draw_floating_texts() const;

    // 擊殺統計管理
    int get_kill_count() const { return m_kill_count; }

private:
    int m_start_x;   // 棋盤在螢幕上的起始 X 座標
    int m_start_y;   // 棋盤在螢幕上的起始 Y 座標
    int m_cell_size; // 每個格子的大小 (像素)

    std::shared_ptr<Hero> m_hero_a;                  // 英雄 A 的智慧指標
    std::shared_ptr<Hero> m_hero_b;                  // 英雄 B 的智慧指標
    std::vector<std::shared_ptr<Minion>> m_minions;  // 所有小兵的動態陣列

    std::vector<FloatingText> m_floating_texts;      // 浮動文字清單
    int m_kill_count;                                // 累計擊殺小兵數量
};
