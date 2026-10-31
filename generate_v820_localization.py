import arabic_reshaper, pathlib
from bidi.algorithm import get_display
from fontTools.ttLib import TTFont

def ar(text):
    # remove checkmark or any emoji that might not exist in TTF
    clean_text = text.replace("✔", "")
    return get_display(arabic_reshaper.reshape(clean_text))

words = [
    ('PLAY', 'ابدأ اللعب', 'PLAY SOLO'),
    ('CREATE_LOBBY', 'إنشاء غرفة', 'CREATE LOBBY'),
    ('FIND_LOBBY', 'البحث عن غرفة', 'FIND LOBBY'),
    ('SETTINGS', 'الإعدادات', 'SETTINGS'),
    ('EXIT_GAME', 'خروج', 'EXIT GAME'),
    ('ESC_BACK', '[ESC] رجوع', '[ESC] BACK'),
    ('ENTER_SELECT', '[ENTER] اختيار', '[ENTER] SELECT'),
    ('LOBBY_TITLE', 'غرفة الانتظار', 'LOBBY'),
    ('YOUR_CODE', 'رمز الغرفة الخاص بك', 'YOUR LOBBY CODE'),
    ('SELECT_MODE', 'اختر الوضع', 'SELECT MODE'),
    ('INVITE_FRIENDS', 'دعوة الأصدقاء', 'INVITE FRIENDS'),
    ('ESC_LEAVE', '[ESC] مغادرة', '[ESC] LEAVE'),
    ('SELECT_CHAR', '[A / D] اختيار الشخصية', '[A / D] SELECT CHARACTER'),
    ('READY_BTN', '[SPACE] جاهز', '[SPACE] READY'),
    ('NOT_READY_BTN', '[SPACE] غير جاهز', '[SPACE] NOT READY'),
    ('WAITING_STATUS', '[في الانتظار...]', '[WAITING...]'),
    ('READY_STATUS', '[جاهز]', '[READY]'),
    ('MIN_PLAYERS_ALERT', 'يلزم وجود لاعبين اثنين وضغط الجميع على جاهز!', 'Need 2+ players & all READY!'),
    ('SOLO_TEST', '[تجربة فردية]', '[Solo Practice]'),
    ('FIND_TITLE', 'الغرف المتاحة على الشبكة المحلية', 'FIND LOBBY / AVAILABLE LAN ROOMS'),
    ('ROOM_CODE_LBL', 'رمز الغرفة:', 'Room Code:'),
    ('JOIN_BY_CODE', 'انضم بالرمز', 'JOIN BY CODE'),
    ('COOP_RACE', 'سباق تعاوني', 'FART RUN CO-OP'),
    ('PLAYERS_COUNT', 'لاعبين', 'PLAYERS'),
    ('JOIN_ROOM_BTN', 'انضمام', 'JOIN ROOM'),
    ('WAITING_HOST_MSG', 'في انتظار إنشاء غرفة على شبكة الـ Wi-Fi...', 'Waiting for a player on your Wi-Fi / LAN to click CREATE LOBBY...'),
    ('CREATE_NOW_BTN', '+ أنشئ غرفة الآن', '+ CREATE LOBBY NOW'),
    ('TAB_GENERAL', 'عام', 'GENERAL'),
    ('TAB_GRAPHICS', 'الرسوميات', 'GRAPHICS'),
    ('TAB_AUDIO', 'الصوت', 'AUDIO'),
    ('TAB_CONTROLS', 'التحكم', 'CONTROLS'),
    ('VOL_MASTER', 'مستوى الصوت الرئيسي', 'Master Volume'),
    ('VOL_MUSIC', 'الموسيقى', 'Music'),
    ('VOL_SFX', 'المؤثرات الصوتية', 'Sound Effects'),
    ('VOL_FART', 'مستوى صوت الضراط', 'Fart Volume'),
    ('QUALITY_PRESET', 'الجودة', 'Quality'),
    ('DEVICE_MODE', 'نمط الجهاز والتحكم', 'Device Profile'),
    ('LANGUAGE_LBL', 'اللغة', 'Language'),
    ('LANG_VALUE', '< العربية >', '< English >'),
    ('OPEN_EDITOR', '[ فتح محرر المراحل ]', '[ OPEN LEVEL EDITOR ]'),
    ('LEVEL_EDITOR_ROW', 'محرر المراحل والعوائق', 'Level & Obstacle Editor'),
    ('DEV_AUTO', '< تلقائي >', '< Auto-Detect >'),
    ('DEV_PC', '< كمبيوتر (كيبورد وماوس) >', '< PC (Keyboard & Mouse) >'),
    ('DEV_MOBILE', '< جوال (شاشة لمس) >', '< Mobile (Touchscreen) >'),
    ('SHOW_FPS_ROW', 'إظهار معدل الإطارات (F5)', 'Show FPS Counter (F5)'),
    ('OFF_LBL', 'إيقاف', 'OFF'),
    ('ON_LBL', 'تشغيل', 'ON'),
    ('QUAL_LOW', '< منخفضة >', '< Low >'),
    ('QUAL_MED', '< متوسطة >', '< Medium >'),
    ('QUAL_HIGH', '< عالية >', '< High >'),
    ('QUAL_ULTRA', '< فائقة >', '< Ultra >'),
    ('CAM_SHAKE_ROW', 'اهتزاز الكاميرا', 'Camera Shake'),
    ('MAX_CAM_DIST_ROW', 'أقصى مسافة للكاميرا', 'Max Camera Distance'),
    ('MOUSE_SENS_ROW', 'حساسية الماوس', 'Mouse Sensitivity'),
    ('INVERT_Y_ROW', 'عكس محور Y للماوس', 'Invert Y Axis'),
    ('STEER_KEYS_ROW', 'أزرار الدوران 360° والغاز', 'Steer / Gas Keys'),
    ('TOUCH_SIZE_ROW', 'حجم أيقونات اللمس', 'Touch Buttons Size'),
    ('TOUCH_OPAC_ROW', 'شفافية أيقونات اللمس', 'Touch Buttons Opacity'),
    ('CUSTOM_TOUCH_ROW', 'تعديل أماكن أيقونات التحريك', 'Customize Button Positions'),
    ('DRAG_SCREEN_BTN', '[ تعديل بالسحب على الشاشة ]', '[ DRAG ON SCREEN ]'),
    ('TERMS_OF_SERVICE', 'شروط الخدمة', 'Terms of Service'),
    ('PRIVACY_POLICY', 'سياسة الخصوصية', 'Privacy Policy'),
    ('LEAVE_HUD', 'مغادرة', 'LEAVE'),
    ('SUICIDE_HUD', '[R] انتحار', '[R] SUICIDE'),
    ('LEVEL_EDITOR_HUD', 'محرر المراحل (F1)', 'Level Editor (F1)'),
    ('UNREADY_BTN', 'إلغاء الجاهزية', 'UNREADY'),
    ('READY_WORD', 'جاهز', 'READY'),
    ('START_GAME_BTN', 'ابدأ اللعبة', 'START GAME'),
    ('YOU_MARKER', 'أنت', 'YOU'),
    ('PLAYER_MARKER', 'لاعب', 'PLAYER'),
    ('MODE_FARTRUN', 'سباق تعاوني', 'Fart Run Co-Op'),
    ('MODE_TEAMRACE', 'سباق الفرق', 'Team Race'),
    ('MODE_RAFTRUMBLE', 'معركة القوارب', 'Raft Rumble'),
    ('COMING_SOON', 'قريباً', 'Coming Soon'),
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

full_corpus = ""
for k, ar_text, en_text in words:
    shaped = ar(ar_text)
    full_corpus += shaped + en_text + ar_text
    header_code += f'inline const LocalizedString STR_{k} = {{ (const char*)u8"{shaped}", "{en_text}" }};\n'

pathlib.Path('CustomEngine/native/Localization.h').write_text(header_code, encoding='utf-8')
print('Generated expanded Localization.h successfully!')

# Generate Codepoints.h
font = TTFont('CustomEngine/extracted_assets/fonts/ui_font_bold.ttf')
font_chars = set()
for t in font['cmap'].tables:
    font_chars.update(t.cmap.keys())

needed_cps = set(range(32, 127))
for c in full_corpus:
    needed_cps.add(ord(c))

# Also include basic Arabic and Presentation B ranges if in font
for c in range(0x0600, 0x06FF):
    needed_cps.add(c)
for c in range(0xFE70, 0xFEFF):
    needed_cps.add(c)

valid_cps = sorted(list(needed_cps.intersection(font_chars)))
print(f"Total valid codepoints for Vazirmatn font: {len(valid_cps)}")

cp_code = f"""#pragma once
static const int gArFontCodepoints[{len(valid_cps)}] = {{
    {', '.join(str(c) for c in valid_cps)}
}};
static const int gArFontCodepointCount = {len(valid_cps)};
"""
pathlib.Path("CustomEngine/native/Codepoints.h").write_text(cp_code, encoding="utf-8")
print("Generated Codepoints.h successfully!")

