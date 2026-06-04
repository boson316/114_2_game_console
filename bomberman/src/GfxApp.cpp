#include "GfxApp.hpp"

#include "Bomb.hpp"
#include "Game.hpp"
#include "GfxAssets.hpp"
#include "NetSession.hpp"
#include "PlayMode.hpp"
#include "SaveData.hpp"
#include "UiStrings.hpp"

#include <raylib.h>
#include <raymath.h>

#include <algorithm>
#include <cmath>
#include <unordered_set>

namespace {
constexpr int kCellSize = 70;
constexpr int kCellGap = 4;
constexpr int kCellPitch = kCellSize + kCellGap;
constexpr int kHudHeight = 148;
constexpr int kBattleOverlayTop = 58;
constexpr int kBattleOverlayBottom = 64;
constexpr int kMenuW = 1536;
constexpr int kMenuH = 864;

constexpr char kPickActionEn[] = "Please choose:";
constexpr char kBtnPlayAgainEn[] = "Play Again";
constexpr char kBtnMainMenuEn[] = "Main Menu";

enum class UiScreen { MENU, PLAYING };

enum class Difficulty { EASY, NORMAL, HARD };

struct AnimState {
    Position lastPlayerPos{-1, -1};
    int facing = 0;
    float walkPhase = 0.0f;
    int walkFrame = 0;
    float globalTime = 0.0f;
    float explosionAnim = 0.0f;
    float flashTimer = 0.0f;
};

struct GfxUi {
    GfxAssets assets;
    SaveData save;
    NetSession net;
    bool hubMode = false;
    UiScreen screen = UiScreen::MENU;
    Rectangle endBtnRestart{};
    Rectangle endBtnMenu{};
    Difficulty difficulty = Difficulty::NORMAL;
    PlayMode playMode = PlayMode::SOLO;
    float matchTimeLeft = 180.0f;
    AnimState anim;
    int prevBombCount = 0;
    int prevMaxBombs = 1;
    int prevBlastRadius = 2;
    int prevSpeedLevel = 0;
    int prevScore = 0;
    GameState prevGameState = GameState::INIT;
};

const char* pickActionLabel(const GfxUi& ui) {
    return ui.hubMode ? kPickActionEn : UiStrings::PickAction;
}
const char* playAgainLabel(const GfxUi& ui) {
    return ui.hubMode ? kBtnPlayAgainEn : UiStrings::PlayAgain;
}
const char* mainMenuLabel(const GfxUi& ui) {
    return ui.hubMode ? kBtnMainMenuEn : UiStrings::MainMenu;
}

int uiFontSize(const GfxUi& ui, int base) {
    return static_cast<int>(base * ui.save.uiFontPercent / 100.0f);
}

GameConfig configForDifficulty(Difficulty d) {
    GameConfig cfg;
    switch (d) {
        case Difficulty::EASY:
            cfg.enemyCount = 1;
            cfg.destructibleDensity = 0.38f;
            cfg.powerupDropChance = 0.35f;
            cfg.enemyBombPlaceChance = 0.22f;
            cfg.enemyBombCooldown = 1.8f;
            cfg.enemyMoveInterval = 0.46f;
            cfg.enemyUseRL = true;
            cfg.enemyAggressive = true;
            cfg.enemyEngageRadius = 99;
            break;
        case Difficulty::HARD:
            cfg.enemyCount = 3;
            cfg.destructibleDensity = 0.42f;
            cfg.powerupDropChance = 0.18f;
            cfg.enemyBombPlaceChance = 0.65f;
            cfg.enemyBombCooldown = 0.75f;
            cfg.enemyMoveInterval = 0.30f;
            cfg.enemyBlastRadius = 2;
            cfg.enemyUseRL = true;
            cfg.enemyAggressive = true;
            cfg.enemyEngageRadius = 99;
            break;
        case Difficulty::NORMAL:
        default:
            cfg.enemyCount = 2;
            cfg.destructibleDensity = 0.44f;
            cfg.powerupDropChance = 0.25f;
            cfg.enemyBombPlaceChance = 0.42f;
            cfg.enemyBombCooldown = 0.95f;
            cfg.enemyMoveInterval = 0.36f;
            cfg.enemyBlastRadius = 2;
            cfg.enemyUseRL = true;
            cfg.enemyAggressive = true;
            cfg.enemyEngageRadius = 99;
            break;
    }
    return GameConfig::clamped(cfg);
}

bool cellInList(const std::vector<Position>& cells, Position pos) {
    for (const Position& p : cells) {
        if (p == pos) {
            return true;
        }
    }
    return false;
}

void drawTextureFit(const Texture2D& tex, int cx, int cy, float maxSize, float rotation = 0.0f) {
    if (tex.id == 0) {
        return;
    }
    const float scale = maxSize / static_cast<float>(std::max(tex.width, tex.height));
    const float w = static_cast<float>(tex.width) * scale;
    const float h = static_cast<float>(tex.height) * scale;
    const Rectangle src{0, 0, static_cast<float>(tex.width), static_cast<float>(tex.height)};
    const Rectangle dst{cx - w / 2.0f, cy - h / 2.0f, w, h};
    DrawTexturePro(tex, src, dst, {w / 2.0f, h / 2.0f}, rotation, WHITE);
}

void drawCartoonPanel(Rectangle rect, Color fill, Color border, float roundness = 0.18f) {
    const Rectangle shadow{rect.x + 3.0f, rect.y + 4.0f, rect.width, rect.height};
    DrawRectangleRounded(shadow, roundness, 10, Color{0, 0, 0, 50});
    DrawRectangleRounded(rect, roundness, 10, fill);
    DrawRectangleRoundedLines(rect, roundness, 10, border);
}

void drawMenuBackground(float t) {
    DrawRectangleGradientV(0, 0, kMenuW, kMenuH, Color{168, 215, 255, 255}, Color{72, 118, 210, 255});
    for (int i = 0; i < 18; ++i) {
        const float phase = t * 0.35f + static_cast<float>(i) * 0.9f;
        const float x = std::fmod(static_cast<float>(i) * 137.0f + std::sin(phase) * 40.0f,
                                  static_cast<float>(kMenuW));
        const float y = 40.0f + std::fmod(static_cast<float>(i) * 83.0f + std::cos(phase * 0.8f) * 30.0f,
                                          static_cast<float>(kMenuH - 80));
        const float r = 28.0f + static_cast<float>(i % 5) * 14.0f;
        DrawCircle(static_cast<int>(x), static_cast<int>(y), r, Color{255, 255, 255, 18});
    }
    DrawCircle(kMenuW - 80, 90, 120, Color{255, 200, 120, 25});
    DrawCircle(70, kMenuH - 100, 90, Color{120, 200, 255, 22});
}

void drawMenuCard(Rectangle card) {
    const Rectangle shadow{card.x + 6.0f, card.y + 8.0f, card.width, card.height};
    DrawRectangleRounded(shadow, 0.06f, 16, Color{25, 45, 90, 70});
    DrawRectangleRounded(card, 0.06f, 16, Color{252, 253, 255, 255});
    DrawRectangleRoundedLines(card, 0.06f, 16, Color{210, 220, 235, 255});
}

void drawMenuHeader(const GfxUi& ui, Rectangle card, float t) {
    const Rectangle header{card.x, card.y, card.width, 108.0f};
    DrawRectangleRounded(header, 0.06f, 16, Color{255, 120, 95, 255});
    const Rectangle headerInner{card.x, card.y + 4.0f, card.width, 100.0f};
    DrawRectangleGradientH(static_cast<int>(headerInner.x), static_cast<int>(headerInner.y),
                         static_cast<int>(headerInner.width), static_cast<int>(headerInner.height),
                         Color{255, 175, 90, 255}, Color{255, 105, 130, 255});

    const float bob = std::sin(t * 2.0f) * 4.0f;
    if (ui.assets.enemy.id != 0) {
        drawTextureFit(ui.assets.enemy, static_cast<int>(card.x + 72.0f),
                       static_cast<int>(card.y + 58.0f + bob), 52.0f);
    }
    if (ui.assets.bomb.id != 0) {
        drawTextureFit(ui.assets.bomb, static_cast<int>(card.x + card.width - 72.0f),
                       static_cast<int>(card.y + 58.0f - bob), 44.0f);
    }

    const Rectangle titleBox{card.x + 88.0f, card.y + 18.0f, card.width - 176.0f, 60.0f};
    drawUiTextInRect(ui.assets, UiStrings::TitleMain, titleBox, uiFontSize(ui, 44),
                     Color{255, 255, 255, 255}, true, 1.55f);

    const Rectangle subBox{card.x + 72.0f, card.y + 74.0f, card.width - 144.0f, 30.0f};
    drawUiTextInRect(ui.assets, UiStrings::Subtitle, subBox, uiFontSize(ui, 21),
                     Color{255, 245, 230, 255}, false, 1.2f);
}

void drawSectionLabel(const GfxUi& ui, const char* text, float x, float y, float w) {
    const Rectangle box{x, y, w, 28.0f};
    drawUiTextInRect(ui.assets, text, box, uiFontSize(ui, 20), Color{90, 105, 130, 255}, false);
    DrawLine(static_cast<int>(x), static_cast<int>(y + 30), static_cast<int>(x + w),
             static_cast<int>(y + 30), Color{220, 228, 240, 255});
}

bool drawPillButton(const GfxUi& ui, const char* label, Rectangle rect, bool hovered, bool selected) {
    const Color fill = selected   ? Color{255, 214, 96, 255}
                       : hovered  ? Color{235, 242, 252, 255}
                                  : Color{245, 248, 252, 255};
    const Color border =
        selected ? Color{230, 160, 40, 255} : Color{200, 212, 228, 255};
    const Rectangle shadow{rect.x + 1.0f, rect.y + 2.0f, rect.width, rect.height};
    DrawRectangleRounded(shadow, 0.45f, 8, Color{0, 0, 0, selected ? 35 : 18});
    DrawRectangleRounded(rect, 0.45f, 8, fill);
    DrawRectangleRoundedLines(rect, 0.45f, 8, border);
    const Color textColor = selected ? Color{55, 40, 10, 255} : Color{50, 58, 72, 255};
    drawUiTextInRect(ui.assets, label, rect, uiFontSize(ui, 24), textColor, false);
    return hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

bool drawPrimaryButton(const GfxUi& ui, const char* label, Rectangle rect, bool hovered) {
    const Rectangle shadow{rect.x + 2.0f, rect.y + 5.0f, rect.width, rect.height};
    DrawRectangleRounded(shadow, 0.35f, 10, Color{20, 80, 40, 80});
    const Rectangle body{rect.x, rect.y, rect.width, rect.height};
    const Color top = hovered ? Color{110, 220, 140, 255} : Color{88, 200, 120, 255};
    const Color bot = hovered ? Color{55, 170, 95, 255} : Color{42, 150, 82, 255};
    DrawRectangleRounded(body, 0.35f, 10, bot);
    const Rectangle inner{body.x, body.y, body.width, body.height * 0.55f};
    DrawRectangleRounded(inner, 0.35f, 10, top);
    DrawRectangleRoundedLines(body, 0.35f, 10, Color{30, 110, 60, 255});
    drawUiTextInRect(ui.assets, label, body, uiFontSize(ui, 30), Color{255, 255, 255, 255}, true);
    return hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

bool drawGhostButton(const GfxUi& ui, const char* label, Rectangle rect, bool hovered) {
    const Color fill = hovered ? Color{240, 244, 250, 255} : Color{255, 255, 255, 0};
    if (fill.a > 0) {
        DrawRectangleRounded(rect, 0.4f, 8, fill);
    }
    DrawRectangleRoundedLines(rect, 0.4f, 8, Color{170, 182, 200, 255});
    drawUiTextInRect(ui.assets, label, rect, uiFontSize(ui, 22), Color{80, 92, 110, 255}, false);
    return hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

const char* difficultyDesc(Difficulty d) {
    switch (d) {
        case Difficulty::EASY:
            return UiStrings::EasyDesc;
        case Difficulty::HARD:
            return UiStrings::HardDesc;
        case Difficulty::NORMAL:
        default:
            return UiStrings::NormalDesc;
    }
}

void drawMenu(GfxUi& ui, bool& requestQuit, bool& returnToHub) {
    const float t = ui.anim.globalTime;
    drawMenuBackground(t);

    const Rectangle card{(kMenuW - 1000.0f) / 2.0f, 24.0f, 1000.0f, 708.0f};
    drawMenuCard(card);
    drawMenuHeader(ui, card, t);

    const float pad = card.x + 36.0f;
    const float innerW = card.width - 72.0f;
    const Vector2 mouse = GetMousePosition();

    float y = card.y + 124.0f;
    drawSectionLabel(ui, UiStrings::PickDifficulty, pad, y, innerW);
    y += 40.0f;

    const float pillW = (innerW - 24.0f) / 3.0f;
    const float pillH = 48.0f;
    const Rectangle btnEasy{pad, y, pillW, pillH};
    const Rectangle btnNormal{pad + pillW + 12.0f, y, pillW, pillH};
    const Rectangle btnHard{pad + (pillW + 12.0f) * 2.0f, y, pillW, pillH};
    if (drawPillButton(ui, UiStrings::Easy, btnEasy, CheckCollisionPointRec(mouse, btnEasy),
                       ui.difficulty == Difficulty::EASY)) {
        ui.difficulty = Difficulty::EASY;
        PlaySound(ui.assets.sfxMenu);
    }
    if (drawPillButton(ui, UiStrings::Normal, btnNormal, CheckCollisionPointRec(mouse, btnNormal),
                       ui.difficulty == Difficulty::NORMAL)) {
        ui.difficulty = Difficulty::NORMAL;
        PlaySound(ui.assets.sfxMenu);
    }
    if (drawPillButton(ui, UiStrings::Hard, btnHard, CheckCollisionPointRec(mouse, btnHard),
                       ui.difficulty == Difficulty::HARD)) {
        ui.difficulty = Difficulty::HARD;
        PlaySound(ui.assets.sfxMenu);
    }

    const Rectangle diffHint{pad, y + pillH + 6.0f, innerW, 28.0f};
    drawUiTextInRect(ui.assets, difficultyDesc(ui.difficulty), diffHint, uiFontSize(ui, 18),
                     Color{120, 130, 150, 255}, false);

    y += pillH + 44.0f;
    drawSectionLabel(ui, UiStrings::PickMode, pad, y, innerW);
    y += 40.0f;

    const float modeW = (innerW - 36.0f) / 4.0f;
    const float modeH = 44.0f;
    const Rectangle btnSolo{pad, y, modeW, modeH};
    const Rectangle btnDuo{pad + modeW + 12.0f, y, modeW, modeH};
    const Rectangle btnHost{pad + (modeW + 12.0f) * 2.0f, y, modeW, modeH};
    const Rectangle btnJoin{pad + (modeW + 12.0f) * 3.0f, y, modeW, modeH};
    if (drawPillButton(ui, UiStrings::ModeSolo, btnSolo, CheckCollisionPointRec(mouse, btnSolo),
                       ui.playMode == PlayMode::SOLO)) {
        ui.playMode = PlayMode::SOLO;
        PlaySound(ui.assets.sfxMenu);
    }
    if (drawPillButton(ui, UiStrings::ModeDuo, btnDuo, CheckCollisionPointRec(mouse, btnDuo),
                       ui.playMode == PlayMode::LOCAL_DUO)) {
        ui.playMode = PlayMode::LOCAL_DUO;
        PlaySound(ui.assets.sfxMenu);
    }
    if (drawPillButton(ui, UiStrings::ModeHost, btnHost, CheckCollisionPointRec(mouse, btnHost),
                       ui.playMode == PlayMode::ONLINE_HOST)) {
        ui.playMode = PlayMode::ONLINE_HOST;
        PlaySound(ui.assets.sfxMenu);
    }
    if (drawPillButton(ui, UiStrings::ModeJoin, btnJoin, CheckCollisionPointRec(mouse, btnJoin),
                       ui.playMode == PlayMode::ONLINE_CLIENT)) {
        ui.playMode = PlayMode::ONLINE_CLIENT;
        PlaySound(ui.assets.sfxMenu);
    }

    y += modeH + 28.0f;
    const Rectangle btnStart{pad, y, innerW * 0.58f, 64.0f};
    const Rectangle scoreBox{pad + innerW * 0.62f, y + 6.0f, innerW * 0.38f, 52.0f};
    DrawRectangleRounded(scoreBox, 0.25f, 8, Color{245, 248, 255, 255});
    DrawRectangleRoundedLines(scoreBox, 0.25f, 8, Color{210, 220, 235, 255});
    drawUiTextInRect(ui.assets, TextFormat("%s %d", UiStrings::HighScore, ui.save.highScore),
                     scoreBox, uiFontSize(ui, 22), Color{200, 140, 40, 255}, false);

    if (drawPrimaryButton(ui, UiStrings::StartGame, btnStart, CheckCollisionPointRec(mouse, btnStart))) {
        PlaySound(ui.assets.sfxMenu);
        ui.screen = UiScreen::PLAYING;
    }

    y += 76.0f;
    const Rectangle btnFontDown{pad, y, 120.0f, 44.0f};
    const Rectangle btnFontUp{pad + 132.0f, y, 120.0f, 44.0f};
    const Rectangle btnBackHub{pad + innerW - 360.0f, y, 168.0f, 44.0f};
    const Rectangle btnQuit{pad + innerW - 168.0f, y, 168.0f, 44.0f};
    if (ui.hubMode &&
        drawGhostButton(ui, mainMenuLabel(ui), btnBackHub, CheckCollisionPointRec(mouse, btnBackHub))) {
        PlaySound(ui.assets.sfxMenu);
        returnToHub = true;
    }
    if (drawGhostButton(ui, UiStrings::FontDown, btnFontDown, CheckCollisionPointRec(mouse, btnFontDown))) {
        ui.save.uiFontPercent = std::max(90, ui.save.uiFontPercent - 5);
        ui.save.save();
        PlaySound(ui.assets.sfxMenu);
    }
    if (drawGhostButton(ui, UiStrings::FontUp, btnFontUp, CheckCollisionPointRec(mouse, btnFontUp))) {
        ui.save.uiFontPercent = std::min(150, ui.save.uiFontPercent + 5);
        ui.save.save();
        PlaySound(ui.assets.sfxMenu);
    }
    if (drawGhostButton(ui, UiStrings::QuitGame, btnQuit, CheckCollisionPointRec(mouse, btnQuit))) {
        PlaySound(ui.assets.sfxMenu);
        requestQuit = true;
    }

    const Rectangle helpBox{pad, y + 56.0f, innerW, 148.0f};
    DrawRectangleRounded(helpBox, 0.08f, 10, Color{248, 250, 254, 255});
    DrawRectangleRoundedLines(helpBox, 0.08f, 10, Color{225, 232, 242, 255});
    const Rectangle ht{helpBox.x + 16.0f, helpBox.y + 10.0f, helpBox.width - 32.0f, 30.0f};
    drawUiTextInRect(ui.assets, UiStrings::ControlsTitle, ht, uiFontSize(ui, 20),
                     Color{70, 85, 110, 255}, false);
    const float helpW = helpBox.width - 32.0f;
    const Rectangle l1{helpBox.x + 16.0f, helpBox.y + 40.0f, helpW, 30.0f};
    const Rectangle l2{helpBox.x + 16.0f, helpBox.y + 72.0f, helpW, 30.0f};
    const Rectangle l3{helpBox.x + 16.0f, helpBox.y + 104.0f, helpW, 30.0f};
    drawUiTextInRect(ui.assets, UiStrings::ControlsLine1, l1, uiFontSize(ui, 17),
                     Color{55, 62, 78, 255}, false);
    drawUiTextInRect(ui.assets, UiStrings::ControlsLine2, l2, uiFontSize(ui, 17),
                     Color{55, 62, 78, 255}, false);
    drawUiTextInRect(ui.assets, UiStrings::ControlsLine3, l3, uiFontSize(ui, 17),
                     Color{140, 75, 50, 255}, false);
}

void updateAnim(AnimState& anim, const Game& game, float dt) {
    anim.globalTime += dt;

    const Position cur = game.getPlayer().getPosition();
    if (cur != anim.lastPlayerPos) {
        const Position delta{cur.x - anim.lastPlayerPos.x, cur.y - anim.lastPlayerPos.y};
        if (delta.y < 0) {
            anim.facing = 1;
        } else if (delta.y > 0) {
            anim.facing = 0;
        } else if (delta.x < 0) {
            anim.facing = 2;
        } else if (delta.x > 0) {
            anim.facing = 3;
        }
        anim.walkPhase += dt * 10.0f;
        if (anim.walkPhase >= 1.0f) {
            anim.walkPhase = 0.0f;
            anim.walkFrame = 1 - anim.walkFrame;
        }
        anim.lastPlayerPos = cur;
    }

    if (!game.getExplosionCells().empty()) {
        anim.explosionAnim += dt * 16.0f;
        anim.flashTimer = 0.08f;
    } else {
        anim.explosionAnim = 0.0f;
    }
    if (anim.flashTimer > 0.0f) {
        anim.flashTimer -= dt;
    }
}

void playGameSfx(GfxUi& ui, Game& game) {
    const int bombs = static_cast<int>(game.getActiveBombs().size());
    if (bombs > ui.prevBombCount) {
        PlaySound(ui.assets.sfxPlace);
    }
    ui.prevBombCount = bombs;

    if (!game.getExplosionCells().empty() && ui.anim.explosionAnim < 0.05f) {
        PlaySound(ui.assets.sfxExplosion);
    }

    if (game.getPlayerMaxBombs() > ui.prevMaxBombs ||
        game.getPlayerBlastRadius() > ui.prevBlastRadius ||
        game.getSpeedLevel() > ui.prevSpeedLevel) {
        PlaySound(ui.assets.sfxPickup);
    }
    ui.prevMaxBombs = game.getPlayerMaxBombs();
    ui.prevBlastRadius = game.getPlayerBlastRadius();
    ui.prevSpeedLevel = game.getSpeedLevel();

    const GameState st = game.getState();
    if (st != ui.prevGameState) {
        if (st == GameState::VICTORY) {
            PlaySound(ui.assets.sfxVictory);
        } else if (st == GameState::GAME_OVER) {
            PlaySound(ui.assets.sfxGameOver);
        }
    }
    ui.prevGameState = st;
}

void tryMoveAction(Game& game, InputAction action, int keyA, int keyB, PlayerSlot slot) {
    if (IsKeyPressed(keyA) || IsKeyPressed(keyB)) {
        game.applyAction(action, slot);
    }
}

void pollGameInput(Game& game, PlayerSlot slot = PlayerSlot::One) {
    if (IsKeyPressed(KEY_ESCAPE)) {
        game.applyAction(InputAction::QUIT);
        return;
    }
    if (IsKeyPressed(KEY_R)) {
        game.applyAction(InputAction::RESTART);
    }
    if (game.getState() != GameState::PLAYING) {
        return;
    }
    if (slot == PlayerSlot::One) {
        tryMoveAction(game, InputAction::MOVE_UP, KEY_W, KEY_UP, slot);
        tryMoveAction(game, InputAction::MOVE_DOWN, KEY_S, KEY_DOWN, slot);
        tryMoveAction(game, InputAction::MOVE_LEFT, KEY_A, KEY_LEFT, slot);
        tryMoveAction(game, InputAction::MOVE_RIGHT, KEY_D, KEY_RIGHT, slot);
        if (IsKeyPressed(KEY_SPACE)) {
            game.applyAction(InputAction::PLACE_BOMB, slot);
        }
        return;
    }
    tryMoveAction(game, InputAction::MOVE_UP, KEY_UP, KEY_UP, slot);
    tryMoveAction(game, InputAction::MOVE_DOWN, KEY_DOWN, KEY_DOWN, slot);
    tryMoveAction(game, InputAction::MOVE_LEFT, KEY_LEFT, KEY_LEFT, slot);
    tryMoveAction(game, InputAction::MOVE_RIGHT, KEY_RIGHT, KEY_RIGHT, slot);
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER)) {
        game.applyAction(InputAction::PLACE_BOMB, slot);
    }
}

void pollClientInput(Game& game, NetSession& net) {
    if (game.getState() != GameState::PLAYING) {
        return;
    }
    InputAction action = InputAction::NONE;
    if (IsKeyPressed(KEY_UP)) {
        action = InputAction::MOVE_UP;
    } else if (IsKeyPressed(KEY_DOWN)) {
        action = InputAction::MOVE_DOWN;
    } else if (IsKeyPressed(KEY_LEFT)) {
        action = InputAction::MOVE_LEFT;
    } else if (IsKeyPressed(KEY_RIGHT)) {
        action = InputAction::MOVE_RIGHT;
    } else if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER)) {
        action = InputAction::PLACE_BOMB;
    }
    if (action != InputAction::NONE) {
        net.clientSendInput(action);
    }
}

