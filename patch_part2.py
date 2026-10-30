import re

with open("CustomEngine/native/GameEngine.cpp", "r", encoding="utf-8") as f:
    c = f.read()

# 1. Update Left & Right Pirate rendering to be naked with localized butt swell & eye bulge
char_start = c.find("// Left Pirate (Player 1) seated facing inward on the dinghy rim")
char_end = c.find("// H. Ragdoll Pirates Ejected during Boat Destruction")
assert char_start != -1 and char_end != -1, "Character render section not found"

char_render_new = '''// Left Pirate (Player 1: Green Hippo - Naked Cartoon)
                    float p1Redness = gasP1.redness;
                    float p1ButtSwell = (gasP1.boneScale - 1.0f) * 2.5f;
                    float p1EyeBulge  = gasP1.isSustained ? gasP1.sustainedCharge : 0.0f;
                    SetShaderValue(shaderLighting, uRednessLoc,   &p1Redness,   SHADER_UNIFORM_FLOAT);
                    SetShaderValue(shaderLighting, uButtSwellLoc, &p1ButtSwell, SHADER_UNIFORM_FLOAT);
                    SetShaderValue(shaderLighting, uEyeBulgeLoc,  &p1EyeBulge,  SHADER_UNIFORM_FLOAT);

                    rlPushMatrix();
                        float p1X = sinf(p1RimAngle) * 1.05f;
                        float p1Z = cosf(p1RimAngle) * 1.05f;
                        rlTranslatef(p1X, 0.26f + gasP1.trembleOffset, p1Z);
                        float p1FaceYaw = (p1RimAngle * RAD2DEG) + 180.0f;
                        rlRotatef(p1FaceYaw, 0, 1, 0);

                        // Authentic Naked Hippo (No clothes matching Image 2!)
                        DrawModel(charA, Vector3{ 0, 0, 0 }, 1.0f, WHITE);
                    rlPopMatrix();

                    // Right Pirate (Player 2: Blue/Brown Hippo - Naked Cartoon)
                    float p2Redness = gasP2.redness;
                    float p2ButtSwell = (gasP2.boneScale - 1.0f) * 2.5f;
                    float p2EyeBulge  = gasP2.isSustained ? gasP2.sustainedCharge : 0.0f;
                    SetShaderValue(shaderLighting, uRednessLoc,   &p2Redness,   SHADER_UNIFORM_FLOAT);
                    SetShaderValue(shaderLighting, uButtSwellLoc, &p2ButtSwell, SHADER_UNIFORM_FLOAT);
                    SetShaderValue(shaderLighting, uEyeBulgeLoc,  &p2EyeBulge,  SHADER_UNIFORM_FLOAT);

                    rlPushMatrix();
                        float p2X = sinf(p2RimAngle) * 1.05f;
                        float p2Z = cosf(p2RimAngle) * 1.05f;
                        rlTranslatef(p2X, 0.26f + gasP2.trembleOffset, p2Z);
                        float p2FaceYaw = (p2RimAngle * RAD2DEG) + 180.0f;
                        rlRotatef(p2FaceYaw, 0, 1, 0);

                        // Authentic Naked Hippo (No clothes matching Image 2!)
                        DrawModel(charB, Vector3{ 0, 0, 0 }, 1.0f, WHITE);
                    rlPopMatrix();

                    // Reset uniforms for raft and all subsequent models
                    float zeroReset = 0.0f;
                    SetShaderValue(shaderLighting, uRednessLoc,   &zeroReset, SHADER_UNIFORM_FLOAT);
                    SetShaderValue(shaderLighting, uButtSwellLoc, &zeroReset, SHADER_UNIFORM_FLOAT);
                    SetShaderValue(shaderLighting, uEyeBulgeLoc,  &zeroReset, SHADER_UNIFORM_FLOAT);

                rlPopMatrix();
            }

            '''

c = c[:char_start] + char_render_new + c[char_end:]

