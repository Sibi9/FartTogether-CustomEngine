import arabic_reshaper, pathlib
from bidi.algorithm import get_display

def ar(text):
    return get_display(arabic_reshaper.reshape(text))

words = [
    ('PLAY', 'ابدأ اللعب', 'PLAY'),
    ('CREATE_LOBBY', 'إنشاء غرفة', 'CREATE LOBBY'),
    ('FIND_LOBBY', 'البحث عن غرفة', 'FIND LOBBY'),
    ('SETTINGS', 'الإعدادات', 'SETTINGS'),
    ('EXIT_GAME', 'خروج', 'EXIT GAME'),
    ('ESC_BACK', '[ESC] رجوع', '[ESC] BACK'),
    ('ENTER_SELECT', '[ENTER] اختيار', '[ENTER] SELECT'),
    ('LOBBY_TITLE', 'غرفة الانتظار', 'LOBBY'),
    ('YOUR_CODE', 'رمز الغرفة الخاص بك', 'YOUR LOBBY CODE'),
    ('SELECT_MODE', 'اختر نمط اللعب', 'SELECT MODE'),
    ('INVITE_FRIENDS', 'دعوة الأصدقاء', 'INVITE FRIENDS'),
    ('ESC_LEAVE', '[ESC] مغادرة', '[ESC] LEAVE'),
    ('SELECT_CHAR', '[A / D] اختيار الشخصية', '[A / D] SELECT CHARACTER'),
    ('READY_BTN', '[SPACE] جاهز', '[SPACE] READY'),
    ('NOT_READY_BTN', '[SPACE] غير جاهز', '[SPACE] NOT READY'),
    ('WAITING_STATUS', '[في الانتظار...]', '[WAITING...]'),
    ('READY_STATUS', '[جاهز ✔]', '[READY ✔]'),
    ('MIN_PLAYERS_ALERT', 'يلزم وجود لاعبين اثنين وضغط الجميع على جاهز لبدء اللعب!', 'Need at least 2 players & all READY to start!'),
    ('SOLO_TEST', '[تجربة فردية]', '[Solo Practice]'),
    ('FIND_TITLE', 'الغرف المتاحة على الشبكة المحلية', 'FIND LOBBY / AVAILABLE LAN ROOMS'),
    ('ROOM_CODE_LBL', 'رمز الغرفة:', 'Room Code:'),
    ('JOIN_BY_CODE', 'انضم بالرمز', 'JOIN BY CODE'),
    ('COOP_RACE', 'سباق تعاوني', 'FART RUN CO-OP'),
    ('PLAYERS_COUNT', 'لاعبين', 'PLAYERS'),
    ('JOIN_ROOM_BTN', 'انضمام', 'JOIN ROOM'),
    ('WAITING_HOST_MSG', 'في انتظار إنشاء غرفة على شبكة الـ Wi-Fi...', 'Waiting for a player on your Wi-Fi / LAN to click CREATE LOBBY...'),
    ('CREATE_NOW_BTN', '+ أنشئ غرفة الآن', '+ CREATE LOBBY NOW'),
    ('TAB_GENERAL', 'عام', 'General'),
    ('TAB_GRAPHICS', 'الرسومات', 'Graphics'),
    ('TAB_AUDIO', 'الصوت', 'Audio'),
    ('TAB_CONTROLS', 'التحكم', 'Controls'),
    ('VOL_MASTER', 'الصوت العام', 'Master Volume'),
    ('VOL_MUSIC', 'صوت الموسيقى', 'Music Volume'),
    ('VOL_SFX', 'المؤثرات الصوتية', 'SFX Volume'),
    ('VOL_FART', 'صوت الغازات', 'Fart Sound Volume'),
    ('QUALITY_PRESET', 'مستوى الجودة', 'Quality Preset'),
    ('DEVICE_MODE', 'نمط الجهاز', 'Device Mode'),
    ('LANGUAGE_LBL', 'اللغة / Language', 'Language / اللغة'),
    ('OPEN_EDITOR', 'فتح محرر المراحل', 'OPEN LEVEL EDITOR'),
    ('PC_CONTROLS', 'لوحة المفاتيح والماوس', 'PC Keyboard/Mouse'),
    ('MOBILE_CONTROLS', 'أزرار اللمس للجوال', 'Mobile Touch Controls'),
    ('CHECKPOINT_LBL', 'نقطة حفظ', 'CHECKPOINT'),
    ('FINISH_LBL', 'خط النهاية', 'FINISH'),
    ('RESPAWN_LBL', '[R] إعادة المحاولة', '[R] RESPAWN'),
]

header_code = '''#pragma once
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
'''

for k, ar_text, en_text in words:
    shaped = ar(ar_text)
    # escape shaped characters as utf-8 literals
    header_code += f'inline const LocalizedString STR_{k} = {{ (const char*)u8"{shaped}", "{en_text}" }};\n'

pathlib.Path('CustomEngine/native/Localization.h').write_text(header_code, encoding='utf-8')
print('Generated Localization.h successfully!')
