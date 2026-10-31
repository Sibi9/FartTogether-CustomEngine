#pragma once
#include <string>

enum GameLanguage {
    LANG_AR = 0, // Arabic is Default
    LANG_EN = 1  // English
};

extern GameLanguage gCurrentLanguage;

struct LocalizedString {
    const char* ar;
    const char* en;
};

inline const char* LocText(const LocalizedString& ls) {
    return (gCurrentLanguage == LANG_AR) ? ls.ar : ls.en;
}
inline const LocalizedString STR_PLAY = { (const char*)u8"ﺐﻌﻠﻟﺍ ﺃﺪﺑﺍ", "PLAY" };
inline const LocalizedString STR_CREATE_LOBBY = { (const char*)u8"ﺔﻓﺮﻏ ﺀﺎﺸﻧﺇ", "CREATE LOBBY" };
inline const LocalizedString STR_FIND_LOBBY = { (const char*)u8"ﺔﻓﺮﻏ ﻦﻋ ﺚﺤﺒﻟﺍ", "FIND LOBBY" };
inline const LocalizedString STR_SETTINGS = { (const char*)u8"ﺕﺍﺩﺍﺪﻋﻹﺍ", "SETTINGS" };
inline const LocalizedString STR_EXIT_GAME = { (const char*)u8"ﺝﻭﺮﺧ", "EXIT GAME" };
inline const LocalizedString STR_ESC_BACK = { (const char*)u8"[ESC] ﻉﻮﺟﺭ", "[ESC] BACK" };
inline const LocalizedString STR_ENTER_SELECT = { (const char*)u8"[ENTER] ﺭﺎﻴﺘﺧﺍ", "[ENTER] SELECT" };
inline const LocalizedString STR_LOBBY_TITLE = { (const char*)u8"ﺭﺎﻈﺘﻧﻻﺍ ﺔﻓﺮﻏ", "LOBBY" };
inline const LocalizedString STR_YOUR_CODE = { (const char*)u8"ﻚﺑ ﺹﺎﺨﻟﺍ ﺔﻓﺮﻐﻟﺍ ﺰﻣﺭ", "YOUR LOBBY CODE" };
inline const LocalizedString STR_SELECT_MODE = { (const char*)u8"ﺐﻌﻠﻟﺍ ﻂﻤﻧ ﺮﺘﺧﺍ", "SELECT MODE" };
inline const LocalizedString STR_INVITE_FRIENDS = { (const char*)u8"ﺀﺎﻗﺪﺻﻷﺍ ﺓﻮﻋﺩ", "INVITE FRIENDS" };
inline const LocalizedString STR_ESC_LEAVE = { (const char*)u8"[ESC] ﺓﺭﺩﺎﻐﻣ", "[ESC] LEAVE" };
inline const LocalizedString STR_SELECT_CHAR = { (const char*)u8"[A / D] ﺔﻴﺼﺨﺸﻟﺍ ﺭﺎﻴﺘﺧﺍ", "[A / D] SELECT CHARACTER" };
inline const LocalizedString STR_READY_BTN = { (const char*)u8"[SPACE] ﺰﻫﺎﺟ", "[SPACE] READY" };
inline const LocalizedString STR_NOT_READY_BTN = { (const char*)u8"[SPACE] ﺰﻫﺎﺟ ﺮﻴﻏ", "[SPACE] NOT READY" };
inline const LocalizedString STR_WAITING_STATUS = { (const char*)u8"[...ﺭﺎﻈﺘﻧﻻﺍ ﻲﻓ]", "[WAITING...]" };
inline const LocalizedString STR_READY_STATUS = { (const char*)u8"[ ﺰﻫﺎﺟ]", "[READY ]" };
inline const LocalizedString STR_MIN_PLAYERS_ALERT = { (const char*)u8"!ﺐﻌﻠﻟﺍ ﺀﺪﺒﻟ ﺰﻫﺎﺟ ﻰﻠﻋ ﻊﻴﻤﺠﻟﺍ ﻂﻐﺿﻭ ﻦﻴﻨﺛﺍ ﻦﻴﺒﻋﻻ ﺩﻮﺟﻭ ﻡﺰﻠﻳ", "Need at least 2 players & all READY to start!" };
inline const LocalizedString STR_SOLO_TEST = { (const char*)u8"[ﺔﻳﺩﺮﻓ ﺔﺑﺮﺠﺗ]", "[Solo Practice]" };
inline const LocalizedString STR_FIND_TITLE = { (const char*)u8"ﺔﻴﻠﺤﻤﻟﺍ ﺔﻜﺒﺸﻟﺍ ﻰﻠﻋ ﺔﺣﺎﺘﻤﻟﺍ ﻑﺮﻐﻟﺍ", "FIND LOBBY / AVAILABLE LAN ROOMS" };
inline const LocalizedString STR_ROOM_CODE_LBL = { (const char*)u8":ﺔﻓﺮﻐﻟﺍ ﺰﻣﺭ", "Room Code:" };
inline const LocalizedString STR_JOIN_BY_CODE = { (const char*)u8"ﺰﻣﺮﻟﺎﺑ ﻢﻀﻧﺍ", "JOIN BY CODE" };
inline const LocalizedString STR_COOP_RACE = { (const char*)u8"ﻲﻧﻭﺎﻌﺗ ﻕﺎﺒﺳ", "FART RUN CO-OP" };
inline const LocalizedString STR_PLAYERS_COUNT = { (const char*)u8"ﻦﻴﺒﻋﻻ", "PLAYERS" };
inline const LocalizedString STR_JOIN_ROOM_BTN = { (const char*)u8"ﻡﺎﻤﻀﻧﺍ", "JOIN ROOM" };
inline const LocalizedString STR_WAITING_HOST_MSG = { (const char*)u8"...Wi-Fi ـﻟﺍ ﺔﻜﺒﺷ ﻰﻠﻋ ﺔﻓﺮﻏ ﺀﺎﺸﻧﺇ ﺭﺎﻈﺘﻧﺍ ﻲﻓ", "Waiting for a player on your Wi-Fi / LAN to click CREATE LOBBY..." };
inline const LocalizedString STR_CREATE_NOW_BTN = { (const char*)u8"ﻥﻵﺍ ﺔﻓﺮﻏ ﺊﺸﻧﺃ +", "+ CREATE LOBBY NOW" };
inline const LocalizedString STR_TAB_GENERAL = { (const char*)u8"ﻡﺎﻋ", "General" };
inline const LocalizedString STR_TAB_GRAPHICS = { (const char*)u8"ﺕﺎﻣﻮﺳﺮﻟﺍ", "Graphics" };
inline const LocalizedString STR_TAB_AUDIO = { (const char*)u8"ﺕﻮﺼﻟﺍ", "Audio" };
inline const LocalizedString STR_TAB_CONTROLS = { (const char*)u8"ﻢﻜﺤﺘﻟﺍ", "Controls" };
inline const LocalizedString STR_VOL_MASTER = { (const char*)u8"ﻡﺎﻌﻟﺍ ﺕﻮﺼﻟﺍ", "Master Volume" };
inline const LocalizedString STR_VOL_MUSIC = { (const char*)u8"ﻰﻘﻴﺳﻮﻤﻟﺍ ﺕﻮﺻ", "Music Volume" };
inline const LocalizedString STR_VOL_SFX = { (const char*)u8"ﺔﻴﺗﻮﺼﻟﺍ ﺕﺍﺮﺛﺆﻤﻟﺍ", "SFX Volume" };
inline const LocalizedString STR_VOL_FART = { (const char*)u8"ﺕﺍﺯﺎﻐﻟﺍ ﺕﻮﺻ", "Fart Sound Volume" };
inline const LocalizedString STR_QUALITY_PRESET = { (const char*)u8"ﺓﺩﻮﺠﻟﺍ ﻯﻮﺘﺴﻣ", "Quality Preset" };
inline const LocalizedString STR_DEVICE_MODE = { (const char*)u8"ﺯﺎﻬﺠﻟﺍ ﻂﻤﻧ", "Device Mode" };
inline const LocalizedString STR_LANGUAGE_LBL = { (const char*)u8"Language / ﺔﻐﻠﻟﺍ", "Language / اللغة" };
inline const LocalizedString STR_OPEN_EDITOR = { (const char*)u8"ﻞﺣﺍﺮﻤﻟﺍ ﺭﺮﺤﻣ ﺢﺘﻓ", "OPEN LEVEL EDITOR" };
inline const LocalizedString STR_PC_CONTROLS = { (const char*)u8"ﺱﻭﺎﻤﻟﺍﻭ ﺢﻴﺗﺎﻔﻤﻟﺍ ﺔﺣﻮﻟ", "PC Keyboard/Mouse" };
inline const LocalizedString STR_MOBILE_CONTROLS = { (const char*)u8"ﻝﺍﻮﺠﻠﻟ ﺲﻤﻠﻟﺍ ﺭﺍﺭﺯﺃ", "Mobile Touch Controls" };
inline const LocalizedString STR_CHECKPOINT_LBL = { (const char*)u8"ﻆﻔﺣ ﺔﻄﻘﻧ", "CHECKPOINT" };
inline const LocalizedString STR_FINISH_LBL = { (const char*)u8"ﺔﻳﺎﻬﻨﻟﺍ ﻂﺧ", "FINISH" };
inline const LocalizedString STR_RESPAWN_LBL = { (const char*)u8"[R] ﺔﻟﻭﺎﺤﻤﻟﺍ ﺓﺩﺎﻋﺇ", "[R] RESPAWN" };
