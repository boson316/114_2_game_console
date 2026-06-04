#include "Game.h"

#include "AssetManager.h"
#include "DisplayConfig.h"
#include "HudLayout.h"
#include "MovementValidator.h"
#include <algorithm> // 引入標準庫以使用 std::find
#include <cmath>     // 引入數學庫以計算絕對值

// ============================================================================
// Game 類別實作
// ============================================================================

Game::Game() {
    // 初始化 Raylib 視窗
    InitWindow(kScreenW, kScreenH, "Chess War - Roguelike Chess Card Game");
    SetTargetFPS(60);
    
    // 初始化資源管理器
    AssetManager::get_instance().initialize();

    // 建立棋盤並初始化
    m_board = std::make_unique<Board>(boardStartX(), boardStartY(), boardCellSize());
    m_board->initialize();

    // 建立回合系統並初始化
    m_turn_system = std::make_unique<TurnSystem>();
    m_turn_system->initialize();

    // 建立 GUI 渲染器
    m_game_ui = std::make_unique<GameUI>();

    // 設定初始輸入狀態機與變數
    m_input_state = InputState::IDLE;
    m_active_hero_type = EntityType::HERO_A;
    m_selected_card_idx = -1;
    m_hovered_promotion_idx = -1;
    
    m_highlighted_moves.clear();
    m_highlighted_attacks.clear();
    m_promotion_options.clear();

    m_selected_minion_pos = Position{ -1, -1 };
    m_minion_moves.clear();
    m_minion_attacks.clear();
}

Game::~Game() {
    // 釋放資源與關閉視窗
    AssetManager::get_instance().cleanup();
    CloseWindow();
}

GameLaunchResult Game::run() {
    while (!WindowShouldClose()) {
        // 更新回合狀態機邏輯 (如玩家回合/敵方回合轉換與怪物動作)
        m_turn_system->update(*m_board);

        // 偵測與處理玩家輸入
        handle_input();

        // 更新遊戲內實體與判定
        update_game();

        // 渲染畫面
        render_game();
    }
    return GameLaunchResult::ReturnToHub;
}

