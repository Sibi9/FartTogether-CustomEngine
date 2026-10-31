#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <string.h>
#include "LanNetwork.h"

#pragma comment(lib, "ws2_32.lib")

static SOCKET   gGameSock      = INVALID_SOCKET;
static SOCKET   gDiscoverySock = INVALID_SOCKET;
static LanRole  gLanRole       = LAN_OFFLINE;
static bool     gWsaReady      = false;
static bool     gPeerConnected = false;
static bool     gLanMatchStart = false;
static sockaddr_in gPeerAddr   = { 0 };
static uint32_t gPacketSeq     = 0;
static float    gBeaconTimer   = 0.0f;
static char     gCurrentRoomCode[8] = "8HF78N";
static uint8_t  gHostCharColor   = 0;
static uint8_t  gClientCharColor = 1;
static uint8_t  gClientReady     = 0;
static char     gStatusText[128] = "LAN Offline";
static std::vector<DiscoveredLanRoom> gDiscoveredRooms;

void LanEnsureWsa() {
    if (!gWsaReady) {
        WSADATA wsa;
        if (WSAStartup(MAKEWORD(2, 2), &wsa) == 0) {
            gWsaReady = true;
        }
    }
}

void LanShutdown() {
    if (gGameSock != INVALID_SOCKET) {
        closesocket(gGameSock);
        gGameSock = INVALID_SOCKET;
    }
    if (gDiscoverySock != INVALID_SOCKET) {
        closesocket(gDiscoverySock);
        gDiscoverySock = INVALID_SOCKET;
    }
    gLanRole = LAN_OFFLINE;
    gPeerConnected = false;
    gLanMatchStart = false;
    gDiscoveredRooms.clear();
    snprintf(gStatusText, sizeof(gStatusText), "LAN Offline");
}

bool LanStartDiscoveryListener(uint16_t discoveryPort) {
    LanEnsureWsa();
    if (gDiscoverySock != INVALID_SOCKET) return true;
    gDiscoverySock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (gDiscoverySock == INVALID_SOCKET) return false;

    u_long nonBlocking = 1;
    ioctlsocket(gDiscoverySock, FIONBIO, &nonBlocking);
    BOOL reuse = TRUE;
    setsockopt(gDiscoverySock, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse));
    BOOL bcast = TRUE;
    setsockopt(gDiscoverySock, SOL_SOCKET, SO_BROADCAST, (const char*)&bcast, sizeof(bcast));

    sockaddr_in bindAddr = { 0 };
    bindAddr.sin_family = AF_INET;
    bindAddr.sin_port   = htons(discoveryPort);
    bindAddr.sin_addr.s_addr = INADDR_ANY;
    bind(gDiscoverySock, (sockaddr*)&bindAddr, sizeof(bindAddr));
    return true;
}

bool LanInitHost(uint16_t gamePort, const char* roomCode) {
    LanShutdown();
    LanEnsureWsa();
    strncpy_s(gCurrentRoomCode, roomCode, 7);

    gGameSock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (gGameSock == INVALID_SOCKET) return false;

    u_long nonBlocking = 1;
    ioctlsocket(gGameSock, FIONBIO, &nonBlocking);
    BOOL bcast = TRUE;
    setsockopt(gGameSock, SOL_SOCKET, SO_BROADCAST, (const char*)&bcast, sizeof(bcast));
    BOOL reuse = TRUE;
    setsockopt(gGameSock, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse));

    sockaddr_in addr = { 0 };
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(gamePort);
    addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(gGameSock, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        closesocket(gGameSock);
        gGameSock = INVALID_SOCKET;
        return false;
    }

    gLanRole = LAN_HOST;
    gPeerConnected = false;
    gLanMatchStart = false;
    snprintf(gStatusText, sizeof(gStatusText), "Hosting Room #%s (Port %d)", gCurrentRoomCode, gamePort);
    return true;
}

bool LanJoinRoom(const char* hostIp, uint16_t hostPort, const char* roomCode) {
    if (gGameSock != INVALID_SOCKET) {
        closesocket(gGameSock);
        gGameSock = INVALID_SOCKET;
    }
    LanEnsureWsa();
    strncpy_s(gCurrentRoomCode, roomCode, 7);

    gGameSock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (gGameSock == INVALID_SOCKET) return false;

    u_long nonBlocking = 1;
    ioctlsocket(gGameSock, FIONBIO, &nonBlocking);
    BOOL bcast = TRUE;
    setsockopt(gGameSock, SOL_SOCKET, SO_BROADCAST, (const char*)&bcast, sizeof(bcast));

    sockaddr_in localAddr = { 0 };
    localAddr.sin_family = AF_INET;
    localAddr.sin_port   = htons(0);
    localAddr.sin_addr.s_addr = INADDR_ANY;
    bind(gGameSock, (sockaddr*)&localAddr, sizeof(localAddr));

    memset(&gPeerAddr, 0, sizeof(gPeerAddr));
    gPeerAddr.sin_family = AF_INET;
    gPeerAddr.sin_port   = htons(hostPort);
    gPeerAddr.sin_addr.s_addr = inet_addr(hostIp);

    gLanRole = LAN_CLIENT;
    gPeerConnected = false;
    gLanMatchStart = false;
    snprintf(gStatusText, sizeof(gStatusText), "Joining %s:%d...", hostIp, hostPort);

    LanLobbyPacket joinPkt = { 0 };
    joinPkt.magic = 0x46415254;
    joinPkt.type  = LAN_PKT_JOIN;
    strncpy_s(joinPkt.roomCode, gCurrentRoomCode, 7);
    joinPkt.clientColor = gClientCharColor;
    sendto(gGameSock, (const char*)&joinPkt, sizeof(joinPkt), 0, (sockaddr*)&gPeerAddr, sizeof(gPeerAddr));
    return true;
}

