import re, pathlib

print("=== Patching Engine to v8.1.0 ===")

# 1. Update LanNetwork.h & LanNet.h
lan_h_path = pathlib.Path("CustomEngine/native/LanNetwork.h")
lan_content = lan_h_path.read_text(encoding="utf-8")

if "gHostReady" not in lan_content:
    addition = """
extern bool gHostReady;
extern bool gClientReady;
extern int  gConnectedPlayers;
extern bool gSoloPracticeMode;
inline bool LanInitClient(uint16_t hostPort = 7777, const char* hostIp = "127.0.0.1") { return LanJoinRoom(hostIp, hostPort); }
"""
    lan_content = lan_content.replace("std::vector<DiscoveredLanRoom>& LanGetDiscoveredRooms();",
                                      "std::vector<DiscoveredLanRoom>& LanGetDiscoveredRooms();\n" + addition)
    lan_h_path.write_text(lan_content, encoding="utf-8")
    pathlib.Path("CustomEngine/native/LanNet.h").write_text(lan_content, encoding="utf-8")
    print("Updated LanNetwork.h and LanNet.h with ready flags")

# 2. Update LanNetwork.cpp
lan_cpp_path = pathlib.Path("CustomEngine/native/LanNetwork.cpp")
lan_cpp = lan_cpp_path.read_text(encoding="utf-8")

if "bool gHostReady" not in lan_cpp:
    vars_code = """
bool gHostReady = false;
bool gClientReady = false;
int  gConnectedPlayers = 1;
bool gSoloPracticeMode = false;
"""
    lan_cpp = lan_cpp.replace('static char     gStatusText[128] = "LAN Offline";',
                              'static char     gStatusText[128] = "LAN Offline";\n' + vars_code)
    lan_cpp_path.write_text(lan_cpp, encoding="utf-8")
    print("Updated LanNetwork.cpp with ready variables")

print("LAN files patched successfully!")