void Game::handle_input() {
    // 1. 若處於勝負結算狀態，只處理 R 鍵重置，並攔截所有其他操作
    if (m_input_state == InputState::VICTORY || m_input_state == InputState::DEFEAT) {
        const int action = m_game_ui->poll_game_over_action();
        if (action == 1 || IsKeyPressed(KEY_R)) {
            TraceLog(LOG_INFO, "RESTART: Restarting game...");
            AssetManager::get_instance().play_sound_click();
            m_board->initialize();
            m_turn_system->initialize();

            m_input_state = InputState::IDLE;
            m_active_hero_type = EntityType::HERO_A;
            m_selected_card_idx = -1;
            m_hovered_promotion_idx = -1;
            m_highlighted_moves.clear();
            m_highlighted_attacks.clear();
            m_promotion_options.clear();

            m_selected_minion_pos = Position{-1, -1};
            m_minion_moves.clear();
            m_minion_attacks.clear();
        } else if (action == 2 || IsKeyPressed(KEY_ESCAPE)) {
            CloseWindow();
        }
        return;
    }

    // 2. 僅在玩家回合且視窗未關閉時允許操作
    if (m_turn_system->get_state() != TurnState::PLAYER_TURN) return;

    Vector2 mouse_pos = GetMousePosition();
    bool clicked_left = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    bool clicked_right = IsMouseButtonPressed(MOUSE_BUTTON_RIGHT);

    // 3. 若處於「升變」對話框狀態，攔截所有其他動作，僅處理卡片點擊
    if (m_input_state == InputState::PROMOTION) {
        m_hovered_promotion_idx = -1;
        
        // 動態計算三張升變卡片位置以執行懸停碰撞判定 (與 GameUI 的渲染參數對齊)
        int modal_x = 290;
        int modal_y = 185;
        int card_w = 200;
        int card_h = 210;
        int start_card_x = modal_x + 30;
        int card_y = modal_y + 100;
        int gap = 20;

        for (int i = 0; i < 3; ++i) {
            int cx = start_card_x + i * (card_w + gap);
            Rectangle card_rect{ (float)cx, (float)card_y, (float)card_w, (float)card_h };
            
            if (CheckCollisionPointRec(mouse_pos, card_rect)) {
                m_hovered_promotion_idx = i;
                break;
            }
        }

        // 玩家點選升變選項
        if (clicked_left && m_hovered_promotion_idx != -1) {
            apply_promotion(m_hovered_promotion_idx);
        }
        return; // 直接返回，阻斷所有對棋盤與 HUD 的操作
    }

    // 4. 一般玩家操作狀態
    auto active_hero = (m_active_hero_type == EntityType::HERO_A) ? m_board->get_hero_a() : m_board->get_hero_b();

    // 右鍵點擊取消當前選取的卡牌、路徑高亮與怪物選擇
    if (clicked_right) {
        m_input_state = InputState::IDLE;
        m_selected_card_idx = -1;
        m_highlighted_moves.clear();
        m_highlighted_attacks.clear();
        m_selected_minion_pos = Position{ -1, -1 };
        m_minion_moves.clear();
        m_minion_attacks.clear();
        AssetManager::get_instance().play_sound_click(); // 播放取消音效
        TraceLog(LOG_INFO, "INPUT: Cancelled selection via right-click.");
    }

    // 手動按下空白鍵 (SPACE) 結束回合
    if (IsKeyPressed(KEY_SPACE)) {
        TraceLog(LOG_INFO, "INPUT: Space key pressed! Transitioning to ENEMY_TURN...");
        AssetManager::get_instance().play_sound_click(); // 播放確認音效
        m_turn_system->end_player_turn(*m_board);
        
        m_input_state = InputState::IDLE;
        m_selected_card_idx = -1;
        m_highlighted_moves.clear();
        m_highlighted_attacks.clear();
        m_selected_minion_pos = Position{ -1, -1 };
        m_minion_moves.clear();
        m_minion_attacks.clear();
        return;
    }

    // 左鍵點擊處理
    if (clicked_left) {
        int start_x = m_board->get_start_x();
        int start_y = m_board->get_start_y();
        int cell_size = m_board->get_cell_size();

        // A. 點擊落在 8x8 棋盤網格內
        if (mouse_pos.x >= start_x && mouse_pos.x < start_x + 8 * cell_size &&
            mouse_pos.y >= start_y && mouse_pos.y < start_y + 8 * cell_size) {
            
            int col = (mouse_pos.x - start_x) / cell_size;
            int row = (mouse_pos.y - start_y) / cell_size;
            Position clicked_pos{ col, row };

            // 檢查是否點擊在已高亮的可移動格
            bool clicked_move = false;
            for (const auto& cell : m_highlighted_moves) {
                if (cell == clicked_pos) {
                    clicked_move = true;
                    break;
                }
            }

            // 檢查是否點擊在已高亮的可攻擊格
            bool clicked_attack = false;
            for (const auto& cell : m_highlighted_attacks) {
                if (cell == clicked_pos) {
                    clicked_attack = true;
                    break;
                }
            }

            // 執行卡牌出牌行動 (綠格移動 / 紅格攻擊)
            if ((clicked_move || clicked_attack) && m_selected_card_idx != -1 && active_hero && active_hero->get_ap() > 0) {
                const auto& hand = active_hero->get_hand();
                Card selected_card = hand[m_selected_card_idx];

                if (clicked_attack) {
                    m_board->execute_hero_attack(active_hero, clicked_pos, selected_card);
                    active_hero->remove_card_from_hand(m_selected_card_idx);
                } else if (clicked_move) {
                    m_board->execute_hero_move(active_hero, clicked_pos, selected_card);
                    active_hero->remove_card_from_hand(m_selected_card_idx);
                }

                // 行動完成後重置狀態
                m_input_state = InputState::IDLE;
                m_selected_card_idx = -1;
                m_highlighted_moves.clear();
                m_highlighted_attacks.clear();
            }
            // 若點擊非高亮格，則改為選取角色或怪物
            else {
                auto entity = m_board->get_entity_at(clicked_pos);
                if (entity && !entity->is_dead()) {
                    AssetManager::get_instance().play_sound_click(); // 播放選取音效
                    if (entity->get_type() == EntityType::HERO_A) {
                        m_active_hero_type = EntityType::HERO_A;
                        m_input_state = InputState::IDLE;
                        m_selected_card_idx = -1;
                        m_highlighted_moves.clear();
                        m_highlighted_attacks.clear();
                        m_selected_minion_pos = Position{ -1, -1 };
                        m_minion_moves.clear();
                        m_minion_attacks.clear();
                        TraceLog(LOG_INFO, "INPUT: Selected HERO A (Violet).");
                    } else if (entity->get_type() == EntityType::HERO_B) {
                        m_active_hero_type = EntityType::HERO_B;
                        m_input_state = InputState::IDLE;
                        m_selected_card_idx = -1;
                        m_highlighted_moves.clear();
                        m_highlighted_attacks.clear();
                        m_selected_minion_pos = Position{ -1, -1 };
                        m_minion_moves.clear();
                        m_minion_attacks.clear();
                        TraceLog(LOG_INFO, "INPUT: Selected HERO B (Cyan).");
                    } else if (entity->get_type() == EntityType::MINION) {
                        m_selected_minion_pos = clicked_pos;
                        m_selected_card_idx = -1;
                        m_highlighted_moves.clear();
                        m_highlighted_attacks.clear();
                        
                        m_minion_moves.clear();
                        m_minion_attacks.clear();
                        
                        Position mpos = clicked_pos;
                        Position dirs[4] = { {1, 0}, {-1, 0}, {0, 1}, {0, -1} };
                        
                        // 計算移動範圍：相鄰 4 向空位
                        for (int d = 0; d < 4; ++d) {
                            Position np = { mpos.x + dirs[d].x, mpos.y + dirs[d].y };
                            if (MovementValidator::is_in_board(np) && !m_board->is_cell_occupied(np)) {
                                m_minion_moves.push_back(np);
                            }
                        }
                        
                        // 計算攻擊範圍：移動後與原地鄰近格子
                        std::vector<Position> origins = m_minion_moves;
                        origins.push_back(mpos);
                        
                        for (const auto& origin : origins) {
                            for (int d = 0; d < 4; ++d) {
                                Position ap = { origin.x + dirs[d].x, origin.y + dirs[d].y };
                                if (MovementValidator::is_in_board(ap) && !(ap == mpos)) {
                                    if (std::find(m_minion_attacks.begin(), m_minion_attacks.end(), ap) == m_minion_attacks.end()) {
                                        m_minion_attacks.push_back(ap);
                                    }
                                }
                            }
                        }
                        TraceLog(LOG_INFO, "INPUT: Selected MINION at (%d,%d). Moves: %d, Attacks: %d.", 
                                 mpos.x, mpos.y, (int)m_minion_moves.size(), (int)m_minion_attacks.size());
                    }
                } else {
                    // 點選空白處取消當前選擇
                    m_input_state = InputState::IDLE;
                    m_selected_card_idx = -1;
                    m_highlighted_moves.clear();
                    m_highlighted_attacks.clear();
                    m_selected_minion_pos = Position{ -1, -1 };
                    m_minion_moves.clear();
                    m_minion_attacks.clear();
                }
            }
        }
        // B. 點擊落在右側 HUD 控制面板內
        else if (mouse_pos.x >= hudPanelX() && mouse_pos.x < hudPanelX() + hudPanelW() &&
                 mouse_pos.y >= sy(20) && mouse_pos.y < kScreenH - sy(16)) {
            const int panel_x = hudPanelX();
            const int panel_w = hudPanelW();
            Rectangle lang_btn = {static_cast<float>(panel_x + panel_w - sx(96)),
                                  static_cast<float>(sy(34)), static_cast<float>(sx(88)),
                                  static_cast<float>(sy(30))};
            if (CheckCollisionPointRec(mouse_pos, lang_btn)) {
                AssetManager::get_instance().toggle_language();
                AssetManager::get_instance().play_sound_click();
                return;
            }

            const float col_w = static_cast<float>(hudColW());
            const float col_a = hudColLeft(0);
            const float col_b = hudColLeft(1);
            const int hero_y = heroSectionY();

            // 手牌優先：雙欄 hit test，避免先切英雄清掉選取或 Y 偏移選錯卡
            if (const auto card_hit = hud_hit_test_hand_card(
                    mouse_pos, m_board->get_hero_a(), m_board->get_hero_b())) {
                m_active_hero_type = card_hit->hero;
                active_hero = (card_hit->hero == EntityType::HERO_A) ? m_board->get_hero_a()
                                                                     : m_board->get_hero_b();
                if (active_hero && active_hero->get_ap() > 0) {
                    m_selected_card_idx = card_hit->card_index;
                    m_input_state = InputState::AIMING;
                    m_highlighted_moves.clear();
                    m_highlighted_attacks.clear();
                    m_selected_minion_pos = Position{-1, -1};
                    m_minion_moves.clear();
                    m_minion_attacks.clear();
                    AssetManager::get_instance().play_sound_click();

                    const auto& hand = active_hero->get_hand();
                    const Card selected_card = hand[m_selected_card_idx];
                    TraceLog(LOG_INFO, "INPUT: Selected card %s at index %d.",
                             selected_card.name.c_str(), m_selected_card_idx);

                    m_highlighted_moves = MovementValidator::get_valid_moves(
                        selected_card.piece_type, active_hero->get_pos(), *m_board,
                        active_hero->get_rule_breakers(),
                        (m_active_hero_type == EntityType::HERO_B));
                    m_highlighted_attacks = MovementValidator::get_valid_attacks(
                        selected_card.piece_type, active_hero->get_pos(), *m_board,
                        active_hero->get_rule_breakers());
                    TraceLog(LOG_INFO, "INPUT: Calculated routes for %s. Moves: %d, Attacks: %d.",
                             selected_card.name.c_str(), (int)m_highlighted_moves.size(),
                             (int)m_highlighted_attacks.size());
                }
                return;
            }

            // 非手牌區（統計欄）：點欄切換英雄
            const float hand_top = static_cast<float>(heroHandFirstCardY(hero_y));
            if (mouse_pos.y >= hero_y && mouse_pos.y < hand_top) {
                if (mouse_pos.x >= col_a && mouse_pos.x < col_a + col_w) {
                    if (m_board->get_hero_a() && !m_board->get_hero_a()->is_dead()) {
                        m_active_hero_type = EntityType::HERO_A;
                        m_input_state = InputState::IDLE;
                        m_selected_card_idx = -1;
                        m_highlighted_moves.clear();
                        m_highlighted_attacks.clear();
                        m_selected_minion_pos = Position{-1, -1};
                        m_minion_moves.clear();
                        m_minion_attacks.clear();
                        AssetManager::get_instance().play_sound_click();
                        TraceLog(LOG_INFO, "INPUT: Selected HERO A via HUD.");
                    }
                } else if (mouse_pos.x >= col_b && mouse_pos.x < col_b + col_w) {
                    if (m_board->get_hero_b() && !m_board->get_hero_b()->is_dead()) {
                        m_active_hero_type = EntityType::HERO_B;
                        m_input_state = InputState::IDLE;
                        m_selected_card_idx = -1;
                        m_highlighted_moves.clear();
                        m_highlighted_attacks.clear();
                        m_selected_minion_pos = Position{-1, -1};
                        m_minion_moves.clear();
                        m_minion_attacks.clear();
                        AssetManager::get_instance().play_sound_click();
                        TraceLog(LOG_INFO, "INPUT: Selected HERO B via HUD.");
                    }
                }
            }
        }
    }
}