# 2. Add Level Editor 3D Gizmo rendering inside BeginMode3D / EndMode3D
idx_endmode = c.find("EndMode3D();")
assert idx_endmode != -1, "EndMode3D not found"

gizmo_code = '''
            // 3D Level Editor Visual Highlights & Gizmos (Unity / Godot style)
            if (editorMode) {
                for (size_t i = 0; i < obsList.size(); i++) {
                    bool isSel = ((int)i == selectedObs);
                    Vector3 oPos = obsList[i].position;
                    DrawCircle3D(Vector3{ oPos.x, 0.05f, oPos.z }, obsList[i].radius, Vector3{ 1, 0, 0 }, 90.0f, isSel ? Color{ 255, 235, 50, 180 } : Color{ 80, 180, 255, 90 });

                    if (isSel) {
                        BoundingBox selBBox = { Vector3Subtract(oPos, Vector3{ 1.4f, 1.4f, 1.4f }), Vector3Add(oPos, Vector3{ 1.4f, 1.4f, 1.4f }) };
                        DrawBoundingBox(selBBox, YELLOW);

                        DrawLine3D(oPos, Vector3Add(oPos, Vector3{ 2.2f, 0, 0 }), RED);   // X Axis (Red)
                        DrawLine3D(oPos, Vector3Add(oPos, Vector3{ 0, 2.2f, 0 }), GREEN); // Y Axis (Green)
                        DrawLine3D(oPos, Vector3Add(oPos, Vector3{ 0, 0, 2.2f }), BLUE);  // Z Axis (Blue)
                        DrawSphere(Vector3Add(oPos, Vector3{ 2.2f, 0, 0 }), 0.15f, RED);
                        DrawSphere(Vector3Add(oPos, Vector3{ 0, 2.2f, 0 }), 0.15f, GREEN);
                        DrawSphere(Vector3Add(oPos, Vector3{ 0, 0, 2.2f }), 0.15f, BLUE);
                    }
                }
            }

        '''

c = c[:idx_endmode] + gizmo_code + c[idx_endmode:]

# 3. Replace entire HUD section with Authentic Image 2 UI + Level Editor Inspector
idx_hud_start = c.find("// Authentic Player Overhead Name Tags")
idx_hud_end = c.find("EndDrawing();")
assert idx_hud_start != -1 and idx_hud_end != -1, "HUD section boundaries not found"

