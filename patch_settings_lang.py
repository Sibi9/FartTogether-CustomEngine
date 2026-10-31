import pathlib

engine_path = pathlib.Path("CustomEngine/native/GameEngine.cpp")
text = engine_path.read_text(encoding="utf-8")

old_tabs = 'const char* tabNames[4] = { "GENERAL", "GRAPHICS", "AUDIO", "CONTROLS" };'
new_tabs = 'const char* tabNames[4] = { LocText(STR_TAB_GENERAL), LocText(STR_TAB_GRAPHICS), LocText(STR_TAB_AUDIO), LocText(STR_TAB_CONTROLS) };'
text = text.replace(old_tabs, new_tabs)

old_subtitles = '''                DrawSettingRow(3, "Subtitles & Dialogues", [&](Rectangle rRec, float ry, bool isHov) {
                    DrawTextEx(fontUIBold, "[ ON ]", Vector2{ rRec.x + rRec.width - 120.0f, ry + 22.0f }, 15, 0.4f, Color{ 30, 35, 55, 255 });
                });'''

new_subtitles = '''                DrawSettingRow(3, LocText(STR_LANGUAGE_LBL), [&](Rectangle rRec, float ry, bool isHov) {
                    const char* lNames[2] = { (const char*)u8"<  العربية (افتراضي)  >", "<  English  >" };
                    Rectangle bL = { rRec.x + rRec.width - 260.0f, ry + 16.0f, 230.0f, 34.0f };
                    bool hovL = CheckCollisionPointRec(mousePos, bL);
                    DrawSlantedRect(bL.x, bL.y, bL.width, bL.height, 8.0f, hovL ? Color{ 126, 114, 228, 255 } : Color{ 230, 235, 248, 255 });
                    Vector2 lSz = MeasureTextEx(fontUIBold, lNames[gCurrentLanguage], 13, 0.3f);
                    DrawTextEx(fontUIBold, lNames[gCurrentLanguage], Vector2{ bL.x + (bL.width - lSz.x) * 0.5f, bL.y + 9.0f }, 13, 0.3f, hovL ? WHITE : Color{ 30, 35, 55, 255 });
                    if (hovL && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                        gCurrentLanguage = (gCurrentLanguage == LANG_AR) ? LANG_EN : LANG_AR;
                        gSettings.language = (int)gCurrentLanguage;
                        SaveGameSettingsJSON("game_settings.json");
                    }
                });'''

text = text.replace(old_subtitles, new_subtitles)
engine_path.write_text(text, encoding="utf-8")
print("Settings language option patched successfully!")
