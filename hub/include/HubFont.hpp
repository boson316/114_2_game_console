#pragma once

#include "raylib.h"

struct HubFont {
    Font font{};
    bool loaded = false;
    bool isSans = false;
};

bool loadHubUiFont(HubFont& out);
void unloadHubUiFont(HubFont& font);
int hubMeasureTextWidth(const HubFont& font, const char* text, int fontSize);
void hubDrawText(const HubFont& font, const char* text, int x, int y, int fontSize, Color color);
void hubDrawTextCentered(const HubFont& font, const char* text, Rectangle rect, int fontSize,
                         Color color, float yRatio = 0.38f);
