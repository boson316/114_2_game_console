#include "GameUI.h"

#include "AssetManager.h"
#include "DisplayConfig.h"
#include "HudLayout.h"

#include <algorithm>
#include <cmath>

// ============================================================================
// GameUI 類別實作
// ============================================================================

void GameUI::draw_hud(
    const Board& board, 
    EntityType active_hero_type, 
    int selected_card_idx, 
    int turn_count, 
    TurnState turn_state, 
    bool is_aiming
) {
    auto& assets = AssetManager::get_instance();
    auto hero_a = board.get_hero_a();
    auto hero_b = board.get_hero_b();
    Font font = assets.get_font();

    const int panelX = hudPanelX();
    const int panelY = sy(20);
    const int panelW = hudPanelW();
    const int panelH = kScreenH - panelY - sy(16);
    const int colWInt = hudColW();
    const float colW = static_cast<float>(colWInt);
    const float colA = hudColLeft(0);
    const float colB = hudColLeft(1);
    const float hudPadX = static_cast<float>(sx(8));
    const float hudPadY = static_cast<float>(sy(8));
    const float cardRowGap = static_cast<float>(sy(10));

    auto draw_txt = [&](const std::string& text, float x, float y, float size, Color color) {
        DrawTextEx(font, text.c_str(), Vector2{ x, y }, fs(size), 1.0f, color);
    };

    auto measure_hud = [&](const std::string& text, float size) {
        return MeasureTextEx(font, text.c_str(), fs(size), 1.0f);
    };

    // 1. 畫面上方大標題與擊殺計數顯示 (Sleek Theme)
    draw_txt(assets.loc("title"), static_cast<float>(sx(100)), static_cast<float>(sy(25)), 32.0f,
             assets.get_color_text_primary());
    std::string kill_str = assets.loc("kill_progress") + std::to_string(board.get_kill_count()) + " / 50";
    draw_txt(kill_str, static_cast<float>(sx(350)), static_cast<float>(sy(38)), 16.0f,
             assets.get_color_accent_gold());

    DrawRectangle(panelX, panelY, panelW, panelH, assets.get_color_panel());
    DrawRectangleLines(panelX, panelY, panelW, panelH, assets.get_color_grid_light());

    draw_txt(assets.loc("state_title"), static_cast<float>(panelX + sx(12)),
             static_cast<float>(sy(40)), 22.0f, assets.get_color_text_primary());

    Rectangle langBtn = {static_cast<float>(panelX + panelW - sx(96)), static_cast<float>(sy(34)),
                         static_cast<float>(sx(88)), static_cast<float>(sy(30))};
    bool is_hover_lang = CheckCollisionPointRec(GetMousePosition(), langBtn);
    Color lang_btn_bg = is_hover_lang ? Color{ 51, 65, 85, 255 } : assets.get_color_panel();
    DrawRectangleRec(langBtn, lang_btn_bg);
    DrawRectangleLinesEx(langBtn, 1.0f, assets.get_color_accent_gold());

    const char* btn_lbl = (assets.get_language() == Language::EN) ? "ZH" : "EN";
    const float langSize = fs(14.0f);
    Vector2 lbl_size = MeasureTextEx(font, btn_lbl, langSize, 1.0f);
    DrawTextEx(font, btn_lbl,
               Vector2{langBtn.x + (langBtn.width - lbl_size.x) / 2.0f,
                       langBtn.y + (langBtn.height - lbl_size.y) / 2.0f},
               langSize, 1.0f, assets.get_color_accent_gold());

    std::string turn_str = assets.loc("turn") + std::to_string(turn_count);
    draw_txt(turn_str, static_cast<float>(panelX + sx(12)), static_cast<float>(sy(82)), 17.0f,
             assets.get_color_accent_gold());

    std::string state_str = assets.loc("phase_title") + std::string((turn_state == TurnState::PLAYER_TURN) ? assets.loc("phase_player") : assets.loc("phase_enemy"));
    draw_txt(state_str, static_cast<float>(panelX + sx(12)), static_cast<float>(sy(112)), 15.0f,
             assets.get_color_text_secondary());

    float hudContentBottom = static_cast<float>(heroSectionY());

    auto draw_hero_hand_at = [&](EntityType hero_type, float colL, float colWLocal, float start_y,
                                 const std::shared_ptr<Hero>& hero) -> float {
        if (!hero) {
            return start_y;
        }
        const int statBarW = static_cast<int>(colWLocal - hudPadX * 2.0f);
        (void)statBarW;
        draw_txt(assets.loc("hand"), colL + hudPadX, start_y, 14.0f, assets.get_color_text_primary());
        const auto& hand = hero->get_hand();
        float item_y = start_y + static_cast<float>(sy(26));
        const float cardFont = 12.0f;
        for (size_t i = 0; i < hand.size(); ++i) {
            const bool is_selected =
                (active_hero_type == hero_type && selected_card_idx == static_cast<int>(i));
            const std::string card_info = hud_card_info_label(hand[i]);
            const Vector2 text_sz = measure_hud(card_info, cardFont);
            const float card_h = text_sz.y + hudPadY * 2.0f;
            const Rectangle card_rect{colL, item_y, colWLocal, card_h};
            const bool is_hovered = CheckCollisionPointRec(GetMousePosition(), card_rect);
            Color bg_color = Color{30, 41, 59, 255};
            if (is_selected) {
                bg_color = Color{51, 65, 85, 255};
            } else if (is_hovered) {
                bg_color = Color{38, 50, 71, 255};
            }
            DrawRectangleRec(card_rect, bg_color);
            const Color border_color =
                is_selected ? assets.get_color_accent_gold() : Color{71, 85, 105, 255};
            DrawRectangleLinesEx(card_rect, is_selected ? 1.5f : 1.0f, border_color);
            draw_txt(card_info, colL + hudPadX, item_y + hudPadY, cardFont,
                     is_selected ? assets.get_color_accent_gold() : assets.get_color_text_primary());
            item_y += card_h + cardRowGap;
        }
        return item_y;
    };

    auto draw_stat_bar = [&](int x, int y, int w, int h, float percent, Color fill) {
        DrawRectangle(x, y, w, h, Color{30, 41, 59, 255});
        if (percent > 0.0f) {
            DrawRectangle(x, y, static_cast<int>(w * percent), h, fill);
        }
        DrawRectangleLines(x, y, w, h, Color{71, 85, 105, 100});
    };

    auto draw_hero_panel = [&](float colL, float colWLocal, int section_y, EntityType hero_type,
                               const std::shared_ptr<Hero>& hero, Color title_color,
                               const std::string& title_key) -> float {
        if (!hero) {
            return static_cast<float>(section_y);
        }
        const int statBarW = static_cast<int>(colWLocal - hudPadX * 2.0f);
        std::string title = assets.loc(title_key);
        if (active_hero_type == hero_type) {
            title += " <";
        }
        draw_txt(title, colL + hudPadX, static_cast<float>(section_y + sy(12)), 16.0f, title_color);

        float y = static_cast<float>(section_y + sy(38));
        const int barH = sy(8);
        const int barX = static_cast<int>(colL + hudPadX);

        std::string hp_str = assets.loc("hp") + std::to_string(hero->get_hp()) + "/" +
                             std::to_string(hero->get_max_hp());
        draw_txt(hp_str, colL + hudPadX, y, 14.0f, assets.get_color_hp());
        y += static_cast<float>(sy(22));
        float hp_percent =
            (hero->get_max_hp() > 0) ? static_cast<float>(hero->get_hp()) / hero->get_max_hp() : 0.0f;
        Color hp_bar_color = Color{34, 197, 94, 255};
        if (hp_percent <= 0.2f) {
            hp_bar_color = Color{239, 68, 68, 255};
        } else if (hp_percent <= 0.5f) {
            hp_bar_color = Color{234, 179, 8, 255};
        }
        draw_stat_bar(barX, static_cast<int>(y), statBarW, barH, hp_percent, hp_bar_color);
        y += static_cast<float>(sy(20));

        std::string ap_str = assets.loc("ap") + std::to_string(hero->get_ap());
        draw_txt(ap_str, colL + hudPadX, y, 14.0f, assets.get_color_ap());
        y += static_cast<float>(sy(26));

        std::string xp_str = assets.loc("xp") + std::to_string(hero->get_xp()) + "/5";
        draw_txt(xp_str, colL + hudPadX, y, 14.0f, assets.get_color_accent_gold());
        y += static_cast<float>(sy(22));
        float xp_percent = static_cast<float>(hero->get_xp()) / 5.0f;
        if (xp_percent > 1.0f) {
            xp_percent = 1.0f;
        }
        draw_stat_bar(barX, static_cast<int>(y), statBarW, barH, xp_percent, assets.get_color_accent_gold());
        y += static_cast<float>(sy(18));

        std::string ident_str = assets.loc("current_identity") + hero->get_identity_name();
        draw_txt(ident_str, colL + hudPadX, y, 13.0f, assets.get_color_text_secondary());
        y += static_cast<float>(sy(22));

        std::string pile_str = assets.loc("draw") + std::to_string(hero->get_draw_pile_count()) +
                               "  " + assets.loc("discard") + std::to_string(hero->get_discard_pile_count());
        draw_txt(pile_str, colL + hudPadX, y, 13.0f, assets.get_color_text_secondary());
        y += static_cast<float>(sy(22));

        std::string rules_str = assets.loc("rules");
        const auto& rb = hero->get_rule_breakers();
        if (rb.pawn_backwards) {
            rules_str += assets.loc("rule_pawn");
        }
        if (rb.knight_extended_jump) {
            rules_str += assets.loc("rule_knight");
        }
        if (rb.bishop_piercing) {
            rules_str += assets.loc("rule_bishop");
        }
        if (!rb.pawn_backwards && !rb.knight_extended_jump && !rb.bishop_piercing) {
            rules_str += assets.loc("no_rules");
        }
        draw_txt(rules_str, colL + hudPadX, y, 12.0f, Color{148, 163, 184, 255});
        y += static_cast<float>(sy(14));

        return draw_hero_hand_at(hero_type, colL, colWLocal, y, hero);
    };

    const int heroY = heroSectionY();
    DrawRectangle(panelX, heroY - sy(4), panelW, 1, assets.get_color_grid_light());
    const int divX = panelX + hudPanelPad() + colWInt + hudColGap() / 2;
    DrawLine(divX, heroY, divX, panelY + panelH - sy(8), assets.get_color_grid_light());

    if (hero_a) {
        hudContentBottom = std::max(hudContentBottom,
                                    draw_hero_panel(colA, colW, heroY, EntityType::HERO_A, hero_a,
                                                    assets.get_color_hero_a(), "hero_a"));
    }
    if (hero_b) {
        hudContentBottom = std::max(hudContentBottom,
                                    draw_hero_panel(colB, colW, heroY, EntityType::HERO_B, hero_b,
                                                    assets.get_color_hero_b(), "hero_b"));
    }

    // 操作提示：放在 A / B 欄下方（HUD panel 內）
    const float hintX = static_cast<float>(panelX + hudPanelPad());
    const float hintW = static_cast<float>(panelW - hudPanelPad() * 2);
    const float hintY = hudContentBottom + static_cast<float>(sy(12));
    const float hintLineGap = static_cast<float>(sy(22));
    const float hintPad = static_cast<float>(sy(10));
    const float hintPadX = hintX + static_cast<float>(sx(10));
    const int hintLines = is_aiming ? 4 : 2;
    const Rectangle hintBox{hintX, hintY, hintW,
                            hintPad * 2.0f + hintLineGap * static_cast<float>(hintLines - 1) +
                                static_cast<float>(sy(18))};
    DrawRectangle(static_cast<int>(hintX), static_cast<int>(hintY) - sy(6),
                  static_cast<int>(hintW), 1, assets.get_color_grid_light());
    DrawRectangleRec(hintBox, Color{30, 41, 59, 220});
    DrawRectangleLinesEx(hintBox, 1.0f, assets.get_color_grid_light());

    float lineY = hintY + hintPad;
    if (is_aiming) {
        draw_txt(assets.loc("aiming_move"), hintPadX, lineY, 13.0f, Color{34, 197, 94, 255});
        lineY += hintLineGap;
        draw_txt(assets.loc("aiming_attack"), hintPadX, lineY, 13.0f, Color{239, 68, 68, 255});
        lineY += hintLineGap;
    }
    draw_txt(assets.loc("space_end"), hintPadX, lineY, 14.0f, assets.get_color_accent_gold());
    lineY += hintLineGap;
    draw_txt(assets.loc("ctrl_tip"), hintPadX, lineY, 12.0f, assets.get_color_text_secondary());
}

