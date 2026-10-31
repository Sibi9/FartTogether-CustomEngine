import pathlib, re

engine_path = pathlib.Path("CustomEngine/native/GameEngine.cpp")
text = engine_path.read_text(encoding="utf-8")

# 1. Update version tag to v8.2.0
text = text.replace('static const char* GAME_VERSION_TAG = "v8.1.0";', 'static const char* GAME_VERSION_TAG = "v8.2.0";')
text = text.replace('static const char* GAME_VERSION_TAG = "v8.0.0";', 'static const char* GAME_VERSION_TAG = "v8.2.0";')

# 2. Update font loading sizes to bold cartoon look
old_fonts = """    Font fontUI     = LoadFontEx(ResolveAsset("extracted_assets/fonts/ui_font.ttf").c_str(), 26, (int*)gArFontCodepoints, gArFontCodepointCount);
    Font fontUIBold = LoadFontEx(ResolveAsset("extracted_assets/fonts/ui_font_bold.ttf").c_str(), 26, (int*)gArFontCodepoints, gArFontCodepointCount);"""

new_fonts = """    Font fontUI     = LoadFontEx(ResolveAsset("extracted_assets/fonts/ui_font.ttf").c_str(), 36, (int*)gArFontCodepoints, gArFontCodepointCount);
    Font fontUIBold = LoadFontEx(ResolveAsset("extracted_assets/fonts/ui_font_bold.ttf").c_str(), 42, (int*)gArFontCodepoints, gArFontCodepointCount);
    SetTextureFilter(fontUI.texture, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(fontUIBold.texture, TEXTURE_FILTER_BILINEAR);"""

if old_fonts in text:
    text = text.replace(old_fonts, new_fonts)
    print("Updated font loading sizes & bilinear filtering!")

# 3. Fix Mobile Steer Buttons Arrow Inversion
old_steer_src = "Rectangle src = { 0, 0, isRightArrow ? -(float)texIconArrow.width : (float)texIconArrow.width, (float)texIconArrow.height };"
new_steer_src = "// Icon_ArrowM.png points RIGHT natively -> Right is +width, Left is -width!\n                    float srcW = isRightArrow ? (float)texIconArrow.width : -(float)texIconArrow.width;\n                    Rectangle src = { 0.0f, 0.0f, srcW, (float)texIconArrow.height };"
if old_steer_src in text:
    text = text.replace(old_steer_src, new_steer_src)
    print("Fixed mobile steering arrows direction (Left=<, Right=>)!")

# 4. In Gameplay HUD, localize LEAVE, SUICIDE, LEVEL EDITOR
old_hud_leave = 'DrawText("LEAVE", 52, GetScreenHeight() - 40, 16, RAYWHITE);'
new_hud_leave = 'DrawTextEx(fontUIBold, LocText(STR_LEAVE_HUD), Vector2{ 50.0f, (float)GetScreenHeight() - 41.0f }, 16, 0.3f, RAYWHITE);'
text = text.replace(old_hud_leave, new_hud_leave)

old_hud_suicide = 'DrawText("SUICIDE", 185, GetScreenHeight() - 40, 16, RAYWHITE);'
new_hud_suicide = 'DrawTextEx(fontUIBold, LocText(STR_SUICIDE_HUD), Vector2{ 155.0f, (float)GetScreenHeight() - 41.0f }, 16, 0.3f, Color{ 248, 113, 113, 255 });'
text = text.replace(old_hud_suicide, new_hud_suicide)

old_hud_editor = 'DrawTextEx(fontUIBold, "Level Editor (F1)", Vector2{ btnEditorRec.x + 18, btnEditorRec.y + 7 }, 14, 0.3f, RAYWHITE);'
new_hud_editor = 'DrawTextEx(fontUIBold, LocText(STR_LEVEL_EDITOR_HUD), Vector2{ btnEditorRec.x + 14, btnEditorRec.y + 7 }, 14, 0.3f, RAYWHITE);'
text = text.replace(old_hud_editor, new_hud_editor)

