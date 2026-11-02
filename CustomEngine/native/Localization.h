#pragma once
#include <string>
#include "ArabicShaper.h"

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

// Arabic Main Menu
inline const LocalizedString STR_PLAY = { (const char*)u8"ﺐﻌﻠﻟﺍ ﺃﺪﺑﺍ", "PLAY SOLO" };
inline const LocalizedString STR_CREATE_LOBBY = { (const char*)u8"ﺔﻓﺮﻏ ﺀﺎﺸﻧﺇ", "CREATE LOBBY" };
inline const LocalizedString STR_FIND_LOBBY = { (const char*)u8"ﺔﻓﺮﻏ ﻦﻋ ﺚﺤﺒﻟﺍ", "FIND LOBBY" };
inline const LocalizedString STR_SETTINGS = { (const char*)u8"ﺕﺍﺩﺍﺪﻋﻹﺍ", "SETTINGS" };
inline const LocalizedString STR_EXIT_GAME = { (const char*)u8"ﺝﻭﺮﺧ", "EXIT GAME" };
inline const LocalizedString STR_ESC_BACK = { (const char*)u8"[ESC] ﻉﻮﺟﺭ", "[ESC] BACK" };
inline const LocalizedString STR_ENTER_SELECT = { (const char*)u8"[ENTER] ﺭﺎﻴﺘﺧﺍ", "[ENTER] SELECT" };

// Lobby & Rooms
inline const LocalizedString STR_LOBBY_TITLE = { (const char*)u8"ﺭﺎﻈﺘﻧﻻﺍ ﺔﻓﺮﻏ", "LOBBY" };
inline const LocalizedString STR_YOUR_CODE = { (const char*)u8"ﻚﺑ ﺹﺎﺨﻟﺍ ﺔﻓﺮﻐﻟﺍ ﺰﻣﺭ", "YOUR LOBBY CODE" };
inline const LocalizedString STR_SELECT_MODE = { (const char*)u8"ﻊﺿﻮﻟﺍ ﺮﺘﺧﺍ", "SELECT MODE" };
inline const LocalizedString STR_INVITE_FRIENDS = { (const char*)u8"ﺀﺎﻗﺪﺻﻷﺍ ﺓﻮﻋﺩ", "INVITE FRIENDS" };
inline const LocalizedString STR_ESC_LEAVE = { (const char*)u8"[ESC] ﺓﺭﺩﺎﻐﻣ", "[ESC] LEAVE" };
inline const LocalizedString STR_SELECT_CHAR = { (const char*)u8"[A / D] ﺔﻴﺼﺨﺸﻟﺍ ﺭﺎﻴﺘﺧﺍ", "[A / D] SELECT CHARACTER" };
inline const LocalizedString STR_READY_BTN = { (const char*)u8"[SPACE] ﺰﻫﺎﺟ", "[SPACE] READY" };
inline const LocalizedString STR_NOT_READY_BTN = { (const char*)u8"[SPACE] ﺰﻫﺎﺟ ﺮﻴﻏ", "[SPACE] NOT READY" };
inline const LocalizedString STR_WAITING_STATUS = { (const char*)u8"[...ﺭﺎﻈﺘﻧﻻﺍ ﻲﻓ]", "[WAITING...]" };
inline const LocalizedString STR_READY_STATUS = { (const char*)u8"[ﺰﻫﺎﺟ]", "[READY]" };
inline const LocalizedString STR_MIN_PLAYERS_ALERT = { (const char*)u8"!ﺰﻫﺎﺟ ﻰﻠﻋ ﻊﻴﻤﺠﻟﺍ ﻂﻐﺿﻭ ﻦﻴﻨﺛﺍ ﻦﻴﺒﻋﻻ ﺩﻮﺟﻭ ﻡﺰﻠﻳ", "Need 2+ players & all READY!" };
inline const LocalizedString STR_SOLO_TEST = { (const char*)u8"[ﺔﻳﺩﺮﻓ ﺔﺑﺮﺠﺗ]", "[Solo Practice]" };
inline const LocalizedString STR_FIND_TITLE = { (const char*)u8"ﺔﻴﻠﺤﻤﻟﺍ ﺔﻜﺒﺸﻟﺍ ﻰﻠﻋ ﺔﺣﺎﺘﻤﻟﺍ ﻑﺮﻐﻟﺍ", "FIND LOBBY / AVAILABLE LAN ROOMS" };
inline const LocalizedString STR_ROOM_CODE_LBL = { (const char*)u8":ﺔﻓﺮﻐﻟﺍ ﺰﻣﺭ", "Room Code:" };
inline const LocalizedString STR_JOIN_BY_CODE = { (const char*)u8"ﺰﻣﺮﻟﺎﺑ ﻢﻀﻧﺍ", "JOIN BY CODE" };
inline const LocalizedString STR_COOP_RACE = { (const char*)u8"ﻲﻧﻭﺎﻌﺗ ﻕﺎﺒﺳ", "FART RUN CO-OP" };
inline const LocalizedString STR_PLAYERS_COUNT = { (const char*)u8"ﻦﻴﺒﻋﻻ", "PLAYERS" };
inline const LocalizedString STR_JOIN_ROOM_BTN = { (const char*)u8"ﻡﺎﻤﻀﻧﺍ", "JOIN ROOM" };
inline const LocalizedString STR_WAITING_HOST_MSG = { (const char*)u8"...Wi-Fi ـﻟﺍ ﺔﻜﺒﺷ ﻰﻠﻋ ﺔﻓﺮﻏ ﺀﺎﺸﻧﺇ ﺭﺎﻈﺘﻧﺍ ﻲﻓ", "Waiting for a player on your Wi-Fi / LAN to click CREATE LOBBY..." };
inline const LocalizedString STR_CREATE_NOW_BTN = { (const char*)u8"ﻥﻵﺍ ﺔﻓﺮﻏ ﺊﺸﻧﺃ +", "+ CREATE LOBBY NOW" };