void Game::update_game() {
    // 1. 若處於結算狀態，暫停遊戲更新
    if (m_input_state == InputState::VICTORY || m_input_state == InputState::DEFEAT) {
        return;
    }

    // 2. 更新棋盤實體狀態與已死亡小兵回收
    m_board->update();
    m_board->remove_dead_entities();

    // 3. 自動切換控制對象：若當前選取的英雄死亡，但另一個存活，則自動換人
    auto hero_a = m_board->get_hero_a();
    auto hero_b = m_board->get_hero_b();
    if (hero_a && hero_b) {
        if (hero_a->is_dead() && m_active_hero_type == EntityType::HERO_A && !hero_b->is_dead()) {
            m_active_hero_type = EntityType::HERO_B;
            m_input_state = InputState::IDLE;
            m_selected_card_idx = -1;
            m_highlighted_moves.clear();
            m_highlighted_attacks.clear();
            TraceLog(LOG_INFO, "GAME: Hero A died, automatically switched control to Hero B.");
        } else if (hero_b->is_dead() && m_active_hero_type == EntityType::HERO_B && !hero_a->is_dead()) {
            m_active_hero_type = EntityType::HERO_A;
            m_input_state = InputState::IDLE;
            m_selected_card_idx = -1;
            m_highlighted_moves.clear();
            m_highlighted_attacks.clear();
            TraceLog(LOG_INFO, "GAME: Hero B died, automatically switched control to Hero A.");
        }
    }

    // 4. 檢查是否有英雄滿足升變條件 (XP >= 5)
    check_promotions();

    // 5. 勝負條件判定 (擊殺 50 隻小兵為勝利，雙英雄死亡為失敗)
    if (m_board->get_kill_count() >= 50) {
        m_input_state = InputState::VICTORY;
        AssetManager::get_instance().play_sound_victory(); // 播放勝利音效
        TraceLog(LOG_INFO, "GAME OVER: Victory! 50 minions slain.");
    } else if (hero_a && hero_b && hero_a->is_dead() && hero_b->is_dead()) {
        m_input_state = InputState::DEFEAT;
        AssetManager::get_instance().play_sound_defeat(); // 播放失敗音效
        TraceLog(LOG_INFO, "GAME OVER: Defeat! Both heroes have fallen.");
    }
}

