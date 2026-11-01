#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <stdint.h>
#include <vector>
#include <string>

enum LanRole {
    LAN_OFFLINE = 0,
    LAN_HOST    = 1,
    LAN_CLIENT  = 2
};

enum LanPacketType : uint8_t {
    LAN_PKT_BEACON       = 1,
    LAN_PKT_JOIN         = 2,
    LAN_PKT_ACCEPT       = 3,
    LAN_PKT_LOBBY        = 4,
    LAN_PKT_START        = 5,
    LAN_PKT_STATE        = 6,
    LAN_PKT_INPUT        = 7,
    LAN_PKT_DISCOVER_REQ = 8
};

#pragma pack(push, 1)
struct LanBeaconPacketV830 {
    uint32_t magic;        // 0x46415254 ("FART")
    uint8_t  type;         // LAN_PKT_BEACON
    char     roomCode[8];
    char     roomName[32];
    uint16_t hostPort;
    uint8_t  playerCount;
    uint8_t  maxPlayers;
    uint8_t  hostColor;
    uint8_t  inGame;
};

struct LanBeaconPacket {
    uint32_t magic;        // 0x46415254 ("FART")
    uint8_t  type;         // LAN_PKT_BEACON
    char     roomCode[8];
    char     roomName[32];
    char     hostName[32];
    uint16_t hostPort;
    uint8_t  playerCount;
    uint8_t  maxPlayers;
    uint8_t  hostColor;
    uint8_t  inGame;
};

struct LanLobbyPacket {
    uint32_t magic;        // 0x46415254
    uint8_t  type;         // LAN_PKT_JOIN, LAN_PKT_ACCEPT, LAN_PKT_LOBBY, LAN_PKT_START
    char     roomCode[8];
    char     playerName[32];
    uint8_t  hostColor;
    uint8_t  clientColor;
    uint8_t  hostReady;
    uint8_t  clientReady;
    uint8_t  gameMode;
    uint8_t  playerCount;
};

struct LanStatePacket {
    uint32_t magic;        // 0x46415254
    uint8_t  type;         // LAN_PKT_STATE
    uint32_t seq;
    float    raftX, raftY, raftZ;
    float    velX, velY, velZ;
    float    rotY, angVelY, pitch, roll;
    float    p1RimAngle, p2RimAngle;
    float    p1Energy, p2Energy;
    float    p1Charge, p2Charge;
    float    p1Redness, p2Redness;
    float    p1Swell, p2Swell;
    float    p1Eye, p2Eye;
    uint8_t  p1Sustained, p2Sustained;
    uint8_t  p1Depleted, p2Depleted;
    uint16_t p1TapSeq, p2TapSeq;
    float    raceTimer;
    int32_t  totalDeaths;
    int32_t  currentCP;
    uint8_t  isDestroyed;
};

struct LanInputPacket {
    uint32_t magic;        // 0x46415254
    uint8_t  type;         // LAN_PKT_INPUT
    uint32_t seq;
    float    rimMove;      // -1.0 .. +1.0
    float    clientRimAngle;
    uint8_t  gasDown;      // 1 while holding gas
    uint8_t  gasReleased;  // 1 on release edge
    uint16_t tapBurstSeq;  // Monotonic counter incremented on every quick tap burst!
    uint16_t suicideSeq;   // Monotonic counter incremented on suicide press!
};
#pragma pack(pop)

struct DiscoveredLanRoom {
    std::string roomCode;
    std::string roomName;
    std::string hostName;
    std::string hostIp;
    uint16_t    hostPort;
    uint8_t     playerCount;
    uint8_t     maxPlayers;
    uint8_t     hostColor;
    float       lastSeenTimer;
};

void LanEnsureWsa();
void LanShutdown();
bool LanStartDiscoveryListener(uint16_t discoveryPort = 7778);
bool LanInitHost(uint16_t gamePort = 7777, const char* roomCode = nullptr);
bool LanJoinRoom(const char* hostIp, uint16_t hostPort = 7777, const char* roomCode = nullptr);
void LanUpdateDiscoveryAndLobby(float dt);

LanRole LanGetRole();
bool LanIsConnected();
const char* LanGetStatusText();
const char* LanGetRoomCode();
std::vector<DiscoveredLanRoom>& LanGetDiscoveredRooms();

void LanGenerateSmartRoomCode(char outCode[8]);
bool LanDecodeSmartRoomCode(const char* code, char outIp[64]);

extern bool gHostReady;
extern bool gClientReady;
extern int  gConnectedPlayers;
extern bool gSoloPracticeMode;
inline bool LanInitClient(uint16_t hostPort = 7777, const char* hostIp = "127.0.0.1") { return LanJoinRoom(hostIp, hostPort); }

bool LanHostPollInput(LanInputPacket& outInput);
void LanHostBroadcastState(const LanStatePacket& state);

void LanClientSendInput(const LanInputPacket& input);
bool LanClientPollState(LanStatePacket& outState);
bool LanIsMatchStarted();
void LanSetMatchStarted(bool started);

// Player Names & Lobby Sync
extern char gLocalPlayerName[32];
extern char gRemotePlayerName[32];
void LanSetLocalPlayerName(const char* name);
const char* LanGetLocalPlayerName();
const char* LanGetRemotePlayerName();

void LanSendStartSignal();
void LanClientSendLobbyUpdate(bool ready, uint8_t color);
uint8_t LanGetHostColor();
uint8_t LanGetClientColor();
void LanSetHostColor(uint8_t color);
void LanSetClientColor(uint8_t color);
