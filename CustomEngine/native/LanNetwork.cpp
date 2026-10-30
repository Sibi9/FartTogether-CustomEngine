#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <string.h>
#include "LanNetwork.h"

#pragma comment(lib, "ws2_32.lib")

static SOCKET gSock = INVALID_SOCKET;
static LanRole gRole = LAN_OFFLINE;
static sockaddr_in gPeerAddr = { 0 };
static bool gHasPeer = false;
static uint16_t gPort = 7777;
static char gStatus[128] = "LAN Offline (F2: Host | F3: Join)";

static bool EnsureWsa() {
    static bool initialized = false;
    if (!initialized) {
        WSADATA wsa;
        if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return false;
        initialized = true;
    }
    return true;
}

void LanShutdown() {
    if (gSock != INVALID_SOCKET) {
        closesocket(gSock);
        gSock = INVALID_SOCKET;
    }
    gRole = LAN_OFFLINE;
    gHasPeer = false;
    snprintf(gStatus, sizeof(gStatus), "LAN Offline (F2: Host | F3: Join)");
}

bool LanInitHost(uint16_t port) {
    LanShutdown();
    if (!EnsureWsa()) return false;
    gPort = port;
    gSock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (gSock == INVALID_SOCKET) return false;

    u_long nonBlock = 1;
    ioctlsocket(gSock, FIONBIO, &nonBlock);
    BOOL bcast = TRUE;
    setsockopt(gSock, SOL_SOCKET, SO_BROADCAST, (const char*)&bcast, sizeof(bcast));
    BOOL reuse = TRUE;
    setsockopt(gSock, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse));

    sockaddr_in local = { 0 };
    local.sin_family = AF_INET;
    local.sin_port = htons(port);
    local.sin_addr.s_addr = INADDR_ANY;
    if (bind(gSock, (sockaddr*)&local, sizeof(local)) != 0) {
        LanShutdown();
        return false;
    }
    gRole = LAN_HOST;
    gHasPeer = false;
    snprintf(gStatus, sizeof(gStatus), "LAN HOST (Port %d) - Waiting for P2...", (int)port);
    return true;
}

bool LanInitClient(uint16_t port) {
    LanShutdown();
    if (!EnsureWsa()) return false;
    gPort = port;
    gSock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (gSock == INVALID_SOCKET) return false;

    u_long nonBlock = 1;
    ioctlsocket(gSock, FIONBIO, &nonBlock);
    BOOL bcast = TRUE;
    setsockopt(gSock, SOL_SOCKET, SO_BROADCAST, (const char*)&bcast, sizeof(bcast));

    sockaddr_in local = { 0 };
    local.sin_family = AF_INET;
    local.sin_port = htons(0);
    local.sin_addr.s_addr = INADDR_ANY;
    bind(gSock, (sockaddr*)&local, sizeof(local));

    gRole = LAN_CLIENT;
    gHasPeer = false;
    snprintf(gStatus, sizeof(gStatus), "LAN CLIENT - Searching Host on UDP %d...", (int)port);
    return true;
}

LanRole LanGetRole() { return gRole; }
bool LanIsConnected() { return gHasPeer; }
const char* LanGetStatusText() { return gStatus; }

bool LanHostPollInput(LanInputPacket& outInput) {
    if (gRole != LAN_HOST || gSock == INVALID_SOCKET) return false;
    bool got = false;
    sockaddr_in from = { 0 };
    int fromLen = sizeof(from);
    LanInputPacket pkt;
    while (recvfrom(gSock, (char*)&pkt, sizeof(pkt), 0, (sockaddr*)&from, &fromLen) == sizeof(LanInputPacket)) {
        if (pkt.magic == 0x46415254 && pkt.pktType == 1) {
            gPeerAddr = from;
            if (!gHasPeer) {
                gHasPeer = true;
                snprintf(gStatus, sizeof(gStatus), "LAN HOST - P2 Connected (%s)", inet_ntoa(from.sin_addr));
            }
            outInput = pkt;
            got = true;
        }
    }
    return got;
}

void LanHostBroadcastState(const LanStatePacket& state) {
    if (gRole != LAN_HOST || gSock == INVALID_SOCKET) return;
    if (gHasPeer) {
        sendto(gSock, (const char*)&state, sizeof(state), 0, (sockaddr*)&gPeerAddr, sizeof(gPeerAddr));
    }
}

void LanClientSendInput(const LanInputPacket& input) {
    if (gRole != LAN_CLIENT || gSock == INVALID_SOCKET) return;
    if (gHasPeer) {
        sendto(gSock, (const char*)&input, sizeof(input), 0, (sockaddr*)&gPeerAddr, sizeof(gPeerAddr));
    } else {
        // إرسال لـ 127.0.0.1 (نفس الجهاز) وبث للشبكة المحلية 255.255.255.255
        sockaddr_in loop = { 0 };
        loop.sin_family = AF_INET;
        loop.sin_port = htons(gPort);
        loop.sin_addr.s_addr = inet_addr("127.0.0.1");
        sendto(gSock, (const char*)&input, sizeof(input), 0, (sockaddr*)&loop, sizeof(loop));

        sockaddr_in bcast = { 0 };
        bcast.sin_family = AF_INET;
        bcast.sin_port = htons(gPort);
        bcast.sin_addr.s_addr = INADDR_BROADCAST;
        sendto(gSock, (const char*)&input, sizeof(input), 0, (sockaddr*)&bcast, sizeof(bcast));
    }
}

bool LanClientPollState(LanStatePacket& outState) {
    if (gRole != LAN_CLIENT || gSock == INVALID_SOCKET) return false;
    bool got = false;
    sockaddr_in from = { 0 };
    int fromLen = sizeof(from);
    LanStatePacket pkt;
    while (recvfrom(gSock, (char*)&pkt, sizeof(pkt), 0, (sockaddr*)&from, &fromLen) == sizeof(LanStatePacket)) {
        if (pkt.magic == 0x46415254 && pkt.pktType == 2) {
            gPeerAddr = from;
            if (!gHasPeer) {
                gHasPeer = true;
                snprintf(gStatus, sizeof(gStatus), "LAN CLIENT - Connected to Host (%s)", inet_ntoa(from.sin_addr));
            }
            outState = pkt;
            got = true;
        }
    }
    return got;
}

