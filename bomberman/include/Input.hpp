#pragma once

enum class InputAction {
    NONE,
    MOVE_UP,
    MOVE_DOWN,
    MOVE_LEFT,
    MOVE_RIGHT,
    PLACE_BOMB,
    RESTART,
    QUIT
};

class InputReader {
public:
    static bool poll(InputAction& action);
    static void initTerminal();
    static void restoreTerminal();
};
