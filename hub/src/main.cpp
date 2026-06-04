#include "HubFont.hpp"
#include "GameLaunch.hpp"

#include "raylib.h"

GameLaunchResult runChessWarGame();
GameLaunchResult runBombermanGame();

namespace {

constexpr int kWinW = 1536;
constexpr int kWinH = 864;

struct MenuButton {
    const char* title;
    const char* subtitle;
    Color fill;
    Color hover;
};

bool pointInRect(Vector2 p, Rectangle r) {
    return p.x >= r.x && p.x <= r.x + r.width && p.y >= r.y && p.y <= r.y + r.height;
}

void drawButton(const HubFont& font, const MenuButton& btn, Rectangle r, bool hovered) {
    DrawRectangleRounded(r, 0.15f, 8, hovered ? btn.hover : btn.fill);
    DrawRectangleRoundedLines(r, 0.15f, 8, Color{255, 255, 255, 40});
    hubDrawTextCentered(font, btn.title, r, 42, RAYWHITE, 0.20f);
    hubDrawTextCentered(font, btn.subtitle, r, 26, Color{230, 230, 230, 220}, 0.64f);
}

bool anyMenuKeyDown() {
    return IsKeyDown(KEY_ONE) || IsKeyDown(KEY_TWO) || IsKeyDown(KEY_ESCAPE);
}

void waitForKeysReleased() {
    for (int i = 0; i < 120 && anyMenuKeyDown(); ++i) {
        BeginDrawing();
        ClearBackground(Color{28, 32, 44, 255});
        EndDrawing();
    }
}

int runHubMenu() {
    SetConfigFlags(FLAG_VSYNC_HINT);
    InitWindow(kWinW, kWinH, "114-2 遊戲主控台");
    SetTargetFPS(60);

    HubFont font{};
    loadHubUiFont(font);
    waitForKeysReleased();

    const MenuButton buttons[] = {
        {"西洋棋戰爭", "卡牌 + 西洋棋規則", {58, 92, 168, 255}, {78, 118, 210, 255}},
        {"爆爆王", "炸彈 + BFS AI", {168, 72, 48, 255}, {210, 98, 58, 255}},
    };

    int selected = -1;
    bool keysReady = false;

    while (!WindowShouldClose() && selected < 0) {
        if (!keysReady) {
            keysReady = !anyMenuKeyDown();
        } else {
            int key = GetKeyPressed();
            while (key != 0) {
                if (key == KEY_ESCAPE) {
                    unloadHubUiFont(font);
                    CloseWindow();
                    return -1;
                }
                key = GetKeyPressed();
            }
            if (IsKeyPressed(KEY_ESCAPE)) {
                unloadHubUiFont(font);
                CloseWindow();
                return -1;
            }
        }

        const Vector2 mouse = GetMousePosition();
        const float btnW = 720.0f;
        const float btnH = 144.0f;
        const float btnGap = 56.0f;
        const float x = (kWinW - btnW) / 2.0f;
        const float y0 = 230.0f;
        Rectangle r0{x, y0, btnW, btnH};
        Rectangle r1{x, y0 + btnH + btnGap, btnW, btnH};
        const bool h0 = pointInRect(mouse, r0);
        const bool h1 = pointInRect(mouse, r1);

        if (keysReady) {
            if (IsKeyPressed(KEY_ONE) || (h0 && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))) {
                selected = 0;
            } else if (IsKeyPressed(KEY_TWO) || (h1 && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))) {
                selected = 1;
            }
        }

        BeginDrawing();
        ClearBackground(Color{28, 32, 44, 255});
        hubDrawText(font, "114-2 遊戲主選單", 48, 48, 52, RAYWHITE);
        hubDrawText(font, "選擇遊戲  |  1: 西洋棋  2: 爆爆王  |  Esc: 離開", 48, 108, 28,
                    Color{180, 190, 210, 255});
        if (!font.loaded) {
            hubDrawText(font, "（請執行 bomberman/scripts/fetch_ui_font.py）", 48, 142, 22,
                        Color{220, 160, 80, 255});
        }
        drawButton(font, buttons[0], r0, h0);
        drawButton(font, buttons[1], r1, h1);
        hubDrawText(font, "Chess War + Bomberman (Raylib)", 48, kWinH - 52, 24,
                    Color{140, 150, 170, 255});
        EndDrawing();
    }

    unloadHubUiFont(font);
    CloseWindow();

    return selected;
}

}  // namespace

int main() {
    for (;;) {
        const int pick = runHubMenu();
        if (pick < 0) {
            return 0;
        }
        const GameLaunchResult result =
            (pick == 0) ? runChessWarGame() : runBombermanGame();
        if (result == GameLaunchResult::ExitApp) {
            return 0;
        }
    }
}
