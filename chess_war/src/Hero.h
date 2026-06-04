#pragma once
#include "Entity.h"
#include "Common.h"
#include "Card.h"
#include <string>
#include <vector>

// ============================================================================
// Hero: 玩家控制的英雄類別，繼承自 Entity
// ============================================================================
class Hero : public Entity {
public:
    Hero(EntityType type, Position pos); // type 限制為 HERO_A 或 HERO_B
    ~Hero() override = default;

    // Hero 專屬屬性管理
    int get_xp() const { return m_xp; }
    void add_xp(int amount) { m_xp += amount; }
    void reset_xp() { m_xp = 0; }
    void set_xp(int xp) { m_xp = xp; }
    
    int get_ap() const { return m_ap; }
    void set_ap(int ap) { m_ap = ap; }
    void consume_ap(int amount) { m_ap -= amount; }
    
    ChessPiece get_identity() const { return m_identity; }
    void set_identity(ChessPiece identity) { m_identity = identity; }
    
    // 取得當前身份英文名稱 (相容預設字型)
    std::string get_identity_name() const;
    
    RuleBreakers& get_rule_breakers() { return m_rule_breakers; }
    const RuleBreakers& get_rule_breakers() const { return m_rule_breakers; }

    bool has_moved_this_turn() const { return m_has_moved_this_turn; }
    void set_has_moved_this_turn(bool val) { m_has_moved_this_turn = val; }

    int get_damage() const { return m_damage; }
    void set_damage(int val) { m_damage = val; }

    // --- 卡牌與牌組管理介面 (C++11 std::vector) ---
    void init_starting_deck();
    void shuffle_draw_pile();
    void draw_cards(int count);
    void discard_hand();

    const std::vector<Card>& get_hand() const { return m_hand; }
    size_t get_draw_pile_count() const { return m_draw_pile.size(); }
    size_t get_discard_pile_count() const { return m_discard_pile.size(); }
    // 移除已使用的手牌
    void remove_card_from_hand(size_t index);
    // 動態新增手牌 (例如升變暫時卡)
    void add_card_to_hand(const Card& card);

    // 實作渲染與行為更新
    void draw(int start_x, int start_y, int cell_size) override;
    void update() override;

private:
    int m_xp;                       // 經驗值
    int m_ap;                       // 行動點 (Action Points)
    ChessPiece m_identity;          // 當前西洋棋身份
    RuleBreakers m_rule_breakers;   // 規則破壞者解鎖狀態
    bool m_has_moved_this_turn;     // 本回合是否已移動
    int m_damage;                   // 攻擊力

    // 牌組結構
    std::vector<Card> m_deck;          // 初始 11 張完整牌組
    std::vector<Card> m_draw_pile;     // 抽牌堆
    std::vector<Card> m_discard_pile;  // 棄牌堆
    std::vector<Card> m_hand;          // 當前手牌 (上限 4 張)
};
