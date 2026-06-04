#include "Input.hpp"

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#endif

#include <cctype>

#ifdef _WIN32
namespace {
HANDLE g_consoleIn = INVALID_HANDLE_VALUE;
DWORD g_originalMode = 0;

InputAction mapVirtualKey(WORD vk, wchar_t ch) {
    switch (vk) {
        case VK_UP:
            return InputAction::MOVE_UP;
        case VK_DOWN:
            return InputAction::MOVE_DOWN;
        case VK_LEFT:
            return InputAction::MOVE_LEFT;
        case VK_RIGHT:
            return InputAction::MOVE_RIGHT;
        case VK_SPACE:
            return InputAction::PLACE_BOMB;
        default:
            break;
    }

    if (ch == L' ') {
        return InputAction::PLACE_BOMB;
    }
    if (ch >= 32 && ch < 127) {
        const char c = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
        switch (c) {
            case 'w':
                return InputAction::MOVE_UP;
            case 's':
                return InputAction::MOVE_DOWN;
            case 'a':
                return InputAction::MOVE_LEFT;
            case 'd':
                return InputAction::MOVE_RIGHT;
            case 'r':
                return InputAction::RESTART;
            case 'q':
                return InputAction::QUIT;
            default:
                break;
        }
    }
    return InputAction::NONE;
}

void drainInputBuffer() {
    INPUT_RECORD records[32];
    DWORD read = 0;
    while (PeekConsoleInputW(g_consoleIn, records, 32, &read) && read > 0) {
        ReadConsoleInputW(g_consoleIn, records, read, &read);
    }
}
}  // namespace

void InputReader::initTerminal() {
    g_consoleIn = GetStdHandle(STD_INPUT_HANDLE);
    if (g_consoleIn == INVALID_HANDLE_VALUE) {
        return;
    }

    GetConsoleMode(g_consoleIn, &g_originalMode);
    DWORD mode = g_originalMode;
    mode &= ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT | ENABLE_QUICK_EDIT_MODE | ENABLE_INSERT_MODE);
    mode |= ENABLE_EXTENDED_FLAGS;
    SetConsoleMode(g_consoleIn, mode);
    drainInputBuffer();
}

void InputReader::restoreTerminal() {
    if (g_consoleIn != INVALID_HANDLE_VALUE) {
        SetConsoleMode(g_consoleIn, g_originalMode);
    }
}

bool InputReader::poll(InputAction& action) {
    action = InputAction::NONE;
    if (g_consoleIn == INVALID_HANDLE_VALUE) {
        return false;
    }

    DWORD pending = 0;
    if (!GetNumberOfConsoleInputEvents(g_consoleIn, &pending) || pending == 0) {
        return false;
    }

    INPUT_RECORD record{};
    DWORD read = 0;
    while (pending > 0) {
        if (!ReadConsoleInputW(g_consoleIn, &record, 1, &read) || read == 0) {
            break;
        }
        --pending;

        if (record.EventType != KEY_EVENT) {
            continue;
        }
        const KEY_EVENT_RECORD& key = record.Event.KeyEvent;
        if (!key.bKeyDown) {
            continue;
        }
        if (key.wVirtualKeyCode == VK_SHIFT || key.wVirtualKeyCode == VK_CONTROL ||
            key.wVirtualKeyCode == VK_MENU) {
            continue;
        }

        const InputAction mapped = mapVirtualKey(key.wVirtualKeyCode, key.uChar.UnicodeChar);
        if (mapped != InputAction::NONE) {
            action = mapped;
            return true;
        }
    }
    return false;
}
#else
namespace {
termios g_original{};
bool g_hasOriginal = false;
}  // namespace

void InputReader::initTerminal() {
    if (g_hasOriginal) {
        return;
    }
    tcgetattr(STDIN_FILENO, &g_original);
    termios raw = g_original;
    raw.c_lflag &= static_cast<tcflag_t>(~(ICANON | ECHO));
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);
    fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
    g_hasOriginal = true;
}

void InputReader::restoreTerminal() {
    if (!g_hasOriginal) {
        return;
    }
    tcsetattr(STDIN_FILENO, TCSANOW, &g_original);
    g_hasOriginal = false;
}

bool InputReader::poll(InputAction& action) {
    action = InputAction::NONE;
    char ch = 0;
    const ssize_t n = read(STDIN_FILENO, &ch, 1);
    if (n <= 0) {
        return false;
    }
    const char c = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    switch (c) {
        case 'w':
            action = InputAction::MOVE_UP;
            break;
        case 's':
            action = InputAction::MOVE_DOWN;
            break;
        case 'a':
            action = InputAction::MOVE_LEFT;
            break;
        case 'd':
            action = InputAction::MOVE_RIGHT;
            break;
        case ' ':
            action = InputAction::PLACE_BOMB;
            break;
        case 'r':
            action = InputAction::RESTART;
            break;
        case 'q':
            action = InputAction::QUIT;
            break;
        default:
            break;
    }
    return action != InputAction::NONE;
}
#endif