void drawBoard(GfxUi& ui, const Game& game) {
    const GfxAssets& a = ui.assets;
    const Grid& grid = game.getGrid();
    const Player& player = game.getPlayer();
    const auto& enemies = game.getEnemies();
    const auto& bombs = game.getActiveBombs();
    const auto& explosions = game.getExplosionCells();

    std::unordered_set<Position, PositionHash> bombCells;
    std::unordered_map<Position, BombOwner, PositionHash> bombOwners;
    for (const Bomb& bomb : bombs) {
        if (!bomb.isExplosionFinished()) {
            bombCells.insert(bomb.getPosition());
            bombOwners[bomb.getPosition()] = bomb.getOwner();
        }
    }

    const int expFrame =
        static_cast<int>(ui.anim.explosionAnim) % std::max(1, a.explosionSheet.frameCount);
    const float tileSize = static_cast<float>(kCellSize) * 0.88f;
    const int boardW = grid.getWidth() * kCellPitch;
    const int boardH = grid.getHeight() * kCellPitch;
    DrawRectangle(0, 0, boardW, boardH, Color{58, 118, 72, 255});
    for (int y = 0; y < grid.getHeight(); ++y) {
        for (int x = 0; x < grid.getWidth(); ++x) {
            const int tileX = x * kCellPitch;
            const int tileY = y * kCellPitch;
            DrawRectangle(tileX + 1, tileY + 1, kCellSize + 2, kCellSize + 2,
                          Color{20, 50, 30, 80});
        }
    }

    for (int y = 0; y < grid.getHeight(); ++y) {
        for (int x = 0; x < grid.getWidth(); ++x) {
            const Position pos{x, y};
            const int cx = x * kCellPitch + kCellSize / 2;
            const int cy = y * kCellPitch + kCellSize / 2;
            const CellType cell = grid.getCell(pos);

            if (cellInList(explosions, pos)) {
                drawSpriteFrame(a.explosionSheet, expFrame, cx, cy, tileSize * 1.2f);
                continue;
            }

            if (cell == CellType::INDESTRUCTIBLE) {
                drawTextureFit(a.wallMetal, cx, cy, tileSize);
            } else if (cell == CellType::DESTRUCTIBLE) {
                const Texture2D& destruct =
                    (((x + y) % 3) == 0 && a.wallBush.id != 0) ? a.wallBush : a.wallBrick;
                drawTextureFit(destruct, cx, cy, tileSize);
            } else if (cell == CellType::POWERUP) {
                drawTextureFit(a.floor, cx, cy, tileSize);
                const auto pu = game.getPowerUpAt(pos);
                if (pu.has_value()) {
                    DrawCircle(cx, cy, 18.0f, Color{255, 255, 255, 60});
                    drawTextureFit(a.textureForPowerUp(*pu), cx, cy, tileSize * 0.62f);
                } else {
                    DrawCircle(cx, cy, 12.0f, Color{120, 255, 160, 255});
                }
            } else {
                drawTextureFit(a.floor, cx, cy, tileSize);
            }

            if (bombCells.count(pos) > 0) {
                const float pulse = tileSize * (0.88f + 0.1f * std::sin(ui.anim.globalTime * 12.0f));
                const auto ownerIt = bombOwners.find(pos);
                const bool enemyBomb =
                    ownerIt != bombOwners.end() && ownerIt->second == BombOwner::ENEMY;
                if (enemyBomb) {
                    DrawCircle(cx, cy, pulse * 0.52f, Color{255, 60, 60, 200});
                }
                drawTextureFit(a.bomb, cx, cy, pulse);
            }
        }
    }

    for (const AI_Enemy& e : enemies) {
        if (!e.isAlive()) {
            continue;
        }
        const Position p = e.getPosition();
        const int cx = p.x * kCellPitch + kCellSize / 2;
        const int cy = p.y * kCellPitch + kCellSize / 2;
        const float bob = std::sin(ui.anim.globalTime * 6.0f + p.x) * 3.0f;
        drawTextureFit(a.enemy, cx, static_cast<int>(cy + bob), tileSize * 1.08f);
    }

    if (player.isAlive()) {
        const Position p = player.getPosition();
        const int cx = p.x * kCellPitch + kCellSize / 2;
        const int cy = p.y * kCellPitch + kCellSize / 2;
        const float bounce = (ui.anim.walkFrame == 0) ? 0.0f : -4.0f;
        const float size = tileSize * (1.0f + 0.04f * std::sin(ui.anim.walkPhase * PI));
        const int frameIdx = ui.anim.facing * 2 + ui.anim.walkFrame;
        drawSpriteFrame(a.playerSheet, frameIdx, cx, cy + static_cast<int>(bounce), size);
    }

    if (game.isDuoMode()) {
        const Player& player2 = game.getPlayer2();
        if (player2.isAlive()) {
            const Position p = player2.getPosition();
            const int cx = p.x * kCellPitch + kCellSize / 2;
            const int cy = p.y * kCellPitch + kCellSize / 2;
            const float size = tileSize * 1.02f;
            const SpriteSheet& sheet =
                a.player2Sheet.valid() ? a.player2Sheet : a.playerSheet;
            drawSpriteFrame(sheet, 0, cx, cy, size);
        }
    }

    if (ui.anim.flashTimer > 0.0f) {
        DrawRectangle(0, 0, boardW, boardH, Color{255, 220, 120, 40});
    }

    const int bw = boardW;
    const int bh = boardH;
    DrawRectangleRoundedLines({4.0f, 4.0f, static_cast<float>(bw - 8),
                               static_cast<float>(bh - 8)},
                              0.02f, 16, Color{255, 200, 70, 255});
    DrawRectangleRoundedLines({10.0f, 10.0f, static_cast<float>(bw - 20),
                             static_cast<float>(bh - 20)},
                            0.02f, 16, Color{255, 245, 200, 180});
}

