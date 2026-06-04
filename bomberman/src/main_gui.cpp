#include "Game.hpp"
#include "GfxApp.hpp"

int main() {
    Game game(GameConfig{});
    runGfxApp(game);
    return 0;
}