# 5. Fix Main Menu: Stop background sounds, draw clean static Menu_Oasis_Full, and set Option 0 to PLAY SOLO
# Find the start of SCREEN_MAIN_MENU drawing block
m_idx = text.find("if (currentScreen == SCREEN_MAIN_MENU) {")
if m_idx != -1:
    # Find the end of SCREEN_MAIN_MENU before SCREEN_SETTINGS
    sett_idx = text.find("if (currentScreen == SCREEN_SETTINGS)", m_idx)
    if sett_idx != -1:
        # We replace the entire SCREEN_MAIN_MENU block with clean static oasis menu
        clean_menu_block = """if (currentScreen == SCREEN_MAIN_MENU) {
            ClearBackground(Color{ 18, 22, 34, 255 });

            float scrW = (float)GetScreenWidth();
            float scrH = (float)GetScreenHeight();

            // Mute / Stop background beach and snore sounds in main menu as commanded
            if (IsSoundPlaying(sndMenuBeachOst)) StopSound(sndMenuBeachOst);
            if (IsSoundPlaying(sndSnore)) StopSound(sndSnore);
            if (IsSoundPlaying(sndEatMunch)) StopSound(sndEatMunch);

            // Clean 100% static background (Menu_Oasis_Full.png) - No cutouts, no snot bubbles, no duplicate pirates!
            if (texMenuOasisFull.id != 0) {
                DrawTexturePro(texMenuOasisFull, Rectangle{ 0, 0, (float)texMenuOasisFull.width, (float)texMenuOasisFull.height },
                               Rectangle{ 0, 0, scrW, scrH }, Vector2{0, 0}, 0.0f, WHITE);
            } else if (texMenuOasisClean.id != 0) {
                DrawTexturePro(texMenuOasisClean, Rectangle{ 0, 0, (float)texMenuOasisClean.width, (float)texMenuOasisClean.height },
                               Rectangle{ 0, 0, scrW, scrH }, Vector2{0, 0}, 0.0f, WHITE);
            }

            // Top-Left Cartoon Logo
            if (texMenuLogo.id != 0) {
                float logoW = 345.0f, logoH = 158.0f;
                DrawTexturePro(texMenuLogo, Rectangle{ 0, 0, (float)texMenuLogo.width, (float)texMenuLogo.height },
                               Rectangle{ 38.0f, 32.0f, logoW, logoH }, Vector2{0, 0}, 0.0f, WHITE);
            }

            // 5 Main Menu Items: 0: PLAY SOLO, 1: CREATE LOBBY, 2: FIND LOBBY, 3: SETTINGS, 4: EXIT GAME
            static int menuSel = 0;
            const LocalizedString menuItems[5] = {
                STR_PLAY,
                STR_CREATE_LOBBY,
                STR_FIND_LOBBY,
                STR_SETTINGS,
                STR_EXIT_GAME
            };

            if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W))   menuSel = (menuSel + 4) % 5;
            if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) menuSel = (menuSel + 1) % 5;

            float menuStartX = 0.0f;
            float menuStartY = 210.0f;
            float menuGapY   = 60.0f;

            for (int m = 0; m < 5; m++) {
                float my = menuStartY + m * menuGapY;
                float mw = (menuSel == m) ? 390.0f : 290.0f;
                float mh = 50.0f;

                Rectangle itemRec = { menuStartX, my, mw, mh };
                bool isHov = CheckCollisionPointRec(mousePos, itemRec);
                if (isHov) menuSel = m;

                if (menuSel == m) {
                    DrawSlantedRect(0.0f, my, mw, mh, 16.0f, Color{ 34, 197, 94, 250 });
                    DrawSlantedRectLines(0.0f, my, mw, mh, 16.0f, 2.0f, Color{ 255, 255, 255, 230 });

                    if (texIconArrow.id != 0) {
                        float aw = 20.0f, ah = 28.0f;
                        DrawTexturePro(texIconArrow, Rectangle{ 0, 0, (float)texIconArrow.width, (float)texIconArrow.height },
                                       Rectangle{ 38.0f, my + 11.0f, aw, ah }, Vector2{0, 0}, 0.0f, WHITE);
                    }
                    DrawTextEx(fontUIBold, LocText(menuItems[m]), Vector2{ 68.0f, my + 10.0f }, 24, 0.4f, WHITE);
                } else {
                    DrawTextEx(fontUIBold, LocText(menuItems[m]), Vector2{ 50.0f, my + 12.0f }, 21, 0.4f, Color{ 255, 255, 255, 235 });
                }

                if ((isHov && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) || (menuSel == m && (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)))) {
                    if (m == 0) { // PLAY SOLO (Single Pirate Mode, 360 degree rotation)
                        LanShutdown();
                        singlePirateMode = true;
                        p1RimAngle = 3.14159f;
                        ResetBoatToStart();
                        currentScreen = SCREEN_GAMEPLAY;
                    } else if (m == 1) { // CREATE LOBBY
                        LanInitHost(7777, "8HF78N");
                        gHostReady = false;
                        gClientReady = false;
                        gSoloPracticeMode = false;
                        currentScreen = SCREEN_LAN_LOBBY;
                    } else if (m == 2) { // FIND LOBBY
                        LanStartDiscoveryListener(7778);
                        currentScreen = SCREEN_FIND_LOBBY;
                    } else if (m == 3) { // SETTINGS
                        currentScreen = SCREEN_SETTINGS;
                    } else if (m == 4) { // EXIT GAME
                        CloseWindow();
                        return 0;
                    }
                }
            }

            // Room code box next to FIND LOBBY
            float findY = menuStartY + 2 * menuGapY;
            DrawSlantedRect(380.0f, findY + 3.0f, 190.0f, 44.0f, 10.0f, Color{ 15, 30, 45, 215 });
            DrawSlantedRectLines(380.0f, findY + 3.0f, 190.0f, 44.0f, 10.0f, 1.2f, Color{ 45, 85, 120, 200 });
            DrawTextEx(fontUIBold, "8HF78N", Vector2{ 415.0f, findY + 11.0f }, 22, 0.5f, Color{ 52, 235, 120, 255 });

            // Language Toggle in Top-Right
            Rectangle langBtnRec = { scrW - 190.0f, 20.0f, 165.0f, 42.0f };
            bool hovLang = CheckCollisionPointRec(mousePos, langBtnRec);
            DrawSlantedRect(langBtnRec.x, langBtnRec.y, langBtnRec.width, langBtnRec.height, 10.0f, hovLang ? Color{ 45, 65, 95, 240 } : Color{ 20, 32, 48, 220 });
            DrawSlantedRectLines(langBtnRec.x, langBtnRec.y, langBtnRec.width, langBtnRec.height, 10.0f, 1.2f, Color{ 80, 115, 160, 200 });
            const char* langToggleTxt = (gCurrentLanguage == LANG_AR) ? "English [EN]" : LocText(STR_LANG_VALUE);
            Vector2 ltSz = MeasureTextEx(fontUIBold, langToggleTxt, 16, 0.3f);
            DrawTextEx(fontUIBold, langToggleTxt, Vector2{ langBtnRec.x + (langBtnRec.width - ltSz.x) * 0.5f, langBtnRec.y + 11.0f }, 16, 0.3f, Color{ 255, 225, 75, 255 });
            if (hovLang && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                gCurrentLanguage = (gCurrentLanguage == LANG_AR) ? LANG_EN : LANG_AR;
            }

            // Bottom-Left key hints
            DrawTextEx(fontUIBold, LocText(STR_ESC_BACK), Vector2{ 38.0f, scrH - 42.0f }, 15, 0.3f, Color{ 220, 230, 245, 220 });
            DrawTextEx(fontUIBold, LocText(STR_ENTER_SELECT), Vector2{ 175.0f, scrH - 42.0f }, 15, 0.3f, Color{ 220, 230, 245, 220 });

            // Bottom-Right Version Badge (v8.2.0)
            DrawSlantedRect(scrW - 165.0f, scrH - 52.0f, 145.0f, 38.0f, 10.0f, Color{ 18, 28, 42, 230 });
            DrawSlantedRectLines(scrW - 165.0f, scrH - 52.0f, 145.0f, 38.0f, 10.0f, 1.2f, Color{ 48, 70, 98, 220 });
            Vector2 verSz = MeasureTextEx(fontUIBold, GAME_VERSION_TAG, 18, 0.4f);
            DrawTextEx(fontUIBold, GAME_VERSION_TAG, Vector2{ scrW - 165.0f + (145.0f - verSz.x) * 0.5f, scrH - 44.0f }, 18, 0.4f, Color{ 172, 255, 45, 255 });
        }\n\n        """
        text = text[:m_idx] + clean_menu_block + text[sett_idx:]
        print("Replaced Main Menu with clean static oasis menu and PLAY SOLO mode!")

engine_path.write_text(text, encoding="utf-8")
print("Engine patch part 1 written!")