void drawSegmentBar(Rectangle box, int filled, int maxSeg, Color fill) {
    drawCartoonPanel(box, Color{22, 28, 40, 230}, Color{70, 85, 110, 255}, 0.35f);
    const float pad = 8.0f;
    const float segW = (box.width - pad * 2.0f - (maxSeg - 1) * 3.0f) / static_cast<float>(maxSeg);
    for (int i = 0; i < maxSeg; ++i) {
        const Rectangle seg{box.x + pad + static_cast<float>(i) * (segW + 3.0f), box.y + 10.0f,
                            segW, box.height - 20.0f};
        const Color c = i < filled ? fill : Color{55, 62, 78, 255};
        DrawRectangleRounded(seg, 0.4f, 6, c);
    }
}

void drawBattleOverlay(const GfxUi& ui, const Game& game, int boardW, int boardH) {
    const GfxAssets& a = ui.assets;
    const int mins = static_cast<int>(ui.matchTimeLeft) / 60;
    const int secs = static_cast<int>(ui.matchTimeLeft) % 60;

    const Rectangle timerPill{static_cast<float>(boardW) * 0.5f - 80.0f, 8.0f, 160.0f, 44.0f};
    drawCartoonPanel(timerPill, Color{30, 32, 45, 240}, Color{255, 200, 80, 255}, 0.5f);
    drawUiTextInRect(a, TextFormat("%02d:%02d", mins, secs), timerPill, 30,
                     Color{255, 245, 200, 255}, true, 1.15f);

    const float barY = static_cast<float>(boardH) - static_cast<float>(kBattleOverlayBottom) + 6.0f;
    const float barW = 168.0f;
    const float barH = 38.0f;
    const float gap = 10.0f;
    const float totalW = barW * 2.0f + gap;
    float bx = (static_cast<float>(boardW) - totalW) * 0.5f;

    const Rectangle bombBar{bx, barY, barW, barH};
    bx += barW + gap;
    const Rectangle fireBar{bx, barY, barW, barH};

    drawTextureFit(a.itemBomb, static_cast<int>(bombBar.x + 22.0f),
                   static_cast<int>(bombBar.y + barH / 2.0f), 30.0f);
    drawSegmentBar({bombBar.x + 40.0f, bombBar.y, bombBar.width - 44.0f, bombBar.height},
                   game.getPlayerMaxBombs(), 8, Color{255, 210, 70, 255});

    drawTextureFit(a.itemFire, static_cast<int>(fireBar.x + 22.0f),
                   static_cast<int>(fireBar.y + barH / 2.0f), 30.0f);
    drawSegmentBar({fireBar.x + 40.0f, fireBar.y, fireBar.width - 44.0f, fireBar.height},
                   game.getPlayerBlastRadius(), 8, Color{255, 130, 60, 255});
}

