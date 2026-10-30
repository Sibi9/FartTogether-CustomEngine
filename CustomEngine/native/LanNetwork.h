#pragma once
#include <stdint.h>

enum LanRole {
    LAN_OFFLINE = 0,
    LAN_HOST    = 1,
    LAN_CLIENT  = 2
};

#pragma pack(push, 1)
struct LanInputPacket {
    uint32_t magic; // 0x46415254 ('FART')
    uint8_t  pktType; // 1 = Client Input
    float    p2RimAngle;
    uint8_t  p2InputDown;
    uint8_t  p2InputReleased;
    uint8_t  p2SuicideReq;
};

struct LanStatePacket {
    uint32_t magic; // 0x46415254 ('FART')
    uint8_t  pktType; // 2 = Host World State
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
#pragma pack(pop)

bool LanInitHost(uint16_t port = 7777);
bool LanInitClient(uint16_t port = 7777);
void LanShutdown();
LanRole LanGetRole();
bool LanIsConnected();
const char* LanGetStatusText();

// يستدعى لدى المضيف (Host)
bool LanHostPollInput(LanInputPacket& outInput);
void LanHostBroadcastState(const LanStatePacket& state);

// يستدعى لدى العميل (Client)
void LanClientSendInput(const LanInputPacket& input);
bool LanClientPollState(LanStatePacket& outState);

