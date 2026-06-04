#pragma once

#include <algorithm>

// 1920×1080 的 80%
constexpr int kScreenW = 1536;
constexpr int kScreenH = 864;

constexpr float kLayoutScaleX = static_cast<float>(kScreenW) / 1280.0f;
constexpr float kLayoutScaleY = static_cast<float>(kScreenH) / 720.0f;
constexpr float kFontScale = 1.36f;

inline int sx(int v) { return static_cast<int>(v * kLayoutScaleX); }
inline int sy(int v) { return static_cast<int>(v * kLayoutScaleY); }
inline float fs(float size) { return size * kFontScale; }

inline int hudPanelX() { return sx(960); }
inline int hudPanelPad() { return sx(8); }
inline int hudColGap() { return sx(10); }
inline int hudPanelW() { return kScreenW - hudPanelX() - sx(12); }
inline int hudColW() { return (hudPanelW() - hudColGap() - hudPanelPad() * 2) / 2; }
inline float hudColLeft(int colIndex) {
    return static_cast<float>(hudPanelX() + hudPanelPad() +
                              colIndex * (hudColW() + hudColGap()));
}
inline int heroSectionY() { return sy(138); }

// 與 GameUI::draw_hero_panel 手牌列表起始 Y 對齊（供 Game.cpp 點擊判定）
inline int heroHandCardsY(int sectionY) {
    return sectionY + sy(230);
}

inline int boardCellSize() {
    const int maxW = (hudPanelX() - sx(40) - sx(24)) / 8;
    const int maxH = (kScreenH - sy(78) - sy(24)) / 8;
    return std::min(maxW, maxH);
}
inline int boardStartX() { return sx(40); }
inline int boardStartY() {
    const int boardPx = boardCellSize() * 8;
    const int top = sy(78);
    const int bottom = kScreenH - sy(24);
    return top + (bottom - top - boardPx) / 2;
}