void drawStatBox(const GfxAssets& assets, const Texture2D& icon, Rectangle box, const char* label,
                 int value) {
    drawCartoonPanel(box, Color{45, 55, 75, 255}, Color{90, 110, 140, 255}, 0.15f);
    drawTextureFit(icon, static_cast<int>(box.x + 32.0f),
                   static_cast<int>(box.y + box.height / 2.0f), 44.0f);
    const char* text = TextFormat("%s x%d", label, value);
    const Rectangle textBox{box.x + 56.0f, box.y, box.width - 60.0f, box.height};
    drawUiTextInRect(assets, text, textBox, 30, Color{255, 245, 200, 255}, false);
}

void drawHud(const GfxAssets& assets, const Game& game, int winW, int boardH) {
    const float hudY = static_cast<float>(boardH);
    DrawRectangle(0, boardH, winW, kHudHeight, Color{18, 24, 38, 255});
    DrawRectangle(0, boardH, winW, 6, Color{255, 195, 70, 255});
    DrawLine(0, boardH + 6, winW, boardH + 6, Color{100, 125, 165, 255});

    const Rectangle scoreBox{14.0f, hudY + 14.0f, 220.0f, 72.0f};
    drawCartoonPanel(scoreBox, Color{55, 68, 95, 255}, Color{110, 130, 170, 255}, 0.12f);
    drawUiTextInRect(assets, TextFormat("%s %d", UiStrings::Score, game.getPlayer().getScore()),
                     scoreBox, 36, Color{255, 225, 90, 255}, false);

    const Rectangle enemyBox{244.0f, hudY + 14.0f, 200.0f, 72.0f};
    drawCartoonPanel(enemyBox, Color{95, 45, 55, 255}, Color{180, 80, 90, 255}, 0.12f);
    drawUiTextInRect(assets,
                     TextFormat("%s %d", UiStrings::EnemiesLeft, game.getAliveEnemyCount()),
                     enemyBox, 34, Color{255, 210, 210, 255}, false);

    const Rectangle goalBox{14.0f, hudY + 94.0f, static_cast<float>(winW) - 28.0f, 34.0f};
    drawUiTextInRect(assets, UiStrings::Goal, goalBox, 24, Color{200, 215, 235, 255}, false);

    const float boxW = 178.0f;
    const float boxH = 72.0f;
    const float right = static_cast<float>(winW) - 14.0f;
    const Rectangle bombBox{right - boxW * 2.0f - 10.0f, hudY + 14.0f, boxW, boxH};
    const Rectangle fireBox{right - boxW, hudY + 14.0f, boxW, boxH};
    drawStatBox(assets, assets.itemBomb, bombBox, UiStrings::Bomb, game.getPlayerMaxBombs());
    drawStatBox(assets, assets.itemFire, fireBox, UiStrings::Fire, game.getPlayerBlastRadius());

    const char* hint = UiStrings::HintPlaying;
    if (game.getState() == GameState::GAME_OVER) {
        hint = UiStrings::HintGameOver;
    } else if (game.getState() == GameState::VICTORY) {
        hint = UiStrings::HintVictory;
    }
    const Rectangle hintBox{14.0f, hudY + 128.0f, static_cast<float>(winW) - 28.0f, 40.0f};
    drawUiTextInRect(assets, hint, hintBox, 24, Color{175, 195, 220, 255}, false);
}

