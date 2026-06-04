#include "NetSession.hpp"

#include <cstring>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
using SocketHandle = SOCKET;
constexpr SocketHandle kInvalidSocket = INVALID_SOCKET;
#else
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
using SocketHandle = int;
constexpr SocketHandle kInvalidSocket = -1;
#endif

namespace {
constexpr std::uint8_t kMagic = 0xBB;
constexpr std::uint8_t kPktSnapshot = 1;
constexpr std::uint8_t kPktInput = 2;

#if defined(_WIN32)
bool wsaStarted = false;
void ensureWsa() {
    if (!wsaStarted) {
        WSADATA wsa{};
        WSAStartup(MAKEWORD(2, 2), &wsa);
        wsaStarted = true;
    }
}
SocketHandle toSock(std::uintptr_t v) { return static_cast<SocketHandle>(v); }
std::uintptr_t fromSock(SocketHandle s) { return static_cast<std::uintptr_t>(s); }
void closeSock(SocketHandle s) {
    if (s != kInvalidSocket) {
        closesocket(s);
    }
}
void setNonBlocking(SocketHandle sock, bool on) {
    u_long mode = on ? 1 : 0;
    ioctlsocket(sock, FIONBIO, &mode);
}
#else
void ensureWsa() {}
SocketHandle toSock(std::uintptr_t v) { return static_cast<SocketHandle>(v); }
std::uintptr_t fromSock(SocketHandle s) { return static_cast<std::uintptr_t>(s); }
void closeSock(SocketHandle s) {
    if (s != kInvalidSocket) {
        close(s);
    }
}
void setNonBlocking(SocketHandle sock, bool on) {
    const int flags = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, on ? (flags | O_NONBLOCK) : (flags & ~O_NONBLOCK));
}
#endif

bool sendAll(SocketHandle sock, const void* data, int len) {
    const char* p = static_cast<const char*>(data);
    int sent = 0;
    while (sent < len) {
#if defined(_WIN32)
        const int n = send(sock, p + sent, len - sent, 0);
#else
        const int n =
            static_cast<int>(send(sock, p + sent, static_cast<std::size_t>(len - sent), 0));
#endif
        if (n <= 0) {
            return false;
        }
        sent += n;
    }
    return true;
}

bool recvAll(SocketHandle sock, void* data, int len) {
    char* p = static_cast<char*>(data);
    int got = 0;
    while (got < len) {
#if defined(_WIN32)
        const int n = recv(sock, p + got, len - got, 0);
#else
        const int n =
            static_cast<int>(recv(sock, p + got, static_cast<std::size_t>(len - got), 0));
#endif
        if (n <= 0) {
            return false;
        }
        got += n;
    }
    return true;
}

bool peekRecv(SocketHandle sock, void* data, int len) {
    setNonBlocking(sock, true);
    const bool ok = recvAll(sock, data, len);
    setNonBlocking(sock, false);
    return ok;
}
}  // namespace

void NetSession::close() {
    closeSock(toSock(peerSock_));
    closeSock(toSock(listenSock_));
    peerSock_ = fromSock(kInvalidSocket);
    listenSock_ = fromSock(kInvalidSocket);
    isHost_ = false;
    isClient_ = false;
}

bool NetSession::startHost(std::uint16_t port) {
    close();
    ensureWsa();
    lastError_.clear();

    SocketHandle listenFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenFd == kInvalidSocket) {
        lastError_ = "socket failed";
        return false;
    }
    int yes = 1;
    setsockopt(listenFd, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&yes), sizeof(yes));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);
    if (bind(listenFd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0) {
        lastError_ = "bind failed";
        closeSock(listenFd);
        return false;
    }
    if (::listen(listenFd, 1) != 0) {
        lastError_ = "listen failed";
        closeSock(listenFd);
        return false;
    }

    listenSock_ = fromSock(listenFd);
    isHost_ = true;

    sockaddr_in clientAddr{};
    int clientLen = sizeof(clientAddr);
    SocketHandle peer = accept(listenFd, reinterpret_cast<sockaddr*>(&clientAddr), &clientLen);
    if (peer == kInvalidSocket) {
        lastError_ = "accept failed";
        close();
        return false;
    }
    peerSock_ = fromSock(peer);
    return true;
}

bool NetSession::connect(const char* host, std::uint16_t port) {
    close();
    ensureWsa();
    lastError_.clear();

    SocketHandle sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == kInvalidSocket) {
        lastError_ = "socket failed";
        return false;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (inet_pton(AF_INET, host, &addr.sin_addr) != 1) {
        lastError_ = "bad IP";
        closeSock(sock);
        return false;
    }
    if (::connect(sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0) {
        lastError_ = "connect failed";
        closeSock(sock);
        return false;
    }
    peerSock_ = fromSock(sock);
    isClient_ = true;
    return true;
}

bool NetSession::hostSendSnapshot(const NetworkSnapshot& snap) {
    if (!isHost_ || peerSock_ == fromSock(kInvalidSocket)) {
        return false;
    }
    const std::uint8_t head[2] = {kMagic, kPktSnapshot};
    SocketHandle sock = toSock(peerSock_);
    return sendAll(sock, head, 2) && sendAll(sock, &snap, static_cast<int>(sizeof(snap)));
}

bool NetSession::hostRecvInput(InputAction& action) {
    if (!isHost_ || peerSock_ == fromSock(kInvalidSocket)) {
        return false;
    }
    std::uint8_t pkt[3]{};
    if (!peekRecv(toSock(peerSock_), pkt, 3)) {
        return false;
    }
    if (pkt[0] != kMagic || pkt[1] != kPktInput) {
        return false;
    }
    action = static_cast<InputAction>(pkt[2]);
    return true;
}

bool NetSession::clientSendInput(InputAction action) {
    if (!isClient_ || peerSock_ == fromSock(kInvalidSocket)) {
        return false;
    }
    const std::uint8_t pkt[3] = {kMagic, kPktInput, static_cast<std::uint8_t>(action)};
    return sendAll(toSock(peerSock_), pkt, 3);
}

bool NetSession::clientRecvSnapshot(NetworkSnapshot& snap) {
    if (!isClient_ || peerSock_ == fromSock(kInvalidSocket)) {
        return false;
    }
    std::uint8_t head[2]{};
    SocketHandle sock = toSock(peerSock_);
    setNonBlocking(sock, true);
#if defined(_WIN32)
    const int hn = recv(sock, reinterpret_cast<char*>(head), 2, 0);
#else
    const int hn = static_cast<int>(recv(sock, head, 2, 0));
#endif
    if (hn != 2 || head[0] != kMagic || head[1] != kPktSnapshot) {
        setNonBlocking(sock, false);
        return false;
    }
    const bool ok = recvAll(sock, &snap, static_cast<int>(sizeof(snap)));
    setNonBlocking(sock, false);
    return ok;
}
