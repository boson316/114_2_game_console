#include "Board.h"
#include "AssetManager.h"
#include <algorithm> // 引入標準庫以使用 C++20 std::erase_if

Board::Board(int start_x, int start_y, int cell_size)
    : m_start_x(start_x), m_start_y(start_y), m_cell_size(cell_size) {}

void Board::initialize() {
    // 建立兩位獨立的英雄實體，並設定初始網格座標
    m_hero_a = std::make_shared<Hero>(EntityType::HERO_A, Position{ 2, 7 });
    m_hero_b = std::make_shared<Hero>(EntityType::HERO_B, Position{ 5, 7 });
    
    // 初始化英雄的卡牌組與手牌
    m_hero_a->init_starting_deck();
    m_hero_b->init_starting_deck();

    // 重置擊殺數與浮動文字清單
    m_kill_count = 0;
    m_floating_texts.clear();

    // 預先生成一隻展示用怪物小兵
    spawn_minion(Position{ 4, 4 });
}

void Board::draw() {
    auto& assets = AssetManager::get_instance();

    // 1. 繪製 8x8 棋盤網格
    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            int cell_x = m_start_x + c * m_cell_size;
            int cell_y = m_start_y + r * m_cell_size;
            
            // 交替計算亮色格與暗色格
            Color cell_color = ((r + c) % 2 == 0) ? assets.get_color_grid_light() : assets.get_color_grid_dark();
            DrawRectangle(cell_x, cell_y, m_cell_size, m_cell_size, cell_color);
            
            // 繪製棋盤線框 (半透明背景色覆蓋以達到細膩邊線效果)
            DrawRectangleLines(cell_x, cell_y, m_cell_size, m_cell_size, Color{ 15, 23, 42, 80 });
        }
    }

    // 2. 渲染怪物小兵 (動態陣列)
    for (const auto& minion : m_minions) {
        if (minion && !minion->is_dead()) {
            minion->draw(m_start_x, m_start_y, m_cell_size);
        }
    }

    // 3. 渲染英雄實體 (置於怪物之上，確保視覺層級)
    if (m_hero_a && !m_hero_a->is_dead()) {
        m_hero_a->draw(m_start_x, m_start_y, m_cell_size);
    }
    if (m_hero_b && !m_hero_b->is_dead()) {
        m_hero_b->draw(m_start_x, m_start_y, m_cell_size);
    }

    // 4. 繪製網格上方的漂浮文字特效
    draw_floating_texts();
}

void Board::update() {
    // 更新所有實體的邏輯
    if (m_hero_a) m_hero_a->update();
    if (m_hero_b) m_hero_b->update();
    for (auto& minion : m_minions) {
        if (minion) minion->update();
    }
    
    // 更新浮動文字位置與淡出
    update_floating_texts(GetFrameTime());
}

void Board::spawn_minion(Position pos) {
    // 建立新小兵並加入列表
    m_minions.push_back(std::make_shared<Minion>(pos));
}

void Board::remove_dead_entities() {
    // C++20 std::erase_if：從 std::vector 中直接移除符合條件 (死亡) 的元素，取代傳統的 Erase-Remove 慣用法
    std::erase_if(m_minions, [](const std::shared_ptr<Minion>& minion) {
        return minion->is_dead();
    });
}

bool Board::is_cell_occupied(Position pos) const {
    return get_entity_at(pos) != nullptr;
}

void Board::spawn_minion_randomly() {
    // 怪物上限為 5 隻
    if (m_minions.size() >= 5) return;

    int attempts = 0;
    // 安全隨機嘗試次數上限，防範無窮迴圈
    while (attempts < 100) {
        int rx = GetRandomValue(0, 7);
        int ry = GetRandomValue(0, 7);
        Position test_pos{ rx, ry };
        
        if (!is_cell_occupied(test_pos)) {
            spawn_minion(test_pos);
            TraceLog(LOG_INFO, "ENEMY SPAWN: Spawned new minion at (%d,%d)", rx, ry);
            break;
        }
        attempts++;
    }
}