void Game::render_game() {
    auto& assets = AssetManager::get_instance();
    BeginDrawing();
    
    // 背景清除
    ClearBackground(assets.get_color_bg());
    
    // 1. 繪製棋盤、英雄及小兵
    m_board->draw();

    // 2. 渲染手牌點選後的路徑高亮
    if (m_turn_system->get_state() == TurnState::PLAYER_TURN && m_input_state == InputState::AIMING) {
        int start_x = m_board->get_start_x();
        int start_y = m_board->get_start_y();
        int cell_size = m_board->get_cell_size();
        Vector2 mpos = GetMousePosition();

        // 渲染綠色移動高亮格
        for (const auto& cell : m_highlighted_moves) {
            int cx = start_x + cell.x * cell_size;
            int cy = start_y + cell.y * cell_size;
            DrawRectangle(cx + 2, cy + 2, cell_size - 4, cell_size - 4, Color{ 34, 197, 94, 90 });
            DrawRectangleLines(cx + 2, cy + 2, cell_size - 4, cell_size - 4, Color{ 34, 197, 94, 90 });
            
            // 滑鼠懸停於此格時繪製白色加粗外框
            if (mpos.x >= cx && mpos.x < cx + cell_size && mpos.y >= cy && mpos.y < cy + cell_size) {
                DrawRectangleLinesEx(Rectangle{ (float)cx, (float)cy, (float)cell_size, (float)cell_size }, 2.0f, Color{ 255, 255, 255, 200 });
            }
        }

        // 渲染紅色攻擊高亮格 (有怪物的射程格)
        for (const auto& cell : m_highlighted_attacks) {
            int cx = start_x + cell.x * cell_size;
            int cy = start_y + cell.y * cell_size;
            DrawRectangle(cx + 2, cy + 2, cell_size - 4, cell_size - 4, Color{ 239, 68, 68, 110 });
            DrawRectangleLines(cx + 2, cy + 2, cell_size - 4, cell_size - 4, Color{ 239, 68, 68, 110 });
            
            // 滑鼠懸停於此格時繪製白色加粗外框
            if (mpos.x >= cx && mpos.x < cx + cell_size && mpos.y >= cy && mpos.y < cy + cell_size) {
                DrawRectangleLinesEx(Rectangle{ (float)cx, (float)cy, (float)cell_size, (float)cell_size }, 2.0f, Color{ 255, 255, 255, 200 });
            }
        }
    }

    // 渲染怪物路徑與攻擊範圍高亮 (橘色移動格，紫粉色攻擊格)
    if (m_selected_minion_pos.x != -1) {
        int start_x = m_board->get_start_x();
        int start_y = m_board->get_start_y();
        int cell_size = m_board->get_cell_size();

        // 1. 渲染移動範圍 (橘色)
        for (const auto& cell : m_minion_moves) {
            int cx = start_x + cell.x * cell_size;
            int cy = start_y + cell.y * cell_size;
            DrawRectangle(cx + 4, cy + 4, cell_size - 8, cell_size - 8, Color{ 249, 115, 22, 100 });
            DrawRectangleLines(cx + 4, cy + 4, cell_size - 8, cell_size - 8, Color{ 249, 115, 22, 180 });
        }

        // 2. 渲染攻擊範圍 (紫粉色)
        for (const auto& cell : m_minion_attacks) {
            // 如果同時是移動範圍，則不重複繪製
            if (std::find(m_minion_moves.begin(), m_minion_moves.end(), cell) != m_minion_moves.end()) {
                continue;
            }
            int cx = start_x + cell.x * cell_size;
            int cy = start_y + cell.y * cell_size;
            DrawRectangle(cx + 4, cy + 4, cell_size - 8, cell_size - 8, Color{ 217, 70, 239, 100 });
            DrawRectangleLines(cx + 4, cy + 4, cell_size - 8, cell_size - 8, Color{ 217, 70, 239, 180 });
        }

        // 3. 繪製選中的怪物外框
        int cx = start_x + m_selected_minion_pos.x * cell_size;
        int cy = start_y + m_selected_minion_pos.y * cell_size;
        DrawRectangleLinesEx(Rectangle{ (float)cx, (float)cy, (float)cell_size, (float)cell_size }, 3.0f, assets.get_color_enemy());
    }

    // 3. 繪製選定控制角色的外框
    if (m_turn_system->get_state() == TurnState::PLAYER_TURN) {
        auto active_hero = (m_active_hero_type == EntityType::HERO_A) ? m_board->get_hero_a() : m_board->get_hero_b();
        if (active_hero && !active_hero->is_dead()) {
            Position pos = active_hero->get_pos();
            int start_x = m_board->get_start_x();
            int start_y = m_board->get_start_y();
            int cell_size = m_board->get_cell_size();
            Color active_color = (m_active_hero_type == EntityType::HERO_A) ? assets.get_color_hero_a() : assets.get_color_hero_b();
            
            DrawRectangleLinesEx(Rectangle{ (float)(start_x + pos.x * cell_size), (float)(start_y + pos.y * cell_size), (float)cell_size, (float)cell_size }, 3.0f, active_color);
        }
    }

    // 4. 繪製 HUD 面板
    m_game_ui->draw_hud(
        *m_board, 
        m_active_hero_type, 
        m_selected_card_idx, 
        m_turn_system->get_turn_count(), 
        m_turn_system->get_state(), 
        m_input_state == InputState::AIMING
    );

    // 5. 若處於升變狀態，繪製覆蓋 Modal 對話框 (置於 HUD 之上)
    if (m_input_state == InputState::PROMOTION) {
        m_game_ui->draw_promotion_overlay(m_promoting_hero_type, m_promotion_options, m_hovered_promotion_idx);
    }

    // 6. 繪製回合切換的半透明警告面板
    m_turn_system->draw();

    // 7. 若處於勝負結算狀態，繪製遊戲結束覆蓋面板 (置於最上層)
    if (m_input_state == InputState::VICTORY) {
        m_game_ui->draw_game_over_overlay(true, m_board->get_kill_count());
    } else if (m_input_state == InputState::DEFEAT) {
        m_game_ui->draw_game_over_overlay(false, m_board->get_kill_count());
    }
    
    EndDrawing();
}

