#include "Minion.h"
#include "AssetManager.h"
#include <cmath>      // 引入標準庫以使用 std::abs
#include <algorithm>  // 引入標準庫以使用 std::min/std::max
#include <string>     // 引入標準庫以使用 std::string 和 std::to_string

// 建構子：小兵 HP=2, MaxHP=2
Minion::Minion(Position pos)
    : Entity(EntityType::MINION, pos, 2, 2) {}

void Minion::draw(int start_x, int start_y, int cell_size) {
    auto& assets = AssetManager::get_instance();
    
    // 計算格子在視窗上的左上角座標
    int cell_x = start_x + m_pos.x * cell_size;
    int cell_y = start_y + m_pos.y * cell_size;
    
    int center_x = cell_x + cell_size / 2;
    int center_y = cell_y + cell_size / 2;
    
    // 1. 繪製小兵圓點 (玫紅色)
    DrawCircle(center_x, center_y, cell_size * 0.30f, assets.get_color_enemy());
    
    // 2. 繪製小兵外框
    DrawCircleLines(center_x, center_y, cell_size * 0.30f, assets.get_color_text_primary());
    
    // 3. 繪製格子左上角 HP 顯示 (半透明黑底，紅色邊框)
    int hp_box_w = 20;
    int hp_box_h = 15;
    DrawRectangle(cell_x + 2, cell_y + 2, hp_box_w, hp_box_h, Color{ 15, 23, 42, 180 });
    DrawRectangleLines(cell_x + 2, cell_y + 2, hp_box_w, hp_box_h, assets.get_color_hp());
    
    std::string hp_str = std::to_string(m_hp);
    Vector2 hp_size = MeasureTextEx(assets.get_font(), hp_str.c_str(), 11.0f, 1.0f);
    DrawTextEx(assets.get_font(), hp_str.c_str(), Vector2{ (float)(cell_x + 2 + (hp_box_w - hp_size.x) / 2), (float)(cell_y + 3) }, 11.0f, 1.0f, assets.get_color_text_primary());
}

void Minion::update() {
    // 狀態更新邏輯 (未來實作)
}

Position Minion::calculate_ai_move(Position hero_a_pos, Position hero_b_pos) const {
    // 計算與兩位英雄的曼哈頓距離 (Manhattan Distance)
    int dist_a = std::abs(hero_a_pos.x - m_pos.x) + std::abs(hero_a_pos.y - m_pos.y);
    int dist_b = std::abs(hero_b_pos.x - m_pos.x) + std::abs(hero_b_pos.y - m_pos.y);
    
    // 鎖定較近的英雄為追蹤目標 (距離相同時優先鎖定 A)
    Position target = (dist_a <= dist_b) ? hero_a_pos : hero_b_pos;
    
    Position next_pos = m_pos;
    int dx = target.x - m_pos.x;
    int dy = target.y - m_pos.y;
    
    // 優先沿差距較大的軸向移動一格，以實現最短路徑追蹤
    if (std::abs(dx) >= std::abs(dy)) {
        if (dx > 0) next_pos.x += 1;
        else if (dx < 0) next_pos.x -= 1;
    } else {
        if (dy > 0) next_pos.y += 1;
        else if (dy < 0) next_pos.y -= 1;
    }
    
    // 限制在 8x8 棋盤界線內
    next_pos.x = std::max(0, std::min(7, next_pos.x));
    next_pos.y = std::max(0, std::min(7, next_pos.y));
    
    return next_pos;
}