void LanUpdateDiscoveryAndLobby(float dt) {
    // 1. Host broadcasting beacons every 0.4s to 255.255.255.255:7778 and 127.0.0.1:7778
    if (gLanRole == LAN_HOST && gGameSock != INVALID_SOCKET) {
        gBeaconTimer += dt;
        if (gBeaconTimer >= 0.40f) {
            gBeaconTimer = 0.0f;
            LanBeaconPacket b = { 0 };
            b.magic = 0x46415254;
            b.type  = LAN_PKT_BEACON;
            strncpy_s(b.roomCode, gCurrentRoomCode, 7);
            snprintf(b.roomName, sizeof(b.roomName), "Pirate Room #%s", gCurrentRoomCode);
            b.hostPort    = 7777;
            b.playerCount = gPeerConnected ? 2 : 1;
            b.maxPlayers  = 4;
            b.hostColor   = gHostCharColor;
            b.inGame      = gLanMatchStart ? 1 : 0;

            sockaddr_in bcastAddr = { 0 };
            bcastAddr.sin_family = AF_INET;
            bcastAddr.sin_port   = htons(7778);
            bcastAddr.sin_addr.s_addr = INADDR_BROADCAST;
            sendto(gGameSock, (const char*)&b, sizeof(b), 0, (sockaddr*)&bcastAddr, sizeof(bcastAddr));

            sockaddr_in loopAddr = bcastAddr;
            loopAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
            sendto(gGameSock, (const char*)&b, sizeof(b), 0, (sockaddr*)&loopAddr, sizeof(loopAddr));

            if (gPeerConnected) {
                LanLobbyPacket lp = { 0 };
                lp.magic       = 0x46415254;
                lp.type        = gLanMatchStart ? LAN_PKT_START : LAN_PKT_LOBBY;
                strncpy_s(lp.roomCode, gCurrentRoomCode, 7);
                lp.hostColor   = gHostCharColor;
                lp.clientColor = gClientCharColor;
                lp.clientReady = gClientReady;
                lp.playerCount = 2;
                sendto(gGameSock, (const char*)&lp, sizeof(lp), 0, (sockaddr*)&gPeerAddr, sizeof(gPeerAddr));
            }
        }
    }

    // 2. Client discovery timeout & packet polling on UDP 7778
    for (int i = (int)gDiscoveredRooms.size() - 1; i >= 0; i--) {
        gDiscoveredRooms[i].lastSeenTimer += dt;
        if (gDiscoveredRooms[i].lastSeenTimer > 3.0f) {
            gDiscoveredRooms.erase(gDiscoveredRooms.begin() + i);
        }
    }

    if (gDiscoverySock != INVALID_SOCKET) {
        char buf[256];
        sockaddr_in from = { 0 };
        int fromLen = sizeof(from);
        while (true) {
            int n = recvfrom(gDiscoverySock, buf, sizeof(buf), 0, (sockaddr*)&from, &fromLen);
            if (n < (int)sizeof(LanBeaconPacket)) break;
            const LanBeaconPacket* b = (const LanBeaconPacket*)buf;
            if (b->magic == 0x46415254 && b->type == LAN_PKT_BEACON) {
                char ipStr[64] = "127.0.0.1";
                const char* ntoaRes = inet_ntoa(from.sin_addr);
                if (ntoaRes) strncpy_s(ipStr, ntoaRes, sizeof(ipStr) - 1);

                bool found = false;
                for (auto& r : gDiscoveredRooms) {
                    if (r.roomCode == b->roomCode && r.hostIp == ipStr) {
                        r.playerCount   = b->playerCount;
                        r.hostColor     = b->hostColor;
                        r.lastSeenTimer = 0.0f;
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    DiscoveredLanRoom nr;
                    nr.roomCode      = b->roomCode;
                    nr.roomName      = b->roomName;
                    nr.hostIp        = ipStr;
                    nr.hostPort      = b->hostPort;
                    nr.playerCount   = b->playerCount;
                    nr.maxPlayers    = b->maxPlayers;
                    nr.hostColor     = b->hostColor;
                    nr.lastSeenTimer = 0.0f;
                    gDiscoveredRooms.push_back(nr);
                }
            }
        }
    }

    // 3. Lobby packets handling (Join, Accept, Lobby state, Start)
    if (gGameSock != INVALID_SOCKET && !gLanMatchStart) {
        char buf[256];
        sockaddr_in from = { 0 };
        int fromLen = sizeof(from);
        while (true) {
            int n = recvfrom(gGameSock, buf, sizeof(buf), 0, (sockaddr*)&from, &fromLen);
            if (n <= 0) break;
            if (n >= (int)sizeof(LanLobbyPacket)) {
                const LanLobbyPacket* lp = (const LanLobbyPacket*)buf;
                if (lp->magic != 0x46415254) continue;
                if (gLanRole == LAN_HOST && (lp->type == LAN_PKT_JOIN || lp->type == LAN_PKT_LOBBY)) {
                    gPeerAddr = from;
                    gPeerConnected = true;
                    gClientCharColor = lp->clientColor;
                    gClientReady     = lp->clientReady;
                    snprintf(gStatusText, sizeof(gStatusText), "P2 Connected (%s)", inet_ntoa(from.sin_addr));

                    LanLobbyPacket acc = *lp;
                    acc.type        = LAN_PKT_ACCEPT;
                    acc.hostColor   = gHostCharColor;
                    acc.playerCount = 2;
                    sendto(gGameSock, (const char*)&acc, sizeof(acc), 0, (sockaddr*)&gPeerAddr, sizeof(gPeerAddr));
                } else if (gLanRole == LAN_CLIENT && (lp->type == LAN_PKT_ACCEPT || lp->type == LAN_PKT_LOBBY || lp->type == LAN_PKT_START)) {
                    gPeerConnected = true;
                    gHostCharColor = lp->hostColor;
                    strncpy_s(gCurrentRoomCode, lp->roomCode, 7);
                    snprintf(gStatusText, sizeof(gStatusText), "Connected to Room #%s", gCurrentRoomCode);
                    if (lp->type == LAN_PKT_START) {
                        gLanMatchStart = true;
                    }
                }
            }
        }
    }
}

LanRole LanGetRole() { return gLanRole; }
bool LanIsConnected() { return gPeerConnected; }
const char* LanGetStatusText() { return gStatusText; }
const char* LanGetRoomCode() { return gCurrentRoomCode; }
std::vector<DiscoveredLanRoom>& LanGetDiscoveredRooms() { return gDiscoveredRooms; }
bool LanIsMatchStarted() { return gLanMatchStart; }
void LanSetMatchStarted(bool started) { gLanMatchStart = started; }

bool LanHostPollInput(LanInputPacket& outInput) {
    if (gLanRole != LAN_HOST || gGameSock == INVALID_SOCKET) return false;
    bool got = false;
    sockaddr_in from = { 0 };
    int fromLen = sizeof(from);
    LanInputPacket pkt;
    while (recvfrom(gGameSock, (char*)&pkt, sizeof(pkt), 0, (sockaddr*)&from, &fromLen) == sizeof(LanInputPacket)) {
        if (pkt.magic == 0x46415254 && pkt.type == LAN_PKT_INPUT) {
            gPeerAddr = from;
            gPeerConnected = true;
            outInput = pkt;
            got = true;
        }
    }
    return got;
}

void LanHostBroadcastState(const LanStatePacket& state) {
    if (gLanRole != LAN_HOST || gGameSock == INVALID_SOCKET) return;
    if (gPeerConnected) {
        sendto(gGameSock, (const char*)&state, sizeof(state), 0, (sockaddr*)&gPeerAddr, sizeof(gPeerAddr));
    }
}

void LanClientSendInput(const LanInputPacket& input) {
    if (gLanRole != LAN_CLIENT || gGameSock == INVALID_SOCKET) return;
    if (gPeerConnected) {
        sendto(gGameSock, (const char*)&input, sizeof(input), 0, (sockaddr*)&gPeerAddr, sizeof(gPeerAddr));
    }
}

bool LanClientPollState(LanStatePacket& outState) {
    if (gLanRole != LAN_CLIENT || gGameSock == INVALID_SOCKET) return false;
    bool got = false;
    sockaddr_in from = { 0 };
    int fromLen = sizeof(from);
    LanStatePacket pkt;
    while (recvfrom(gGameSock, (char*)&pkt, sizeof(pkt), 0, (sockaddr*)&from, &fromLen) == sizeof(LanStatePacket)) {
        if (pkt.magic == 0x46415254 && pkt.type == LAN_PKT_STATE) {
            gPeerAddr = from;
            gPeerConnected = true;
            outState = pkt;
            got = true;
        }
    }
    return got;
}