void Board::execute_enemy_actions() {
    if (!m_hero_a || !m_hero_b) return;

    Position pos_a = m_hero_a->get_pos();
    Position pos_b = m_hero_b->get_pos();

    // 依序執行各個怪物的 AI 行動
    for (auto& minion : m_minions) {
        if (!minion || minion->is_dead()) continue;

        Position minion_pos = minion->get_pos();
        
        // 計算當前與英雄們的距離 (曼哈頓距離)
        int dist_a = std::abs(minion_pos.x - pos_a.x) + std::abs(minion_pos.y - pos_a.y);
        int dist_b = std::abs(minion_pos.x - pos_b.x) + std::abs(minion_pos.y - pos_b.y);

        // 1. 若已經相鄰 (距離為 1)，不移動直接發動攻擊
        if (dist_a == 1) {
            m_hero_a->take_damage(1);
            add_floating_text(AssetManager::get_instance().loc("minus_1_hp"), pos_a, Color{ 239, 68, 68, 255 });
            AssetManager::get_instance().play_sound_hit(); // 播放受傷音效
            TraceLog(LOG_INFO, "ENEMY ACTION: Minion at (%d,%d) attacked Hero A (1 Damage)", minion_pos.x, minion_pos.y);
            continue;
        }
        if (dist_b == 1) {
            m_hero_b->take_damage(1);
            add_floating_text(AssetManager::get_instance().loc("minus_1_hp"), pos_b, Color{ 239, 68, 68, 255 });
            AssetManager::get_instance().play_sound_hit(); // 播放受傷音效
            TraceLog(LOG_INFO, "ENEMY ACTION: Minion at (%d,%d) attacked Hero B (1 Damage)", minion_pos.x, minion_pos.y);
            continue;
        }

        // 2. 若不相鄰，朝最近英雄移動一格
        Position next_pos = minion->calculate_ai_move(pos_a, pos_b);

        // 安全移動檢查：僅允許移動至未佔用之格子
        if (!is_cell_occupied(next_pos)) {
            minion->set_pos(next_pos);
            TraceLog(LOG_INFO, "ENEMY ACTION: Minion moved from (%d,%d) to (%d,%d)", minion_pos.x, minion_pos.y, next_pos.x, next_pos.y);

            // 移動後再次檢查是否相鄰，若相鄰則進行攻擊
            int new_dist_a = std::abs(next_pos.x - pos_a.x) + std::abs(next_pos.y - pos_a.y);
            int new_dist_b = std::abs(next_pos.x - pos_b.x) + std::abs(next_pos.y - pos_b.y);

            if (new_dist_a == 1) {
                m_hero_a->take_damage(1);
                add_floating_text(AssetManager::get_instance().loc("minus_1_hp"), pos_a, Color{ 239, 68, 68, 255 });
                AssetManager::get_instance().play_sound_hit(); // 播放受傷音效
                TraceLog(LOG_INFO, "ENEMY ACTION: Minion at (%d,%d) attacked Hero A after moving", next_pos.x, next_pos.y);
            } else if (new_dist_b == 1) {
                m_hero_b->take_damage(1);
                add_floating_text(AssetManager::get_instance().loc("minus_1_hp"), pos_b, Color{ 239, 68, 68, 255 });
                AssetManager::get_instance().play_sound_hit(); // 播放受傷音效
                TraceLog(LOG_INFO, "ENEMY ACTION: Minion at (%d,%d) attacked Hero B after moving", next_pos.x, next_pos.y);
            }
        }
    }
}

std::shared_ptr<Entity> Board::get_entity_at(Position pos) const {
    // 檢查英雄 A 座標
    if (m_hero_a && m_hero_a->get_pos() == pos) {
        return m_hero_a;
    }
    // 檢查英雄 B 座標
    if (m_hero_b && m_hero_b->get_pos() == pos) {
        return m_hero_b;
    }
    // 檢查小兵列表 (使用 C++11 Range-based for loop 遍歷)
    for (const auto& minion : m_minions) {
        if (minion && minion->get_pos() == pos) {
            return minion;
        }
    }
    return nullptr;
}

