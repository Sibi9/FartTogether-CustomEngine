#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
typedef int socklen_t;
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
typedef int SOCKET;
#define INVALID_SOCKET (-1)
#define SOCKET_ERROR (-1)
#define closesocket(s) close(s)
#define ioctlsocket(s, cmd, argp) fcntl(s, F_SETFL, O_NONBLOCK)
#define BOOL int
#define TRUE 1
#define FALSE 0
#define strncpy_s(dst, src, count) do { strncpy((dst), (src), (count)); (dst)[(count)-1] = '\0'; } while(0)
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "LanNetwork.h"

static SOCKET   gGameSock      = INVALID_SOCKET;
static SOCKET   gDiscoverySock = INVALID_SOCKET;
static LanRole  gLanRole       = LAN_OFFLINE;
static bool     gWsaReady      = false;
static bool     gPeerConnected = false;
static bool     gLanMatchStart = false;
static sockaddr_in gPeerAddr   = { 0 };
static uint32_t gPacketSeq     = 0;
static float    gBeaconTimer   = 0.0f;
static float    gProbeTimer    = 0.0f;
static char     gCurrentRoomCode[8] = "A80119";
static uint8_t  gHostCharColor   = 0;
static uint8_t  gClientCharColor = 1;
static char     gStatusText[128] = "LAN Offline";

char gLocalPlayerName[32]  = "Captain 1";
char gRemotePlayerName[32] = "Captain 2";

bool gHostReady = false;
bool gClientReady = false;
int  gConnectedPlayers = 1;
bool gSoloPracticeMode = false;

static std::vector<DiscoveredLanRoom> gDiscoveredRooms;

void LanSetLocalPlayerName(const char* name) {
    if (name && strlen(name) > 0) {
        strncpy_s(gLocalPlayerName, name, sizeof(gLocalPlayerName) - 1);
    }
}
const char* LanGetLocalPlayerName() { return gLocalPlayerName; }
const char* LanGetRemotePlayerName() { return gRemotePlayerName; }