new_hud_code = '''// Authentic Game UI (1:1 Matching Image 2 & Disassembled Sprites)
        // =====================================================================

        // 1. Top-Left: Authentic Hippo Gas Tank Level Bar
        {
            float gasFrac = Clamp(gasP1.gasEnergy / GAS_MAX, 0.0f, 1.0f);
            Color barFillCol = gasP1.gasDepleted ? Color{ 239, 68, 68, 255 } : Color{ 74, 222, 50, 255 };

            DrawRectangleRounded(Rectangle{ 65, 14, 184, 22 }, 0.5f, 6, Color{ 15, 23, 42, 210 });
            DrawRectangleRoundedLinesEx(Rectangle{ 65, 14, 184, 22 }, 0.5f, 6, 1.5f, Color{ 255, 255, 255, 160 });

            if (gasFrac > 0.02f) {
                DrawRectangleRounded(Rectangle{ 67, 16, 180.0f * gasFrac, 18 }, 0.45f, 6, barFillCol);
            }

            DrawTexturePro(texHipo, Rectangle{ 0, 0, (float)texHipo.width, (float)texHipo.height },
                           Rectangle{ 14, 6, 56, 36 }, Vector2{ 0, 0 }, 0.0f, WHITE);
        }

        // 2. Top-Center: Full Course Progress Bar Track & Lap Stopwatch
        {
            int trackStartX = 290;
            int trackEndX = GetScreenWidth() - 290;
            int trackWidth = trackEndX - trackStartX;
            int trackY = 18;

            DrawRectangle(trackStartX, trackY, trackWidth, 5, Color{ 255, 255, 255, 230 });
            DrawRectangleLines(trackStartX, trackY, trackWidth, 5, Color{ 15, 23, 42, 200 });

            for (size_t cp = 0; cp < checkpoints.size(); cp++) {
                float frac = (float)cp / (float)(checkpoints.size() - 1);
                int tickX = trackStartX + (int)(trackWidth * frac);
                DrawRectangle(tickX - 1, trackY - 3, 3, 11, Color{ 30, 41, 59, 255 });
            }

            DrawTexturePro(texFinish, Rectangle{ 0, 0, (float)texFinish.width, (float)texFinish.height },
                           Rectangle{ (float)trackEndX - 8, (float)trackY - 14, 26, 28 }, Vector2{ 0, 0 }, 0.0f, WHITE);

            float courseFrac = Clamp((100.0f - raft.position.z) / 440.0f, 0.0f, 1.0f);
            float pinX = (float)trackStartX + (float)trackWidth * courseFrac;
            DrawTexturePro(texRaftPin, Rectangle{ 0, 0, (float)texRaftPin.width, (float)texRaftPin.height },
                           Rectangle{ pinX - 14, (float)trackY - 13, 28, 28 }, Vector2{ 0, 0 }, 0.0f, WHITE);

            int mins = (int)raceTimer / 60;
            int secs = (int)raceTimer % 60;
            int hundredths = (int)((raceTimer - (int)raceTimer) * 100);

            int clockBadgeW = 150;
            int clockBadgeX = GetScreenWidth() / 2 - clockBadgeW / 2;
            DrawRectangleRounded(Rectangle{ (float)clockBadgeX, 29, (float)clockBadgeW, 26 }, 0.4f, 4, Color{ 15, 23, 42, 210 });
            DrawTexturePro(texClock, Rectangle{ 0, 0, (float)texClock.width, (float)texClock.height },
                           Rectangle{ (float)clockBadgeX + 8, 32, 20, 20 }, Vector2{ 0, 0 }, 0.0f, WHITE);
            DrawText(TextFormat("%02d:%02d.%02d", mins, secs, hundredths), clockBadgeX + 36, 34, 18, WHITE);
        }

        // 3. Top-Right: Death / Attempt Counter
        {
            int deathBadgeW = 110;
            int deathBadgeX = GetScreenWidth() - deathBadgeW - 24;
            DrawRectangleRounded(Rectangle{ (float)deathBadgeX, 14, (float)deathBadgeW, 30 }, 0.4f, 4, Color{ 15, 23, 42, 210 });
            DrawRectangleRoundedLinesEx(Rectangle{ (float)deathBadgeX, 14, (float)deathBadgeW, 30 }, 0.4f, 4, 1.0f, Color{ 255, 255, 255, 120 });

            DrawTexturePro(texCrashed, Rectangle{ 0, 0, (float)texCrashed.width, (float)texCrashed.height },
                           Rectangle{ (float)deathBadgeX + 8, 17, 30, 22 }, Vector2{ 0, 0 }, 0.0f, WHITE);
            DrawText(TextFormat("%d", totalDeaths), deathBadgeX + 46, 20, 18, WHITE);
        }

        // 4. Bottom-Left: [ESC] LEAVE Button
        {
            Rectangle btnLeaveRec = { 20, (float)GetScreenHeight() - 48, 115, 34 };
            bool hoverLeave = CheckCollisionPointRec(mousePos, btnLeaveRec);
            DrawRectangleRounded(btnLeaveRec, 0.35f, 4, hoverLeave ? Color{ 30, 41, 59, 240 } : Color{ 15, 23, 42, 210 });
            DrawRectangleRoundedLinesEx(btnLeaveRec, 0.35f, 4, 1.2f, hoverLeave ? Color{ 148, 163, 184, 255 } : Color{ 71, 85, 105, 180 });

            DrawTexturePro(texEsc, Rectangle{ 0, 0, (float)texEsc.width, (float)texEsc.height },
                           Rectangle{ 26, (float)GetScreenHeight() - 43, 24, 24 }, Vector2{ 0, 0 }, 0.0f, WHITE);
            DrawText("LEAVE", 58, GetScreenHeight() - 40, 16, RAYWHITE);
        }

        // 5. Bottom-Right: [F1] LEVEL EDITOR Toggle Button
        {
            Rectangle btnEditorRec = { (float)GetScreenWidth() - 170, (float)GetScreenHeight() - 48, 150, 34 };
            bool hoverEditor = CheckCollisionPointRec(mousePos, btnEditorRec);
            if ((hoverEditor && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) || IsKeyPressed(KEY_F1) || IsKeyPressed(KEY_E)) {
                editorMode = !editorMode;
                if (editorMode) {
                    editorCamPos = Vector3Add(raft.position, Vector3{ 0.0f, 4.5f, 6.0f });
                    editorCamYaw = raft.rotationY;
                    editorCamPitch = 0.35f;
                    editorMsg = "EDITOR MODE ACTIVE (UNITY / GODOT STYLE)";
                    editorMsgTimer = 3.0f;
                }
            }

            DrawRectangleRounded(btnEditorRec, 0.35f, 4, editorMode ? Color{ 234, 179, 8, 230 } : (hoverEditor ? Color{ 51, 65, 85, 230 } : Color{ 15, 23, 42, 210 }));
            DrawRectangleRoundedLinesEx(btnEditorRec, 0.35f, 4, 1.2f, Color{ 255, 255, 255, 150 });
            DrawText(editorMode ? "[F1] GAME MODE" : "[F1] LEVEL EDITOR", (int)btnEditorRec.x + 12, (int)btnEditorRec.y + 9, 14, editorMode ? Color{ 15, 23, 42, 255 } : RAYWHITE);
        }

        // 6. Interactive 3D Level Editor Inspector Panel (When Editor is Active)
        if (editorMode) {
            float panelW = 320.0f;
            float panelH = 540.0f;
            Rectangle panelRec = { (float)GetScreenWidth() - panelW - 15, 60, panelW, panelH };
            DrawRectangleRounded(panelRec, 0.08f, 4, Color{ 15, 23, 42, 240 });
            DrawRectangleRoundedLinesEx(panelRec, 0.08f, 4, 1.5f, Color{ 234, 179, 8, 220 });

            int py = (int)panelRec.y + 12;
            DrawText("LEVEL EDITOR (UNITY / GODOT)", (int)panelRec.x + 16, py, 16, Color{ 250, 204, 21, 255 });
            py += 26;
            DrawLine((int)panelRec.x + 10, py, (int)panelRec.x + (int)panelW - 10, py, Color{ 51, 65, 85, 255 });
            py += 10;

            if (selectedObs >= 0 && selectedObs < (int)obsList.size()) {
                auto& o = obsList[selectedObs];
                const char* typeNames[] = { "ROTOR_B5", "TIPPING_B2", "PENDULUM_C2", "GATE_F10", "SWING_F3", "BUMPER_SPRING", "ROCK1", "ROCK2" };
                const char* tName = (o.type >= 0 && o.type <= 7) ? typeNames[o.type] : "OBSTACLE";

                DrawText(TextFormat("Selected: #%d (%s)", selectedObs, tName), (int)panelRec.x + 16, py, 14, Color{ 147, 197, 253, 255 }); py += 22;
                DrawText(TextFormat("Position: (%.2f, %.2f, %.2f)", o.position.x, o.position.y, o.position.z), (int)panelRec.x + 16, py, 13, RAYWHITE); py += 18;
                DrawText(TextFormat("Rotation: %.1f deg", o.rotationY), (int)panelRec.x + 16, py, 13, RAYWHITE); py += 24;

                auto DrawBtn = [&](Rectangle rec, const char* label, Color col) -> bool {
                    bool hov = CheckCollisionPointRec(mousePos, rec);
                    DrawRectangleRounded(rec, 0.25f, 3, hov ? ColorBrightness(col, 0.2f) : col);
                    DrawText(label, (int)rec.x + 8, (int)rec.y + 6, 12, RAYWHITE);
                    return hov && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
                };

                if (DrawBtn(Rectangle{ panelRec.x + 16, (float)py, 65, 26 }, "< X -", Color{ 185, 28, 28, 255 }) || IsKeyPressed(KEY_J)) o.position.x -= 0.5f;
                if (DrawBtn(Rectangle{ panelRec.x + 86, (float)py, 65, 26 }, "+ X >", Color{ 185, 28, 28, 255 }) || IsKeyPressed(KEY_L)) o.position.x += 0.5f;
                if (DrawBtn(Rectangle{ panelRec.x + 160, (float)py, 65, 26 }, "< Z -", Color{ 29, 78, 216, 255 }) || IsKeyPressed(KEY_K)) o.position.z -= 0.5f;
                if (DrawBtn(Rectangle{ panelRec.x + 230, (float)py, 65, 26 }, "+ Z >", Color{ 29, 78, 216, 255 }) || IsKeyPressed(KEY_I)) o.position.z += 0.5f;
                py += 34;

                if (DrawBtn(Rectangle{ panelRec.x + 16, (float)py, 65, 26 }, "v Y -", Color{ 21, 128, 61, 255 }) || IsKeyPressed(KEY_O)) o.position.y -= 0.2f;
                if (DrawBtn(Rectangle{ panelRec.x + 86, (float)py, 65, 26 }, "^ Y +", Color{ 21, 128, 61, 255 }) || IsKeyPressed(KEY_U)) o.position.y += 0.2f;
                if (DrawBtn(Rectangle{ panelRec.x + 160, (float)py, 65, 26 }, "↺ -15°", Color{ 107, 33, 168, 255 }) || IsKeyPressed(KEY_R)) o.rotationY -= 15.0f;
                if (DrawBtn(Rectangle{ panelRec.x + 230, (float)py, 65, 26 }, "↻ +15°", Color{ 107, 33, 168, 255 }) || IsKeyPressed(KEY_T)) o.rotationY += 15.0f;
                py += 40;

                if (DrawBtn(Rectangle{ panelRec.x + 16, (float)py, 135, 28 }, "📋 DUPLICATE", Color{ 51, 65, 85, 255 }) || (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_D))) {
                    KineticObstacle dup = o;
                    dup.position.z -= 2.0f;
                    obsList.push_back(dup);
                    selectedObs = (int)obsList.size() - 1;
                    editorMsg = "Obstacle Duplicated!";
                    editorMsgTimer = 2.0f;
                }
                if (DrawBtn(Rectangle{ panelRec.x + 160, (float)py, 135, 28 }, "🗑 DELETE", Color{ 153, 27, 27, 255 }) || IsKeyPressed(KEY_DELETE) || IsKeyPressed(KEY_BACKSPACE)) {
                    obsList.erase(obsList.begin() + selectedObs);
                    selectedObs = -1;
                    editorMsg = "Obstacle Deleted!";
                    editorMsgTimer = 2.0f;
                }
                py += 40;
            } else {
                DrawText("No obstacle selected.\\nClick any obstacle in 3D to select it!", (int)panelRec.x + 16, py, 13, Color{ 148, 163, 184, 255 });
                py += 55;
            }

            Rectangle btnAddRec = { panelRec.x + 16, (float)py, panelW - 32, 32 };
            bool hovAdd = CheckCollisionPointRec(mousePos, btnAddRec);
            if ((hovAdd && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) || IsKeyPressed(KEY_N)) {
                KineticObstacle newO;
                newO.type = OBS_BUMPER_SPRING;
                newO.position = raft.position + Vector3{ 0.0f, 0.65f, -6.0f };
                newO.rotationY = 0.0f;
                newO.animTimer = 0.0f;
                newO.animSpeed = 1.0f;
                newO.radius = 2.5f;
                newO.model = &obsModelA1;
                newO.bumperExpandTimer = 0.0f;
                newO.bumperCooldownTimer = 0.0f;
                newO.isExpanded = false;
                obsList.push_back(newO);
                selectedObs = (int)obsList.size() - 1;
                editorMsg = "New Obstacle Added!";
                editorMsgTimer = 2.0f;
            }
            DrawRectangleRounded(btnAddRec, 0.25f, 3, hovAdd ? Color{ 37, 99, 235, 255 } : Color{ 30, 64, 175, 255 });
            DrawText("+ ADD NEW OBSTACLE [N]", (int)btnAddRec.x + 50, (int)btnAddRec.y + 8, 13, RAYWHITE);
            py += 44;

            Rectangle btnSaveRec = { panelRec.x + 16, (float)py, panelW - 32, 38 };
            bool hovSave = CheckCollisionPointRec(mousePos, btnSaveRec);
            if ((hovSave && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) || (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_S)) || IsKeyPressed(KEY_F5)) {
                SaveObstaclesJSON(LEVEL_FILE_PATH, obsList);
                editorMsg = "LEVEL SAVED TO level_obstacles.json!";
                editorMsgTimer = 3.5f;
            }
            DrawRectangleRounded(btnSaveRec, 0.25f, 3, hovSave ? Color{ 34, 197, 94, 255 } : Color{ 22, 163, 74, 255 });
            DrawText("💾 SAVE LEVEL TO JSON [Ctrl+S]", (int)btnSaveRec.x + 35, (int)btnSaveRec.y + 11, 14, WHITE);
            py += 48;

            DrawRectangleRounded(Rectangle{ panelRec.x + 12, (float)py, panelW - 24, 115 }, 0.15f, 3, Color{ 30, 41, 59, 200 });
            int hy = py + 8;
            DrawText("SHORTCUTS (UNITY / GODOT STYLE):", (int)panelRec.x + 20, hy, 11, Color{ 250, 204, 21, 255 }); hy += 16;
            DrawText("• Left Click: Select Obstacle in 3D", (int)panelRec.x + 20, hy, 11, Color{ 203, 213, 225, 255 }); hy += 15;
            DrawText("• J/L: Move X  |  I/K: Move Z  |  U/O: Move Y", (int)panelRec.x + 20, hy, 11, Color{ 203, 213, 225, 255 }); hy += 15;
            DrawText("• R/T: Rotate  |  Ctrl+D: Duplicate  |  DEL: Delete", (int)panelRec.x + 20, hy, 11, Color{ 203, 213, 225, 255 }); hy += 15;
            DrawText("• Right Click + WASD: Fly Editor Camera", (int)panelRec.x + 20, hy, 11, Color{ 203, 213, 225, 255 }); hy += 15;
            DrawText("• F1 / E: Toggle Editor On/Off", (int)panelRec.x + 20, hy, 11, Color{ 203, 213, 225, 255 });

            if (editorMsgTimer > 0.0f) {
                editorMsgTimer -= dt;
                int toastW = MeasureText(editorMsg.c_str(), 16) + 30;
                int toastX = GetScreenWidth() / 2 - toastW / 2;
                DrawRectangleRounded(Rectangle{ (float)toastX, 75, (float)toastW, 32 }, 0.4f, 4, Color{ 22, 163, 74, 240 });
                DrawText(editorMsg.c_str(), toastX + 15, 82, 16, WHITE);
            }
        }

        '''

c = c[:idx_hud_start] + new_hud_code + c[idx_hud_end:]

# 4. Unload UI textures
old_unload = '    UnloadTexture(texRaftFoam);'
new_unload = '''    UnloadTexture(texRaftFoam);
    UnloadTexture(texHipo);
    UnloadTexture(texClock);
    UnloadTexture(texCrashed);
    UnloadTexture(texRaftPin);
    UnloadTexture(texFinish);
    UnloadTexture(texEsc);'''

assert old_unload in c, "old_unload not found"
c = c.replace(old_unload, new_unload, 1)

with open("CustomEngine/native/GameEngine.cpp", "w", encoding="utf-8") as f:
    f.write(c)

print("Applied patch_part2 successfully!")
