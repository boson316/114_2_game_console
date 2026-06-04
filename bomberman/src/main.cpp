#include "Game.hpp"
#include "TerminalApp.hpp"

int main() {
    Game game(GameConfig{});
    runTerminalApp(game);
    return 0;
}