// Settings Tabs
inline const LocalizedString STR_TAB_GENERAL = { (const char*)u8"ﻡﺎﻋ", "GENERAL" };
inline const LocalizedString STR_TAB_GRAPHICS = { (const char*)u8"ﺕﺎﻴﻣﻮﺳﺮﻟﺍ", "GRAPHICS" };
inline const LocalizedString STR_TAB_AUDIO = { (const char*)u8"ﺕﻮﺼﻟﺍ", "AUDIO" };
inline const LocalizedString STR_TAB_CONTROLS = { (const char*)u8"ﻢﻜﺤﺘﻟﺍ", "CONTROLS" };

// Settings Audio
inline const LocalizedString STR_VOL_MASTER = { (const char*)u8"ﻲﺴﻴﺋﺮﻟﺍ ﺕﻮﺼﻟﺍ ﻯﻮﺘﺴﻣ", "Master Volume" };
inline const LocalizedString STR_VOL_MUSIC = { (const char*)u8"ﺝﺍﻮﻣﻷﺍﻭ ﻰﻘﻴﺳﻮﻤﻟﺍ", "Music (OST & Waves)" };
inline const LocalizedString STR_VOL_SFX = { (const char*)u8"ﺔﻴﺗﻮﺼﻟﺍ ﺕﺍﺮﺛﺆﻤﻟﺍ", "Sound Effects (SFX)" };
inline const LocalizedString STR_VOL_FART = { (const char*)u8"ﻁﺍﺮﻀﻟﺍﻭ ﺯﺎﻐﻟﺍ ﺕﻮﺻ", "Fart & Gas Volume" };

// Settings Graphics
inline const LocalizedString STR_QUALITY_PRESET = { (const char*)u8"ﺕﺎﻣﻮﺳﺮﻟﺍ ﺓﺩﻮﺟ", "Quality Level" };
inline const LocalizedString STR_QUAL_LOW = { (const char*)u8"<  (30 FPS) ﺔﻀﻔﺨﻨﻣ  >", "<  Low (30 FPS)  >" };
inline const LocalizedString STR_QUAL_MED = { (const char*)u8"<  (60 FPS) ﺔﻄﺳﻮﺘﻣ  >", "<  Medium (60 FPS)  >" };
inline const LocalizedString STR_QUAL_HIGH = { (const char*)u8"<  (60 FPS) ﺔﻴﻟﺎﻋ  >", "<  High (60 FPS)  >" };
inline const LocalizedString STR_QUAL_ULTRA = { (const char*)u8"<  (120 FPS) ﺔﻘﺋﺎﻓ  >", "<  Ultra (120 FPS)  >" };
inline const LocalizedString STR_CAM_SHAKE_ROW = { (const char*)u8"ﺍﺮﻴﻣﺎﻜﻟﺍ ﺯﺍﺰﺘﻫﺍ", "Camera Shake" };
inline const LocalizedString STR_MAX_CAM_DIST_ROW = { (const char*)u8"ﺍﺮﻴﻣﺎﻜﻠﻟ ﺔﻓﺎﺴﻣ ﻰﺼﻗﺃ", "Max Camera Distance" };
inline const LocalizedString STR_SHOW_FPS_ROW = { (const char*)u8"(F5) ﺕﺍﺭﺎﻃﻹﺍ ﻝﺪﻌﻣ ﺭﺎﻬﻇﺇ", "Show FPS Counter (F5)" };

