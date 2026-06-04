#pragma once

#include "PowerUpType.hpp"

#include <raylib.h>

#include <string>
#include <vector>

struct SpriteSheet {
    Texture2D texture{};
    int frameWidth = 64;
    int frameHeight = 64;
    int columns = 1;
    int frameCount = 1;

    Rectangle frameRect(int index) const;
    bool valid() const { return texture.id != 0; }
};

struct GfxAssets {
    Texture2D floor{};
    Texture2D wallMetal{};
    Texture2D wallBrick{};
    Texture2D wallBush{};
    Texture2D enemy{};
    Texture2D bomb{};
    Texture2D menuTitle{};
    Texture2D uiButton{};
    Texture2D uiPanel{};

    SpriteSheet playerSheet{};
    SpriteSheet player2Sheet{};
    SpriteSheet explosionSheet{};

    Texture2D itemBomb{};
    Texture2D itemFire{};
    Texture2D itemSpeed{};

    Sound sfxPlace{};
    Sound sfxExplosion{};
    Sound sfxMenu{};
    Sound sfxVictory{};
    Sound sfxGameOver{};
    Sound sfxPickup{};

    Font uiFont{};

    bool loaded = false;
    bool hasUiFont = false;
    /** 台北黑體／思源類黑體：不做假粗描邊 */
    bool uiFontIsSans = false;

    bool load();
    void unload();

    const Texture2D& textureForPowerUp(PowerUpType type) const;
};

int measureUiText(const GfxAssets& assets, const char* text, int fontSize,
                  float spacingMul = 1.0f);
Vector2 measureUiTextSize(const GfxAssets& assets, const char* text, int fontSize,
                          float spacingMul = 1.0f);
void drawUiText(const GfxAssets& assets, const char* text, int x, int y, int fontSize, Color color,
                float spacingMul = 1.0f);
void drawUiTextMedium(const GfxAssets& assets, const char* text, int x, int y, int fontSize,
                      Color color);
void drawUiTextBold(const GfxAssets& assets, const char* text, int x, int y, int fontSize,
                    Color color);
void drawUiTextInRect(const GfxAssets& assets, const char* text, Rectangle rect, int fontSize,
                      Color color, bool bold = true, float spacingMul = 1.0f);
void drawSpriteFrame(const SpriteSheet& sheet, int frameIndex, int cx, int cy, float maxSize,
                     float rotation = 0.0f);

std::string gfxAssetPath(const char* relativePath);