void drawEndOverlay(GfxUi& ui, const Game& game, int w, int h) {
    ui.endBtnRestart = {};
    ui.endBtnMenu = {};
    if (game.getState() != GameState::GAME_OVER && game.getState() != GameState::VICTORY) {
        return;
    }
    DrawRectangle(0, 0, w, h, Color{0, 0, 0, 190});
    const bool victory = game.getState() == GameState::VICTORY;
    const char* title = victory ? UiStrings::Win : UiStrings::Lose;
    const Rectangle box{static_cast<float>(w - 560) / 2.0f, static_cast<float>(h - 320) / 2.0f,
                        560.0f, 300.0f};
    drawCartoonPanel(box, victory ? Color{70, 190, 100, 255} : Color{210, 70, 70, 255},
                    Color{40, 40, 40, 255}, 0.2f);
    drawUiTextInRect(ui.assets, title, {box.x, box.y + 12.0f, box.width, 72.0f}, 56,
                     victory ? Color{255, 255, 245, 255} : Color{255, 245, 245, 255}, true);
    drawUiTextInRect(ui.assets, pickActionLabel(ui), {box.x, box.y + 88.0f, box.width, 36.0f}, 28,
                     Color{255, 240, 200, 255}, false);

    const float btnW = 220.0f;
    const float btnH = 52.0f;
    const float gap = 28.0f;
    const float btnY = box.y + 140.0f;
    const float btnX = box.x + (box.width - btnW * 2.0f - gap) * 0.5f;
    ui.endBtnRestart = {btnX, btnY, btnW, btnH};
    ui.endBtnMenu = {btnX + btnW + gap, btnY, btnW, btnH};

    const Vector2 mouse = GetMousePosition();
    auto drawEndBtn = [&](Rectangle r, const char* label, bool primary) {
        const bool hover = CheckCollisionPointRec(mouse, r);
        Color fill = primary ? Color{76, 175, 80, 255} : Color{100, 116, 139, 255};
        if (hover) {
            fill = primary ? Color{102, 187, 106, 255} : Color{148, 163, 184, 255};
        }
        DrawRectangleRounded(r, 0.35f, 8, fill);
        DrawRectangleRoundedLines(r, 0.35f, 8, Color{40, 40, 40, 255});
        drawUiTextInRect(ui.assets, label, r, 30, RAYWHITE, true);
    };
    drawEndBtn(ui.endBtnRestart, playAgainLabel(ui), true);
    drawEndBtn(ui.endBtnMenu, mainMenuLabel(ui), false);
}

