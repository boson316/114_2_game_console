#pragma once
#include "raylib.h"
#include "Language.h"

// ============================================================================
// AssetManager: 集中管理遊戲資源、像素色塊與色表系統 (Sleek Dark Mode)
// 採用單例模式 (Singleton Pattern) 確保資源的唯一性與生命週期管理
// ============================================================================
class AssetManager {
public:
    // C++11 區域靜態變數 (Meyers' Singleton)，保證執行緒安全與延遲初始化
    static AssetManager& get_instance();

    // 停用複製建構與指派運算子，防止單例複本產生
    AssetManager(const AssetManager&) = delete;
    AssetManager& operator=(const AssetManager&) = delete;
    AssetManager(AssetManager&&) = delete;
    AssetManager& operator=(AssetManager&&) = delete;

    // 初始化與釋放資源
    void initialize();
    void cleanup();

    // 取得 Sleek Dark Mode 色表色彩
    Color get_color_bg() const { return m_color_bg; }
    Color get_color_panel() const { return m_color_panel; }
    Color get_color_grid_light() const { return m_color_grid_light; }
    Color get_color_grid_dark() const { return m_color_grid_dark; }
    Color get_color_hero_a() const { return m_color_hero_a; }
    Color get_color_hero_b() const { return m_color_hero_b; }
    Color get_color_enemy() const { return m_color_enemy; }
    Color get_color_text_primary() const { return m_color_text_primary; }
    Color get_color_text_secondary() const { return m_color_text_secondary; }
    Color get_color_accent_gold() const { return m_color_accent_gold; }
    Color get_color_hp() const { return m_color_hp; }
    Color get_color_ap() const { return m_color_ap; }
    Color get_color_xp() const { return m_color_xp; }

    // --- 繁體中文字型與互動音效管理 ---
    Font get_font() const { 
        if (m_language == Language::EN) {
            return GetFontDefault();
        }
        return m_font; 
    }
    void play_sound_click() { PlaySound(m_sound_click); }
    void play_sound_move() { PlaySound(m_sound_move); }
    void play_sound_attack() { PlaySound(m_sound_attack); }
    void play_sound_hit() { PlaySound(m_sound_hit); }
    void play_sound_death() { PlaySound(m_sound_death); }
    void play_sound_promotion() { PlaySound(m_sound_promotion); }
    void play_sound_turn_player() { PlaySound(m_sound_turn_player); }
    void play_sound_turn_enemy() { PlaySound(m_sound_turn_enemy); }
    void play_sound_victory() { PlaySound(m_sound_victory); }
    void play_sound_defeat() { PlaySound(m_sound_defeat); }

    // --- 語言語系管理 (L10n) ---
    Language get_language() const { return m_language; }
    void set_language(Language lang) { m_language = lang; }
    void toggle_language() { m_language = (m_language == Language::EN) ? Language::ZH : Language::EN; }
    std::string loc(const std::string& key) const { return L10n::get(key, m_language); }

private:
    AssetManager();
    ~AssetManager() = default;

    // 類別成員變數加 m_ 前綴，符合規範
    Color m_color_bg;             // 主背景深灰藍 (#0f172a)
    Color m_color_panel;          // UI面板背景 (#1e293b)
    Color m_color_grid_light;     // 棋盤亮色格 (#334155)
    Color m_color_grid_dark;      // 棋盤暗色格 (#1e293b)
    Color m_color_hero_a;         // 英雄 A 幻紫 (#8b5cf6)
    Color m_color_hero_b;         // 英雄 B 霓青 (#06b6d4)
    Color m_color_enemy;          // 敵方怪物玫紅 (#f43f5e)
    Color m_color_text_primary;   // 主要文字白 (#f8fafc)
    Color m_color_text_secondary; // 次要文字灰 (#cbd5e1)
    Color m_color_accent_gold;    // 金色強調色 (#f59e0b)
    Color m_color_hp;             // 生命值紅 (#ef4444)
    Color m_color_ap;             // 行動點綠 (#10b981)
    Color m_color_xp;             // 經驗值橘藍 (#3b82f6)

    Font m_font;                  // 載入的繁體中文字型

    // 互動音效成員
    Sound m_sound_click;
    Sound m_sound_move;
    Sound m_sound_attack;
    Sound m_sound_hit;
    Sound m_sound_death;
    Sound m_sound_promotion;
    Sound m_sound_turn_player;
    Sound m_sound_turn_enemy;
    Sound m_sound_victory;
    Sound m_sound_defeat;

    Language m_language;          // 語言設定 (EN/ZH)
};
