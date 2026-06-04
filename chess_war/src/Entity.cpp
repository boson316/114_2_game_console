#include "Entity.h"
#include <algorithm> // 引入標準庫以使用 std::max

// 建構子初始化成員變數
Entity::Entity(EntityType type, Position pos, int hp, int max_hp)
    : m_type(type), m_pos(pos), m_hp(hp), m_max_hp(max_hp) {}

// 遭受傷害計算 (C++11 std::max 用於防範 HP 溢出至負數)
void Entity::take_damage(int amount) {
    m_hp = std::max(0, m_hp - amount);
}