int pollEndOverlayAction(const GfxUi& ui) {
    if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        return 0;
    }
    if (CheckCollisionPointRec(GetMousePosition(), ui.endBtnRestart)) {
        return 1;
    }
    if (CheckCollisionPointRec(GetMousePosition(), ui.endBtnMenu)) {
        return 2;
    }
    return 0;
}

void resizeForGame(const Game& game) {
    const int winW = game.getGrid().getWidth() * kCellPitch;
    const int winH = game.getGrid().getHeight() * kCellPitch + kHudHeight;
    SetWindowSize(winW, winH);
    SetWindowTitle("Bomber Battle");
}

void syncHudTrackers(GfxUi& ui, const Game& game) {
    ui.prevBombCount = static_cast<int>(game.getActiveBombs().size());
    ui.prevMaxBombs = game.getPlayerMaxBombs();
    ui.prevBlastRadius = game.getPlayerBlastRadius();
    ui.prevSpeedLevel = game.getSpeedLevel();
    ui.prevScore = game.getPlayer().getScore();
    ui.prevGameState = game.getState();
}
}  // namespace

GameLaunchResult runGfxApp(Game& game, bool hubMode) {
    GfxUi ui;
    ui.hubMode = hubMode;
    ui.save.load();
    ui.difficulty = static_cast<Difficulty>(std::clamp(ui.save.lastDifficulty, 0, 2));
    ui.playMode = ui.save.lastPlayMode;

    SetConfigFlags(FLAG_VSYNC_HINT);
    InitWindow(kMenuW, kMenuH, "Bomber Battle - Menu");
    SetTargetFPS(game.getConfig().targetFPS);

    if (!ui.assets.load()) {
        CloseWindow();
        return GameLaunchResult::ReturnToHub;
    }

    bool requestQuit = false;
    bool returnToHub = false;

    while (!WindowShouldClose() && !requestQuit && !returnToHub &&
           game.getState() != GameState::QUIT) {
        const float dt = GetFrameTime();

        if (ui.screen == UiScreen::MENU) {
            ui.anim.globalTime += dt;
            if (IsKeyPressed(KEY_ESCAPE)) {
                requestQuit = true;
            }

            BeginDrawing();
            drawMenu(ui, requestQuit, returnToHub);
            EndDrawing();

            if (ui.screen == UiScreen::PLAYING) {
                ui.net.close();
                game.setConfig(configForDifficulty(ui.difficulty));
                game.setPlayMode(ui.playMode);
                if (ui.playMode == PlayMode::ONLINE_HOST) {
                    if (!ui.net.startHost()) {
                        ui.screen = UiScreen::MENU;
                    }
                } else if (ui.playMode == PlayMode::ONLINE_CLIENT) {
                    if (!ui.net.connect(ui.save.hostIp)) {
                        ui.screen = UiScreen::MENU;
                    }
                }
                if (ui.screen == UiScreen::PLAYING) {
                    game.applyAction(InputAction::RESTART);
                    ui.anim = AnimState{};
                    ui.matchTimeLeft = 180.0f;
                    syncHudTrackers(ui, game);
                    resizeForGame(game);
                }
            }
            continue;
        }

        if (IsKeyPressed(KEY_ESCAPE)) {
            if (ui.hubMode) {
                requestQuit = true;
                break;
            }
            ui.screen = UiScreen::MENU;
            ui.net.close();
            SetWindowSize(kMenuW, kMenuH);
            SetWindowTitle("Bomber Battle - Menu");
            continue;
        }

        if (game.getState() == GameState::GAME_OVER || game.getState() == GameState::VICTORY) {
            const int endAct = pollEndOverlayAction(ui);
            if (endAct == 1 || IsKeyPressed(KEY_R)) {
                game.applyAction(InputAction::RESTART);
            } else if (endAct == 2) {
                if (ui.hubMode) {
                    returnToHub = true;
                    break;
                }
                ui.screen = UiScreen::MENU;
                ui.net.close();
                SetWindowSize(kMenuW, kMenuH);
                SetWindowTitle("Bomber Battle - Menu");
            }
        }

        if (ui.playMode == PlayMode::ONLINE_CLIENT && ui.net.isActive()) {
            pollClientInput(game, ui.net);
            NetworkSnapshot snap{};
            if (ui.net.clientRecvSnapshot(snap)) {
                game.importSnapshot(snap);
            }
            if (IsKeyPressed(KEY_R)) {
                game.applyAction(InputAction::RESTART);
            }
        } else {
            pollGameInput(game, PlayerSlot::One);
            if (game.isDuoMode() && ui.playMode == PlayMode::LOCAL_DUO) {
                pollGameInput(game, PlayerSlot::Two);
            }
            if (ui.playMode == PlayMode::ONLINE_HOST && ui.net.isActive()) {
                InputAction remote = InputAction::NONE;
                if (ui.net.hostRecvInput(remote) && remote != InputAction::NONE) {
                    game.applyNetworkInput(remote);
                }
            }
            game.update(dt);
            if (game.getState() == GameState::PLAYING && ui.matchTimeLeft > 0.0f) {
                ui.matchTimeLeft -= dt;
            }
            if (ui.playMode == PlayMode::ONLINE_HOST && ui.net.isActive()) {
                NetworkSnapshot snap{};
                game.exportSnapshot(snap);
                ui.net.hostSendSnapshot(snap);
            }
        }

        updateAnim(ui.anim, game, dt);
        playGameSfx(ui, game);

        if (game.getPlayer().getScore() > ui.save.highScore) {
            ui.save.highScore = game.getPlayer().getScore();
        }

        const int winW = game.getGrid().getWidth() * kCellPitch;
        const int boardH = game.getGrid().getHeight() * kCellPitch;

        BeginDrawing();
        ClearBackground(Color{42, 62, 48, 255});
        drawBoard(ui, game);
        drawBattleOverlay(ui, game, winW, boardH);
        drawEndOverlay(ui, game, winW, boardH);
        drawHud(ui.assets, game, winW, boardH);
        EndDrawing();
    }

    ui.save.lastDifficulty = static_cast<int>(ui.difficulty);
    ui.save.lastPlayMode = ui.playMode;
    ui.save.save();
    ui.net.close();
    ui.assets.unload();
    CloseWindow();
    if (requestQuit) {
        return GameLaunchResult::ExitApp;
    }
    return GameLaunchResult::ReturnToHub;
}
