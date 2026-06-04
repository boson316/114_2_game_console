#pragma once

#include "PlayMode.hpp"

#include <string>

struct SaveData {
    int highScore = 0;
    int uiFontPercent = 115;
    int lastDifficulty = 1;
    PlayMode lastPlayMode = PlayMode::SOLO;
    char hostIp[64] = "127.0.0.1";

    static std::string defaultPath();

    bool load(const std::string& path = defaultPath());
    bool save(const std::string& path = defaultPath()) const;
};
