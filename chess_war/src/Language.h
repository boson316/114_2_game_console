// Language.h
#pragma once
#include <unordered_map>
#include <string>

enum class Language { EN, ZH };

class L10n {
public:
    static const std::unordered_map<std::string, std::pair<std::string,std::string>>& table() {
        static const std::unordered_map<std::string, std::pair<std::string,std::string>> dict = {
            // key                EN (first)                ZH (second)
            {"title",            {"Chess War",            "棋境戰爭"}},
            {"kill_progress",    {"Kills: ",              "擊殺進度："}},
            {"turn",             {"Turn: ",               "回合："}},
            {"phase_title",      {"Phase: ",              "階段："}},
            {"phase_player",     {"Player Turn",          "玩家回合"}},
            {"phase_enemy",      {"Enemy Turn",           "敵方回合"}},
            {"hero_a",           {"Hero A (Purple)",      "英雄 A (紫)"}},
            {"hero_b",           {"Hero B (Cyan)",        "英雄 B (青)"}},
            {"hp",               {"HP: ",                 "血量："}},
            {"ap",               {"AP: ",                 "行動點："}},
            {"xp",               {"XP: ",                 "經驗值："}},
            {"draw",             {"Draw: ",               "抽:"}},
            {"discard",          {"Discard: ",            "棄:"}},
            {"rules",            {"Rules: ",              "解鎖規則："}},
            {"no_rules",         {"None",                 "無"}},
            {"hand",             {"Hand:",                "手牌列表："}},
            {"card_consumes",    {"(Cost: 1 AP)",         "(消耗: 1 AP)"}},
            {"space_end",        {"SPACE: End Turn",      "SPACE: 結束回合"}},
            {"ctrl_tip",         {"Right-click cancel | ESC quit", "右鍵取消選擇 | ESC 鍵退出"}},
            {"promotion",        {"Promotion!",           "英雄升變！"}},
            {"promotion_desc",   {"Choose one of three upgrades to strengthen your hero:",
                                   "選擇以下三個升級之一來強化您的英雄："}},
            {"victory",          {"Victory!",             "戰役勝利"}},
            {"defeat",           {"Defeat!",              "全軍覆沒"}},
            {"kill_total",       {"Total Kills: ",        "累計擊殺怪物數量："}},
            {"restart_hint",     {"Press [R] to Restart | [ESC] to Exit",
                                   "按下 [R] 鍵重新開始 | [ESC] 鍵退出"}},
            
            // Chess Pieces
            {"piece_none",       {"None",                 "無"}},
            {"piece_pawn",       {"Pawn",                 "士兵"}},
            {"piece_rook",       {"Rook",                 "城堡"}},
            {"piece_knight",     {"Knight",               "騎士"}},
            {"piece_bishop",     {"Bishop",               "主教"}},
            {"piece_queen",      {"Queen",                "皇后"}},
            {"piece_king",       {"King",                 "國王"}},
            {"piece_unknown",    {"Unknown",              "未知"}},

            // Card Names
            {"card_pawn",        {"Pawn",                 "士兵"}},
            {"card_knight",      {"Knight",               "騎士"}},
            {"card_bishop",      {"Bishop",               "主教"}},
            {"card_rook",        {"Rook",                 "城堡"}},
            {"card_king",        {"King",                 "國王"}},
            {"card_queen",       {"Queen",                "皇后"}},
            {"card_temp_queen",  {"Temp Queen",           "臨時皇后"}},

            // Rule Breakers
            {"rule_pawn",        {" Pawn Back",           " 兵後退"}},
            {"rule_knight",      {" Knight Jump",         " 馬越步"}},
            {"rule_bishop",      {" Bishop Pierce",       " 象穿透"}},

            // UI prompts
            {"aiming_move",      {"Click green cell to [MOVE]", "點擊綠色格子進行 [移動]"}},
            {"aiming_attack",    {"Click red cell to [ATTACK]", "點擊紅色格子進行 [攻擊]"}},
            {"state_title",      {"Game State",           "遊戲狀態"}},
            {"select_action",    {"Select",               "選擇"}},
            {"current_identity", {"Identity: ",           "當前身份："}},

            // Floating Texts
            {"minus_1_hp",       {" -1 HP",               " -1 HP"}},
            {"plus_1_ap",        {" +1 AP",               " +1 AP"}},
            {"plus_1_xp",        {" +1 XP",               " +1 XP"}},
            {"backline_promo",   {"Promoted!",            "底線升變！"}},
            {"killed_text",      {"KILLED!",              "已擊殺！"}},

            // Promotion Cards Details
            {"promo_dmg_title",  {"+1 Attack Damage",     "+1 攻擊力"}},
            {"promo_dmg_desc",   {"Increases hero's\nbase damage by 1.", "增加英雄的\n基礎傷害 1 點。"}},
            {"promo_hp_title",   {"+1 Max HP",            "+1 最大生命值"}},
            {"promo_hp_desc",    {"Increases Max HP by 1\nand heals 1 HP.", "增加最大生命值 1 點\n並治療 1 點生命值。"}},
            {"promo_pawn_title", {"Pawn Backwards",       "士兵後退"}},
            {"promo_pawn_desc",  {"Pawn cards can now\nmove/attack backward.", "士兵卡牌現在可以\n後退移動與攻擊。"}},
            {"promo_knight_title",{"Knight Leap",          "騎士躍擊"}},
            {"promo_knight_desc",{"Knight cards can now\nperform 3x1 leap.", "騎士卡牌現在可以\n進行 3x1 距離跳躍。"}},
            {"promo_bishop_title",{"Bishop Pierce",        "主教穿透"}},
            {"promo_bishop_desc",{"Bishop attacks can now\npierce board obstacles.", "主教攻擊現在可以\n穿透棋盤障礙物。"}},

            // Game Over Description Details
            {"desc_victory",     {"You have successfully defended the Chess Kingdom!", "您已成功捍衛了西洋棋王國！"}},
            {"desc_defeat",      {"Both heroes have fallen. The Chess Kingdom collapsed...", "兩位英雄皆已陣亡。西洋棋王國瓦解了..."}},
            {"pick_action",      {"Please choose:", "請選擇："}},
            {"btn_restart",      {"Play Again", "再玩一局"}},
            {"btn_main_menu",    {"Main Menu", "返回主選單"}},

            // Turn System Banner Subtitles
            {"banner_sub_player",{"AP reset! Hand discarded & 4 cards drawn...", "行動點重置！手牌已棄置並各抽 4 張牌..."}},
            {"banner_sub_enemy", {"Monsters are marching and attacking...", "怪物正在行軍與發動攻擊中..."}}
        };
        return dict;
    }

    static std::string get(const std::string& key, Language lang) {
        const auto& it = table().find(key);
        if (it == table().end()) return key;               // fallback
        return (lang == Language::EN) ? it->second.first : it->second.second;
    }
};