void GameUI::draw_promotion_overlay(
    EntityType promoting_hero_type, 
    const std::vector<PromotionOption>& options, 
    int hovered_option_idx
) {
    auto& assets = AssetManager::get_instance();
    Font font = assets.get_font();

    auto draw_txt = [&](const std::string& text, float x, float y, float size, Color color) {
        DrawTextEx(font, text.c_str(), Vector2{ x, y }, size, 1.0f, color);
    };

    // 1. 全螢幕半透明磨砂黑遮罩
    DrawRectangle(0, 0, kScreenW, kScreenH, Color{15, 23, 42, 180});

    // 2. 升變 modal 面板邊框與容器 (X=290, Y=185, W=700, H=350)
    int modal_x = 290;
    int modal_y = 185;
    int modal_w = 700;
    int modal_h = 350;

    DrawRectangle(modal_x, modal_y, modal_w, modal_h, assets.get_color_panel());
    DrawRectangleLines(modal_x, modal_y, modal_w, modal_h, assets.get_color_accent_gold());

    // 3. 標題與指引
    std::string hero_name = (promoting_hero_type == EntityType::HERO_A) ? assets.loc("hero_a") : assets.loc("hero_b");
    Color hero_color = (promoting_hero_type == EntityType::HERO_A) ? assets.get_color_hero_a() : assets.get_color_hero_b();

    draw_txt(assets.loc("promotion"), (float)(modal_x + 30), (float)(modal_y + 25), 24.0f, assets.get_color_accent_gold());
    Vector2 promo_title_size = MeasureTextEx(font, assets.loc("promotion").c_str(), 24.0f, 1.0f);
    draw_txt(hero_name, (float)(modal_x + 35 + promo_title_size.x), (float)(modal_y + 28), 20.0f, hero_color);
    draw_txt(assets.loc("promotion_desc"), (float)(modal_x + 30), (float)(modal_y + 60), 14.0f, assets.get_color_text_secondary());

    // 4. 繪製 3 個卡片式選項
    int card_w = 200;
    int card_h = 210;
    int start_card_x = modal_x + 30;
    int card_y = modal_y + 100;
    int gap = 20;

    for (size_t i = 0; i < options.size(); ++i) {
        int cx = start_card_x + i * (card_w + gap);
        int cy = card_y;

        bool is_hovered = ((int)i == hovered_option_idx);

        // 微動畫：選取懸停時卡片往上微移 4 像素
        if (is_hovered) {
            cy -= 4;
        }

        Rectangle card_rect{ (float)cx, (float)cy, (float)card_w, (float)card_h };

        Color card_bg = is_hovered ? Color{ 38, 50, 71, 255 } : Color{ 30, 41, 59, 255 };
        DrawRectangleRec(card_rect, card_bg);

        Color card_border = is_hovered ? assets.get_color_accent_gold() : Color{ 71, 85, 105, 255 };
        DrawRectangleLinesEx(card_rect, is_hovered ? 2.0f : 1.0f, card_border);

        // 依升變類型自訂圖示/文字配色以加強視覺層級
        Color accent_color = assets.get_color_text_primary();
        std::string opt_title;
        std::string opt_desc;

        // 翻譯升變選項標題與描述
        if (options[i].type == PromotionType::INCREASE_DAMAGE) {
            accent_color = assets.get_color_hp(); // 攻擊力用血紅色強調
            opt_title = assets.loc("promo_dmg_title");
            opt_desc = assets.loc("promo_dmg_desc");
        } else if (options[i].type == PromotionType::INCREASE_MAX_HP) {
            accent_color = Color{ 34, 197, 94, 255 }; // 生命上限用綠色
            opt_title = assets.loc("promo_hp_title");
            opt_desc = assets.loc("promo_hp_desc");
        } else if (options[i].type == PromotionType::UNLOCK_PAWN_BACKWARDS) {
            accent_color = assets.get_color_accent_gold(); // 規則破壞用金黃色
            opt_title = assets.loc("promo_pawn_title");
            opt_desc = assets.loc("promo_pawn_desc");
        } else if (options[i].type == PromotionType::UNLOCK_KNIGHT_EXTENDED) {
            accent_color = assets.get_color_accent_gold();
            opt_title = assets.loc("promo_knight_title");
            opt_desc = assets.loc("promo_knight_desc");
        } else if (options[i].type == PromotionType::UNLOCK_BISHOP_PIERCING) {
            accent_color = assets.get_color_accent_gold();
            opt_title = assets.loc("promo_bishop_title");
            opt_desc = assets.loc("promo_bishop_desc");
        } else {
            opt_title = options[i].title;
            opt_desc = options[i].description;
        }

        // 渲染選項標題
        draw_txt(opt_title, (float)(cx + 15), (float)(cy + 25), 14.0f, accent_color);
        
        // 渲染選項描述 (分行繪製)
        size_t split_pos = opt_desc.find('\n');
        if (split_pos != std::string::npos) {
            std::string line1 = opt_desc.substr(0, split_pos);
            std::string line2 = opt_desc.substr(split_pos + 1);
            draw_txt(line1, (float)(cx + 15), (float)(cy + 65), 11.0f, assets.get_color_text_secondary());
            draw_txt(line2, (float)(cx + 15), (float)(cy + 85), 11.0f, assets.get_color_text_secondary());
        } else {
            draw_txt(opt_desc, (float)(cx + 15), (float)(cy + 65), 11.0f, assets.get_color_text_secondary());
        }

        // 底部 SELECT 按鈕效果
        std::string select_str = assets.loc("select_action");
        Vector2 select_text_size = MeasureTextEx(font, select_str.c_str(), 11.0f, 1.0f);
        if (is_hovered) {
            DrawRectangle(cx + 30, cy + card_h - 40, card_w - 60, 25, assets.get_color_accent_gold());
            draw_txt(select_str, (float)(cx + (card_w - select_text_size.x) / 2.0f), (float)(cy + card_h - 34), 11.0f, Color{ 15, 23, 42, 255 });
        } else {
            DrawRectangleLines(cx + 30, cy + card_h - 40, card_w - 60, 25, Color{ 71, 85, 105, 150 });
            draw_txt(select_str, (float)(cx + (card_w - select_text_size.x) / 2.0f), (float)(cy + card_h - 34), 11.0f, Color{ 148, 163, 184, 255 });
        }
    }
}

