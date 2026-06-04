#pragma once
#include "Entity.h"

// ============================================================================
// Minion: 敵方普通怪物類別，繼承自 Entity
// ============================================================================
class Minion : public Entity {
public:
    Minion(Position pos);
    ~Minion() override = default;

    // 實作渲染與行為更新
    void draw(int start_x, int start_y, int cell_size) override;
    void update() override;

    // AI 行動規劃：傳入兩個英雄的座標，計算朝向最近英雄移動一格 (上下左右) 的目標位置
    Position calculate_ai_move(Position hero_a_pos, Position hero_b_pos) const;
};
