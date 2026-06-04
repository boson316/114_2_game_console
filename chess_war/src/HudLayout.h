#pragma once

#include "Entity.h"
#include "raylib.h"

#include <memory>
#include <optional>
#include <string>

struct Card;
class Hero;

struct HudCardHit {
    EntityType hero;
    int card_index;
};

// 與 GameUI::draw_hero_panel 第一張手牌頂部 Y 對齊
int heroHandFirstCardY(int section_y);

std::string hud_card_info_label(const Card& card);

std::optional<HudCardHit> hud_hit_test_hand_card(
    Vector2 mouse,
    const std::shared_ptr<Hero>& hero_a,
    const std::shared_ptr<Hero>& hero_b);
