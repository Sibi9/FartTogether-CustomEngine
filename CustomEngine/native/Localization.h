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
inline const LocalizedString STR_PLAY = { (const char*)u8"ﺐﻌﻠﻟﺍ ﺃﺪﺑﺍ", "PLAY SOLO" };
inline const LocalizedString STR_CREATE_LOBBY = { (const char*)u8"ﺔﻓﺮﻏ ﺀﺎﺸﻧﺇ", "CREATE LOBBY" };
inline const LocalizedString STR_FIND_LOBBY = { (const char*)u8"ﺔﻓﺮﻏ ﻦﻋ ﺚﺤﺒﻟﺍ", "FIND LOBBY" };
inline const LocalizedString STR_SETTINGS = { (const char*)u8"ﺕﺍﺩﺍﺪﻋﻹﺍ", "SETTINGS" };
inline const LocalizedString STR_EXIT_GAME = { (const char*)u8"ﺝﻭﺮﺧ", "EXIT GAME" };
inline const LocalizedString STR_ESC_BACK = { (const char*)u8"[ESC] ﻉﻮﺟﺭ", "[ESC] BACK" };
inline const LocalizedString STR_ENTER_SELECT = { (const char*)u8"[ENTER] ﺭﺎﻴﺘﺧﺍ", "[ENTER] SELECT" };
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
inline const LocalizedString STR_TAB_GENERAL = { (const char*)u8"ﻡﺎﻋ", "GENERAL" };
inline const LocalizedString STR_TAB_GRAPHICS = { (const char*)u8"ﺕﺎﻴﻣﻮﺳﺮﻟﺍ", "GRAPHICS" };
inline const LocalizedString STR_TAB_AUDIO = { (const char*)u8"ﺕﻮﺼﻟﺍ", "AUDIO" };
inline const LocalizedString STR_TAB_CONTROLS = { (const char*)u8"ﻢﻜﺤﺘﻟﺍ", "CONTROLS" };
inline const LocalizedString STR_VOL_MASTER = { (const char*)u8"ﻲﺴﻴﺋﺮﻟﺍ ﺕﻮﺼﻟﺍ ﻯﻮﺘﺴﻣ", "Master Volume" };
inline const LocalizedString STR_VOL_MUSIC = { (const char*)u8"ﻰﻘﻴﺳﻮﻤﻟﺍ", "Music" };
inline const LocalizedString STR_VOL_SFX = { (const char*)u8"ﺔﻴﺗﻮﺼﻟﺍ ﺕﺍﺮﺛﺆﻤﻟﺍ", "Sound Effects" };
inline const LocalizedString STR_VOL_FART = { (const char*)u8"ﻁﺍﺮﻀﻟﺍ ﺕﻮﺻ ﻯﻮﺘﺴﻣ", "Fart Volume" };
inline const LocalizedString STR_QUALITY_PRESET = { (const char*)u8"ﺓﺩﻮﺠﻟﺍ", "Quality" };
inline const LocalizedString STR_DEVICE_MODE = { (const char*)u8"ﻢﻜﺤﺘﻟﺍﻭ ﺯﺎﻬﺠﻟﺍ ﻂﻤﻧ", "Device Profile" };
inline const LocalizedString STR_LANGUAGE_LBL = { (const char*)u8"ﺔﻐﻠﻟﺍ", "Language" };
inline const LocalizedString STR_LANG_VALUE = { (const char*)u8"< ﺔﻴﺑﺮﻌﻟﺍ >", "< English >" };
inline const LocalizedString STR_OPEN_EDITOR = { (const char*)u8"[ ﻞﺣﺍﺮﻤﻟﺍ ﺭﺮﺤﻣ ﺢﺘﻓ ]", "[ OPEN LEVEL EDITOR ]" };
inline const LocalizedString STR_LEVEL_EDITOR_ROW = { (const char*)u8"ﻖﺋﺍﻮﻌﻟﺍﻭ ﻞﺣﺍﺮﻤﻟﺍ ﺭﺮﺤﻣ", "Level & Obstacle Editor" };
inline const LocalizedString STR_DEV_AUTO = { (const char*)u8"< ﻲﺋﺎﻘﻠﺗ >", "< Auto-Detect >" };
inline const LocalizedString STR_DEV_PC = { (const char*)u8"< (ﺱﻭﺎﻣﻭ ﺩﺭﻮﺒﻴﻛ) ﺮﺗﻮﻴﺒﻤﻛ >", "< PC (Keyboard & Mouse) >" };
inline const LocalizedString STR_DEV_MOBILE = { (const char*)u8"< (ﺲﻤﻟ ﺔﺷﺎﺷ) ﻝﺍﻮﺟ >", "< Mobile (Touchscreen) >" };
inline const LocalizedString STR_SHOW_FPS_ROW = { (const char*)u8"(F5) ﺕﺍﺭﺎﻃﻹﺍ ﻝﺪﻌﻣ ﺭﺎﻬﻇﺇ", "Show FPS Counter (F5)" };
inline const LocalizedString STR_OFF_LBL = { (const char*)u8"ﻑﺎﻘﻳﺇ", "OFF" };
inline const LocalizedString STR_ON_LBL = { (const char*)u8"ﻞﻴﻐﺸﺗ", "ON" };
inline const LocalizedString STR_QUAL_LOW = { (const char*)u8"< ﺔﻀﻔﺨﻨﻣ >", "< Low >" };
inline const LocalizedString STR_QUAL_MED = { (const char*)u8"< ﺔﻄﺳﻮﺘﻣ >", "< Medium >" };
inline const LocalizedString STR_QUAL_HIGH = { (const char*)u8"< ﺔﻴﻟﺎﻋ >", "< High >" };
inline const LocalizedString STR_QUAL_ULTRA = { (const char*)u8"< ﺔﻘﺋﺎﻓ >", "< Ultra >" };
inline const LocalizedString STR_CAM_SHAKE_ROW = { (const char*)u8"ﺍﺮﻴﻣﺎﻜﻟﺍ ﺯﺍﺰﺘﻫﺍ", "Camera Shake" };
inline const LocalizedString STR_MAX_CAM_DIST_ROW = { (const char*)u8"ﺍﺮﻴﻣﺎﻜﻠﻟ ﺔﻓﺎﺴﻣ ﻰﺼﻗﺃ", "Max Camera Distance" };
inline const LocalizedString STR_MOUSE_SENS_ROW = { (const char*)u8"ﺱﻭﺎﻤﻟﺍ ﺔﻴﺳﺎﺴﺣ", "Mouse Sensitivity" };
inline const LocalizedString STR_INVERT_Y_ROW = { (const char*)u8"ﺱﻭﺎﻤﻠﻟ Y ﺭﻮﺤﻣ ﺲﻜﻋ", "Invert Y Axis" };
inline const LocalizedString STR_STEER_KEYS_ROW = { (const char*)u8"ﺯﺎﻐﻟﺍﻭ °360 ﻥﺍﺭﻭﺪﻟﺍ ﺭﺍﺭﺯﺃ", "Steer / Gas Keys" };
inline const LocalizedString STR_TOUCH_SIZE_ROW = { (const char*)u8"ﺲﻤﻠﻟﺍ ﺕﺎﻧﻮﻘﻳﺃ ﻢﺠﺣ", "Touch Buttons Size" };
inline const LocalizedString STR_TOUCH_OPAC_ROW = { (const char*)u8"ﺲﻤﻠﻟﺍ ﺕﺎﻧﻮﻘﻳﺃ ﺔﻴﻓﺎﻔﺷ", "Touch Buttons Opacity" };
inline const LocalizedString STR_CUSTOM_TOUCH_ROW = { (const char*)u8"ﻚﻳﺮﺤﺘﻟﺍ ﺕﺎﻧﻮﻘﻳﺃ ﻦﻛﺎﻣﺃ ﻞﻳﺪﻌﺗ", "Customize Button Positions" };
inline const LocalizedString STR_DRAG_SCREEN_BTN = { (const char*)u8"[ ﺔﺷﺎﺸﻟﺍ ﻰﻠﻋ ﺐﺤﺴﻟﺎﺑ ﻞﻳﺪﻌﺗ ]", "[ DRAG ON SCREEN ]" };
inline const LocalizedString STR_TERMS_OF_SERVICE = { (const char*)u8"ﺔﻣﺪﺨﻟﺍ ﻁﻭﺮﺷ", "Terms of Service" };
inline const LocalizedString STR_PRIVACY_POLICY = { (const char*)u8"ﺔﻴﺻﻮﺼﺨﻟﺍ ﺔﺳﺎﻴﺳ", "Privacy Policy" };
inline const LocalizedString STR_LEAVE_HUD = { (const char*)u8"ﺓﺭﺩﺎﻐﻣ", "LEAVE" };
inline const LocalizedString STR_SUICIDE_HUD = { (const char*)u8"[R] ﺭﺎﺤﺘﻧﺍ", "[R] SUICIDE" };
inline const LocalizedString STR_LEVEL_EDITOR_HUD = { (const char*)u8"(F1) ﻞﺣﺍﺮﻤﻟﺍ ﺭﺮﺤﻣ", "Level Editor (F1)" };
inline const LocalizedString STR_UNREADY_BTN = { (const char*)u8"ﺔﻳﺰﻫﺎﺠﻟﺍ ﺀﺎﻐﻟﺇ", "UNREADY" };
inline const LocalizedString STR_READY_WORD = { (const char*)u8"ﺰﻫﺎﺟ", "READY" };
inline const LocalizedString STR_START_GAME_BTN = { (const char*)u8"ﺔﺒﻌﻠﻟﺍ ﺃﺪﺑﺍ", "START GAME" };
inline const LocalizedString STR_YOU_MARKER = { (const char*)u8"ﺖﻧﺃ", "YOU" };
inline const LocalizedString STR_PLAYER_MARKER = { (const char*)u8"ﺐﻋﻻ", "PLAYER" };
inline const LocalizedString STR_MODE_FARTRUN = { (const char*)u8"ﻲﻧﻭﺎﻌﺗ ﻕﺎﺒﺳ", "Fart Run Co-Op" };
inline const LocalizedString STR_MODE_TEAMRACE = { (const char*)u8"ﻕﺮﻔﻟﺍ ﻕﺎﺒﺳ", "Team Race" };
inline const LocalizedString STR_MODE_RAFTRUMBLE = { (const char*)u8"ﺏﺭﺍﻮﻘﻟﺍ ﺔﻛﺮﻌﻣ", "Raft Rumble" };
inline const LocalizedString STR_COMING_SOON = { (const char*)u8"ﺎﺒﻳﺮﻗ", "Coming Soon" };
inline const LocalizedString STR_CHECKPOINT_LBL = { (const char*)u8"ﻆﻔﺣ ﺔﻄﻘﻧ", "CHECKPOINT" };
inline const LocalizedString STR_FINISH_LBL = { (const char*)u8"ﺔﻳﺎﻬﻨﻟﺍ ﻂﺧ", "FINISH" };
inline const LocalizedString STR_RESPAWN_LBL = { (const char*)u8"[R] ﺔﻟﻭﺎﺤﻤﻟﺍ ﺓﺩﺎﻋﺇ", "[R] RESPAWN" };

inline const LocalizedString& STR_DEVICE_PROFILE = STR_DEVICE_MODE;
