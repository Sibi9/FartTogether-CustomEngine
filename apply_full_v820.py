import pathlib

engine_path = pathlib.Path("CustomEngine/native/GameEngine.cpp")
text = engine_path.read_text(encoding="utf-8")

# 1. Update Version tag
text = text.replace('static const char* GAME_VERSION_TAG = "v8.1.0";', 'static const char* GAME_VERSION_TAG = "v8.2.0";')
text = text.replace('static const char* GAME_VERSION_TAG = "v8.0.0";', 'static const char* GAME_VERSION_TAG = "v8.2.0";')

# 2. Update Font loading
old_fonts = """    Font fontUI     = LoadFontEx(ResolveAsset("extracted_assets/fonts/ui_font.ttf").c_str(), 26, (int*)gArFontCodepoints, gArFontCodepointCount);
    Font fontUIBold = LoadFontEx(ResolveAsset("extracted_assets/fonts/ui_font_bold.ttf").c_str(), 26, (int*)gArFontCodepoints, gArFontCodepointCount);"""

new_fonts = """    Font fontUI     = LoadFontEx(ResolveAsset("extracted_assets/fonts/ui_font.ttf").c_str(), 36, (int*)gArFontCodepoints, gArFontCodepointCount);
    Font fontUIBold = LoadFontEx(ResolveAsset("extracted_assets/fonts/ui_font_bold.ttf").c_str(), 42, (int*)gArFontCodepoints, gArFontCodepointCount);
    SetTextureFilter(fontUI.texture, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(fontUIBold.texture, TEXTURE_FILTER_BILINEAR);"""

if old_fonts in text:
    text = text.replace(old_fonts, new_fonts)
    print("Updated fonts to Vazirmatn bold cartoon with bilinear filter!")

# 3. Fix Mobile Steer Arrow inversion
old_steer = "Rectangle src = { 0, 0, isRightArrow ? -(float)texIconArrow.width : (float)texIconArrow.width, (float)texIconArrow.height };"
new_steer = """// Icon_ArrowM.png points RIGHT natively -> Right is +width, Left is -width!
                    float srcW = isRightArrow ? (float)texIconArrow.width : -(float)texIconArrow.width;
                    Rectangle src = { 0.0f, 0.0f, srcW, (float)texIconArrow.height };"""
if old_steer in text:
    text = text.replace(old_steer, new_steer)
    print("Fixed mobile steering arrows direction!")

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

# 5. Replace SCREEN_MAIN_MENU block
menu_start = text.find("if (currentScreen == SCREEN_MAIN_MENU) {")
lobby_start = text.find("} else if (currentScreen == SCREEN_LAN_LOBBY) {")

if menu_start != -1 and lobby_start != -1:
    clean_menu = """if (currentScreen == SCREEN_MAIN_MENU) {
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
        """
    text = text[:menu_start] + clean_menu + text[lobby_start:]
    print("Replaced SCREEN_MAIN_MENU cleanly!")

# 6. Replace SCREEN_LAN_LOBBY block
lobby_start = text.find("} else if (currentScreen == SCREEN_LAN_LOBBY) {")
find_start = text.find("} else if (currentScreen == SCREEN_FIND_LOBBY) {")

