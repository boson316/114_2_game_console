#include "Hero.h"
#include "AssetManager.h"
#include <string>
#include <random>    // 引入隨機數庫用於洗牌
#include <algorithm> // 引入標準演算法以進行隨機打亂

// 建構子：初始 HP=5, MaxHP=5, XP=0, AP=2, 身份=無狀態
Hero::Hero(EntityType type, Position pos)
    : Entity(type, pos, 5, 5), m_xp(0), m_ap(2), m_identity(ChessPiece::NONE),
      m_has_moved_this_turn(false), m_damage(1) {}

void Hero::remove_card_from_hand(size_t index) {
    if (index < m_hand.size()) {
        // 僅將非暫時性卡片移入棄牌堆
        if (!m_hand[index].is_temporary) {
            m_discard_pile.push_back(m_hand[index]);
        }
        m_hand.erase(m_hand.begin() + index);
    }
}

void Hero::add_card_to_hand(const Card& card) {
    m_hand.push_back(card);
}

std::string Hero::get_identity_name() const {
    auto& assets = AssetManager::get_instance();
    switch (m_identity) {
        case ChessPiece::NONE:   return assets.loc("piece_none");
        case ChessPiece::PAWN:   return assets.loc("piece_pawn");
        case ChessPiece::ROOK:   return assets.loc("piece_rook");
        case ChessPiece::KNIGHT: return assets.loc("piece_knight");
        case ChessPiece::BISHOP: return assets.loc("piece_bishop");
        case ChessPiece::QUEEN:  return assets.loc("piece_queen");
        case ChessPiece::KING:   return assets.loc("piece_king");
    }
    return assets.loc("piece_unknown");
}

void Hero::draw(int start_x, int start_y, int cell_size) {
    auto& assets = AssetManager::get_instance();
    
    // 計算該格子在視窗上的左上角座標
    int cell_x = start_x + m_pos.x * cell_size;
    int cell_y = start_y + m_pos.y * cell_size;
    
    // 依據英雄類型取得色彩
    Color hero_color = (m_type == EntityType::HERO_A) ? assets.get_color_hero_a() : assets.get_color_hero_b();
    
    // 1. 繪製英雄圓點 (置中，半徑約為格子的 35%)
    int center_x = cell_x + cell_size / 2;
    int center_y = cell_y + cell_size / 2 - 4; // 稍微往上偏，留空間給底部文字
    DrawCircle(center_x, center_y, cell_size * 0.32f, hero_color);
    
    // 2. 繪製英雄外框以增加層次感
    DrawCircleLines(center_x, center_y, cell_size * 0.32f, assets.get_color_text_primary());
    
    // 3. 繪製格子左上角 HP 顯示（加半透明底色，例如深紅色/黑色半透明）
    int hp_box_w = 22;
    int hp_box_h = 16;
    DrawRectangle(cell_x + 2, cell_y + 2, hp_box_w, hp_box_h, Color{ 15, 23, 42, 180 }); // 半透明背景
    DrawRectangleLines(cell_x + 2, cell_y + 2, hp_box_w, hp_box_h, assets.get_color_hp()); // 紅色細邊框
    
    std::string hp_str = std::to_string(m_hp);
    Vector2 hp_size = MeasureTextEx(assets.get_font(), hp_str.c_str(), 12.0f, 1.0f);
    // 數字置中繪製於半透明底盒內
    DrawTextEx(assets.get_font(), hp_str.c_str(), Vector2{ (float)(cell_x + 2 + (hp_box_w - hp_size.x) / 2), (float)(cell_y + 3) }, 12.0f, 1.0f, assets.get_color_text_primary());
    
    // 4. 繪製下方的當前身份與剩餘 AP 顯示 (格式：身份 [AP])
    std::string info_str = get_identity_name() + ":" + std::to_string(m_ap);
    Vector2 info_size = MeasureTextEx(assets.get_font(), info_str.c_str(), 11.0f, 1.0f);
    int info_x = cell_x + (cell_size - (int)info_size.x) / 2;
    int info_y = cell_y + cell_size - 14;
    
    // 加上極小半透明底條以利於不同棋盤格顏色上閱讀
    DrawRectangle(info_x - 3, info_y - 1, (int)info_size.x + 6, 14, Color{ 15, 23, 42, 150 });
    DrawTextEx(assets.get_font(), info_str.c_str(), Vector2{ (float)info_x, (float)info_y }, 11.0f, 1.0f, assets.get_color_text_secondary());
}

void Hero::update() {
    // 狀態更新邏輯 (未來實作)
}

void Hero::init_starting_deck() {
    m_deck.clear();
    
    // 根據指示配置 11 張西洋棋子卡牌：
    // 3x 士兵 (Pawn)
    for (int i = 0; i < 3; ++i) {
        m_deck.push_back(Card{ "士兵", ChessPiece::PAWN, 1, "選擇：以士兵方式移動/攻擊" });
    }
    // 2x 騎士 (Knight)
    for (int i = 0; i < 2; ++i) {
        m_deck.push_back(Card{ "騎士", ChessPiece::KNIGHT, 1, "選擇：以騎士方式移動/攻擊" });
    }
    // 2x 主教 (Bishop)
    for (int i = 0; i < 2; ++i) {
        m_deck.push_back(Card{ "主教", ChessPiece::BISHOP, 1, "選擇：以主教方式移動/攻擊" });
    }
    // 2x 城堡 (Rook)
    for (int i = 0; i < 2; ++i) {
        m_deck.push_back(Card{ "城堡", ChessPiece::ROOK, 1, "選擇：以城堡方式移動/攻擊" });
    }
    // 1x 國王 (King)
    m_deck.push_back(Card{ "國王", ChessPiece::KING, 1, "選擇：以國王方式移動/攻擊" });
    // 1x 皇后 (Queen)
    m_deck.push_back(Card{ "皇后", ChessPiece::QUEEN, 1, "選擇：以皇后方式移動/攻擊" });

    // 複製到抽牌堆並初始化其餘堆疊
    m_draw_pile = m_deck;
    m_discard_pile.clear();
    m_hand.clear();

    // 進行初始洗牌與抽牌
    shuffle_draw_pile();
    draw_cards(4);
}

void Hero::shuffle_draw_pile() {
    // 使用 C++11 std::mt19937 與 std::random_device 作為亂數生成器，保證洗牌隨機度
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(m_draw_pile.begin(), m_draw_pile.end(), g);
}

void Hero::draw_cards(int count) {
    for (int i = 0; i < count; ++i) {
        if (m_draw_pile.empty()) {
            if (m_discard_pile.empty()) {
                // 若連棄牌堆都沒牌，則無法再抽牌
                break;
            }
            // 棄牌堆洗回抽牌堆，實現循環抽牌
            m_draw_pile = m_discard_pile;
            m_discard_pile.clear();
            shuffle_draw_pile();
            TraceLog(LOG_INFO, "DECK: Shuffled discard pile back into draw pile for Hero %s", 
                     (m_type == EntityType::HERO_A) ? "A" : "B");
        }
        
        if (!m_draw_pile.empty()) {
            // 從抽牌堆尾端抽取卡牌至手牌中 (C++11 std::vector)
            m_hand.push_back(m_draw_pile.back());
            m_draw_pile.pop_back();
        }
    }
}

void Hero::discard_hand() {
    // 將當前所有手牌移入棄牌堆中 (僅限非暫時卡)，並清空手牌
    for (const auto& card : m_hand) {
        if (!card.is_temporary) {
            m_discard_pile.push_back(card);
        }
    }
    m_hand.clear();
}
