#include "GfxAssets.hpp"
#include "UiStrings.hpp"

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

#ifndef ASSETS_DIR
#define ASSETS_DIR "assets"
#endif

namespace {
// UI atlas 基準像素（黑體縮放後筆畫較細，略大一點較清楚）。
constexpr int kUiFontBaseSize = 96;
constexpr float kUiFontSpacingScale = 0.045f;

float uiFontSpacing(int fontSize, float spacingMul = 1.0f) {
    return std::max(2.0f, static_cast<float>(fontSize) * kUiFontSpacingScale * spacingMul);
}

Texture2D loadTexSized(const char* file, int targetW, int targetH) {
    const std::string path = gfxAssetPath(file);
    if (!FileExists(path.c_str())) {
        return {};
    }
    Image img = LoadImage(path.c_str());
    if (img.data == nullptr) {
        return {};
    }
    if (img.width != targetW || img.height != targetH) {
        ImageResize(&img, targetW, targetH);
    }
    Texture2D tex = LoadTextureFromImage(img);
    UnloadImage(img);
    SetTextureFilter(tex, TEXTURE_FILTER_POINT);
    return tex;
}

Texture2D loadTexTile(const char* file) { return loadTexSized(file, 64, 64); }

Texture2D loadTexAny(const char* file) {
    const std::string path = gfxAssetPath(file);
    if (!FileExists(path.c_str())) {
        return {};
    }
    Image img = LoadImage(path.c_str());
    if (img.data == nullptr) {
        return {};
    }
    Texture2D tex = LoadTextureFromImage(img);
    UnloadImage(img);
    SetTextureFilter(tex, TEXTURE_FILTER_POINT);
    return tex;
}

Texture2D loadTexTitle(const char* file) {
    const std::string path = gfxAssetPath(file);
    if (!FileExists(path.c_str())) {
        return {};
    }
    Image img = LoadImage(path.c_str());
    if (img.data == nullptr) {
        return {};
    }
    constexpr int targetW = 480;
    const int targetH = std::max(1, static_cast<int>(img.height) * targetW / std::max(1, img.width));
    ImageResize(&img, targetW, targetH);
    Texture2D tex = LoadTextureFromImage(img);
    UnloadImage(img);
    SetTextureFilter(tex, TEXTURE_FILTER_POINT);
    return tex;
}

SpriteSheet loadSpriteSheet(const char* file, int frameW, int frameH, int frameCount) {
    SpriteSheet sheet{};
    sheet.frameWidth = frameW;
    sheet.frameHeight = frameH;
    sheet.frameCount = frameCount;

    const std::string path = gfxAssetPath(file);
    if (!FileExists(path.c_str())) {
        return sheet;
    }
    Image img = LoadImage(path.c_str());
    if (img.data == nullptr) {
        return sheet;
    }
    sheet.columns = std::max(1, img.width / frameW);
    sheet.texture = LoadTextureFromImage(img);
    UnloadImage(img);
    SetTextureFilter(sheet.texture, TEXTURE_FILTER_POINT);
    return sheet;
}

Texture2D makeFallback(int w, int h, Color fill, Color border) {
    Image img = GenImageColor(w, h, fill);
    ImageDrawRectangleLines(&img, {0.0f, 0.0f, static_cast<float>(w), static_cast<float>(h)}, 2,
                            border);
    Texture2D tex = LoadTextureFromImage(img);
    UnloadImage(img);
    SetTextureFilter(tex, TEXTURE_FILTER_POINT);
    return tex;
}

Sound makeTone(float hz, float seconds) {
    const int sampleRate = 44100;
    const int frameCount = static_cast<int>(sampleRate * seconds);
    auto* data = static_cast<float*>(MemAlloc(static_cast<unsigned int>(frameCount) * sizeof(float)));
    for (int i = 0; i < frameCount; ++i) {
        const float t = static_cast<float>(i) / static_cast<float>(sampleRate);
        const float env = 1.0f - static_cast<float>(i) / static_cast<float>(frameCount);
        data[i] = std::sin(2.0f * PI * hz * t) * env * 0.35f;
    }
    Wave wave{};
    wave.frameCount = frameCount;
    wave.sampleRate = sampleRate;
    wave.sampleSize = 32;
    wave.channels = 1;
    wave.data = data;
    Sound snd = LoadSoundFromWave(wave);
    MemFree(data);
    SetSoundVolume(snd, 0.4f);
    return snd;
}

Texture2D firstPathTile(const char* const* paths, int count) {
    for (int i = 0; i < count; ++i) {
        Texture2D tex = loadTexTile(paths[i]);
        if (tex.id != 0) {
            return tex;
        }
    }
    return {};
}
}  // namespace