// Settings General
inline const LocalizedString STR_DEVICE_MODE = { (const char*)u8"ﻢﻜﺤﺘﻟﺍﻭ ﺯﺎﻬﺠﻟﺍ ﻂﻤﻧ", "Device Profile" };
inline const LocalizedString STR_LANGUAGE_LBL = { (const char*)u8"ﺔﻐﻠﻟﺍ", "Language" };
inline const LocalizedString STR_LANG_VALUE = { (const char*)u8"<  (ﻲﺿﺍﺮﺘﻓﺍ) ﺔﻴﺑﺮﻌﻟﺍ  >", "<  English  >" };
inline const LocalizedString STR_LANG_CURRENT = { (const char*)u8"<  (ﻲﺿﺍﺮﺘﻓﺍ) ﺔﻴﺑﺮﻌﻟﺍ  >", "<  English  >" };
inline const LocalizedString STR_DEV_AUTO = { (const char*)u8"<  ﻲﺋﺎﻘﻠﺘﻟﺍ ﻑﺮﻌﺘﻟﺍ  >", "<  Auto-Detect  >" };
inline const LocalizedString STR_DEV_PC = { (const char*)u8"<  (ﺓﺭﺄﻓﻭ ﺢﻴﺗﺎﻔﻣ ﺔﺣﻮﻟ) ﺮﺗﻮﻴﺒﻤﻛ  >", "<  PC (Keyboard/Mouse)  >" };
inline const LocalizedString STR_DEV_MOBILE = { (const char*)u8"<  (ﺲﻤﻠﻟﺍ ﺔﺷﺎﺷ) ﻝﺍﻮﺟ  >", "<  Mobile (Touchscreen)  >" };
inline const LocalizedString STR_INVERT_Y_ROW = { (const char*)u8"ﺓﺭﺄﻔﻠﻟ Y ﺭﻮﺤﻣ ﺲﻜﻋ", "Invert Y Axis" };

// Settings Controls
inline const LocalizedString STR_TAB_PC_MODE = { (const char*)u8"ﺮﺗﻮﻴﺒﻤﻜﻟﺍ ﻢﻜﺤﺗ", "PC Keyboard/Mouse" };
inline const LocalizedString STR_TAB_MOBILE_MODE = { (const char*)u8"ﺲﻤﻠﻟﺍ ﺔﺷﺎﺷ ﻢﻜﺤﺗ", "Mobile Touch Controls" };
inline const LocalizedString STR_P1_GAS_CONTROLS = { (const char*)u8"ﺯﺎﻐﻟﺍ ﻕﻼﻃﺇﻭ ﻪﻴﺟﻮﺘﻟﺍ ﺭﺍﺭﺯﺃ", "P1 Fart Gas Controls" };
inline const LocalizedString STR_P1_GAS_KEYS = { (const char*)u8"(ﺯﺎﻏ) W/ﺔﻓﺎﺴﻣ ﻭ (ﻦﻴﻤﻳ) D / (ﺭﺎﺴﻳ) A", "A (Left) / D (Right) & Space/W (Gas)" };
inline const LocalizedString STR_LEVEL_EDITOR_ROW = { (const char*)u8"ﻖﺋﺍﻮﻌﻟﺍﻭ ﻞﺣﺍﺮﻤﻟﺍ ﺭﺮﺤﻣ", "Level & Obstacle Editor" };
inline const LocalizedString STR_LEVEL_EDITOR_KEY = { (const char*)u8"[ F1 ﺭﺯ ]", "[ Press F1 Key ]" };
inline const LocalizedString STR_SHOW_FPS_KEY = { (const char*)u8"[ F5 ﺭﺯ ]", "[ F5 Key ]" };
inline const LocalizedString STR_SUICIDE_KEY = { (const char*)u8"[ R ﺭﺯ ]", "[ R Key ]" };
inline const LocalizedString STR_TOUCH_SIZE_ROW = { (const char*)u8"ﺲﻤﻠﻟﺍ ﺭﺍﺭﺯﺃ ﻢﺠﺣ", "Touch Button Scale" };
inline const LocalizedString STR_RESET_STEER_POS = { (const char*)u8"ﻪﻴﺟﻮﺘﻟﺍ ﺭﺍﺭﺯﺃ ﻂﺒﺿ ﺓﺩﺎﻋﺇ", "Reset Left/Right Pos" };
inline const LocalizedString STR_RESET_GAS_POS = { (const char*)u8"ﺯﺎﻐﻟﺍ ﺭﺯ ﻂﺒﺿ ﺓﺩﺎﻋﺇ", "Reset Gas Button Pos" };

