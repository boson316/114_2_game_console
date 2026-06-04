#include "EnemyRLPolicy.hpp"

#include "Position.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>

namespace {
bool parseJsonNumberArray(const std::string& text, std::vector<float>& out) {
    const std::size_t qpos = text.find("\"q\"");
    if (qpos == std::string::npos) {
        return false;
    }
    const std::size_t start = text.find('[', qpos);
    const std::size_t end = text.find(']', start);
    if (start == std::string::npos || end == std::string::npos || end <= start) {
        return false;
    }
    out.clear();
    std::string chunk = text.substr(start + 1, end - start - 1);
    std::stringstream ss(chunk);
    std::string token;
    while (std::getline(ss, token, ',')) {
        try {
            out.push_back(std::stof(token));
        } catch (...) {
            return false;
        }
    }
    return !out.empty();
}
}  // namespace

bool EnemyRLPolicy::tryLoad(const std::string& path) {
    loaded_ = false;
    q_.clear();
    std::ifstream in(path);
    if (!in) {
        return false;
    }
    std::string text((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    if (!parseJsonNumberArray(text, q_)) {
        return false;
    }
    if (static_cast<int>(q_.size()) != kStateCount * kActionCount) {
        q_.clear();
        return false;
    }
    loaded_ = true;
    return true;
}

int EnemyRLPolicy::encodeState(Position enemy, Position player, bool inDanger, bool bombAtFeet,
                               const Grid& grid) {
    const int dx = std::clamp(player.x - enemy.x, -3, 3) + 3;
    const int dy = std::clamp(player.y - enemy.y, -3, 3) + 3;

    int wallMask = 0;
    int bit = 1;
    for (const Position& dir : Direction::ALL) {
        const Position next = enemy + dir;
        if (!grid.isInBounds(next) || !grid.isPassable(next)) {
            wallMask |= bit;
        }
        bit <<= 1;
    }

    return dx + dy * kDxBins + (inDanger ? 1 : 0) * kDxBins * kDyBins +
           (bombAtFeet ? 1 : 0) * kDxBins * kDyBins * 2 + wallMask * kDxBins * kDyBins * 4;
}

int EnemyRLPolicy::bestAction(int state) const {
    if (!loaded_ || state < 0 || state >= kStateCount) {
        return 4;
    }
    int best = 0;
    float bestQ = q_[static_cast<std::size_t>(state) * kActionCount];
    for (int a = 1; a < kActionCount; ++a) {
        const float v = q_[static_cast<std::size_t>(state) * kActionCount + a];
        if (v > bestQ) {
            bestQ = v;
            best = a;
        }
    }
    return best;
}

Position EnemyRLPolicy::actionToStep(int action) {
    switch (action) {
        case 0:
            return Direction::UP;
        case 1:
            return Direction::DOWN;
        case 2:
            return Direction::LEFT;
        case 3:
            return Direction::RIGHT;
        default:
            return {0, 0};
    }
}
