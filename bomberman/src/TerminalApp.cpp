#include "TerminalApp.hpp"

#include "Game.hpp"
#include "Input.hpp"
#include "Renderer.hpp"

#include <chrono>
#include <thread>

using Clock = std::chrono::steady_clock;

void runTerminalApp(Game& game) {
    Renderer renderer;
    InputReader::initTerminal();

    const int targetFPS = game.getConfig().targetFPS;
    const auto frameDuration = std::chrono::duration<float>(1.0f / static_cast<float>(targetFPS));
    auto lastTime = Clock::now();

    while (game.getState() != GameState::QUIT) {
        const auto now = Clock::now();
        const float deltaTime = std::chrono::duration<float>(now - lastTime).count();
        lastTime = now;

        InputAction action = InputAction::NONE;
        while (InputReader::poll(action)) {
            game.applyAction(action);
        }

        game.update(deltaTime);
        renderer.render(game.getGrid(), game.getPlayer(), game.getEnemies(), game.getActiveBombs(),
                        game.getExplosionCells(), game.getState(), game.getPlayer().getScore());

        const auto frameEnd = Clock::now();
        const auto elapsed = frameEnd - now;
        if (elapsed < frameDuration) {
            std::this_thread::sleep_for(frameDuration - elapsed);
        }
    }

    InputReader::restoreTerminal();
}