// Switch labels
inline const LocalizedString STR_ON = { (const char*)u8"ﻞﻴﻐﺸﺗ", "ON" };
inline const LocalizedString STR_OFF = { (const char*)u8"ﻑﺎﻘﻳﺇ", "OFF" };
inline const LocalizedString STR_ON_LBL = STR_ON;
inline const LocalizedString STR_OFF_LBL = STR_OFF;

// HUD & In-Game
inline const LocalizedString STR_LEAVE_HUD = { (const char*)u8"ﺓﺭﺩﺎﻐﻣ", "LEAVE" };
inline const LocalizedString STR_SUICIDE_HUD = { (const char*)u8"ﺭﺎﺤﺘﻧﺍ", "SUICIDE" };
inline const LocalizedString STR_CAPTAIN_1 = { (const char*)u8"1 ﻥﺎﻄﺒﻘﻟﺍ", "Captain 1" };
inline const LocalizedString STR_CAPTAIN_2 = { (const char*)u8"2 ﻥﺎﻄﺒﻘﻟﺍ", "Captain 2" };

// Game Modes & Game Status
inline const LocalizedString STR_MODE_FARTRUN = { (const char*)u8"ﻲﻧﻭﺎﻌﺗ ﻕﺎﺒﺳ", "Fart Run Co-Op" };
inline const LocalizedString STR_MODE_TEAMRACE = { (const char*)u8"ﻕﺮﻔﻟﺍ ﻕﺎﺒﺳ", "Team Race" };
inline const LocalizedString STR_MODE_RAFTRUMBLE = { (const char*)u8"ﺏﺭﺍﻮﻘﻟﺍ ﺔﻛﺮﻌﻣ", "Raft Rumble" };
inline const LocalizedString STR_COMING_SOON = { (const char*)u8"ﺎﺒﻳﺮﻗ", "Coming Soon" };
inline const LocalizedString STR_CHECKPOINT_LBL = { (const char*)u8"ﻆﻔﺣ ﺔﻄﻘﻧ", "CHECKPOINT" };
inline const LocalizedString STR_FINISH_LBL = { (const char*)u8"ﺔﻳﺎﻬﻨﻟﺍ ﻂﺧ", "FINISH" };
inline const LocalizedString STR_RESPAWN_LBL = { (const char*)u8"[R] ﺔﻟﻭﺎﺤﻤﻟﺍ ﺓﺩﺎﻋﺇ", "[R] RESPAWN" };
inline const LocalizedString STR_UNREADY_BTN = { (const char*)u8"ﺔﻳﺰﻫﺎﺠﻟﺍ ﺀﺎﻐﻟﺇ", "UNREADY" };
inline const LocalizedString STR_READY_WORD = { (const char*)u8"ﺰﻫﺎﺟ", "READY" };
inline const LocalizedString STR_START_GAME_BTN = { (const char*)u8"ﺔﺒﻌﻠﻟﺍ ﺃﺪﺑﺍ", "START GAME" };
inline const LocalizedString STR_YOU_MARKER = { (const char*)u8"ﺖﻧﺃ", "YOU" };
inline const LocalizedString STR_PLAYER_MARKER = { (const char*)u8"ﺐﻋﻻ", "PLAYER" };

// Footer Links
inline const LocalizedString STR_TERMS_OF_SERVICE = { (const char*)u8"ﺔﻣﺪﺨﻟﺍ ﻁﻭﺮﺷ", "Terms of Service" };
inline const LocalizedString STR_PRIVACY_POLICY = { (const char*)u8"ﺔﻴﺻﻮﺼﺨﻟﺍ ﺔﺳﺎﻴﺳ", "Privacy Policy" };

inline const LocalizedString& STR_DEVICE_PROFILE = STR_DEVICE_MODE;
