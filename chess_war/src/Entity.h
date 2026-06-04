#pragma once
#include "raylib.h"

// ============================================================================
// Position: 管理網格座標的結構 (0-based 索引)
// ============================================================================
struct Position {
    int x; // 行 (Column, 0 ~ 7)
    int y; // 列 (Row, 0 ~ 7)

    // C++11 重載 == 運算子，便於座標比對
    bool operator==(const Position& other) const {
        return x == other.x && y == other.y;
    }
};

// 實體類型列舉
enum class EntityType {
    HERO_A,
    HERO_B,
    MINION
};

// ============================================================================
// Entity: 棋盤上所有角色與怪物的抽象基底類別
// ============================================================================
class Entity {
public:
    Entity(EntityType type, Position pos, int hp, int max_hp);
    virtual ~Entity() = default; // 虛擬解構子確保衍生類別正確釋放記憶體

    // Getters & Setters
    EntityType get_type() const { return m_type; }
    Position get_pos() const { return m_pos; }
    void set_pos(Position pos) { m_pos = pos; }
    
    int get_hp() const { return m_hp; }
    void set_hp(int hp) { m_hp = hp; }
    int get_max_hp() const { return m_max_hp; }
    void set_max_hp(int max_hp) { m_max_hp = max_hp; }

    // 基礎屬性修改
    void take_damage(int amount);
    bool is_dead() const { return m_hp <= 0; }

    // 純虛擬函式，由衍生類別 Hero 與 Minion 實作其專屬渲染與行為
    virtual void draw(int start_x, int start_y, int cell_size) = 0;
    virtual void update() = 0;

protected:
    EntityType m_type; // 實體類型
    Position m_pos;     // 棋盤座標位置
    int m_hp;          // 當前生命值
    int m_max_hp;      // 最大生命值
};