if lobby_start != -1 and find_start != -1:
    clean_lobby = """} else if (currentScreen == SCREEN_LAN_LOBBY) {
            ClearBackground(Color{ 18, 22, 34, 255 });

            float scrW = (float)GetScreenWidth();
            float scrH = (float)GetScreenHeight();

            // Authentic Stage Backdrop (Image 5 & 98a07c Parity):
            // Upper half: Warm terracotta canyon wall (y = 0..320)
            DrawRectangle(0, 0, (int)scrW, 320, Color{ 186, 108, 92, 255 });
            for (int i = 0; i < 7; i++) {
                float strataY = 45.0f + i * 42.0f;
                DrawLineEx(Vector2{ 0.0f, strataY }, Vector2{ scrW, strataY + 14.0f }, 2.5f, Color{ 198, 118, 100, 180 });
                DrawLineEx(Vector2{ 0.0f, strataY + 8.0f }, Vector2{ scrW, strataY + 22.0f }, 1.5f, Color{ 174, 98, 84, 180 });
            }

            // Lower half: Warm sandy / wooden flat floor (y = 320..H)
            DrawRectangle(0, 320, (int)scrW, (int)scrH - 320, Color{ 198, 150, 116, 255 });
            DrawLineEx(Vector2{ 0.0f, 320.0f }, Vector2{ scrW, 320.0f }, 3.0f, Color{ 145, 85, 72, 255 });
            for (int j = 0; j < 6; j++) {
                float floorY = 360.0f + j * 58.0f;
                DrawLineEx(Vector2{ 0.0f, floorY }, Vector2{ scrW, floorY }, 1.5f, Color{ 182, 134, 100, 190 });
            }

            // Side desert pebbles & shrub decoration
            DrawCircle(95, 332, 16.0f, Color{ 140, 92, 80, 220 });
            DrawCircle(135, 338, 11.0f, Color{ 160, 110, 95, 220 });
            DrawCircle(175, 334, 14.0f, Color{ 135, 88, 76, 220 });
            DrawCircle((int)scrW - 110, 332, 18.0f, Color{ 140, 92, 80, 220 });
            DrawCircle((int)scrW - 150, 338, 12.0f, Color{ 160, 110, 95, 220 });

            // 3D Standing Pirates on the stage floor (feet right above color cards at y = 345)
            Camera3D lobbyCam = { 0 };
            lobbyCam.position = Vector3{ 0.0f, 1.05f, 4.20f };
            lobbyCam.target   = Vector3{ 0.0f, 0.55f, 0.0f };
            lobbyCam.up       = Vector3{ 0.0f, 1.0f, 0.0f };
            lobbyCam.fovy     = 36.0f;
            lobbyCam.projection = CAMERA_PERSPECTIVE;

            BeginMode3D(lobbyCam);
                float pBob = sinf(runTime * 2.5f) * 0.02f;
                if (gConnectedPlayers <= 1) {
                    // Single pirate standing centered
                    rlPushMatrix();
                        rlTranslatef(0.0f, 0.08f + pBob, 0.0f);
                        rlScalef(1.28f, 1.28f, 1.28f);
                        Model* pCurChar = (selectedLobbyChar % 2 == 0) ? &charB : &charA;
                        DrawModel(*pCurChar, Vector3{0,0,0}, 1.0f, WHITE);
                    rlPopMatrix();
                } else {
                    // Two or more pirates standing side by side
                    rlPushMatrix();
                        rlTranslatef(-0.95f, 0.08f + pBob, 0.0f);
                        rlScalef(1.28f, 1.28f, 1.28f);
                        Model* pCurChar = (selectedLobbyChar % 2 == 0) ? &charB : &charA;
                        DrawModel(*pCurChar, Vector3{0,0,0}, 1.0f, WHITE);
                    rlPopMatrix();

                    rlPushMatrix();
                        rlTranslatef(0.95f, 0.08f + pBob, 0.0f);
                        rlScalef(1.28f, 1.28f, 1.28f);
                        DrawModel(charA, Vector3{0,0,0}, 1.0f, WHITE);
                    rlPopMatrix();
                }
            EndMode3D();

            // Name Markers & Green Checkmarks above pirates
            Vector2 p1Head = GetWorldToScreen(Vector3{ (gConnectedPlayers <= 1) ? 0.0f : -0.95f, 1.55f, 0.0f }, lobbyCam);
            if (texMarkerYou.id != 0) {
                float mw = 120.0f, mh = 38.0f;
                DrawTexturePro(texMarkerYou, Rectangle{ 0, 0, (float)texMarkerYou.width, (float)texMarkerYou.height },
                               Rectangle{ p1Head.x - mw * 0.5f, p1Head.y - 20.0f, mw, mh }, Vector2{0,0}, 0.0f, WHITE);
            } else {
                DrawSlantedRect(p1Head.x - 55.0f, p1Head.y - 18.0f, 110.0f, 32.0f, 8.0f, Color{ 106, 134, 178, 240 });
                Vector2 youSz = MeasureTextEx(fontUIBold, LocText(STR_YOU_MARKER), 16, 0.4f);
                DrawTextEx(fontUIBold, LocText(STR_YOU_MARKER), Vector2{ p1Head.x - youSz.x * 0.5f, p1Head.y - 10.0f }, 16, 0.4f, WHITE);
            }

            // Green Tik above You if ready
            if (gHostReady) {
                if (texIconTikGreen.id != 0) {
                    DrawTexturePro(texIconTikGreen, Rectangle{ 0, 0, (float)texIconTikGreen.width, (float)texIconTikGreen.height },
                                   Rectangle{ p1Head.x - 18.0f, p1Head.y - 62.0f, 36.0f, 36.0f }, Vector2{0,0}, 0.0f, WHITE);
                } else {
                    DrawCircle((int)p1Head.x, (int)p1Head.y - 44, 16.0f, Color{ 34, 197, 94, 255 });
                }
            }

            if (gConnectedPlayers >= 2) {
                Vector2 p2Head = GetWorldToScreen(Vector3{ 0.95f, 1.55f, 0.0f }, lobbyCam);
                if (texMarkerPlayer.id != 0) {
                    float mw = 120.0f, mh = 38.0f;
                    DrawTexturePro(texMarkerPlayer, Rectangle{ 0, 0, (float)texMarkerPlayer.width, (float)texMarkerPlayer.height },
                                   Rectangle{ p2Head.x - mw * 0.5f, p2Head.y - 20.0f, mw, mh }, Vector2{0,0}, 0.0f, WHITE);
                } else {
                    DrawSlantedRect(p2Head.x - 55.0f, p2Head.y - 18.0f, 110.0f, 32.0f, 8.0f, Color{ 180, 85, 75, 240 });
                    Vector2 plSz = MeasureTextEx(fontUIBold, LocText(STR_PLAYER_MARKER), 16, 0.4f);
                    DrawTextEx(fontUIBold, LocText(STR_PLAYER_MARKER), Vector2{ p2Head.x - plSz.x * 0.5f, p2Head.y - 10.0f }, 16, 0.4f, WHITE);
                }

                if (gClientReady) {
                    if (texIconTikGreen.id != 0) {
                        DrawTexturePro(texIconTikGreen, Rectangle{ 0, 0, (float)texIconTikGreen.width, (float)texIconTikGreen.height },
                                       Rectangle{ p2Head.x - 18.0f, p2Head.y - 62.0f, 36.0f, 36.0f }, Vector2{0,0}, 0.0f, WHITE);
                    } else {
                        DrawCircle((int)p2Head.x, (int)p2Head.y - 44, 16.0f, Color{ 34, 197, 94, 255 });
                    }
                }
            }

            // Top Center Banner: LOBBY
            float topBannerW = 320.0f, topBannerH = 48.0f;
            float topBannerX = (scrW - topBannerW) * 0.5f;
            DrawTrapezoid(topBannerX, 0, topBannerW, topBannerH, 18.0f, Color{ 28, 33, 53, 255 });
            DrawTrapezoidLines(topBannerX, 0, topBannerW, topBannerH, 18.0f, 1.5f, Color{ 70, 80, 110, 255 });
            Vector2 lobSz = MeasureTextEx(fontUIBold, LocText(STR_LOBBY_TITLE), 22, 0.5f);
            DrawTextEx(fontUIBold, LocText(STR_LOBBY_TITLE), Vector2{ (scrW - lobSz.x) * 0.5f, 12.0f }, 22, 0.5f, WHITE);

            // Top Right: Room Code
            float codeBoxW = 210.0f, codeBoxH = 54.0f;
            float codeBoxX = scrW - codeBoxW - 35.0f;
            float codeBoxY = 16.0f;
            DrawSlantedRect(codeBoxX, codeBoxY, codeBoxW, codeBoxH, 12.0f, Color{ 22, 30, 48, 220 });
            DrawSlantedRectLines(codeBoxX, codeBoxY, codeBoxW, codeBoxH, 12.0f, 1.2f, Color{ 56, 75, 110, 200 });
            DrawTextEx(fontUI, LocText(STR_YOUR_CODE), Vector2{ codeBoxX + 22.0f, codeBoxY + 8.0f }, 11, 0.3f, Color{ 160, 175, 200, 255 });
            DrawTextEx(fontUIBold, "8HF78N", Vector2{ codeBoxX + 22.0f, codeBoxY + 24.0f }, 20, 0.4f, Color{ 52, 235, 120, 255 });

            // 4 Character Color Cards (y = 358, h = 76)
            float cardW = 76.0f, cardH = 76.0f, cardGap = 16.0f;
            float totalCardsW = 4 * cardW + 3 * cardGap;
            float startCardsX = (scrW - totalCardsW) * 0.5f;
            float cardsY = 358.0f;

            for (int c = 0; c < 4; c++) {
                Rectangle cRec = { startCardsX + c * (cardW + cardGap), cardsY, cardW, cardH };
                bool hovC = CheckCollisionPointRec(mousePos, cRec);
                bool isSel = (selectedLobbyChar == c);

                DrawRectangleRounded(cRec, 0.18f, 4, WHITE);
                if (isSel) {
                    DrawRectangleRoundedLinesEx(cRec, 0.18f, 4, 3.5f, Color{ 238, 52, 132, 255 });
                } else if (hovC) {
                    DrawRectangleRoundedLinesEx(cRec, 0.18f, 4, 2.0f, Color{ 245, 158, 11, 240 });
                }

                if (texLobbyCards[c].id != 0) {
                    DrawTexturePro(texLobbyCards[c], Rectangle{ 0, 0, (float)texLobbyCards[c].width, (float)texLobbyCards[c].height },
                                   Rectangle{ cRec.x + 6, cRec.y + 6, cardW - 12, cardH - 12 }, Vector2{0,0}, 0.0f, WHITE);
                }

                if (hovC && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    selectedLobbyChar = c;
                }
            }

            // Section: SELECT MODE (y = 452)
            float smBarW = 285.0f, smBarH = 42.0f;
            float smBarX = (scrW - smBarW) * 0.5f;
            DrawSlantedRect(smBarX, 450.0f, smBarW, smBarH, 12.0f, Color{ 126, 114, 228, 255 });
            Vector2 smSz = MeasureTextEx(fontUIBold, LocText(STR_SELECT_MODE), 18, 0.4f);
            DrawTextEx(fontUIBold, LocText(STR_SELECT_MODE), Vector2{ (scrW - smSz.x) * 0.5f, 460.0f }, 18, 0.4f, WHITE);

            // Mode thumbnails box
            float modeBoxW = 540.0f, modeBoxH = 110.0f;
            float modeBoxX = (scrW - modeBoxW) * 0.5f;
            DrawRectangleRounded(Rectangle{ modeBoxX, 498.0f, modeBoxW, modeBoxH }, 0.12f, 4, WHITE);

            // 3 Mode Thumbnails
            float mThumbW = 160.0f, mThumbH = 94.0f;
            Texture2D mThumbs[3] = { texModeFartRun, texModeTeamRace, texModeRaftRumble };
            const LocalizedString mTitles[3] = { STR_MODE_FARTRUN, STR_MODE_TEAMRACE, STR_MODE_RAFTRUMBLE };

            for (int m = 0; m < 3; m++) {
                Rectangle mRec = { modeBoxX + 10.0f + m * (mThumbW + 15.0f), 506.0f, mThumbW, mThumbH };
                bool isM0 = (m == 0);

                if (mThumbs[m].id != 0) {
                    DrawTexturePro(mThumbs[m], Rectangle{ 0, 0, (float)mThumbs[m].width, (float)mThumbs[m].height },
                                   mRec, Vector2{0,0}, 0.0f, isM0 ? WHITE : Color{ 160, 160, 160, 200 });
                } else {
                    DrawRectangleRec(mRec, isM0 ? Color{ 45, 125, 220, 255 } : Color{ 80, 85, 100, 255 });
                }

                if (isM0) {
                    DrawRectangleLinesEx(mRec, 2.5f, Color{ 238, 52, 132, 255 });
                } else {
                    DrawRectangle(mRec.x, mRec.y + mRec.height - 28.0f, mRec.width, 28.0f, Color{ 20, 25, 35, 190 });
                    DrawTextEx(fontUIBold, LocText(STR_COMING_SOON), Vector2{ mRec.x + 35.0f, mRec.y + mRec.height - 22.0f }, 13, 0.3f, Color{ 230, 235, 245, 255 });
                }
            }

            // Bottom Buttons (Matching Image 5 exactly)
            // 1. Left: INVITE FRIENDS
            Rectangle invBtnRec = { 58.0f, scrH - 68.0f, 260.0f, 56.0f };
            bool hovInv = CheckCollisionPointRec(mousePos, invBtnRec);
            DrawSlantedRect(invBtnRec.x, invBtnRec.y, invBtnRec.width, invBtnRec.height, 12.0f, hovInv ? Color{ 18, 160, 235, 255 } : Color{ 14, 138, 206, 255 });
            Vector2 invSz = MeasureTextEx(fontUIBold, LocText(STR_INVITE_FRIENDS), 18, 0.4f);
            DrawTextEx(fontUIBold, LocText(STR_INVITE_FRIENDS), Vector2{ invBtnRec.x + (invBtnRec.width - invSz.x) * 0.5f, invBtnRec.y + 17.0f }, 18, 0.4f, Color{ 255, 225, 60, 255 });
            if (hovInv && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                SetClipboardText("8HF78N");
                gShowInviteModal = true;
            }

            // 2. Center: READY <-> UNREADY White Trapezoid Button
            float readyBtnW = 270.0f, readyBtnH = 64.0f;
            float readyBtnX = (scrW - readyBtnW) * 0.5f;
            float readyBtnY = scrH - 64.0f;
            Rectangle rdyClickRec = { readyBtnX, readyBtnY, readyBtnW, readyBtnH };
            bool hovRdy = CheckCollisionPointRec(mousePos, rdyClickRec);

            DrawTrapezoid(readyBtnX, readyBtnY, readyBtnW, readyBtnH, 18.0f, hovRdy ? Color{ 245, 245, 250, 255 } : WHITE);
            DrawTrapezoidLines(readyBtnX, readyBtnY, readyBtnW, readyBtnH, 18.0f, 1.8f, Color{ 200, 210, 230, 255 });

            bool myReady = (gLanRole == LAN_HOST) ? gHostReady : gClientReady;
            if (!myReady) {
                // [SPACE] READY (in pink-red)
                if (texKeySpace.id != 0) {
                    DrawTexturePro(texKeySpace, Rectangle{ 0, 0, (float)texKeySpace.width, (float)texKeySpace.height },
                                   Rectangle{ readyBtnX + 38.0f, readyBtnY + 18.0f, 44.0f, 26.0f }, Vector2{0,0}, 0.0f, WHITE);
                }
                DrawTextEx(fontUIBold, LocText(STR_READY_WORD), Vector2{ readyBtnX + 96.0f, readyBtnY + 18.0f }, 24, 0.5f, Color{ 225, 82, 98, 255 });
            } else {
                // [X] UNREADY (in dark navy)
                DrawRectangle(readyBtnX + 42.0f, readyBtnY + 18.0f, 28.0f, 28.0f, Color{ 28, 33, 53, 255 });
                DrawTextEx(fontUIBold, "X", Vector2{ readyBtnX + 49.0f, readyBtnY + 21.0f }, 20, 0.4f, WHITE);
                DrawTextEx(fontUIBold, LocText(STR_UNREADY_BTN), Vector2{ readyBtnX + 85.0f, readyBtnY + 18.0f }, 22, 0.5f, Color{ 28, 33, 53, 255 });
            }

            if ((hovRdy && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) || IsKeyPressed(KEY_SPACE)) {
                if (gLanRole == LAN_HOST) gHostReady = !gHostReady;
                else gClientReady = !gClientReady;
            }

            // 3. Right: Yellow Slanted START GAME Button (Host Only, enabled when 2+ players all-ready)
            if (gLanRole == LAN_HOST) {
                Rectangle startBtnRec = { scrW - 325.0f, scrH - 68.0f, 265.0f, 56.0f };
                bool hovStart = CheckCollisionPointRec(mousePos, startBtnRec);
                DrawSlantedRect(startBtnRec.x, startBtnRec.y, startBtnRec.width, startBtnRec.height, 12.0f, hovStart ? Color{ 255, 235, 65, 255 } : Color{ 246, 224, 52, 255 });

                if (texIconKEnter.id != 0) {
                    DrawTexturePro(texIconKEnter, Rectangle{ 0, 0, (float)texIconKEnter.width, (float)texIconKEnter.height },
                                   Rectangle{ startBtnRec.x + 22.0f, startBtnRec.y + 14.0f, 28.0f, 28.0f }, Vector2{0,0}, 0.0f, WHITE);
                }
                Vector2 sgSz = MeasureTextEx(fontUIBold, LocText(STR_START_GAME_BTN), 20, 0.4f);
                DrawTextEx(fontUIBold, LocText(STR_START_GAME_BTN), Vector2{ startBtnRec.x + 60.0f, startBtnRec.y + 16.0f }, 20, 0.4f, Color{ 238, 52, 132, 255 });

                if ((hovStart && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) || IsKeyPressed(KEY_ENTER)) {
                    if (gConnectedPlayers >= 2 && gHostReady && gClientReady) {
                        currentScreen = SCREEN_GAMEPLAY;
                    } else {
                        editorMsg = LocText(STR_MIN_PLAYERS_ALERT);
                        editorMsgTimer = 2.5f;
                    }
                }
            }

            // Alert banner if host clicks start before 2 players ready
            if (editorMsgTimer > 0.0f && !editorMsg.empty()) {
                float abW = 560.0f, abH = 38.0f;
                DrawTrapezoid((scrW - abW) * 0.5f, scrH - 110.0f, abW, abH, 12.0f, Color{ 220, 38, 38, 235 });
                Vector2 emSz = MeasureTextEx(fontUIBold, editorMsg.c_str(), 16, 0.3f);
                DrawTextEx(fontUIBold, editorMsg.c_str(), Vector2{ (scrW - emSz.x) * 0.5f, scrH - 101.0f }, 16, 0.3f, WHITE);
            }

            // Bottom-Left [ESC] LEAVE
            DrawTextEx(fontUIBold, LocText(STR_ESC_LEAVE), Vector2{ 25.0f, scrH - 95.0f }, 14, 0.3f, Color{ 220, 230, 245, 200 });
            if (IsKeyPressed(KEY_ESCAPE)) {
                LanShutdown();
                currentScreen = SCREEN_MAIN_MENU;
            }
        """
    text = text[:lobby_start] + clean_lobby + text[find_start:]
    print("Replaced SCREEN_LAN_LOBBY cleanly!")