void Game::check_promotions() {
    // 當前已處於升變面板狀態時，不重複檢查
    if (m_input_state == InputState::PROMOTION) return;

    // 優先檢查 Hero A，若滿足則觸發 Hero A 升變
    if (m_board->get_hero_a() && m_board->get_hero_a()->get_xp() >= 5) {
        start_promotion(EntityType::HERO_A);
    }
    // 否則檢查 Hero B
    else if (m_board->get_hero_b() && m_board->get_hero_b()->get_xp() >= 5) {
        start_promotion(EntityType::HERO_B);
    }
}

void Game::start_promotion(EntityType hero_type) {
    m_promoting_hero_type = hero_type;
    auto hero = (hero_type == EntityType::HERO_A) ? m_board->get_hero_a() : m_board->get_hero_b();
    if (!hero) return;

    TraceLog(LOG_INFO, "PROMOTION TRIGGERED: Generating options for %s", (hero_type == EntityType::HERO_A) ? "Hero A" : "Hero B");

    const auto& rule_breakers = hero->get_rule_breakers();

    // 建立所有潛在強化選項候選池 (C++11 std::vector)
    std::vector<PromotionOption> pool;
    
    // 基礎屬性強化必定加入候選池
    pool.push_back({ PromotionType::INCREASE_DAMAGE, "+1 Damage", "Increase the hero's\nbase damage by 1." });
    pool.push_back({ PromotionType::INCREASE_MAX_HP, "+1 Max HP", "Increase Max HP by 1\nand heal 1 HP." });

    // 規則破壞選項：僅在英雄「尚未解鎖該項規則」時才加入候選池
    if (!rule_breakers.pawn_backwards) {
        pool.push_back({ PromotionType::UNLOCK_PAWN_BACKWARDS, "Pawn Backwards", "Pawn cards can now\nmove/attack backwards." });
    }
    if (!rule_breakers.knight_extended_jump) {
        pool.push_back({ PromotionType::UNLOCK_KNIGHT_EXTENDED, "Knight Extended", "Knight cards can now\njump 3x1 spaces." });
    }
    if (!rule_breakers.bishop_piercing) {
        pool.push_back({ PromotionType::UNLOCK_BISHOP_PIERCING, "Bishop Piercing", "Bishop attacks can now\npierce obstacles." });
    }

    // 確保候選池中至少有 3 個選項以利挑選 (若全數解鎖，則利用屬性強化填補)
    while (pool.size() < 3) {
        if (GetRandomValue(0, 1) == 0) {
            pool.push_back({ PromotionType::INCREASE_DAMAGE, "+1 Damage", "Increase the hero's\nbase damage by 1." });
        } else {
            pool.push_back({ PromotionType::INCREASE_MAX_HP, "+1 Max HP", "Increase Max HP by 1\nand heal 1 HP." });
        }
    }

    // 隨機選取 3 個不重複的選項 (使用 Raylib GetRandomValue 挑選索引)
    m_promotion_options.clear();
    std::vector<int> selected_indices;

    while (selected_indices.size() < 3) {
        int idx = GetRandomValue(0, (int)pool.size() - 1);
        if (std::find(selected_indices.begin(), selected_indices.end(), idx) == selected_indices.end()) {
            selected_indices.push_back(idx);
        }
    }

    for (int idx : selected_indices) {
        m_promotion_options.push_back(pool[idx]);
    }

    // 進入 PROMOTION 狀態並重置卡片點擊狀態
    m_input_state = InputState::PROMOTION;
    m_hovered_promotion_idx = -1;
    AssetManager::get_instance().play_sound_promotion(); // 播放升變觸發音效
}

