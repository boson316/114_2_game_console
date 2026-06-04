#pragma once

#include "Input.hpp"

#include <cstdint>
#include <string>

struct NetworkSnapshot {
    static constexpr int kMaxEnemies = 4;
    static constexpr int kMaxCells = 15 * 13;

    uint8_t gameState = 0;
    uint16_t gridW = 0;
    uint16_t gridH = 0;
    int16_t p1x = 0;
    int16_t p1y = 0;
    int16_t p2x = 0;
    int16_t p2y = 0;
    uint8_t p1alive = 1;
    uint8_t p2alive = 0;
    uint8_t duo = 0;
    int32_t score = 0;
    uint8_t enemyCount = 0;
    int16_t enemyX[kMaxEnemies]{};
    int16_t enemyY[kMaxEnemies]{};
    uint8_t enemyAlive[kMaxEnemies]{};
    uint16_t cellCount = 0;
    uint8_t cells[kMaxCells]{};
};

class NetSession {
public:
    bool startHost(std::uint16_t port = 7755);
    bool connect(const char* host, std::uint16_t port = 7755);
    void close();

    bool isHost() const { return isHost_; }
    bool isClient() const { return isClient_; }
    bool isActive() const { return isHost_ || isClient_; }
    const std::string& lastError() const { return lastError_; }

    bool hostSendSnapshot(const NetworkSnapshot& snap);
    bool hostRecvInput(InputAction& action);

    bool clientSendInput(InputAction action);
    bool clientRecvSnapshot(NetworkSnapshot& snap);

private:
    bool isHost_ = false;
    bool isClient_ = false;
    std::string lastError_;
#if defined(_WIN32)
    std::uintptr_t listenSock_ = static_cast<std::uintptr_t>(-1);
    std::uintptr_t peerSock_ = static_cast<std::uintptr_t>(-1);
#else
    int listenSock_ = -1;
    int peerSock_ = -1;
#endif
};
