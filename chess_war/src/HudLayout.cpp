#include "HudLayout.h"

#include "AssetManager.h"
#include "Card.h"
#include "DisplayConfig.h"
#include "Hero.h"

int heroHandFirstCardY(int section_y) {
    // draw_hero_panel 累加至手牌標題 + sy(26) 第一列卡片
    return section_y + sy(230);
}

std::string hud_card_info_label(const Card& card) {
    auto& assets = AssetManager::get_instance();
    std::string card_name;
    if (card.is_temporary) {
        card_name = assets.loc("card_temp_queen");
    } else {
        switch (card.piece_type) {
            case ChessPiece::PAWN:   card_name = assets.loc("card_pawn"); break;
            case ChessPiece::KNIGHT: card_name = assets.loc("card_knight"); break;
            case ChessPiece::BISHOP: card_name = assets.loc("card_bishop"); break;
            case ChessPiece::ROOK:   card_name = assets.loc("card_rook"); break;
            case ChessPiece::KING:   card_name = assets.loc("card_king"); break;
            case ChessPiece::QUEEN:  card_name = assets.loc("card_queen"); break;
            default:                 card_name = card.name; break;
        }
    }
    return card_name + " " + assets.loc("card_consumes");
}

std::optional<HudCardHit> hud_hit_test_hand_card(
    Vector2 mouse,
    const std::shared_ptr<Hero>& hero_a,
    const std::shared_ptr<Hero>& hero_b) {
    const int hero_y = heroSectionY();
    const float col_w = static_cast<float>(hudColW());
    const float pad_y = static_cast<float>(sy(8));
    const float row_gap = static_cast<float>(sy(10));
    const float card_font = fs(12.0f);
    const float first_y = static_cast<float>(heroHandFirstCardY(hero_y));

    auto& assets = AssetManager::get_instance();
    const Font font = assets.get_font();

    struct Col {
        EntityType type;
        float col_l;
        std::shared_ptr<Hero> hero;
    };
    const Col cols[] = {
        {EntityType::HERO_A, hudColLeft(0), hero_a},
        {EntityType::HERO_B, hudColLeft(1), hero_b},
    };

    for (const Col& col : cols) {
        if (!col.hero || col.hero->is_dead() || col.hero->get_ap() <= 0) {
            continue;
        }
        float item_y = first_y;
        const auto& hand = col.hero->get_hand();
        for (size_t i = 0; i < hand.size(); ++i) {
            const std::string card_info = hud_card_info_label(hand[i]);
            const Vector2 text_sz = MeasureTextEx(font, card_info.c_str(), card_font, 1.0f);
            const float card_h = text_sz.y + pad_y * 2.0f;
            const Rectangle rect{col.col_l, item_y, col_w, card_h};
            if (CheckCollisionPointRec(mouse, rect)) {
                return HudCardHit{col.type, static_cast<int>(i)};
            }
            item_y += card_h + row_gap;
        }
    }
    return std::nullopt;
}