void Game::apply_promotion(int option_idx) {
    if (option_idx < 0 || option_idx >= 3) return;

    auto hero = (m_promoting_hero_type == EntityType::HERO_A) ? m_board->get_hero_a() : m_board->get_hero_b();
    if (!hero) return;

    const auto& opt = m_promotion_options[option_idx];

    // 根據選取的種類施加對應的效果 (Roguelike Growth 強化)
    switch (opt.type) {
        case PromotionType::INCREASE_DAMAGE:
            hero->set_damage(hero->get_damage() + 1);
            TraceLog(LOG_INFO, "PROMOTION APPLY: Increased Damage of %s by 1 (Current: %d)", 
                     (m_promoting_hero_type == EntityType::HERO_A) ? "Hero A" : "Hero B", hero->get_damage());
            break;
            
        case PromotionType::INCREASE_MAX_HP:
            hero->set_max_hp(hero->get_max_hp() + 1);
            hero->set_hp(hero->get_hp() + 1); // 升級 Max HP 同時恢復 1 HP
            TraceLog(LOG_INFO, "PROMOTION APPLY: Increased Max HP of %s by 1 (Current Max: %d, Current HP: %d)", 
                     (m_promoting_hero_type == EntityType::HERO_A) ? "Hero A" : "Hero B", hero->get_max_hp(), hero->get_hp());
            break;
            
        case PromotionType::UNLOCK_PAWN_BACKWARDS:
            hero->get_rule_breakers().pawn_backwards = true;
            TraceLog(LOG_INFO, "PROMOTION APPLY: Unlocked Pawn Backwards rule for %s", 
                     (m_promoting_hero_type == EntityType::HERO_A) ? "Hero A" : "Hero B");
            break;
            
        case PromotionType::UNLOCK_KNIGHT_EXTENDED:
            hero->get_rule_breakers().knight_extended_jump = true;
            TraceLog(LOG_INFO, "PROMOTION APPLY: Unlocked Knight Extended Jump rule for %s", 
                     (m_promoting_hero_type == EntityType::HERO_A) ? "Hero A" : "Hero B");
            break;
            
        case PromotionType::UNLOCK_BISHOP_PIERCING:
            hero->get_rule_breakers().bishop_piercing = true;
            TraceLog(LOG_INFO, "PROMOTION APPLY: Unlocked Bishop Piercing rule for %s", 
                     (m_promoting_hero_type == EntityType::HERO_A) ? "Hero A" : "Hero B");
            break;
    }

    // 扣除 5 XP
    hero->set_xp(hero->get_xp() - 5);

    // 重置所有選卡狀態，恢復 IDLE 以繼續遊戲
    m_input_state = InputState::IDLE;
    m_selected_card_idx = -1;
    m_hovered_promotion_idx = -1;
    
    m_highlighted_moves.clear();
    m_highlighted_attacks.clear();
    m_promotion_options.clear();
}
