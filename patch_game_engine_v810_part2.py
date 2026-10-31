import pathlib

engine_path = pathlib.Path("CustomEngine/native/GameEngine.cpp")
text = engine_path.read_text(encoding="utf-8")

# Let's locate the exact start and end markers
start_marker = "// Smart Mobile Touch HUD Controls Rendering (Plan v8.0 Task 5)"
end_marker = "} else if (currentScreen == SCREEN_SETTINGS) {"

start_idx = text.find(start_marker)
end_idx = text.find(end_marker)

if start_idx == -1 or end_idx == -1:
    print(f"Error: Markers not found! start={start_idx}, end={end_idx}")
    exit(1)

new_block = '''// =====================================================================
        // Smart Mobile Touch HUD Controls Rendering (Plan v8.1.0 Task 5 - Zero Text Cartoon)
        // =====================================================================
        if (isMobileTouchHUD && !editorMode) {
            float alphaMul = Clamp(gSettings.touchBtnOpacity, 0.30f, 1.0f);
            unsigned char aOuter = (unsigned char)(240 * alphaMul);
            unsigned char aInner = (unsigned char)(220 * alphaMul);

            // 1. Cartoon Steer Left & Steer Right Buttons (Original Game Arrow Icon, ZERO text)
            auto DrawCartoonSteerBtn = [&](Vector2 center, float rad, bool isRightArrow, bool isPressed) {
                Color rimCol  = isPressed ? Color{ 255, 230, 60, aOuter } : Color{ 245, 158, 18, aOuter };
                Color bodyCol = isPressed ? Color{ 255, 38, 118, aInner } : Color{ 30, 45, 72, aInner };
                DrawCircleV(Vector2{ center.x, center.y + 4.0f }, rad + 4.0f, Color{ 10, 15, 25, (unsigned char)(140 * alphaMul) });
                DrawCircleV(center, rad + 4.0f, rimCol);
                DrawCircleV(center, rad - 3.0f, bodyCol);
                // Cartoon gloss highlight
                DrawCircleV(Vector2{ center.x - rad * 0.25f, center.y - rad * 0.28f }, rad * 0.35f, Color{ 255, 255, 255, (unsigned char)(45 * alphaMul) });

                if (texIconArrow.id != 0) {
                    float aw = rad * 0.95f, ah = rad * 1.15f;
                    Rectangle src = { 0, 0, isRightArrow ? -(float)texIconArrow.width : (float)texIconArrow.width, (float)texIconArrow.height };
                    Rectangle dst = { center.x, center.y, aw, ah };
                    DrawTexturePro(texIconArrow, src, dst, Vector2{ aw * 0.5f, ah * 0.5f }, 0.0f, WHITE);
                } else {
                    DrawTextEx(fontUIBold, isRightArrow ? ">" : "<", Vector2{ center.x - 8.0f, center.y - 12.0f }, 24, 0.3f, WHITE);
                }
            };

            DrawCartoonSteerBtn(posLeftBtn,  rSteer, false, touchSteerLeft);
            DrawCartoonSteerBtn(posRightBtn, rSteer, true,  touchSteerRight);

            // 2. Cartoon Fart Gas Button (Rubber Float Ring Style + 3 Animated Green Fart Clouds — ZERO TEXT!)
            {
                float pulse = touchGas ? (1.08f + 0.06f * sinf(runTime * 18.0f)) : 1.0f;
                float gRad  = rGas * pulse;
                Color rimCol  = touchGas ? Color{ 255, 90, 30, aOuter } : Color{ 250, 185, 25, aOuter };
                Color bodyCol = touchGas ? Color{ 220, 48, 48, aInner } : Color{ 42, 168, 52, aInner };

                DrawCircleV(Vector2{ posGasBtn.x, posGasBtn.y + 5.0f }, gRad + 5.0f, Color{ 10, 15, 25, (unsigned char)(150 * alphaMul) });
                DrawCircleV(posGasBtn, gRad + 5.0f, rimCol);
                DrawCircleV(posGasBtn, gRad - 3.0f, bodyCol);

                // Animated swirling green fart clouds inside button
                Texture2D fartIconTex = texFart[0];
                if (fartIconTex.id != 0) {
                    float fSz1 = gRad * 1.35f;
                    float fSz2 = gRad * 1.05f;
                    Rectangle srcF = { 0, 0, (float)fartIconTex.width, (float)fartIconTex.height };
                    DrawTexturePro(fartIconTex, srcF, Rectangle{ posGasBtn.x + gRad * 0.12f, posGasBtn.y + gRad * 0.08f, fSz1, fSz1 * 0.78f },
                                   Vector2{ fSz1 * 0.5f, fSz1 * 0.39f }, sinf(runTime * 3.0f) * 12.0f, Color{ 135, 255, 55, aOuter });
                    DrawTexturePro(fartIconTex, srcF, Rectangle{ posGasBtn.x - gRad * 0.18f, posGasBtn.y - gRad * 0.10f, fSz2, fSz2 * 0.78f },
                                   Vector2{ fSz2 * 0.5f, fSz2 * 0.39f }, -15.0f + cosf(runTime * 4.0f) * 10.0f, Color{ 195, 255, 95, aOuter });
                } else {
                    DrawCircleV(Vector2{ posGasBtn.x + gRad * 0.18f, posGasBtn.y + gRad * 0.10f }, gRad * 0.42f, Color{ 130, 245, 50, aOuter });
                    DrawCircleV(Vector2{ posGasBtn.x - gRad * 0.15f, posGasBtn.y + gRad * 0.18f }, gRad * 0.35f, Color{ 155, 255, 70, aOuter });
                    DrawCircleV(Vector2{ posGasBtn.x + gRad * 0.05f, posGasBtn.y - gRad * 0.15f }, gRad * 0.38f, Color{ 175, 255, 95, aOuter });
                }

                // Cartoon speed thrust streaks
                DrawLineEx(Vector2{ posGasBtn.x - gRad * 0.55f, posGasBtn.y + gRad * 0.25f },
                           Vector2{ posGasBtn.x - gRad * 0.15f, posGasBtn.y + gRad * 0.05f }, 3.5f, Color{ 255, 255, 255, (unsigned char)(190 * alphaMul) });
                DrawLineEx(Vector2{ posGasBtn.x - gRad * 0.48f, posGasBtn.y + gRad * 0.42f },
                           Vector2{ posGasBtn.x - gRad * 0.10f, posGasBtn.y + gRad * 0.22f }, 3.0f, Color{ 255, 255, 255, (unsigned char)(170 * alphaMul) });
            }

            // 3. Settings Gear Button
            DrawCircleV(posGearBtn, rGear, Color{ 51, 65, 85, 220 });
            DrawCircleLines((int)posGearBtn.x, (int)posGearBtn.y, rGear, Color{ 203, 213, 225, 200 });
            DrawTextEx(fontUIBold, "*", Vector2{ posGearBtn.x - 6.0f, posGearBtn.y - 10.0f }, 20, 0.3f, WHITE);
        }

        // Live FPS Counter Badge — Relocated to (x = 258, y = 14) to NEVER overlap race timer
        if (gSettings.showFPS) {
            float fpsX = 258.0f, fpsY = 14.0f;
            DrawRectangleRounded(Rectangle{ fpsX, fpsY, 110.0f, 28.0f }, 0.4f, 4, Color{ 15, 23, 42, 230 });
            DrawRectangleRoundedLinesEx(Rectangle{ fpsX, fpsY, 110.0f, 28.0f }, 0.4f, 4, 1.5f, Color{ 34, 197, 94, 255 });
            DrawTextEx(fontUIBold, TextFormat("FPS: %d", GetFPS()), Vector2{ fpsX + 16.0f, fpsY + 6.0f }, 15, 0.4f, Color{ 34, 197, 94, 255 });
        }
        } else if (currentScreen == SCREEN_MAIN_MENU) {
            ClearBackground(Color{ 18, 22, 34, 255 });

            float scrW = (float)GetScreenWidth();
            float scrH = (float)GetScreenHeight();
            float scaleX = scrW / 1280.0f;
            float scaleY = scrH / 720.0f;

            // 1. 2.5D Animated Oasis Artwork
            if (texMenuOasisClean.id != 0) {
                DrawTexturePro(texMenuOasisClean, Rectangle{ 0, 0, (float)texMenuOasisClean.width, (float)texMenuOasisClean.height },
                               Rectangle{ 0, 0, scrW, scrH }, Vector2{0, 0}, 0.0f, WHITE);
            }

            // Subtle water shimmer
            float pondRipple = sinf(runTime * 2.0f) * 0.06f + 0.06f;
            DrawRectangleRounded(Rectangle{ 320.0f * scaleX, 410.0f * scaleY, 490.0f * scaleX, 185.0f * scaleY }, 0.45f, 6,
                                 Color{ 64, 224, 208, (unsigned char)(pondRipple * 255.0f) });

            // 2. Animated Center Purple Pirate Sleeping on Raft
            float raftDriftX = sinf(runTime * 0.95f) * 20.0f + cosf(runTime * 0.55f) * 8.0f;
            float raftDriftY = sinf(runTime * 1.65f) * 6.5f;
            float raftTilt   = sinf(runTime * 1.25f) * 3.2f;

            float snoreCycle = fmodf(runTime, 3.6f);
            if (snoreCycle < 0.08f) {
                if (!IsSoundPlaying(sndSnore)) PlaySound(sndSnore);
            }
            float bellyBreath = 1.0f + 0.045f * sinf(snoreCycle * 3.14159f / 1.8f);

            if (texMenuRaftSleep.id != 0) {
                float rw = 260.0f * scaleX * bellyBreath;
                float rh = 195.0f * scaleY * bellyBreath;
                Rectangle srcRaft = { 0, 0, (float)texMenuRaftSleep.width, (float)texMenuRaftSleep.height };
                Rectangle dstRaft = { (670.0f + raftDriftX) * scaleX, (417.0f + raftDriftY) * scaleY, rw, rh };
                Vector2 raftOrigin = { rw * 0.5f, rh * 0.5f };
                DrawTexturePro(texMenuRaftSleep, srcRaft, dstRaft, raftOrigin, raftTilt, WHITE);
            }

            // Animated Snot Bubble (Fixed in v8.1.0: Small realistic 4px..14px at pirate's nose, no giant circle!)
            Vector2 snoutPos = { (644.0f + raftDriftX) * scaleX, (372.0f + raftDriftY) * scaleY };
            if (snoreCycle < 3.0f) {
                float growT = snoreCycle / 3.0f;
                float snotRadius = (4.0f + 10.0f * sinf(growT * 3.14159f * 0.5f)) * scaleY;
                snotRadius += sinf(runTime * 15.0f) * 0.8f * scaleY; // subtle wobble
                Vector2 bubbleCenter = { snoutPos.x - snotRadius * 0.35f, snoutPos.y - snotRadius * 0.45f };

                DrawCircleV(bubbleCenter, snotRadius, Color{ 175, 240, 255, 170 });
                DrawCircleLines((int)bubbleCenter.x, (int)bubbleCenter.y, snotRadius, Color{ 240, 252, 255, 230 });
                DrawCircleV(Vector2{ bubbleCenter.x - snotRadius * 0.3f, bubbleCenter.y - snotRadius * 0.3f }, snotRadius * 0.28f, Color{ 255, 255, 255, 220 });
            } else {
                // Popping animation (POP!)
                float popT = (snoreCycle - 3.0f) / 0.6f;
                for (int d = 0; d < 6; d++) {
                    float ang = (float)d * (6.28318f / 6.0f);
                    float dropDist = (8.0f + popT * 20.0f) * scaleY;
                    Vector2 dropPos = { snoutPos.x + cosf(ang) * dropDist, snoutPos.y + sinf(ang) * dropDist };
                    DrawCircleV(dropPos, (2.5f * (1.0f - popT)) * scaleY, Color{ 180, 242, 255, (unsigned char)(210 * (1.0f - popT)) });
                }
                DrawTextEx(fontUIBold, "POP!", Vector2{ snoutPos.x + 6.0f * scaleX, snoutPos.y - 18.0f * scaleY }, 15.0f * scaleY, 0.4f, Color{ 255, 255, 130, (unsigned char)(255 * (1.0f - popT)) });
            }

            // 3. Orange Pirate Eating at Table + Munching & Periodic Burping
            float eatCycle = fmodf(runTime, 9.6f);
            if (eatCycle < 7.8f) {
                float subCycle = fmodf(eatCycle, 2.6f);
                if (subCycle >= 0.8f && subCycle < 2.4f) {
                    if (!IsSoundPlaying(sndEatMunch)) PlaySound(sndEatMunch);
                    for (int c = 0; c < 3; c++) {
                        float cx = (855.0f + sinf(runTime * 12.0f + c * 2.1f) * 12.0f) * scaleX;
                        float cy = (485.0f + fmodf(runTime * 28.0f + c * 8.0f, 22.0f)) * scaleY;
                        DrawCircle((int)cx, (int)cy, 2.8f * scaleY, Color{ 245, 190, 85, 220 });
                    }
                }
            } else {
                float burpT = (eatCycle - 7.8f) / 1.8f;
                if (burpT < 0.12f) {
                    if (!IsSoundPlaying(sndBurp)) PlaySound(sndBurp);
                }
                for (int r = 0; r < 3; r++) {
                    float ringPhase = fmodf(burpT * 2.2f + (float)r * 0.33f, 1.0f);
                    float ringR = (8.0f + ringPhase * 42.0f) * scaleY;
                    float ringX = (855.0f - ringPhase * 28.0f) * scaleX;
                    float ringY = (475.0f - ringPhase * 24.0f) * scaleY;
                    DrawCircleLines((int)ringX, (int)ringY, ringR, Color{ 85, 235, 55, (unsigned char)(210 * (1.0f - ringPhase)) });
                }
                DrawTextEx(fontUIBold, "*BURP!*", Vector2{ 805.0f * scaleX, (430.0f - burpT * 22.0f) * scaleY }, 18.0f * scaleY, 0.4f, Color{ 130, 255, 70, (unsigned char)(255 * (1.0f - burpT)) });

                float fartPulse = 1.0f + sinf(runTime * 22.0f) * 0.22f;
                DrawCircle((int)(1015.0f * scaleX), (int)(615.0f * scaleY), 34.0f * scaleY * fartPulse, Color{ 105, 230, 42, 160 });
                DrawCircle((int)(1038.0f * scaleX), (int)(605.0f * scaleY), 24.0f * scaleY * fartPulse, Color{ 140, 255, 65, 180 });
            }

            // 4. Logo Top-Left
            if (texMainLogo.id != 0) {
                float logoW = 340.0f * scaleX;
                float logoH = 175.0f * scaleY;
                DrawTexturePro(texMainLogo, Rectangle{ 0, 0, (float)texMainLogo.width, (float)texMainLogo.height },
                               Rectangle{ 42.0f * scaleX, 22.0f * scaleY, logoW, logoH }, Vector2{0, 0}, 0.0f, WHITE);
            }

            // Language Switcher Button Top-Right [ العربية / English ]
            Rectangle langBtnRec = { scrW - 190.0f * scaleX, 22.0f * scaleY, 160.0f * scaleX, 42.0f * scaleY };
            bool hovLang = CheckCollisionPointRec(mousePos, langBtnRec);
            DrawSlantedRect(langBtnRec.x, langBtnRec.y, langBtnRec.width, langBtnRec.height, 12.0f, hovLang ? Color{ 45, 65, 95, 240 } : Color{ 25, 38, 58, 220 });
            DrawSlantedRectLines(langBtnRec.x, langBtnRec.y, langBtnRec.width, langBtnRec.height, 12.0f, 1.5f, hovLang ? Color{ 56, 189, 248, 255 } : Color{ 70, 95, 130, 200 });
            const char* langStr = (gCurrentLanguage == LANG_AR) ? "English [EN]" : (const char*)u8"العربية [AR]";
            Vector2 langSz = MeasureTextEx(fontUIBold, langStr, 15 * scaleY, 0.4f);
            DrawTextEx(fontUIBold, langStr, Vector2{ langBtnRec.x + (langBtnRec.width - langSz.x) * 0.5f, langBtnRec.y + 12.0f * scaleY }, 15 * scaleY, 0.4f, Color{ 255, 235, 60, 255 });
            if (hovLang && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                gCurrentLanguage = (gCurrentLanguage == LANG_AR) ? LANG_EN : LANG_AR;
                gSettings.language = (int)gCurrentLanguage;
                SaveGameSettingsJSON("game_settings.json");
            }

            // 5. Clean Menu Items (v8.1.0: Added PLAY at top, removed Credits and Local Co-op)
            const LocalizedString menuItems[5] = {
                STR_PLAY,
                STR_CREATE_LOBBY,
                STR_FIND_LOBBY,
                STR_SETTINGS,
                STR_EXIT_GAME
            };
            float menuYStarts[5] = { 215.0f, 278.0f, 342.0f, 420.0f, 482.0f };

            static int menuSelectedIdx = 0;
            if (IsKeyPressed(KEY_UP))   { menuSelectedIdx = (menuSelectedIdx <= 0) ? 4 : menuSelectedIdx - 1; }
            if (IsKeyPressed(KEY_DOWN)) { menuSelectedIdx = (menuSelectedIdx >= 4) ? 0 : menuSelectedIdx + 1; }

            for (int m = 0; m < 5; m++) {
                float my = menuYStarts[m] * scaleY;
                float mw = ((m < 3) ? 320.0f : 290.0f) * scaleX;
                float mh = ((m < 3) ? 50.0f : 42.0f) * scaleY;
                Rectangle itemHit = { 35.0f * scaleX, my, mw, mh };
                bool isHov = CheckCollisionPointRec(mousePos, itemHit);
                if (isHov) menuSelectedIdx = m;
                bool isSel = (menuSelectedIdx == m);

                if (isSel) {
                    DrawSlantedRect(32.0f * scaleX, my, mw + 30.0f * scaleX, mh, 14.0f, (m == 0) ? Color{ 34, 197, 94, 255 } : Color{ 255, 38, 118, 255 });
                    if (texIconArrow.id != 0) {
                        DrawTexturePro(texIconArrow, Rectangle{ 0, 0, (float)texIconArrow.width, (float)texIconArrow.height },
                                       Rectangle{ 42.0f * scaleX, my + mh * 0.5f - 11.0f * scaleY, 22.0f * scaleX, 22.0f * scaleY }, Vector2{0,0}, 0.0f, WHITE);
                    } else {
                        DrawTextEx(fontUIBold, ">", Vector2{ 42.0f * scaleX, my + 10.0f * scaleY }, 24 * scaleY, 0.4f, WHITE);
                    }
                }

                float textX = (isSel ? 74.0f : 52.0f) * scaleX;
                float textY = my + (mh - ((m < 3) ? 22.0f : 17.0f) * scaleY) * 0.5f;
                DrawTextEx(fontUIBold, LocText(menuItems[m]), Vector2{ textX, textY }, (m < 3) ? (int)(22 * scaleY) : (int)(17 * scaleY), 0.4f, WHITE);

                if ((isHov && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) || (isSel && (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)))) {
                    if (m == 0) { // PLAY / ابدأ اللعب
                        singlePirateMode = false;
                        currentScreen = SCREEN_GAMEPLAY;
                    } else if (m == 1) { // CREATE LOBBY
                        LanInitHost(7777, "8HF78N");
                        currentScreen = SCREEN_LAN_LOBBY;
                    } else if (m == 2) { // FIND LOBBY
                        LanStartDiscoveryListener(7778);
                        currentScreen = SCREEN_FIND_LOBBY;
                    } else if (m == 3) { // SETTINGS
                        currentScreen = SCREEN_SETTINGS;
                    } else if (m == 4) { // EXIT GAME
                        break;
                    }
                }
            }

            // Room Code Box next to FIND LOBBY
            Rectangle boxFindRec = { 385.0f * scaleX, 342.0f * scaleY, 175.0f * scaleX, 50.0f * scaleY };
            bool hovCode = CheckCollisionPointRec(mousePos, boxFindRec);
            DrawSlantedRect(boxFindRec.x, boxFindRec.y, boxFindRec.width, boxFindRec.height, 14.0f, hovCode ? Color{ 32, 62, 75, 235 } : Color{ 22, 45, 55, 210 });
            DrawSlantedRectLines(boxFindRec.x, boxFindRec.y, boxFindRec.width, boxFindRec.height, 14.0f, 1.8f, hovCode ? Color{ 175, 255, 45, 255 } : Color{ 60, 105, 120, 220 });
            DrawTextEx(fontUIBold, "8HF78N", Vector2{ boxFindRec.x + 36.0f * scaleX, boxFindRec.y + 12.0f * scaleY }, (int)(22 * scaleY), 0.5f, Color{ 175, 255, 45, 255 });
            if (hovCode && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                LanStartDiscoveryListener(7778);
                currentScreen = SCREEN_FIND_LOBBY;
            }

            // Bottom Bar
            float botY = scrH - 46.0f * scaleY;
            DrawTextEx(fontUIBold, LocText(STR_ESC_BACK), Vector2{ 42.0f * scaleX, botY + 8.0f * scaleY }, (int)(14 * scaleY), 0.4f, Color{ 148, 163, 184, 255 });
            DrawTextEx(fontUIBold, LocText(STR_ENTER_SELECT), Vector2{ 175.0f * scaleX, botY + 8.0f * scaleY }, (int)(14 * scaleY), 0.4f, Color{ 148, 163, 184, 255 });

            // Bottom-Right: Replaced DISCORD with Official Game Version Tag Badge (v8.1.0)
            float vW = 155.0f * scaleX, vH = 38.0f * scaleY;
            float vX = scrW - vW - 30.0f * scaleX, vY = scrH - vH - 12.0f * scaleY;
            DrawSlantedRect(vX, vY, vW, vH, 10.0f, Color{ 20, 32, 46, 225 });
            DrawSlantedRectLines(vX, vY, vW, vH, 10.0f, 1.5f, Color{ 172, 255, 45, 200 });
            Vector2 verSz = MeasureTextEx(fontUIBold, GAME_VERSION_TAG, 18 * scaleY, 0.4f);
            DrawTextEx(fontUIBold, GAME_VERSION_TAG, Vector2{ vX + (vW - verSz.x) * 0.5f, vY + 9.0f * scaleY }, 18 * scaleY, 0.4f, Color{ 172, 255, 45, 255 });

        } else if (currentScreen == SCREEN_LAN_LOBBY) {
            ClearBackground(Color{ 18, 22, 34, 255 });

            // Bright sunny sky gradient (Plan v8.1.0 Task 3)
            DrawRectangleGradientV(0, 0, GetScreenWidth(), GetScreenHeight(), Color{ 78, 152, 238, 255 }, Color{ 186, 224, 252, 255 });

            Camera3D lobbyCam = { 0 };
            lobbyCam.position = Vector3{ 15.55f, 2.5f, 87.0f };
            lobbyCam.target   = Vector3{ 15.55f, 1.1f, 80.0f };
            lobbyCam.up       = Vector3{ 0.0f, 1.0f, 0.0f };
            lobbyCam.fovy     = 42.0f;
            lobbyCam.projection = CAMERA_PERSPECTIVE;

            BeginMode3D(lobbyCam);
                DrawModel(wallModel, trackOffset, 1.0f, WHITE);
                rlPushMatrix();
                    float pBob = sinf(runTime * 2.5f) * 0.025f;
                    rlTranslatef(15.55f, 0.15f + pBob, 80.0f);
                    rlRotatef(0.0f, 0, 1, 0);
                    rlScalef(1.25f, 1.25f, 1.25f);
                    Model* pCurChar = (selectedLobbyChar % 2 == 0) ? &charB : &charA;
                    DrawModel(*pCurChar, Vector3{0,0,0}, 1.0f, WHITE);
                rlPopMatrix();

                // If 2+ players connected, show second pirate standing side by side!
                if (gConnectedPlayers >= 2) {
                    rlPushMatrix();
                        rlTranslatef(13.65f, 0.15f + pBob, 80.0f);
                        rlRotatef(0.0f, 0, 1, 0);
                        rlScalef(1.25f, 1.25f, 1.25f);
                        DrawModel(charA, Vector3{0,0,0}, 1.0f, WHITE);
                    rlPopMatrix();
                }
            EndMode3D();

            // Status badges above pirate heads
            Vector2 headScreen = GetWorldToScreen(Vector3{ 15.55f, 2.25f + sinf(runTime * 2.5f) * 0.025f, 80.0f }, lobbyCam);
            const char* readyBadgeText = gHostReady ? LocText(STR_READY_STATUS) : LocText(STR_WAITING_STATUS);
            Color readyBadgeCol = gHostReady ? Color{ 34, 197, 94, 255 } : Color{ 250, 204, 21, 255 };
            Vector2 rbSz = MeasureTextEx(fontUIBold, readyBadgeText, 16, 0.4f);
            DrawRectangleRounded(Rectangle{ headScreen.x - rbSz.x * 0.5f - 8.0f, headScreen.y - 32.0f, rbSz.x + 16.0f, 26.0f }, 0.4f, 4, Color{ 20, 25, 40, 220 });
            DrawTextEx(fontUIBold, readyBadgeText, Vector2{ headScreen.x - rbSz.x * 0.5f, headScreen.y - 28.0f }, 16, 0.4f, readyBadgeCol);

            // Top Center Banner: LOBBY
            float topBannerW = 320.0f, topBannerH = 48.0f;
            float topBannerX = ((float)GetScreenWidth() - topBannerW) * 0.5f;
            DrawTrapezoid(topBannerX, 0, topBannerW, topBannerH, 18.0f, Color{ 28, 33, 53, 255 });
            DrawTrapezoidLines(topBannerX, 0, topBannerW, topBannerH, 18.0f, 1.5f, Color{ 70, 80, 110, 255 });
            Vector2 lobSz = MeasureTextEx(fontUIBold, LocText(STR_LOBBY_TITLE), 22, 0.5f);
            DrawTextEx(fontUIBold, LocText(STR_LOBBY_TITLE), Vector2{ ((float)GetScreenWidth() - lobSz.x) * 0.5f, 12.0f }, 22, 0.5f, WHITE);

            // Top-Left Solo Practice Test button
            Rectangle soloBtnRec = { 25.0f, 16.0f, 160.0f, 44.0f };
            bool hovSolo = CheckCollisionPointRec(mousePos, soloBtnRec);
            DrawSlantedRect(soloBtnRec.x, soloBtnRec.y, soloBtnRec.width, soloBtnRec.height, 10.0f, hovSolo ? Color{ 56, 189, 248, 255 } : Color{ 14, 116, 144, 225 });
            Vector2 soloSz = MeasureTextEx(fontUIBold, LocText(STR_SOLO_TEST), 14, 0.3f);
            DrawTextEx(fontUIBold, LocText(STR_SOLO_TEST), Vector2{ soloBtnRec.x + (soloBtnRec.width - soloSz.x) * 0.5f, soloBtnRec.y + 13.0f }, 14, 0.3f, WHITE);
            if (hovSolo && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                gSoloPracticeMode = true;
                singlePirateMode = false;
                currentScreen = SCREEN_GAMEPLAY;
            }

            // Top Right: Room Code
            float codeBoxW = 210.0f, codeBoxH = 54.0f;
            float codeBoxX = (float)GetScreenWidth() - codeBoxW - 35.0f;
            float codeBoxY = 16.0f;
            DrawSlantedRect(codeBoxX, codeBoxY, codeBoxW, codeBoxH, 12.0f, Color{ 22, 30, 48, 220 });
            DrawSlantedRectLines(codeBoxX, codeBoxY, codeBoxW, codeBoxH, 12.0f, 1.2f, Color{ 56, 75, 110, 200 });
            DrawTextEx(fontUI, LocText(STR_YOUR_CODE), Vector2{ codeBoxX + 22.0f, codeBoxY + 8.0f }, 11, 0.3f, Color{ 160, 175, 200, 255 });
            DrawTextEx(fontUIBold, "8HF78N", Vector2{ codeBoxX + 22.0f, codeBoxY + 24.0f }, 22, 0.5f, Color{ 175, 255, 45, 255 });
            if (texIconCopy.id != 0) {
                DrawTexturePro(texIconCopy, Rectangle{ 0, 0, (float)texIconCopy.width, (float)texIconCopy.height },
                               Rectangle{ codeBoxX + codeBoxW - 38.0f, codeBoxY + 24.0f, 20.0f, 20.0f }, Vector2{0,0}, 0.0f, Color{ 175, 255, 45, 255 });
            }

            // 4 Character Cards
            float cardW = 90.0f, cardH = 96.0f, cardGap = 16.0f;
            float startCardX = ((float)GetScreenWidth() - (4 * cardW + 3 * cardGap)) * 0.5f;
            float cardY = (float)GetScreenHeight() * 0.50f;

            for (int c = 0; c < 4; c++) {
                Rectangle cRec = { startCardX + c * (cardW + cardGap), cardY, cardW, cardH };
                bool isHov = CheckCollisionPointRec(mousePos, cRec);
                bool isSel = (selectedLobbyChar == c);

                DrawRectangleRounded(cRec, 0.12f, 4, Color{ 245, 248, 252, 255 });
                if (texLobbyCards[c].id != 0) {
                    DrawTexturePro(texLobbyCards[c], Rectangle{ 0, 0, (float)texLobbyCards[c].width, (float)texLobbyCards[c].height },
                                   Rectangle{ cRec.x + 8.0f, cRec.y + 6.0f, cardW - 16.0f, cardH - 12.0f }, Vector2{0,0}, 0.0f, WHITE);
                }
                if (isSel) {
                    DrawRectangleRoundedLinesEx(cRec, 0.12f, 4, 3.5f, Color{ 238, 52, 132, 255 });
                } else if (isHov) {
                    DrawRectangleRoundedLinesEx(cRec, 0.12f, 4, 2.0f, Color{ 56, 189, 248, 255 });
                }

                if (isHov && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    selectedLobbyChar = c;
                }
            }
            if (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT))  selectedLobbyChar = (selectedLobbyChar + 3) % 4;
            if (IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT)) selectedLobbyChar = (selectedLobbyChar + 1) % 4;

            // Mode Selector
            float modeBarW = 540.0f, modeBarH = 34.0f;
            float modeBarX = ((float)GetScreenWidth() - modeBarW) * 0.5f;
            float modeBarY = cardY + cardH + 16.0f;
            DrawSlantedRect(modeBarX, modeBarY, modeBarW, modeBarH, 10.0f, Color{ 126, 114, 228, 255 });
            Vector2 smSz = MeasureTextEx(fontUIBold, LocText(STR_SELECT_MODE), 15, 0.4f);
            DrawTextEx(fontUIBold, LocText(STR_SELECT_MODE), Vector2{ ((float)GetScreenWidth() - smSz.x) * 0.5f, modeBarY + 8.0f }, 15, 0.4f, WHITE);

            float mCardW = 165.0f, mCardH = 75.0f, mCardGap = 15.0f;
            float mStartX = ((float)GetScreenWidth() - (3 * mCardW + 2 * mCardGap)) * 0.5f;
            float mCardsY = modeBarY + modeBarH + 6.0f;

            Texture2D modeTexs[3] = { texModeFartRun, texModeTeamRace, texModeRaftRumble };
            for (int mi = 0; mi < 3; mi++) {
                Rectangle mcRec = { mStartX + mi * (mCardW + mCardGap), mCardsY, mCardW, mCardH };
                DrawRectangleRounded(mcRec, 0.10f, 4, Color{ 240, 243, 250, 255 });
                if (modeTexs[mi].id != 0) {
                    Color mTint = (mi == 0) ? WHITE : Color{ 160, 160, 170, 180 };
                    DrawTexturePro(modeTexs[mi], Rectangle{ 0, 0, (float)modeTexs[mi].width, (float)modeTexs[mi].height },
                                   Rectangle{ mcRec.x + 4.0f, mcRec.y + 4.0f, mCardW - 8.0f, mCardH - 8.0f }, Vector2{0,0}, 0.0f, mTint);
                }
                if (mi == 0) {
                    DrawRectangleRoundedLinesEx(mcRec, 0.10f, 4, 2.5f, Color{ 238, 52, 132, 255 });
                } else {
                    DrawRectangleRounded(Rectangle{ mcRec.x, mcRec.y + mCardH - 20.0f, mCardW, 20.0f }, 0.1f, 4, Color{ 40, 45, 60, 200 });
                    Vector2 csSz = MeasureTextEx(fontUIBold, "Coming Soon", 11, 0.3f);
                    DrawTextEx(fontUIBold, "Coming Soon", Vector2{ mcRec.x + (mCardW - csSz.x) * 0.5f, mcRec.y + mCardH - 16.0f }, 11, 0.3f, Color{ 200, 210, 225, 255 });
                }
            }

            // Bottom-Left: INVITE FRIENDS Button (v8.1.0: Active & opens modal popup!)
            float invW = 210.0f, invH = 46.0f;
            float invX = 35.0f, invY = (float)GetScreenHeight() - 115.0f;
            Rectangle invRec = { invX, invY, invW, invH };
            bool hovInv = CheckCollisionPointRec(mousePos, invRec);
            DrawSlantedRect(invX, invY, invW, invH, 12.0f, hovInv ? Color{ 24, 160, 235, 255 } : Color{ 14, 138, 206, 255 });
            Vector2 invSz = MeasureTextEx(fontUIBold, LocText(STR_INVITE_FRIENDS), 15, 0.4f);
            DrawTextEx(fontUIBold, LocText(STR_INVITE_FRIENDS), Vector2{ invX + (invW - invSz.x) * 0.5f, invY + 12.0f }, 15, 0.4f, Color{ 255, 235, 60, 255 });
            if (hovInv && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                SetClipboardText("Fart Together LAN Room Code: 8HF78N (UDP Port 7777)");
                gShowInviteModal = true;
                editorMsg = (gCurrentLanguage == LANG_AR) ? (const char*)u8"تم نسخ رمز الغرفة للحافظة!" : "Room code copied to clipboard!";
                editorMsgTimer = 2.5f;
            }

            // Bottom Bar
            float lBotH = 54.0f;
            float lBotY = (float)GetScreenHeight() - lBotH;
            DrawRectangle(0, (int)lBotY, GetScreenWidth(), (int)lBotH, Color{ 18, 24, 38, 255 });

            Rectangle btnLeaveRec = { 25.0f, lBotY + 10.0f, 130.0f, 34.0f };
            bool hovLeave = CheckCollisionPointRec(mousePos, btnLeaveRec);
            DrawSlantedRect(btnLeaveRec.x, btnLeaveRec.y, btnLeaveRec.width, btnLeaveRec.height, 10.0f, hovLeave ? Color{ 45, 55, 75, 255 } : Color{ 28, 36, 52, 255 });
            DrawTextEx(fontUIBold, LocText(STR_ESC_LEAVE), Vector2{ btnLeaveRec.x + 16.0f, btnLeaveRec.y + 8.0f }, 13, 0.3f, WHITE);
            if ((hovLeave && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) || IsKeyPressed(KEY_ESCAPE)) {
                currentScreen = SCREEN_MAIN_MENU;
            }

            DrawTextEx(fontUI, LocText(STR_SELECT_CHAR), Vector2{ (float)GetScreenWidth() - 240.0f, lBotY + 18.0f }, 12, 0.3f, Color{ 160, 175, 200, 255 });

            // Strict Ready Rule Alert (v8.1.0)
            if (gConnectedPlayers < 2) {
                Vector2 warnSz = MeasureTextEx(fontUIBold, LocText(STR_MIN_PLAYERS_ALERT), 14, 0.4f);
                float warnX = ((float)GetScreenWidth() - warnSz.x) * 0.5f;
                DrawRectangleRounded(Rectangle{ warnX - 16.0f, lBotY - 32.0f, warnSz.x + 32.0f, 26.0f }, 0.4f, 4, Color{ 220, 38, 38, 220 });
                DrawTextEx(fontUIBold, LocText(STR_MIN_PLAYERS_ALERT), Vector2{ warnX, lBotY - 27.0f }, 14, 0.4f, WHITE);
            }

            // Prominent Ready Button
            float rdyW = 270.0f, rdyH = 62.0f;
            float rdyX = ((float)GetScreenWidth() - rdyW) * 0.5f;
            float rdyY = (float)GetScreenHeight() - rdyH;
            Rectangle rdyHit = { rdyX, rdyY, rdyW, rdyH };
            bool hovRdy = CheckCollisionPointRec(mousePos, rdyHit);

            Color rdyBg = gHostReady ? Color{ 34, 197, 94, 255 } : (hovRdy ? Color{ 255, 235, 240, 255 } : WHITE);
            DrawTrapezoid(rdyX, rdyY, rdyW, rdyH, 20.0f, rdyBg);
            DrawTrapezoidLines(rdyX, rdyY, rdyW, rdyH, 20.0f, 2.5f, Color{ 238, 45, 95, 255 });

            const char* rdyBtnStr = gHostReady ? LocText(STR_READY_STATUS) : LocText(STR_READY_BTN);
            Vector2 rdySz = MeasureTextEx(fontUIBold, rdyBtnStr, 22, 0.5f);
            DrawTextEx(fontUIBold, rdyBtnStr, Vector2{ ((float)GetScreenWidth() - rdySz.x) * 0.5f, rdyY + 18.0f }, 22, 0.5f, gHostReady ? WHITE : Color{ 238, 45, 95, 255 });

            if ((hovRdy && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) || IsKeyPressed(KEY_SPACE)) {
                gHostReady = !gHostReady;
                if (gConnectedPlayers >= 2 && gHostReady && gClientReady) {
                    singlePirateMode = false;
                    currentScreen = SCREEN_GAMEPLAY;
                } else if (gConnectedPlayers < 2) {
                    editorMsg = LocText(STR_MIN_PLAYERS_ALERT);
                    editorMsgTimer = 3.0f;
                }
            }

            // Invite Modal Popup
            if (gShowInviteModal) {
                DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Color{ 10, 15, 25, 210 });
                float mBoxW = 460.0f, mBoxH = 260.0f;
                float mBoxX = ((float)GetScreenWidth() - mBoxW) * 0.5f;
                float mBoxY = ((float)GetScreenHeight() - mBoxH) * 0.5f;
                DrawRectangleRounded(Rectangle{ mBoxX, mBoxY, mBoxW, mBoxH }, 0.08f, 6, Color{ 22, 30, 48, 255 });
                DrawRectangleRoundedLinesEx(Rectangle{ mBoxX, mBoxY, mBoxW, mBoxH }, 0.08f, 6, 2.0f, Color{ 56, 189, 248, 255 });

                DrawTextEx(fontUIBold, LocText(STR_INVITE_FRIENDS), Vector2{ mBoxX + 30.0f, mBoxY + 24.0f }, 22, 0.4f, WHITE);
                DrawTextEx(fontUI, "LAN UDP Auto-Discovery: Active (Port 7778)", Vector2{ mBoxX + 30.0f, mBoxY + 60.0f }, 14, 0.3f, Color{ 74, 222, 128, 255 });
                DrawTextEx(fontUI, "Friends on the same Wi-Fi see your room automatically!", Vector2{ mBoxX + 30.0f, mBoxY + 84.0f }, 13, 0.3f, Color{ 203, 213, 225, 255 });

                Rectangle codeRec = { mBoxX + 30.0f, mBoxY + 115.0f, 400.0f, 48.0f };
                DrawRectangleRounded(codeRec, 0.1f, 4, Color{ 15, 20, 32, 255 });
                DrawTextEx(fontUIBold, "ROOM CODE: 8HF78N  (Port: 7777)", Vector2{ codeRec.x + 20.0f, codeRec.y + 14.0f }, 18, 0.4f, Color{ 175, 255, 45, 255 });

                Rectangle copyBtn = { mBoxX + 30.0f, mBoxY + 185.0f, 190.0f, 46.0f };
                bool hovCp = CheckCollisionPointRec(mousePos, copyBtn);
                DrawSlantedRect(copyBtn.x, copyBtn.y, copyBtn.width, copyBtn.height, 10.0f, hovCp ? Color{ 34, 197, 94, 255 } : Color{ 22, 163, 74, 255 });
                DrawTextEx(fontUIBold, "Copy Code", Vector2{ copyBtn.x + 48.0f, copyBtn.y + 14.0f }, 15, 0.4f, WHITE);
                if (hovCp && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    SetClipboardText("8HF78N");
                    editorMsg = "Code Copied!";
                    editorMsgTimer = 2.0f;
                }

                Rectangle closeBtn = { mBoxX + mBoxW - 220.0f, mBoxY + 185.0f, 190.0f, 46.0f };
                bool hovCl = CheckCollisionPointRec(mousePos, closeBtn);
                DrawSlantedRect(closeBtn.x, closeBtn.y, closeBtn.width, closeBtn.height, 10.0f, hovCl ? Color{ 239, 68, 68, 255 } : Color{ 185, 28, 28, 255 });
                DrawTextEx(fontUIBold, "Close", Vector2{ closeBtn.x + 68.0f, closeBtn.y + 14.0f }, 15, 0.4f, WHITE);
                if (hovCl && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    gShowInviteModal = false;
                }
            }

        } else if (currentScreen == SCREEN_FIND_LOBBY) {
            ClearBackground(Color{ 18, 22, 34, 255 });
            if (texMenuOasisClean.id != 0) {
                DrawTexturePro(texMenuOasisClean, Rectangle{ 0, 0, (float)texMenuOasisClean.width, (float)texMenuOasisClean.height },
                               Rectangle{ 0, 0, (float)GetScreenWidth(), (float)GetScreenHeight() }, Vector2{0,0}, 0.0f, WHITE);
            }
            // Soft warm cartoon tint (Task 4 Parity - NO green military radar!)
            DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Color{ 28, 24, 52, 120 });

            // Top Header: Back Button & Title
            Rectangle backRec = { 30.0f, 32.0f, 120.0f, 54.0f };
            bool hovBack = CheckCollisionPointRec(mousePos, backRec);
            DrawSlantedRect(backRec.x, backRec.y, backRec.width, backRec.height, 14.0f, hovBack ? Color{ 255, 60, 135, 255 } : Color{ 255, 38, 118, 255 });
            if (texIconArrow.id != 0) {
                DrawTexturePro(texIconArrow, Rectangle{ 0, 0, -(float)texIconArrow.width, (float)texIconArrow.height },
                               Rectangle{ 45.0f, backRec.y + 14.0f, 26.0f, 26.0f }, Vector2{0,0}, 0.0f, WHITE);
            }
            if ((hovBack && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) || IsKeyPressed(KEY_ESCAPE)) {
                currentScreen = SCREEN_MAIN_MENU;
            }

            // Purple slanted banner title
            DrawSlantedRect(175.0f, 32.0f, 720.0f, 54.0f, 12.0f, Color{ 138, 122, 245, 255 });
            DrawTextEx(fontUIBold, LocText(STR_FIND_TITLE), Vector2{ 200.0f, 44.0f }, 24, 0.4f, WHITE);

            LanStartDiscoveryListener(7778);
            LanUpdateDiscoveryAndLobby(GetFrameTime());

            // Quick Join Code Bar
            float barW = 920.0f, barH = 58.0f;
            float barX = ((float)GetScreenWidth() - barW) * 0.5f;
            float barY = 110.0f;
            DrawRectangleRounded(Rectangle{ barX, barY, barW, barH }, 0.12f, 4, Color{ 252, 252, 255, 255 });
            DrawTextEx(fontUIBold, LocText(STR_ROOM_CODE_LBL), Vector2{ barX + 24.0f, barY + 18.0f }, 18, 0.4f, Color{ 40, 50, 75, 255 });

            Rectangle codeInRec = { barX + 160.0f, barY + 8.0f, 180.0f, 42.0f };
            DrawRectangleRounded(codeInRec, 0.1f, 4, Color{ 235, 240, 250, 255 });
            DrawTextEx(fontUIBold, "8HF78N", Vector2{ codeInRec.x + 38.0f, codeInRec.y + 10.0f }, 20, 0.5f, Color{ 238, 52, 132, 255 });

            Rectangle btnJoinCode = { barX + barW - 220.0f, barY + 7.0f, 200.0f, 44.0f };
            bool hovJCode = CheckCollisionPointRec(mousePos, btnJoinCode);
            DrawSlantedRect(btnJoinCode.x, btnJoinCode.y, btnJoinCode.width, btnJoinCode.height, 10.0f, hovJCode ? Color{ 255, 60, 135, 255 } : Color{ 255, 38, 118, 255 });
            Vector2 jcSz = MeasureTextEx(fontUIBold, LocText(STR_JOIN_BY_CODE), 16, 0.4f);
            DrawTextEx(fontUIBold, LocText(STR_JOIN_BY_CODE), Vector2{ btnJoinCode.x + (btnJoinCode.width - jcSz.x) * 0.5f, btnJoinCode.y + 12.0f }, 16, 0.4f, WHITE);
            if (hovJCode && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                LanJoinRoom("127.0.0.1", 7777, "8HF78N");
                currentScreen = SCREEN_LAN_LOBBY;
            }

            // Discovered Rooms List (Wide Horizontal Cards)
            float listX = barX;
            float listY = 190.0f;
            float cardW = barW;
            float cardH = 76.0f;
            float cardGap = 14.0f;

            auto& rooms = LanGetDiscoveredRooms();

            if (rooms.empty()) {
                Rectangle emptyCard = { listX, listY, cardW, 200.0f };
                DrawRectangleRounded(emptyCard, 0.08f, 4, Color{ 252, 252, 255, 245 });
                DrawRectangleRoundedLinesEx(emptyCard, 0.08f, 4, 2.0f, Color{ 138, 122, 245, 180 });

                if (texHipo.id != 0) {
                    float hSz = 72.0f;
                    DrawTexturePro(texHipo, Rectangle{ 0, 0, (float)texHipo.width, (float)texHipo.height },
                                   Rectangle{ listX + 45.0f, listY + 35.0f, hSz, hSz }, Vector2{0,0}, sinf(runTime * 3.0f) * 6.0f, WHITE);
                }

                DrawTextEx(fontUIBold, LocText(STR_WAITING_HOST_MSG), Vector2{ listX + 140.0f, listY + 45.0f }, 18, 0.4f, Color{ 40, 50, 75, 255 });
                DrawTextEx(fontUI, "Create a room on another PC/phone on this Wi-Fi, or test with Local Host below.", Vector2{ listX + 140.0f, listY + 76.0f }, 14, 0.3f, Color{ 100, 115, 140, 255 });

                Rectangle btnCreateNow = { listX + 140.0f, listY + 115.0f, 240.0f, 50.0f };
                bool hovCNow = CheckCollisionPointRec(mousePos, btnCreateNow);
                DrawSlantedRect(btnCreateNow.x, btnCreateNow.y, btnCreateNow.width, btnCreateNow.height, 10.0f, hovCNow ? Color{ 255, 60, 135, 255 } : Color{ 255, 38, 118, 255 });
                Vector2 cnSz = MeasureTextEx(fontUIBold, LocText(STR_CREATE_NOW_BTN), 16, 0.4f);
                DrawTextEx(fontUIBold, LocText(STR_CREATE_NOW_BTN), Vector2{ btnCreateNow.x + (btnCreateNow.width - cnSz.x) * 0.5f, btnCreateNow.y + 15.0f }, 16, 0.4f, WHITE);
                if (hovCNow && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    LanInitHost(7777, "8HF78N");
                    currentScreen = SCREEN_LAN_LOBBY;
                }

                Rectangle joinLocalBtn = { listX + 410.0f, listY + 115.0f, 240.0f, 50.0f };
                bool hovJLocal = CheckCollisionPointRec(mousePos, joinLocalBtn);
                DrawSlantedRect(joinLocalBtn.x, joinLocalBtn.y, joinLocalBtn.width, joinLocalBtn.height, 10.0f, hovJLocal ? Color{ 34, 197, 94, 255 } : Color{ 22, 163, 74, 255 });
                DrawTextEx(fontUIBold, "[ JOIN LOCAL HOST ]", Vector2{ joinLocalBtn.x + 30.0f, joinLocalBtn.y + 16.0f }, 15, 0.4f, WHITE);
                if (hovJLocal && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    LanJoinRoom("127.0.0.1", 7777, "8HF78N");
                    currentScreen = SCREEN_LAN_LOBBY;
                }
            } else {
                for (size_t r = 0; r < rooms.size() && r < 4; r++) {
                    float ry = listY + r * (cardH + cardGap);
                    Rectangle rCard = { listX, ry, cardW, cardH };
                    bool hovCard = CheckCollisionPointRec(mousePos, rCard);

                    DrawRectangleRounded(rCard, 0.12f, 4, hovCard ? Color{ 250, 232, 82, 255 } : Color{ 252, 252, 255, 255 });
                    DrawRectangleRoundedLinesEx(rCard, 0.12f, 4, 1.8f, Color{ 138, 122, 245, 200 });

                    // Pirate Face Icon
                    Texture2D faceTex = texLobbyCards[rooms[r].hostColor % 4];
                    if (faceTex.id != 0) {
                        DrawTexturePro(faceTex, Rectangle{ 0, 0, (float)faceTex.width, (float)faceTex.height },
                                       Rectangle{ listX + 16.0f, ry + 10.0f, 56.0f, 56.0f }, Vector2{0,0}, 0.0f, WHITE);
                    }

                    DrawTextEx(fontUIBold, rooms[r].roomName.c_str(), Vector2{ listX + 88.0f, ry + 16.0f }, 19, 0.4f, Color{ 30, 40, 65, 255 });
                    std::string subInfo = "Host IP: " + rooms[r].hostIp + ":" + std::to_string(rooms[r].hostPort) + "  |  Code: " + rooms[r].roomCode;
                    DrawTextEx(fontUI, subInfo.c_str(), Vector2{ listX + 88.0f, ry + 44.0f }, 13, 0.3f, Color{ 90, 105, 130, 255 });

                    // Player Count Badge
                    std::string pCountStr = std::to_string(rooms[r].playerCount) + "/" + std::to_string(rooms[r].maxPlayers) + " " + LocText(STR_PLAYERS_COUNT);
                    DrawTextEx(fontUIBold, pCountStr.c_str(), Vector2{ listX + cardW - 320.0f, ry + 26.0f }, 15, 0.4f, Color{ 70, 85, 115, 255 });

                    Rectangle btnJoin = { listX + cardW - 170.0f, ry + 12.0f, 150.0f, 52.0f };
                    bool hovJoin = CheckCollisionPointRec(mousePos, btnJoin);
                    DrawSlantedRect(btnJoin.x, btnJoin.y, btnJoin.width, btnJoin.height, 10.0f, hovJoin ? Color{ 40, 215, 80, 255 } : Color{ 34, 197, 94, 255 });
                    Vector2 jSz = MeasureTextEx(fontUIBold, LocText(STR_JOIN_ROOM_BTN), 18, 0.4f);
                    DrawTextEx(fontUIBold, LocText(STR_JOIN_ROOM_BTN), Vector2{ btnJoin.x + (btnJoin.width - jSz.x) * 0.5f, btnJoin.y + 16.0f }, 18, 0.4f, WHITE);

                    if ((hovJoin || hovCard) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                        LanJoinRoom(rooms[r].hostIp.c_str(), rooms[r].hostPort, rooms[r].roomCode.c_str());
                        currentScreen = SCREEN_LAN_LOBBY;
                    }
                }
            }

            // Bottom Create Button
            float fBotY = (float)GetScreenHeight() - 52.0f;
            Rectangle btnCreateRec = { listX, fBotY, 240.0f, 40.0f };
            bool hovCreate = CheckCollisionPointRec(mousePos, btnCreateRec);
            DrawSlantedRect(btnCreateRec.x, btnCreateRec.y, btnCreateRec.width, btnCreateRec.height, 10.0f, hovCreate ? Color{ 245, 158, 11, 255 } : Color{ 217, 119, 6, 255 });
            DrawTextEx(fontUIBold, LocText(STR_CREATE_NOW_BTN), Vector2{ btnCreateRec.x + 22.0f, btnCreateRec.y + 11.0f }, 14, 0.4f, WHITE);
            if (hovCreate && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                LanInitHost(7777, "8HF78N");
                currentScreen = SCREEN_LAN_LOBBY;
            }

        '''

text = text[:start_idx] + new_block + text[end_idx:]
engine_path.write_text(text, encoding="utf-8")
print("Replaced HUD, Main Menu, Lobby and Find Lobby successfully!")