Rectangle SpriteSheet::frameRect(int index) const {
    const int col = index % columns;
    const int row = index / columns;
    return {static_cast<float>(col * frameWidth), static_cast<float>(row * frameHeight),
            static_cast<float>(frameWidth), static_cast<float>(frameHeight)};
}

void drawSpriteFrame(const SpriteSheet& sheet, int frameIndex, int cx, int cy, float maxSize,
                     float rotation) {
    if (!sheet.valid()) {
        return;
    }
    const int idx = frameIndex % std::max(1, sheet.frameCount);
    const Rectangle src = sheet.frameRect(idx);
    const float scale = maxSize / static_cast<float>(std::max(sheet.frameWidth, sheet.frameHeight));
    const float w = static_cast<float>(sheet.frameWidth) * scale;
    const float h = static_cast<float>(sheet.frameHeight) * scale;
    const Rectangle dst{cx - w / 2.0f, cy - h / 2.0f, w, h};
    DrawTexturePro(sheet.texture, src, dst, {w / 2.0f, h / 2.0f}, rotation, WHITE);
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

std::vector<int> buildUiCodepoints() {
    std::vector<int> codepoints;
    for (int cp = 32; cp < 127; ++cp) {
        codepoints.push_back(cp);
    }
    for (int i = 0; i < UiStrings::kCodepointCount; ++i) {
        codepoints.push_back(UiStrings::kCodepoints[i]);
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

bool isCustomLoadedFont(const Font& font) {
    if (font.texture.id == 0) {
        return false;
    }
    const Font def = GetFontDefault();
    return def.texture.id == 0 || font.texture.id != def.texture.id;
}

bool fontSupportsUi(const Font& font) {
    constexpr int kProbe[] = {0x958B, 0x7C21, 0x904A};  // 開 簡 遊
    for (int cp : kProbe) {
        if (!uiFontHasGlyph(font, cp)) {
            return false;
        }
    }
    return true;
}

bool pathLooksLikeSans(const std::string& path) {
    return path.find("Taipei") != std::string::npos || path.find("NotoSans") != std::string::npos;
}

bool loadUiFont(Font& uiFont, bool& hasUiFont, bool& uiFontIsSans) {
    static std::vector<int> codepoints = buildUiCodepoints();
    uiFontIsSans = false;

    const std::string fontDir = gfxAssetPath("fonts");
    const std::string kaiuBundled = fontDir + "/kaiu.ttf";

    std::vector<std::string> candidates;
    auto tryAdd = [&](const std::string& path) {
        if (!path.empty() && FileExists(path.c_str())) {
            candidates.push_back(path);
        }
    };

    // 台北黑體（OFL）— 與參考圖第二行相同風格
    const char* taipeiNames[] = {"TaipeiSansTCBeta-Regular.ttf", "TaipeiSansTC-Regular.ttf",
                                 nullptr};
    for (const char** name = taipeiNames; *name != nullptr; ++name) {
        tryAdd(fontDir + "/" + *name);
    }
    tryAdd("C:/Windows/Fonts/TaipeiSansTCBeta-Regular.ttf");
    tryAdd("C:/Windows/Fonts/TaipeiSansTC-Regular.ttf");

    // 未安裝台北黑體時：Windows 內建 Noto Sans TC（接近思源黑體，仍為黑體）
    tryAdd("C:/Windows/Fonts/NotoSansTC-Regular.otf");
    tryAdd("C:/Windows/Fonts/NotoSansTC-Medium.otf");

    // 最後才退回標楷（勿用 .ttc）
    tryAdd("C:/Windows/Fonts/DFKai-SB.ttf");
    tryAdd(kaiuBundled);
    tryAdd("C:/Windows/Fonts/kaiu.ttf");

    for (const std::string& path : candidates) {
        Font candidate = LoadFontEx(path.c_str(), kUiFontBaseSize, codepoints.data(),
                                    static_cast<int>(codepoints.size()));
        if (fontSupportsUi(candidate)) {
            uiFont = candidate;
            SetTextureFilter(uiFont.texture, TEXTURE_FILTER_POINT);
            hasUiFont = true;
            uiFontIsSans = pathLooksLikeSans(path);
            TraceLog(LOG_INFO, "FONT: UI loaded from %s (sans=%d)", path.c_str(),
                     uiFontIsSans ? 1 : 0);
            return true;
        }
        if (isCustomLoadedFont(candidate)) {
            UnloadFont(candidate);
        }
    }
    TraceLog(LOG_WARNING, "FONT: No UI font; run: python scripts/fetch_ui_font.py");
    hasUiFont = false;
    return false;
}

int measureUiText(const GfxAssets& assets, const char* text, int fontSize, float spacingMul) {
    if (assets.hasUiFont) {
        return static_cast<int>(
            MeasureTextEx(assets.uiFont, text, static_cast<float>(fontSize),
                          uiFontSpacing(fontSize, spacingMul))
                .x);
    }
    return MeasureText(text, fontSize);
}

Vector2 measureUiTextSize(const GfxAssets& assets, const char* text, int fontSize,
                          float spacingMul) {
    if (assets.hasUiFont) {
        return MeasureTextEx(assets.uiFont, text, static_cast<float>(fontSize),
                             uiFontSpacing(fontSize, spacingMul));
    }
    return {static_cast<float>(MeasureText(text, fontSize)),
            static_cast<float>(fontSize)};
}

void drawUiText(const GfxAssets& assets, const char* text, int x, int y, int fontSize, Color color,
                float spacingMul) {
    if (assets.hasUiFont) {
        const float spacing = uiFontSpacing(fontSize, spacingMul);
        const float px = std::round(static_cast<float>(x));
        const float py = std::round(static_cast<float>(y));
        DrawTextEx(assets.uiFont, text, {px, py}, static_cast<float>(fontSize), spacing, color);
        return;
    }
    DrawText(text, x, y, fontSize, color);
}

void drawUiTextMedium(const GfxAssets& assets, const char* text, int x, int y, int fontSize,
                      Color color) {
    if (!assets.hasUiFont) {
        DrawText(text, x, y, fontSize, color);
        return;
    }
    drawUiText(assets, text, x, y, fontSize, color);
    if (assets.uiFontIsSans) {
        return;
    }
    Color stroke = color;
    stroke.a = static_cast<unsigned char>(std::min(255, static_cast<int>(color.a) * 45 / 100));
    drawUiText(assets, text, x + 1, y, fontSize, stroke);
}

void drawUiTextBold(const GfxAssets& assets, const char* text, int x, int y, int fontSize,
                    Color color) {
    if (!assets.hasUiFont) {
        DrawText(text, x, y, fontSize, color);
        return;
    }
    // 正楷清晰優先：僅 1px 淡陰影（白字／淺底用），不描邊、不疊字。
    const bool lightFill =
        color.r > 200 && color.g > 200 && color.b > 180 && color.a > 200;
    if (lightFill) {
        const Color shadow{40, 30, 20, 90};
        drawUiText(assets, text, x + 1, y + 1, fontSize, shadow);
    }
    drawUiText(assets, text, x, y, fontSize, color);
}

void drawUiTextInRect(const GfxAssets& assets, const char* text, Rectangle rect, int fontSize,
                      Color color, bool bold, float spacingMul) {
    const Vector2 sz = measureUiTextSize(assets, text, fontSize, spacingMul);
    const int x = static_cast<int>(std::round(rect.x + (rect.width - sz.x) * 0.5f));
    const int y = static_cast<int>(std::round(rect.y + (rect.height - sz.y) * 0.5f));
    if (bold) {
        drawUiTextBold(assets, text, x, y, fontSize, color);
    } else {
        if (assets.hasUiFont) {
            drawUiText(assets, text, x, y, fontSize, color, spacingMul);
        } else {
            drawUiTextMedium(assets, text, x, y, fontSize, color);
        }
    }
}

std::string gfxAssetPath(const char* relativePath) {
    const std::string baked = std::string(ASSETS_DIR) + "/" + relativePath;
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

const Texture2D& GfxAssets::textureForPowerUp(PowerUpType type) const {
    switch (type) {
        case PowerUpType::FIRE_UP:
            return itemFire.id != 0 ? itemFire : itemBomb;
        case PowerUpType::SPEED_UP:
            return itemSpeed.id != 0 ? itemSpeed : itemBomb;
        case PowerUpType::BOMB_UP:
        default:
            return itemBomb;
    }
}

bool GfxAssets::load() {
    if (loaded) {
        return true;
    }

    InitAudioDevice();

    const char* floorPaths[] = {"sprites/tiles/floor.png", "sprites/floor.png", nullptr};
    const char* metalPaths[] = {"sprites/tiles/wall_metal.png", "sprites/wall_metal.png", nullptr};
    const char* brickPaths[] = {"sprites/tiles/wall_brick.png", "sprites/wall_brick.png", nullptr};

    floor = firstPathTile(floorPaths, 2);
    wallMetal = firstPathTile(metalPaths, 2);
    wallBrick = firstPathTile(brickPaths, 2);
    wallBush = loadTexTile("sprites/tiles/wall_bush.png");
    enemy = loadTexTile("sprites/actors/enemy.png");
    if (enemy.id == 0) {
        enemy = loadTexTile("sprites/enemy.png");
    }
    bomb = loadTexTile("sprites/bomb.png");
    itemBomb = loadTexTile("sprites/items/item_bomb.png");
    itemFire = loadTexTile("sprites/items/item_fire.png");
    itemSpeed = loadTexTile("sprites/items/item_speed.png");
    uiButton = loadTexAny("sprites/ui/button.png");
    uiPanel = loadTexAny("sprites/ui/panel.png");
    menuTitle = loadTexTitle("sprites/ui/menu_title.png");
    if (menuTitle.id == 0) {
        menuTitle = loadTexTitle("sprites/menu_title.png");
    }

    playerSheet = loadSpriteSheet("sprites/actors/player_sheet.png", 64, 64, 8);
    player2Sheet = loadSpriteSheet("sprites/actors/player2_sheet.png", 64, 64, 8);
    if (!player2Sheet.valid()) {
        player2Sheet = playerSheet;
    }
    explosionSheet = loadSpriteSheet("sprites/fx/explosion_sheet.png", 64, 64, 4);
    if (!explosionSheet.valid()) {
        explosionSheet = loadSpriteSheet("sprites/explosion.png", 64, 64, 1);
    }

    if (floor.id == 0) {
        floor = makeFallback(64, 64, {55, 120, 55, 255}, {40, 90, 40, 255});
    }
    if (wallMetal.id == 0) {
        wallMetal = makeFallback(64, 64, {120, 125, 140, 255}, {80, 85, 100, 255});
    }
    if (wallBrick.id == 0) {
        wallBrick = makeFallback(64, 64, {200, 130, 70, 255}, {140, 85, 45, 255});
    }
    if (enemy.id == 0) {
        enemy = makeFallback(64, 64, {240, 70, 70, 255}, {160, 40, 40, 255});
    }
    if (bomb.id == 0) {
        bomb = makeFallback(64, 64, {40, 40, 40, 255}, {255, 200, 0, 255});
    }
    if (itemBomb.id == 0) {
        itemBomb = makeFallback(64, 64, {255, 80, 80, 255}, {180, 40, 40, 255});
    }
    if (itemFire.id == 0) {
        itemFire = makeFallback(64, 64, {255, 140, 40, 255}, {200, 90, 20, 255});
    }
    if (itemSpeed.id == 0) {
        itemSpeed = makeFallback(64, 64, {80, 200, 255, 255}, {40, 120, 200, 255});
    }
    if (!playerSheet.valid()) {
        playerSheet.texture = makeFallback(64, 64, {70, 150, 255, 255}, {40, 90, 180, 255});
        playerSheet.frameCount = 1;
        playerSheet.columns = 1;
    }
    if (!explosionSheet.valid()) {
        explosionSheet.texture = makeFallback(64, 64, {255, 180, 40, 255}, {255, 80, 20, 255});
        explosionSheet.frameCount = 4;
        explosionSheet.columns = 1;
    }

    sfxPlace = makeTone(720.0f, 0.07f);
    sfxExplosion = makeTone(140.0f, 0.25f);
    SetSoundVolume(sfxExplosion, 0.5f);
    sfxMenu = makeTone(520.0f, 0.05f);
    sfxVictory = makeTone(880.0f, 0.12f);
    sfxGameOver = makeTone(180.0f, 0.35f);
    SetSoundVolume(sfxGameOver, 0.45f);
    sfxPickup = makeTone(660.0f, 0.1f);
    SetSoundVolume(sfxPickup, 0.45f);

    loadUiFont(uiFont, hasUiFont, uiFontIsSans);

    loaded = true;
    return true;
}

void GfxAssets::unload() {
    if (!loaded) {
        return;
    }
    loaded = false;

    const Texture2D textures[] = {floor,     wallMetal, wallBrick, wallBush, enemy, bomb,
                                  menuTitle, uiButton,  uiPanel,   itemBomb, itemFire, itemSpeed};
    for (const Texture2D& tex : textures) {
        if (tex.id != 0) {
            UnloadTexture(tex);
        }
    }
    if (playerSheet.valid()) {
        UnloadTexture(playerSheet.texture);
        playerSheet = {};
    }
    if (explosionSheet.valid()) {
        UnloadTexture(explosionSheet.texture);
        explosionSheet = {};
    }

    UnloadSound(sfxPlace);
    UnloadSound(sfxExplosion);
    UnloadSound(sfxMenu);
    UnloadSound(sfxVictory);
    UnloadSound(sfxGameOver);
    UnloadSound(sfxPickup);
    if (hasUiFont) {
        UnloadFont(uiFont);
        hasUiFont = false;
        uiFontIsSans = false;
    }
    CloseAudioDevice();
}