bool Board::execute_hero_move(std::shared_ptr<Hero> hero, Position to, Card card) {
    if (!hero) return false;

    // 1. 處理王車易位
    bool is_castling = false;
    auto target_entity = get_entity_at(to);
    if (target_entity && target_entity->get_type() == EntityType::HERO_A && hero->get_type() == EntityType::HERO_B) {
        auto hero_a = std::static_pointer_cast<Hero>(target_entity);
        if (card.piece_type == ChessPiece::ROOK && 
            !hero->has_moved_this_turn() && 
            hero_a->get_identity() == ChessPiece::KING && 
            hero_a->has_moved_this_turn()) {
            
            // 互換座標
            Position pos_b = hero->get_pos();
            Position pos_a = hero_a->get_pos();
            hero->set_pos(pos_a);
            hero_a->set_pos(pos_b);
            
            is_castling = true;
            TraceLog(LOG_INFO, "CASTLING: Swapped Hero A and Hero B!");
        }
    }

    if (!is_castling) {
        // 普通移動
        hero->set_pos(to);
    }

    // 2. 更新身份（如果是本回合第一次出牌）
    if (hero->get_identity() == ChessPiece::NONE) {
        hero->set_identity(card.piece_type);
        TraceLog(LOG_INFO, "IDENTITY SET: Hero identity set to %s from first card played.", card.name.c_str());
    }

    // 3. 標記已移動
    hero->set_has_moved_this_turn(true);

    // 4. 消耗 1 AP
    hero->consume_ap(1);

    // 5. 士兵底線升變判定 (y == 0)
    if (card.piece_type == ChessPiece::PAWN && hero->get_pos().y == 0) {
        hero->set_ap(hero->get_ap() + 1);
        Card temp_queen{ "臨時皇后", ChessPiece::QUEEN, 1, "選擇：以臨時皇后方式移動/攻擊", true };
        hero->add_card_to_hand(temp_queen);
        auto& assets = AssetManager::get_instance();
        add_floating_text(assets.loc("backline_promo"), hero->get_pos(), Color{ 234, 179, 8, 255 });
        add_floating_text(assets.loc("plus_1_ap"), hero->get_pos(), Color{ 34, 197, 94, 255 });
        AssetManager::get_instance().play_sound_promotion(); // 播放升變音效
        TraceLog(LOG_INFO, "PROMOTION: Pawn reached y == 0. Promoted, gained +1 AP and Temp Queen.");
    }

    // 播放移動音效
    AssetManager::get_instance().play_sound_move();
    return true;
}

