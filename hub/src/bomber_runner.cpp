#include "Game.hpp"
#include "GfxApp.hpp"
#include "GameLaunch.hpp"

GameLaunchResult runBombermanGame() {
    Game game(GameConfig{});
    return runGfxApp(game, true);
}
