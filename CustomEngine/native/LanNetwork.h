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
    LAN_PKT_BEACON = 1,
    LAN_PKT_JOIN   = 2,
    LAN_PKT_ACCEPT = 3,
    LAN_PKT_LOBBY  = 4,
    LAN_PKT_START  = 5,
    LAN_PKT_STATE  = 6,
    LAN_PKT_INPUT  = 7
};

#pragma pack(push, 1)
struct LanBeaconPacket {
    uint32_t magic;        // 0x46415254 ("FART")
    uint8_t  type;         // LAN_PKT_BEACON
    char     roomCode[8];  // e.g. "8HF78N"
    char     roomName[32]; // e.g. "Pirate Room #8HF78N"
    uint16_t hostPort;     // 7777
    uint8_t  playerCount;  // 1..4
    uint8_t  maxPlayers;   // 4
    uint8_t  hostColor;    // 0..3
    uint8_t  inGame;       // 0 = In Lobby, 1 = Racing
};

struct LanLobbyPacket {
    uint32_t magic;
    uint8_t  type;         // LAN_PKT_JOIN, LAN_PKT_ACCEPT, LAN_PKT_LOBBY, LAN_PKT_START
    char     roomCode[8];
    uint8_t  hostColor;
    uint8_t  clientColor;
    uint8_t  clientReady;
    uint8_t  gameMode;
    uint8_t  playerCount;
};

struct LanStatePacket {
    uint32_t magic; // 0x46415254
    uint8_t  type;  // LAN_PKT_STATE
    uint32_t seq;
    float    raftX, raftY, raftZ;
    float    velX, velY, velZ;
    float    rotY, pitch, roll;
    float    p1RimAngle, p2RimAngle;
    float    p1Gas, p2Gas;
    float    p1Redness, p2Redness;
    float    p1Swell, p2Swell;
    float    p1Eye, p2Eye;
    float    raceTimer;
    int32_t  totalDeaths;
    int32_t  currentCP;
    uint8_t  isDestroyed;
};

struct LanInputPacket {
    uint32_t magic; // 0x46415254
    uint8_t  type;  // LAN_PKT_INPUT
    uint32_t seq;
    float    rimMove;
    uint8_t  gasDown;
    uint8_t  gasReleased;
    uint8_t  suicideReq;
};
#pragma pack(pop)

struct DiscoveredLanRoom {
    std::string roomCode;
    std::string roomName;
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
bool LanInitHost(uint16_t gamePort = 7777, const char* roomCode = "8HF78N");
bool LanJoinRoom(const char* hostIp, uint16_t hostPort = 7777, const char* roomCode = "8HF78N");
inline bool LanInitClient(uint16_t hostPort = 7777, const char* hostIp = "127.0.0.1") { return LanJoinRoom(hostIp, hostPort); }
void LanUpdateDiscoveryAndLobby(float dt);

LanRole LanGetRole();
bool LanIsConnected();
const char* LanGetStatusText();
const char* LanGetRoomCode();
std::vector<DiscoveredLanRoom>& LanGetDiscoveredRooms();

bool LanHostPollInput(LanInputPacket& outInput);
void LanHostBroadcastState(const LanStatePacket& state);

void LanClientSendInput(const LanInputPacket& input);
bool LanClientPollState(LanStatePacket& outState);
bool LanIsMatchStarted();
void LanSetMatchStarted(bool started);
