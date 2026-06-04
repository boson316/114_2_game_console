#include "Game.h"
#include "GameLaunch.hpp"

#include <memory>

GameLaunchResult runChessWarGame() {
    std::unique_ptr<Game> game = std::make_unique<Game>();
    return game->run();
}