bool Board::execute_hero_attack(std::shared_ptr<Hero> hero, Position to, Card card) {
    if (!hero) return false;

    auto target_entity = get_entity_at(to);
    if (!target_entity || target_entity->get_type() != EntityType::MINION) {
        return false;
    }

    auto minion = std::static_pointer_cast<Minion>(target_entity);

    // 播放攻擊音效
    AssetManager::get_instance().play_sound_attack();

    // 1. 扣除生命值
    int prev_hp = minion->get_hp();
    int dmg = hero->get_damage();
    minion->take_damage(dmg);
    TraceLog(LOG_INFO, "ATTACK: Hero attacked minion at (%d,%d) dealing %d damage (HP: %d -> %d)", 
             to.x, to.y, dmg, prev_hp, minion->get_hp());

    // 播放受傷音效
    AssetManager::get_instance().play_sound_hit();

    // 彈出傷害浮動文字 (紅色)
    add_floating_text("-" + std::to_string(dmg), to, Color{ 239, 68, 68, 255 });

    // 2. 更新身份（如果是本回合第一次出牌）
    if (hero->get_identity() == ChessPiece::NONE) {
        hero->set_identity(card.piece_type);
        TraceLog(LOG_INFO, "IDENTITY SET: Hero identity set to %s from first card played.", card.name.c_str());
    }

    // 3. 吃子位移：若怪物已死，且目標格在 9 宮格內，英雄移動到該格
    if (minion->is_dead()) {
        // 播放怪物死亡音效
        AssetManager::get_instance().play_sound_death();

        Position hero_pos = hero->get_pos();
        int dx = std::abs(to.x - hero_pos.x);
        int dy = std::abs(to.y - hero_pos.y);
        
        if (dx <= 1 && dy <= 1) {
            hero->set_pos(to);
            hero->set_has_moved_this_turn(true);
            TraceLog(LOG_INFO, "CAPTURE MOVE: Hero captured minion and moved to (%d,%d)!", to.x, to.y);

            // 士兵吃子位移到底線升變判定 (y == 0)
            if (card.piece_type == ChessPiece::PAWN && hero->get_pos().y == 0) {
                hero->set_ap(hero->get_ap() + 1);
                Card temp_queen{ "臨時皇后", ChessPiece::QUEEN, 1, "選擇：以臨時皇后方式移動/攻擊", true };
                hero->add_card_to_hand(temp_queen);
                auto& assets = AssetManager::get_instance();
                add_floating_text(assets.loc("backline_promo"), hero->get_pos(), Color{ 234, 179, 8, 255 });
                add_floating_text(assets.loc("plus_1_ap"), hero->get_pos(), Color{ 34, 197, 94, 255 });
                AssetManager::get_instance().play_sound_promotion(); // 播放升變音效
                TraceLog(LOG_INFO, "PROMOTION: Pawn captured minion at y == 0. Promoted, gained +1 AP and Temp Queen.");
            }
        }
        
        // 彈出擊殺與 +1 XP 特效文字
        auto& assets = AssetManager::get_instance();
        add_floating_text(assets.loc("killed_text"), to, Color{ 148, 163, 184, 255 });
        add_floating_text(assets.loc("plus_1_xp"), to, Color{ 234, 179, 8, 255 });

        // 增加累計擊殺小兵統計
        m_kill_count++;

        // 增加英雄 XP +1 (擊殺怪物)
        hero->add_xp(1);
        TraceLog(LOG_INFO, "XP UP: Hero gained 1 XP (Current: %d)", hero->get_xp());
    }

    // 4. 消耗 1 AP
    hero->consume_ap(1);

    return true;
}

// ============================================================================
// 浮動文字特效輔助方法實作 (C++11/C++20)
// ============================================================================

void Board::add_floating_text(const std::string& text, Position grid_pos, Color color) {
    // 計算該格子中心在螢幕上的位置 (像素座標)
    float screen_x = m_start_x + grid_pos.x * m_cell_size + m_cell_size / 2.0f;
    float screen_y = m_start_y + grid_pos.y * m_cell_size + 15.0f; // 格子稍微偏上

    // 加上隨機的 X/Y 偏移，防範多個字體重疊
    screen_x += GetRandomValue(-10, 10);
    screen_y += GetRandomValue(-8, 8);

    m_floating_texts.push_back(FloatingText{ text, screen_x, screen_y, color, 1.0f, 1.2f });
}

void Board::update_floating_texts(float dt) {
    // 飄移與生命週期遞減
    for (auto& ft : m_floating_texts) {
        ft.y -= 35.0f * dt;      // 向上飄移 35 像素/秒
        ft.lifetime -= dt;
        
        // 剩餘時間低於 0.5 秒時，計算漸變透明度
        if (ft.lifetime < 0.5f) {
            ft.alpha = ft.lifetime / 0.5f;
        }
    }

    // C++20 std::erase_if 自動回收生命耗盡的漂浮文字
    std::erase_if(m_floating_texts, [](const FloatingText& ft) {
        return ft.lifetime <= 0.0f;
    });
}

void Board::draw_floating_texts() const {
    auto& assets = AssetManager::get_instance();
    Font font = assets.get_font();
    for (const auto& ft : m_floating_texts) {
        Color text_color = ft.color;
        text_color.a = (unsigned char)(ft.alpha * 255);

        Vector2 text_size = MeasureTextEx(font, ft.text.c_str(), 12.0f, 1.0f);
        // 繪製黑色立體細陰影
        DrawTextEx(font, ft.text.c_str(), Vector2{ (float)((int)ft.x - text_size.x / 2 + 1), (float)((int)ft.y + 1) }, 12.0f, 1.0f, Color{ 15, 23, 42, text_color.a });
        DrawTextEx(font, ft.text.c_str(), Vector2{ (float)((int)ft.x - text_size.x / 2), (float)((int)ft.y) }, 12.0f, 1.0f, text_color);
    }
}
