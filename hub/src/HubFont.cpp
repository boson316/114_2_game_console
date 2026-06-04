#include "HubFont.hpp"

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

#ifndef HUB_ASSETS_DIR
#define HUB_ASSETS_DIR "assets"
#endif

namespace {

constexpr int kUiFontBaseSize = 96;
constexpr float kUiFontSpacingScale = 0.045f;

float uiFontSpacing(int fontSize) {
    return std::max(2.0f, static_cast<float>(fontSize) * kUiFontSpacingScale);
}

std::string hubAssetPath(const char* relativePath) {
    const std::string baked = std::string(HUB_ASSETS_DIR) + "/" + relativePath;
    if (FileExists(baked.c_str())) {
        return baked;
    }
    const std::string besideExe =
        std::string(GetApplicationDirectory()) + "assets/" + relativePath;
    if (FileExists(besideExe.c_str())) {
        return besideExe;
    }
    return baked;
}

void appendUtf8Codepoints(const char* text, std::vector<int>& codepoints) {
    const unsigned char* s = reinterpret_cast<const unsigned char*>(text);
    while (*s != 0) {
        int cp = 0;
        if (*s <= 0x7F) {
            cp = *s++;
        } else if ((*s & 0xE0) == 0xC0) {
            cp = ((*s & 0x1F) << 6) | (s[1] & 0x3F);
            s += 2;
        } else if ((*s & 0xF0) == 0xE0) {
            cp = ((*s & 0x0F) << 12) | ((s[1] & 0x3F) << 6) | (s[2] & 0x3F);
            s += 3;
        } else if ((*s & 0xF8) == 0xF0) {
            cp = ((*s & 0x07) << 18) | ((s[1] & 0x3F) << 12) | ((s[2] & 0x3F) << 6) |
                 (s[3] & 0x3F);
            s += 4;
        } else {
            ++s;
            continue;
        }
        codepoints.push_back(cp);
    }
}

std::vector<int> buildHubCodepoints() {
    std::vector<int> codepoints;
    for (int cp = 32; cp < 127; ++cp) {
        codepoints.push_back(cp);
    }
    const char* texts[] = {
        "114-2 遊戲主選單",
        "選擇遊戲  |  1: 西洋棋  2: 爆爆王  |  Esc: 離開",
        "西洋棋戰爭",
        "卡牌 + 西洋棋規則",
        "爆爆王",
        "炸彈 + BFS AI",
        "Chess War + Bomberman (Raylib)",
        "（請執行 bomberman/scripts/fetch_ui_font.py）",
        nullptr,
    };
    for (const char** t = texts; *t != nullptr; ++t) {
        appendUtf8Codepoints(*t, codepoints);
    }
    std::sort(codepoints.begin(), codepoints.end());
    codepoints.erase(std::unique(codepoints.begin(), codepoints.end()), codepoints.end());
    return codepoints;
}

bool uiFontHasGlyph(const Font& font, int codepoint) {
    if (font.texture.id == 0) {
        return false;
    }
    const GlyphInfo glyph = GetGlyphInfo(font, codepoint);
    return glyph.image.data != nullptr && glyph.image.width > 0;
}

bool fontSupportsUi(const Font& font) {
    constexpr int kProbe[] = {0x958B, 0x7C21, 0x904A};
    for (int cp : kProbe) {
        if (!uiFontHasGlyph(font, cp)) {
            return false;
        }
    }
    return true;
}

bool isCustomLoadedFont(const Font& font) {
    if (font.texture.id == 0) {
        return false;
    }
    const Font def = GetFontDefault();
    return def.texture.id == 0 || font.texture.id != def.texture.id;
}

bool pathLooksLikeSans(const std::string& path) {
    return path.find("Taipei") != std::string::npos || path.find("NotoSans") != std::string::npos;
}

}  // namespace

bool loadHubUiFont(HubFont& out) {
    static std::vector<int> codepoints = buildHubCodepoints();
    out.loaded = false;
    out.isSans = false;

    const std::string fontDir = hubAssetPath("fonts");
    const std::string kaiuBundled = fontDir + "/kaiu.ttf";

    std::vector<std::string> candidates;
    auto tryAdd = [&](const std::string& path) {
        if (!path.empty() && FileExists(path.c_str())) {
            candidates.push_back(path);
        }
    };

    const char* taipeiNames[] = {"TaipeiSansTCBeta-Regular.ttf", "TaipeiSansTC-Regular.ttf",
                                 nullptr};
    for (const char** name = taipeiNames; *name != nullptr; ++name) {
        tryAdd(fontDir + "/" + *name);
    }
    tryAdd("C:/Windows/Fonts/TaipeiSansTCBeta-Regular.ttf");
    tryAdd("C:/Windows/Fonts/TaipeiSansTC-Regular.ttf");
    tryAdd("C:/Windows/Fonts/NotoSansTC-Regular.otf");
    tryAdd("C:/Windows/Fonts/NotoSansTC-Medium.otf");
    tryAdd("C:/Windows/Fonts/DFKai-SB.ttf");
    tryAdd(kaiuBundled);
    tryAdd("C:/Windows/Fonts/kaiu.ttf");

    for (const std::string& path : candidates) {
        Font candidate = LoadFontEx(path.c_str(), kUiFontBaseSize, codepoints.data(),
                                    static_cast<int>(codepoints.size()));
        if (fontSupportsUi(candidate)) {
            out.font = candidate;
            SetTextureFilter(out.font.texture, TEXTURE_FILTER_POINT);
            out.loaded = true;
            out.isSans = pathLooksLikeSans(path);
            TraceLog(LOG_INFO, "HUB FONT: loaded from %s", path.c_str());
            return true;
        }
        if (isCustomLoadedFont(candidate)) {
            UnloadFont(candidate);
        }
    }
    TraceLog(LOG_WARNING, "HUB FONT: missing; run: cd bomberman && python scripts/fetch_ui_font.py");
    return false;
}

void unloadHubUiFont(HubFont& font) {
    if (font.loaded && isCustomLoadedFont(font.font)) {
        UnloadFont(font.font);
    }
    font.loaded = false;
    font.isSans = false;
}

int hubMeasureTextWidth(const HubFont& font, const char* text, int fontSize) {
    if (font.loaded) {
        return static_cast<int>(
            MeasureTextEx(font.font, text, static_cast<float>(fontSize), uiFontSpacing(fontSize))
                .x);
    }
    return MeasureText(text, fontSize);
}

void hubDrawText(const HubFont& font, const char* text, int x, int y, int fontSize, Color color) {
    if (font.loaded) {
        const float spacing = uiFontSpacing(fontSize);
        DrawTextEx(font.font, text, {std::round(static_cast<float>(x)),
                                     std::round(static_cast<float>(y))},
                   static_cast<float>(fontSize), spacing, color);
        return;
    }
    DrawText(text, x, y, fontSize, color);
}

void hubDrawTextCentered(const HubFont& font, const char* text, Rectangle rect, int fontSize,
                         Color color, float yRatio) {
    const float spacing = font.loaded ? uiFontSpacing(fontSize) : 1.0f;
    Vector2 size =
        font.loaded
            ? MeasureTextEx(font.font, text, static_cast<float>(fontSize), spacing)
            : Vector2{static_cast<float>(MeasureText(text, fontSize)), static_cast<float>(fontSize)};
    const float x = rect.x + (rect.width - size.x) * 0.5f;
    const float y = rect.y + rect.height * yRatio;
    hubDrawText(font, text, static_cast<int>(x), static_cast<int>(y), fontSize, color);
}