void GameUI::draw_game_over_overlay(bool victory, int kill_count) {
    auto& assets = AssetManager::get_instance();
    Font font = assets.get_font();

    auto draw_txt = [&](const std::string& text, float x, float y, float size, Color color) {
        DrawTextEx(font, text.c_str(), Vector2{ x, y }, size, 1.0f, color);
    };

    // 1. 全螢幕半透明磨砂黑遮罩 (Sleek glassmorphism overlay)
    DrawRectangle(0, 0, kScreenW, kScreenH, Color{15, 23, 42, 200});

    int modal_w = sx(550);
    int modal_h = sy(380);
    int modal_x = (kScreenW - modal_w) / 2;
    int modal_y = (kScreenH - modal_h) / 2;

    // 繪製陰影
    DrawRectangle(modal_x + 8, modal_y + 8, modal_w, modal_h, Color{ 8, 10, 15, 150 });

    // 繪製主背景板
    DrawRectangle(modal_x, modal_y, modal_w, modal_h, assets.get_color_panel());

    // 繪製外框線 (金色/玫紅色)
    Color border_color = victory ? assets.get_color_accent_gold() : assets.get_color_enemy();
    DrawRectangleLinesEx(Rectangle{ (float)modal_x, (float)modal_y, (float)modal_w, (float)modal_h }, 2.0f, border_color);

    // 3. 勝負主題大橫幅
    Color banner_bg = victory ? Color{ 234, 179, 8, 30 } : Color{ 244, 63, 94, 30 };
    Color banner_border = victory ? Color{ 234, 179, 8, 100 } : Color{ 244, 63, 94, 100 };
    DrawRectangle(modal_x + 15, modal_y + 20, modal_w - 30, 80, banner_bg);
    DrawRectangleLinesEx(Rectangle{ (float)(modal_x + 15), (float)(modal_y + 20), (float)(modal_w - 30), 80.0f }, 1.0f, banner_border);

    std::string banner_title = victory ? assets.loc("victory") : assets.loc("defeat");
    Color title_color = victory ? assets.get_color_accent_gold() : assets.get_color_enemy();
    
    // 計算標題寬度
    const float titleSize = fs(40.0f);
    Vector2 title_size = MeasureTextEx(font, banner_title.c_str(), titleSize, 1.0f);
    draw_txt(banner_title, static_cast<float>(modal_x + (modal_w - title_size.x) / 2),
             static_cast<float>(modal_y + sy(42)), titleSize, title_color);

    std::string kill_str = assets.loc("kill_total") + std::to_string(kill_count);
    const float killSize = fs(20.0f);
    Vector2 kill_size = MeasureTextEx(font, kill_str.c_str(), killSize, 1.0f);
    draw_txt(kill_str, static_cast<float>(modal_x + (modal_w - kill_size.x) / 2),
             static_cast<float>(modal_y + sy(130)), killSize, assets.get_color_text_primary());

    std::string desc_str = victory ? assets.loc("desc_victory") : assets.loc("desc_defeat");
    const float descSize = fs(16.0f);
    Vector2 desc_size = MeasureTextEx(font, desc_str.c_str(), descSize, 1.0f);
    draw_txt(desc_str, static_cast<float>(modal_x + (modal_w - desc_size.x) / 2),
             static_cast<float>(modal_y + sy(175)), descSize, assets.get_color_text_secondary());

    std::string pick_str = assets.loc("pick_action");
    const float pickSize = fs(18.0f);
    Vector2 pick_size = MeasureTextEx(font, pick_str.c_str(), pickSize, 1.0f);
    draw_txt(pick_str, static_cast<float>(modal_x + (modal_w - pick_size.x) / 2),
             static_cast<float>(modal_y + sy(220)), pickSize, assets.get_color_accent_gold());

    const float btnW = static_cast<float>(sx(200));
    const float btnH = static_cast<float>(sy(44));
    const float gap = static_cast<float>(sx(24));
    const float btnY = static_cast<float>(modal_y + sy(255));
    const float btnX0 = static_cast<float>(modal_x) + (modal_w - btnW * 2.0f - gap) * 0.5f;
    m_btn_restart_ = {btnX0, btnY, btnW, btnH};
    m_btn_menu_ = {btnX0 + btnW + gap, btnY, btnW, btnH};

    auto drawBtn = [&](Rectangle r, const std::string& label, bool primary) {
        const bool hover = CheckCollisionPointRec(GetMousePosition(), r);
        Color bg = primary ? Color{59, 130, 246, 255} : Color{71, 85, 105, 255};
        if (hover) {
            bg = primary ? Color{96, 165, 250, 255} : Color{100, 116, 139, 255};
        }
        DrawRectangleRec(r, bg);
        DrawRectangleLinesEx(r, 2.0f, assets.get_color_accent_gold());
        const float lblSize = fs(20.0f);
        Vector2 sz = MeasureTextEx(font, label.c_str(), lblSize, 1.0f);
        draw_txt(label, r.x + (r.width - sz.x) / 2.0f, r.y + (r.height - sz.y) / 2.0f - 2.0f,
                 lblSize, RAYWHITE);
    };
    drawBtn(m_btn_restart_, assets.loc("btn_restart"), true);
    drawBtn(m_btn_menu_, assets.loc("btn_main_menu"), false);
}

int GameUI::poll_game_over_action() const {
    if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        return 0;
    }
    if (CheckCollisionPointRec(GetMousePosition(), m_btn_restart_)) {
        return 1;
    }
    if (CheckCollisionPointRec(GetMousePosition(), m_btn_menu_)) {
        return 2;
    }
    return 0;
}