void LanEnsureWsa() {
#ifdef _WIN32
    if (!gWsaReady) {
        WSADATA wsa;
        if (WSAStartup(MAKEWORD(2, 2), &wsa) == 0) {
            gWsaReady = true;
        }
    }
#else
    gWsaReady = true;
#endif
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

#ifdef _WIN32
#ifndef SIO_UDP_CONNRESET
#define SIO_UDP_CONNRESET _WSAIOW(IOC_VENDOR, 12)
#endif
static void SetUdpConnReset(SOCKET s) {
    if (s == INVALID_SOCKET) return;
    BOOL bNewBehavior = FALSE;
    DWORD dwBytesReturned = 0;
    WSAIoctl(s, SIO_UDP_CONNRESET, &bNewBehavior, sizeof(bNewBehavior), NULL, 0, &dwBytesReturned, NULL, NULL);
}
#else
static void SetUdpConnReset(SOCKET s) { (void)s; }
#endif

static uint32_t LanGetLocalIPv4() {
    LanEnsureWsa();
    const char* testTargets[] = { "192.168.1.1", "192.168.0.1", "10.0.0.1", "8.8.8.8", "1.1.1.1" };
    for (const char* tgt : testTargets) {
        SOCKET s = socket(AF_INET, SOCK_DGRAM, 0);
        if (s == INVALID_SOCKET) continue;
        sockaddr_in target;
        memset(&target, 0, sizeof(target));
        target.sin_family = AF_INET;
        target.sin_port = htons(53);
        target.sin_addr.s_addr = inet_addr(tgt);
        connect(s, (sockaddr*)&target, sizeof(target));
        sockaddr_in local;
        memset(&local, 0, sizeof(local));
        socklen_t len = sizeof(local);
        getsockname(s, (sockaddr*)&local, &len);
        closesocket(s);
        uint32_t ip = ntohl(local.sin_addr.s_addr);
        if (ip != 0 && ip != 0x7F000001) return ip;
    }
    return 0x7F000001;
}

void LanGenerateSmartRoomCode(char outCode[8]) {
    uint32_t ip = LanGetLocalIPv4();
    uint8_t b1 = (ip >> 16) & 0xFF;
    uint8_t b2 = (ip >> 8) & 0xFF;
    uint8_t b3 = ip & 0xFF;
    snprintf(outCode, 8, "%02X%02X%02X", b1, b2, b3);
}

bool LanDecodeSmartRoomCode(const char* code, char outIp[64]) {
    if (!code || strlen(code) != 6) return false;
    unsigned int b1 = 0, b2 = 0, b3 = 0;
    if (sscanf(code, "%02x%02x%02x", &b1, &b2, &b3) != 3 &&
        sscanf(code, "%02X%02X%02X", &b1, &b2, &b3) != 3) {
        return false;
    }
    uint32_t myIp = LanGetLocalIPv4();
    uint8_t b0 = (myIp >> 24) & 0xFF;
    if (b0 != 10 && b0 != 172 && b0 != 192) b0 = 192;
    snprintf(outIp, 64, "%u.%u.%u.%u", b0, b1, b2, b3);
    return true;
}

bool LanStartDiscoveryListener(uint16_t discoveryPort) {
    LanEnsureWsa();
    if (gDiscoverySock != INVALID_SOCKET) return true;
    gDiscoverySock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (gDiscoverySock == INVALID_SOCKET) return false;

    SetUdpConnReset(gDiscoverySock);

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
    if (roomCode && strlen(roomCode) > 0) {
        strncpy_s(gCurrentRoomCode, roomCode, 7);
    } else {
        LanGenerateSmartRoomCode(gCurrentRoomCode);
    }

    gGameSock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (gGameSock == INVALID_SOCKET) return false;

    SetUdpConnReset(gGameSock);

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

    // Also open discovery listener on 7778 for Host
    LanStartDiscoveryListener(7778);

    gLanRole = LAN_HOST;
    gPeerConnected = false;
    gLanMatchStart = false;
    gHostReady = false;
    gClientReady = false;
    gConnectedPlayers = 1;
    snprintf(gStatusText, sizeof(gStatusText), "Hosting Room #%s (Port %d)", gCurrentRoomCode, gamePort);
    return true;
}

bool LanJoinRoom(const char* hostIp, uint16_t hostPort, const char* roomCode) {
    if (gGameSock != INVALID_SOCKET) {
        closesocket(gGameSock);
        gGameSock = INVALID_SOCKET;
    }
    LanEnsureWsa();

    char resolvedIp[64] = "127.0.0.1";
    if (hostIp && strlen(hostIp) > 0) {
        if (strchr(hostIp, '.') == nullptr && strlen(hostIp) == 6) {
            if (LanDecodeSmartRoomCode(hostIp, resolvedIp)) {
                if (!roomCode || strlen(roomCode) == 0) roomCode = hostIp;
            } else {
                strncpy_s(resolvedIp, hostIp, sizeof(resolvedIp) - 1);
            }
        } else {
            strncpy_s(resolvedIp, hostIp, sizeof(resolvedIp) - 1);
        }
    }

    if (roomCode && strlen(roomCode) > 0) {
        strncpy_s(gCurrentRoomCode, roomCode, 7);
    }

    gGameSock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (gGameSock == INVALID_SOCKET) return false;

    SetUdpConnReset(gGameSock);

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
    gPeerAddr.sin_addr.s_addr = inet_addr(resolvedIp);

    gLanRole = LAN_CLIENT;
    gPeerConnected = false;
    gLanMatchStart = false;
    gHostReady = false;
    gClientReady = false;
    snprintf(gStatusText, sizeof(gStatusText), "Joining %s:%d...", resolvedIp, hostPort);

    LanLobbyPacket joinPkt = { 0 };
    joinPkt.magic = 0x46415254;
    joinPkt.type  = LAN_PKT_JOIN;
    strncpy_s(joinPkt.roomCode, gCurrentRoomCode, 7);
    strncpy_s(joinPkt.playerName, gLocalPlayerName, sizeof(joinPkt.playerName) - 1);
    joinPkt.clientColor = gClientCharColor;
    sendto(gGameSock, (const char*)&joinPkt, sizeof(joinPkt), 0, (sockaddr*)&gPeerAddr, sizeof(gPeerAddr));
    return true;
}

uint8_t LanGetHostColor() { return gHostCharColor; }
uint8_t LanGetClientColor() { return gClientCharColor; }
void LanSetHostColor(uint8_t color) { gHostCharColor = color; }
void LanSetClientColor(uint8_t color) { gClientCharColor = color; }

void LanSendStartSignal() {
    if (gLanRole == LAN_HOST && gGameSock != INVALID_SOCKET && gPeerConnected) {
        LanLobbyPacket startPkt = { 0 };
        startPkt.magic       = 0x46415254;
        startPkt.type        = LAN_PKT_START;
        strncpy_s(startPkt.roomCode, gCurrentRoomCode, 7);
        strncpy_s(startPkt.playerName, gLocalPlayerName, sizeof(startPkt.playerName) - 1);
        startPkt.hostColor   = gHostCharColor;
        startPkt.clientColor = gClientCharColor;
        startPkt.clientReady = 1;
        startPkt.playerCount = 2;
        for (int b = 0; b < 6; b++) {
            sendto(gGameSock, (const char*)&startPkt, sizeof(startPkt), 0, (sockaddr*)&gPeerAddr, sizeof(gPeerAddr));
        }
    }
}

void LanClientSendLobbyUpdate(bool ready, uint8_t color) {
    gClientReady = ready;
    gClientCharColor = color;
    if (gLanRole == LAN_CLIENT && gGameSock != INVALID_SOCKET) {
        LanLobbyPacket lp = { 0 };
        lp.magic       = 0x46415254;
        lp.type        = gPeerConnected ? LAN_PKT_LOBBY : LAN_PKT_JOIN;
        strncpy_s(lp.roomCode, gCurrentRoomCode, 7);
        strncpy_s(lp.playerName, gLocalPlayerName, sizeof(lp.playerName) - 1);
        lp.clientColor = color;
        lp.clientReady = ready ? 1 : 0;
        lp.playerCount = gConnectedPlayers;
        sendto(gGameSock, (const char*)&lp, sizeof(lp), 0, (sockaddr*)&gPeerAddr, sizeof(gPeerAddr));
    }
}

void LanUpdateDiscoveryAndLobby(float dt) {
    uint32_t myIp = LanGetLocalIPv4();
    uint32_t subnetBcast = (myIp & 0xFFFFFF00) | 0x000000FF;

    // 1. Host broadcasting beacons every 0.25s
    if (gLanRole == LAN_HOST && gGameSock != INVALID_SOCKET) {
        gBeaconTimer += dt;
        if (gBeaconTimer >= 0.25f) {
            gBeaconTimer = 0.0f;
            LanBeaconPacket b = { 0 };
            b.magic = 0x46415254;
            b.type  = LAN_PKT_BEACON;
            strncpy_s(b.roomCode, gCurrentRoomCode, 7);
            snprintf(b.roomName, sizeof(b.roomName), "Pirate Room #%s", gCurrentRoomCode);
            strncpy_s(b.hostName, gLocalPlayerName, sizeof(b.hostName) - 1);
            b.hostPort    = 7777;
            b.playerCount = gPeerConnected ? 2 : 1;
            b.maxPlayers  = 4;
            b.hostColor   = gHostCharColor;
            b.inGame      = gLanMatchStart ? 1 : 0;

            // Global broadcast 255.255.255.255 to 7778 and 7777
            sockaddr_in bcastAddr = { 0 };
            bcastAddr.sin_family = AF_INET;
            bcastAddr.sin_port   = htons(7778);
            bcastAddr.sin_addr.s_addr = INADDR_BROADCAST;
            sendto(gGameSock, (const char*)&b, sizeof(b), 0, (sockaddr*)&bcastAddr, sizeof(bcastAddr));

            bcastAddr.sin_port = htons(7777);
            sendto(gGameSock, (const char*)&b, sizeof(b), 0, (sockaddr*)&bcastAddr, sizeof(bcastAddr));

            // Subnet-directed broadcast x.x.x.255 to 7778 and 7777
            sockaddr_in subAddr = bcastAddr;
            subAddr.sin_addr.s_addr = htonl(subnetBcast);
            subAddr.sin_port = htons(7778);
            sendto(gGameSock, (const char*)&b, sizeof(b), 0, (sockaddr*)&subAddr, sizeof(subAddr));
            subAddr.sin_port = htons(7777);
            sendto(gGameSock, (const char*)&b, sizeof(b), 0, (sockaddr*)&subAddr, sizeof(subAddr));

            // Loopback 127.0.0.1:7778
            sockaddr_in loopAddr = bcastAddr;
            loopAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
            loopAddr.sin_port = htons(7778);
            sendto(gGameSock, (const char*)&b, sizeof(b), 0, (sockaddr*)&loopAddr, sizeof(loopAddr));

            if (gPeerConnected) {
                LanLobbyPacket lp = { 0 };
                lp.magic       = 0x46415254;
                lp.type        = gLanMatchStart ? LAN_PKT_START : LAN_PKT_LOBBY;
                strncpy_s(lp.roomCode, gCurrentRoomCode, 7);
                strncpy_s(lp.playerName, gLocalPlayerName, sizeof(lp.playerName) - 1);
                lp.hostColor   = gHostCharColor;
                lp.clientColor = gClientCharColor;
                lp.clientReady = gClientReady ? 1 : 0;
                lp.playerCount = 2;
                sendto(gGameSock, (const char*)&lp, sizeof(lp), 0, (sockaddr*)&gPeerAddr, sizeof(gPeerAddr));
            }
        }
    }

    // 2. Client sending periodic heartbeat / retry join in lobby every 0.25s
    if (gLanRole == LAN_CLIENT && gGameSock != INVALID_SOCKET) {
        static float clientSyncTimer = 0.0f;
        clientSyncTimer += dt;
        if (clientSyncTimer >= 0.25f || !gPeerConnected) {
            clientSyncTimer = 0.0f;
            LanLobbyPacket lp = { 0 };
            lp.magic       = 0x46415254;
            lp.type        = gPeerConnected ? LAN_PKT_LOBBY : LAN_PKT_JOIN;
            strncpy_s(lp.roomCode, gCurrentRoomCode, 7);
            strncpy_s(lp.playerName, gLocalPlayerName, sizeof(lp.playerName) - 1);
            lp.clientColor = gClientCharColor;
            lp.clientReady = gClientReady ? 1 : 0;
            lp.playerCount = gConnectedPlayers;
            sendto(gGameSock, (const char*)&lp, sizeof(lp), 0, (sockaddr*)&gPeerAddr, sizeof(gPeerAddr));
        }
    }

    // 3. Client sending active discovery probe requests every 0.5s when discovering
    if (gLanRole == LAN_OFFLINE && gDiscoverySock != INVALID_SOCKET) {
        gProbeTimer += dt;
        if (gProbeTimer >= 0.5f) {
            gProbeTimer = 0.0f;
            LanBeaconPacket probe = { 0 };
            probe.magic = 0x46415254;
            probe.type  = LAN_PKT_DISCOVER_REQ;
            strncpy_s(probe.hostName, gLocalPlayerName, sizeof(probe.hostName) - 1);

            // Broadcast to 7777 and 7778
            sockaddr_in bcastAddr = { 0 };
            bcastAddr.sin_family = AF_INET;
            bcastAddr.sin_port   = htons(7778);
            bcastAddr.sin_addr.s_addr = INADDR_BROADCAST;
            sendto(gDiscoverySock, (const char*)&probe, sizeof(probe), 0, (sockaddr*)&bcastAddr, sizeof(bcastAddr));

            bcastAddr.sin_port = htons(7777);
            sendto(gDiscoverySock, (const char*)&probe, sizeof(probe), 0, (sockaddr*)&bcastAddr, sizeof(bcastAddr));

            // Subnet-directed broadcast to 7777 and 7778
            sockaddr_in subAddr = bcastAddr;
            subAddr.sin_addr.s_addr = htonl(subnetBcast);
            subAddr.sin_port = htons(7778);
            sendto(gDiscoverySock, (const char*)&probe, sizeof(probe), 0, (sockaddr*)&subAddr, sizeof(subAddr));
            subAddr.sin_port = htons(7777);
            sendto(gDiscoverySock, (const char*)&probe, sizeof(probe), 0, (sockaddr*)&subAddr, sizeof(subAddr));

            // Subnet Unicast Sweep across 1..254 to port 7777 and 7778 (guaranteed against router broadcast block)
            uint32_t baseSubnet = myIp & 0xFFFFFF00;
            for (uint32_t i = 1; i <= 254; i++) {
                uint32_t targetIp = baseSubnet | i;
                if (targetIp == myIp) continue;
                sockaddr_in sweepAddr = { 0 };
                sweepAddr.sin_family = AF_INET;
                sweepAddr.sin_addr.s_addr = htonl(targetIp);
                sweepAddr.sin_port = htons(7777);
                sendto(gDiscoverySock, (const char*)&probe, sizeof(probe), 0, (sockaddr*)&sweepAddr, sizeof(sweepAddr));
                sweepAddr.sin_port = htons(7778);
                sendto(gDiscoverySock, (const char*)&probe, sizeof(probe), 0, (sockaddr*)&sweepAddr, sizeof(sweepAddr));
            }
        }
    }

    // 4. Discovery room cleanup
    for (int i = (int)gDiscoveredRooms.size() - 1; i >= 0; i--) {
        gDiscoveredRooms[i].lastSeenTimer += dt;
        if (gDiscoveredRooms[i].lastSeenTimer > 3.0f) {
            gDiscoveredRooms.erase(gDiscoveredRooms.begin() + i);
        }
    }

    // 5. Poll Discovery Socket (UDP 7778)
    if (gDiscoverySock != INVALID_SOCKET) {
        char buf[512];
        sockaddr_in from = { 0 };
        socklen_t fromLen = sizeof(from);
        while (true) {
            fromLen = sizeof(from);
            int n = recvfrom(gDiscoverySock, buf, sizeof(buf), 0, (sockaddr*)&from, &fromLen);
            if (n <= 0) break;
            if (n < 5) continue;

            uint32_t magic = 0;
            memcpy(&magic, buf, 4);
            if (magic != 0x46415254) continue;
            uint8_t type = (uint8_t)buf[4];

            if (type == LAN_PKT_DISCOVER_REQ && gLanRole == LAN_HOST) {
                // Host replies directly via unicast to requester
                LanBeaconPacket reply = { 0 };
                reply.magic = 0x46415254;
                reply.type  = LAN_PKT_BEACON;
                strncpy_s(reply.roomCode, gCurrentRoomCode, 7);
                snprintf(reply.roomName, sizeof(reply.roomName), "Pirate Room #%s", gCurrentRoomCode);
                strncpy_s(reply.hostName, gLocalPlayerName, sizeof(reply.hostName) - 1);
                reply.hostPort    = 7777;
                reply.playerCount = gPeerConnected ? 2 : 1;
                reply.maxPlayers  = 4;
                reply.hostColor   = gHostCharColor;
                reply.inGame      = gLanMatchStart ? 1 : 0;
                sendto(gDiscoverySock, (const char*)&reply, sizeof(reply), 0, (sockaddr*)&from, fromLen);
                if (gGameSock != INVALID_SOCKET) {
                    sendto(gGameSock, (const char*)&reply, sizeof(reply), 0, (sockaddr*)&from, fromLen);
                }
            } else if (type == LAN_PKT_BEACON) {
                char ipStr[64] = "127.0.0.1";
                const char* ntoaRes = inet_ntoa(from.sin_addr);
                if (ntoaRes) strncpy_s(ipStr, ntoaRes, sizeof(ipStr) - 1);

                char rCode[8] = { 0 };
                char rName[32] = { 0 };
                char hName[32] = "Host";
                uint16_t hPort = 7777;
                uint8_t pCount = 1, maxP = 4, hCol = 0;

                if (n >= (int)sizeof(LanBeaconPacket)) {
                    const LanBeaconPacket* bp = (const LanBeaconPacket*)buf;
                    strncpy_s(rCode, bp->roomCode, 7);
                    strncpy_s(rName, bp->roomName, sizeof(rName) - 1);
                    strncpy_s(hName, bp->hostName, sizeof(hName) - 1);
                    hPort  = bp->hostPort;
                    pCount = bp->playerCount;
                    maxP   = bp->maxPlayers;
                    hCol   = bp->hostColor;
                } else if (n >= 51) {
                    const LanBeaconPacketV830* bp = (const LanBeaconPacketV830*)buf;
                    strncpy_s(rCode, bp->roomCode, 7);
                    strncpy_s(rName, bp->roomName, sizeof(rName) - 1);
                    hPort  = bp->hostPort;
                    pCount = bp->playerCount;
                    maxP   = bp->maxPlayers;
                    hCol   = bp->hostColor;
                    snprintf(hName, sizeof(hName), "Host_%s", rCode);
                }

                if (strlen(hName) == 0) strncpy_s(hName, "Host", sizeof(hName) - 1);

                bool found = false;
                for (auto& r : gDiscoveredRooms) {
                    if (r.roomCode == rCode && r.hostIp == ipStr) {
                        r.playerCount   = pCount;
                        r.hostColor     = hCol;
                        r.hostName      = hName;
                        r.lastSeenTimer = 0.0f;
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    DiscoveredLanRoom nr;
                    nr.roomCode      = rCode;
                    nr.roomName      = rName;
                    nr.hostName      = hName;
                    nr.hostIp        = ipStr;
                    nr.hostPort      = hPort;
                    nr.playerCount   = pCount;
                    nr.maxPlayers    = maxP;
                    nr.hostColor     = hCol;
                    nr.lastSeenTimer = 0.0f;
                    gDiscoveredRooms.push_back(nr);
                }
            }
        }
    }

    // 6. Lobby packets & Discovery on Game Socket (UDP 7777)
    if (gGameSock != INVALID_SOCKET) {
        char buf[512];
        sockaddr_in from = { 0 };
        socklen_t fromLen = sizeof(from);
        while (true) {
            fromLen = sizeof(from);
            int n = recvfrom(gGameSock, buf, sizeof(buf), 0, (sockaddr*)&from, &fromLen);
            if (n <= 0) break;
            if (n < 5) continue;

            uint32_t magic = 0;
            memcpy(&magic, buf, 4);
            if (magic != 0x46415254) continue;
            uint8_t type = (uint8_t)buf[4];

            // 6a. Check if it's a discovery probe on 7777
            if (type == LAN_PKT_DISCOVER_REQ && gLanRole == LAN_HOST) {
                LanBeaconPacket reply = { 0 };
                reply.magic = 0x46415254;
                reply.type  = LAN_PKT_BEACON;
                strncpy_s(reply.roomCode, gCurrentRoomCode, 7);
                snprintf(reply.roomName, sizeof(reply.roomName), "Pirate Room #%s", gCurrentRoomCode);
                strncpy_s(reply.hostName, gLocalPlayerName, sizeof(reply.hostName) - 1);
                reply.hostPort    = 7777;
                reply.playerCount = gPeerConnected ? 2 : 1;
                reply.maxPlayers  = 4;
                reply.hostColor   = gHostCharColor;
                reply.inGame      = gLanMatchStart ? 1 : 0;
                sendto(gGameSock, (const char*)&reply, sizeof(reply), 0, (sockaddr*)&from, fromLen);
                continue;
            }

            // 6b. Check if it's a beacon received on 7777 in offline discovery mode
            if (type == LAN_PKT_BEACON && gLanRole == LAN_OFFLINE) {
                char ipStr[64] = "127.0.0.1";
                const char* ntoaRes = inet_ntoa(from.sin_addr);
                if (ntoaRes) strncpy_s(ipStr, ntoaRes, sizeof(ipStr) - 1);

                char rCode[8] = { 0 };
                char rName[32] = { 0 };
                char hName[32] = "Host";
                uint16_t hPort = 7777;
                uint8_t pCount = 1, maxP = 4, hCol = 0;

                if (n >= (int)sizeof(LanBeaconPacket)) {
                    const LanBeaconPacket* bp = (const LanBeaconPacket*)buf;
                    strncpy_s(rCode, bp->roomCode, 7);
                    strncpy_s(rName, bp->roomName, sizeof(rName) - 1);
                    strncpy_s(hName, bp->hostName, sizeof(hName) - 1);
                    hPort  = bp->hostPort;
                    pCount = bp->playerCount;
                    maxP   = bp->maxPlayers;
                    hCol   = bp->hostColor;
                } else if (n >= 51) {
                    const LanBeaconPacketV830* bp = (const LanBeaconPacketV830*)buf;
                    strncpy_s(rCode, bp->roomCode, 7);
                    strncpy_s(rName, bp->roomName, sizeof(rName) - 1);
                    hPort  = bp->hostPort;
                    pCount = bp->playerCount;
                    maxP   = bp->maxPlayers;
                    hCol   = bp->hostColor;
                    snprintf(hName, sizeof(hName), "Host_%s", rCode);
                }

                if (strlen(hName) == 0) strncpy_s(hName, "Host", sizeof(hName) - 1);

                bool found = false;
                for (auto& r : gDiscoveredRooms) {
                    if (r.roomCode == rCode && r.hostIp == ipStr) {
                        r.playerCount   = pCount;
                        r.hostColor     = hCol;
                        r.hostName      = hName;
                        r.lastSeenTimer = 0.0f;
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    DiscoveredLanRoom nr;
                    nr.roomCode      = rCode;
                    nr.roomName      = rName;
                    nr.hostName      = hName;
                    nr.hostIp        = ipStr;
                    nr.hostPort      = hPort;
                    nr.playerCount   = pCount;
                    nr.maxPlayers    = maxP;
                    nr.hostColor     = hCol;
                    nr.lastSeenTimer = 0.0f;
                    gDiscoveredRooms.push_back(nr);
                }
                continue;
            }

            // 6c. Lobby packets (Join, Accept, Lobby state, Start)
            if (n >= 45 && !gLanMatchStart) {
                const LanLobbyPacket* lp = (const LanLobbyPacket*)buf;
                if (gLanRole == LAN_HOST && (lp->type == LAN_PKT_JOIN || lp->type == LAN_PKT_LOBBY)) {
                    gPeerAddr = from;
                    gPeerConnected = true;
                    gClientCharColor = lp->clientColor;
                    gClientReady = (lp->clientReady != 0);
                    gConnectedPlayers = 2;
                    if (strlen(lp->playerName) > 0) {
                        strncpy_s(gRemotePlayerName, lp->playerName, sizeof(gRemotePlayerName) - 1);
                    }

                    LanLobbyPacket acceptPkt = { 0 };
                    acceptPkt.magic       = 0x46415254;
                    acceptPkt.type        = LAN_PKT_ACCEPT;
                    strncpy_s(acceptPkt.roomCode, gCurrentRoomCode, 7);
                    strncpy_s(acceptPkt.playerName, gLocalPlayerName, sizeof(acceptPkt.playerName) - 1);
                    acceptPkt.hostColor   = gHostCharColor;
                    acceptPkt.clientColor = gClientCharColor;
                    acceptPkt.clientReady = gClientReady ? 1 : 0;
                    acceptPkt.playerCount = 2;
                    sendto(gGameSock, (const char*)&acceptPkt, sizeof(acceptPkt), 0, (sockaddr*)&gPeerAddr, sizeof(gPeerAddr));
                    snprintf(gStatusText, sizeof(gStatusText), "%s joined! (Room #%s)", gRemotePlayerName, gCurrentRoomCode);
                } else if (gLanRole == LAN_CLIENT) {
                    if (lp->type == LAN_PKT_ACCEPT || lp->type == LAN_PKT_LOBBY) {
                        gPeerConnected = true;
                        gHostCharColor = lp->hostColor;
                        gConnectedPlayers = lp->playerCount;
                        if (strlen(lp->playerName) > 0) {
                            strncpy_s(gRemotePlayerName, lp->playerName, sizeof(gRemotePlayerName) - 1);
                        }
                        snprintf(gStatusText, sizeof(gStatusText), "Connected to %s (#%s)", gRemotePlayerName, gCurrentRoomCode);
                    } else if (lp->type == LAN_PKT_START) {
                        gLanMatchStart = true;
                        snprintf(gStatusText, sizeof(gStatusText), "Race Started!");
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
    socklen_t fromLen = sizeof(from);
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
    socklen_t fromLen = sizeof(from);
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