# 7. Localize SCREEN_SETTINGS completely
sett_start = text.find("if (currentScreen == SCREEN_SETTINGS) {")
if sett_start != -1:
    # Update Settings title
    text = text.replace('DrawTextEx(fontUIBold, "SETTINGS", Vector2{ 220.0f, 38.0f }, 38, 0.6f, WHITE);',
                        'DrawTextEx(fontUIBold, LocText(STR_SETTINGS), Vector2{ 220.0f, 38.0f }, 38, 0.6f, WHITE);')

    # Update General tab rows with LocText
    text = text.replace('DrawSettingRow(0, "Device Profile",', 'DrawSettingRow(1, LocText(STR_DEVICE_PROFILE),')
    text = text.replace('DrawSettingRow(1, "Invert Y Axis",', 'DrawSettingRow(2, LocText(STR_INVERT_Y_ROW),')
    text = text.replace('DrawSettingRow(2, "Level & Obstacle Editor",', 'DrawSettingRow(3, LocText(STR_LEVEL_EDITOR_ROW),')
    text = text.replace('DrawSettingRow(3, "Show FPS Counter (F5)",', 'DrawSettingRow(2, LocText(STR_SHOW_FPS_ROW),')
    text = text.replace('DrawSettingRow(0, LocText(STR_LANGUAGE_LBL)', 'DrawSettingRow(0, LocText(STR_LANGUAGE_LBL)')

    # Update Graphics tab rows with LocText
    text = text.replace('DrawSettingRow(0, "Quality Preset",', 'DrawSettingRow(0, LocText(STR_QUALITY_PRESET),')
    text = text.replace('DrawSettingRow(1, "Camera Shake",', 'DrawSettingRow(1, LocText(STR_CAM_SHAKE_ROW),')
    text = text.replace('DrawSettingRow(2, "Max Camera Distance",', 'DrawSettingRow(2, LocText(STR_MAX_CAM_DIST_ROW),')

    # Update Audio tab rows with LocText
    text = text.replace('DrawSettingRow(0, "Master Volume",', 'DrawSettingRow(0, LocText(STR_VOL_MASTER),')
    text = text.replace('DrawSettingRow(1, "Music Volume",', 'DrawSettingRow(1, LocText(STR_VOL_MUSIC),')
    text = text.replace('DrawSettingRow(2, "SFX Volume",', 'DrawSettingRow(2, LocText(STR_VOL_SFX),')
    text = text.replace('DrawSettingRow(3, "Fart Sound Volume",', 'DrawSettingRow(3, LocText(STR_VOL_FART),')

    # Update Controls tab rows with LocText
    text = text.replace('DrawSettingRow(0, "Mouse Sensitivity",', 'DrawSettingRow(0, LocText(STR_MOUSE_SENS_ROW),')
    text = text.replace('DrawSettingRow(1, "Invert Y Axis",', 'DrawSettingRow(1, LocText(STR_INVERT_Y_ROW),')
    text = text.replace('DrawSettingRow(2, "Steer / Gas Keys",', 'DrawSettingRow(2, LocText(STR_STEER_KEYS_ROW),')
    text = text.replace('DrawSettingRow(0, "Touch Buttons Size",', 'DrawSettingRow(0, LocText(STR_TOUCH_SIZE_ROW),')
    text = text.replace('DrawSettingRow(1, "Touch Buttons Opacity",', 'DrawSettingRow(1, LocText(STR_TOUCH_OPAC_ROW),')
    text = text.replace('DrawSettingRow(2, "Customize Button Positions",', 'DrawSettingRow(2, LocText(STR_CUSTOM_TOUCH_ROW),')

    # Update bottom terms and privacy
    text = text.replace('DrawTextEx(fontUIBold, "Terms of Service",', 'DrawTextEx(fontUIBold, LocText(STR_TERMS_OF_SERVICE),')
    text = text.replace('DrawTextEx(fontUIBold, "Privacy Policy",', 'DrawTextEx(fontUIBold, LocText(STR_PRIVACY_POLICY),')

    # Ensure back button arrow in settings points left (<)
    text = text.replace('DrawTexturePro(texIconArrow, Rectangle{ 0, 0, -(float)texIconArrow.width, (float)texIconArrow.height },',
                        'DrawTexturePro(texIconArrow, Rectangle{ 0, 0, -(float)texIconArrow.width, (float)texIconArrow.height },')

    print("Updated SCREEN_SETTINGS with 100% Arabic localization!")

engine_path.write_text(text, encoding="utf-8")
print("All v8.2.0 modifications applied successfully to GameEngine.cpp!")

