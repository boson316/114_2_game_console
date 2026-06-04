#include "SaveData.hpp"

#include <filesystem>
#include <fstream>
#include <sstream>

namespace {
void trim(std::string& s) {
    while (!s.empty() && (s.back() == '\r' || s.back() == '\n' || s.back() == ' ')) {
        s.pop_back();
    }
}
}  // namespace

std::string SaveData::defaultPath() {
    return "save/profile.save";
}

bool SaveData::load(const std::string& path) {
    std::ifstream in(path);
    if (!in) {
        return false;
    }
    std::string line;
    while (std::getline(in, line)) {
        trim(line);
        if (line.empty() || line[0] == '#') {
            continue;
        }
        const auto eq = line.find('=');
        if (eq == std::string::npos) {
            continue;
        }
        const std::string key = line.substr(0, eq);
        const std::string val = line.substr(eq + 1);
        if (key == "high_score") {
            highScore = std::stoi(val);
        } else if (key == "ui_font_percent") {
            uiFontPercent = std::stoi(val);
        } else if (key == "last_difficulty") {
            lastDifficulty = std::stoi(val);
        } else if (key == "last_play_mode") {
            lastPlayMode = static_cast<PlayMode>(std::stoi(val));
        } else if (key == "host_ip") {
            val.copy(hostIp, sizeof(hostIp) - 1);
            hostIp[sizeof(hostIp) - 1] = '\0';
        }
    }
    if (uiFontPercent < 90) {
        uiFontPercent = 90;
    }
    if (uiFontPercent > 150) {
        uiFontPercent = 150;
    }
    return true;
}

bool SaveData::save(const std::string& path) const {
    const std::size_t slash = path.find_last_of("/\\");
    if (slash != std::string::npos) {
        std::error_code ec;
        std::filesystem::create_directories(path.substr(0, slash), ec);
    }
    std::ofstream out(path, std::ios::trunc);
    if (!out) {
        return false;
    }
    out << "# Bomber Battle profile (safe to edit)\n";
    out << "high_score=" << highScore << "\n";
    out << "ui_font_percent=" << uiFontPercent << "\n";
    out << "last_difficulty=" << lastDifficulty << "\n";
    out << "last_play_mode=" << static_cast<int>(lastPlayMode) << "\n";
    out << "host_ip=" << hostIp << "\n";
    return true;
}
