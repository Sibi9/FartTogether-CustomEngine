import re

path = "CustomEngine/native/GameEngine.cpp"
with open(path, "r", encoding="utf-8") as f:
    code = f.read()

# 1. Insert Font Loading right after texEsc SetTextureFilter
old_tex_setup = """    SetTextureFilter(texFinish, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(texEsc, TEXTURE_FILTER_BILINEAR);"""

new_tex_setup = """    SetTextureFilter(texFinish, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(texEsc, TEXTURE_FILTER_BILINEAR);

    // High-Resolution TrueType Fonts for Clean Godot-Style UI
    Font fontUI     = LoadFontEx(ResolveAsset("extracted_assets/fonts/ui_font.ttf").c_str(), 18, 0, 0);
    Font fontUIBold = LoadFontEx(ResolveAsset("extracted_assets/fonts/ui_font_bold.ttf").c_str(), 18, 0, 0);
    if (fontUI.texture.id == 0) fontUI = GetFontDefault();
    else SetTextureFilter(fontUI.texture, TEXTURE_FILTER_BILINEAR);
    if (fontUIBold.texture.id == 0) fontUIBold = GetFontDefault();
    else SetTextureFilter(fontUIBold.texture, TEXTURE_FILTER_BILINEAR);"""

if old_tex_setup in code:
    code = code.replace(old_tex_setup, new_tex_setup)
    print("Added fontUI and fontUIBold loading.")
else:
    print("WARNING: texFinish/texEsc setup not found exactly!")

# 2. Add Font Unloading at the end
old_unload = """    UnloadTexture(texFinish);
    UnloadTexture(texEsc);"""

new_unload = """    UnloadTexture(texFinish);
    UnloadTexture(texEsc);
    if (fontUI.texture.id != GetFontDefault().texture.id) UnloadFont(fontUI);
    if (fontUIBold.texture.id != GetFontDefault().texture.id) UnloadFont(fontUIBold);"""

if old_unload in code:
    code = code.replace(old_unload, new_unload)
    print("Added fontUI unloading.")
else:
    print("WARNING: UnloadTexture block not found exactly!")

# 3. Locate the entire editor panel block from:
# // 5. Bottom-Right: [F1] LEVEL EDITOR Toggle Button
# to the end of editorMsgTimer check:
# if (editorMsgTimer > 0.0f) { ... }
start_tag = "// 5. Bottom-Right: [F1] LEVEL EDITOR Toggle Button"
end_tag = "        EndDrawing();"

start_idx = code.find(start_tag)
end_idx = code.find(end_tag)

if start_idx != -1 and end_idx != -1:
    new_panel_code = """        // 5. Bottom-Right: [F1] LEVEL EDITOR Toggle Button (Godot Style)
        {
            Rectangle btnEditorRec = { (float)GetScreenWidth() - 175, (float)GetScreenHeight() - 44, 160, 32 };
            bool hoverEditor = CheckCollisionPointRec(mousePos, btnEditorRec);
            if ((hoverEditor && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) || IsKeyPressed(KEY_F1)) {
                editorMode = !editorMode;
                if (editorMode) {
                    editorCamPos = Vector3Add(raft.position, Vector3{ 0.0f, 4.5f, 6.0f });
                    editorCamYaw = raft.rotationY;
                    editorCamPitch = 0.35f;
                    editorMsg = "LEVEL EDITOR ACTIVE - GAMEPLAY FROZEN";
                    editorMsgTimer = 3.0f;
                    raft.velocity = Vector3Zero();
                    raft.angularVelocityY = 0.0f;
                    gasP1.isSustained = false; gasP1.pressTimer = 0.0f;
                    gasP2.isSustained = false; gasP2.pressTimer = 0.0f;
                }
            }

            Color btnBg = editorMode ? Color{ 71, 140, 191, 245 } : (hoverEditor ? Color{ 48, 54, 70, 240 } : Color{ 32, 37, 49, 230 });
            Color btnBorder = editorMode ? Color{ 110, 180, 230, 255 } : Color{ 68, 76, 95, 200 };
            DrawRectangleRounded(btnEditorRec, 0.25f, 4, btnBg);
            DrawRectangleRoundedLinesEx(btnEditorRec, 0.25f, 4, 1.0f, btnBorder);
            const char* toggleLbl = editorMode ? "Play Mode (F1)" : "Level Editor (F1)";
            Vector2 tSz = MeasureTextEx(fontUIBold, toggleLbl, 14, 0.4f);
            DrawTextEx(fontUIBold, toggleLbl, Vector2{ btnEditorRec.x + (btnEditorRec.width - tSz.x) * 0.5f, btnEditorRec.y + 8 }, 14, 0.4f, WHITE);
        }

        // 6. Interactive 3D Level Editor Inspector Panel (Authentic Godot 4 Engine Style)
        if (editorMode) {
            float panelW = 340.0f;
            float panelH = 600.0f;
            Rectangle panelRec = { (float)GetScreenWidth() - panelW - 14, 52, panelW, panelH };

            // Subtle Drop Shadow
            DrawRectangleRounded(Rectangle{ panelRec.x + 3, panelRec.y + 4, panelW, panelH }, 0.02f, 4, Color{ 0, 0, 0, 90 });

            // Godot Main Inspector Panel Surface
            DrawRectangleRounded(panelRec, 0.02f, 4, Color{ 32, 37, 49, 252 }); // #202531
            DrawRectangleRoundedLinesEx(panelRec, 0.02f, 4, 1.0f, Color{ 58, 66, 83, 255 }); // #3a4253

            // Godot Dock Tab Bar
            Rectangle tabRec = { panelRec.x, panelRec.y, 115, 30 };
            DrawRectangleRec(Rectangle{ panelRec.x, panelRec.y, panelW, 30 }, Color{ 25, 29, 38, 255 }); // #191d26
            DrawRectangleRec(tabRec, Color{ 32, 37, 49, 255 });
            DrawRectangle((int)tabRec.x, (int)tabRec.y + 28, (int)tabRec.width, 2, Color{ 71, 140, 191, 255 }); // Godot Blue active line
            DrawTextEx(fontUIBold, "Inspector", Vector2{ tabRec.x + 18, tabRec.y + 7 }, 14, 0.4f, Color{ 240, 243, 246, 255 });
            DrawTextEx(fontUI, "3D Level View", Vector2{ panelRec.x + panelW - 105, tabRec.y + 8 }, 12, 0.3f, Color{ 140, 148, 163, 255 });
            DrawLine((int)panelRec.x, (int)panelRec.y + 30, (int)panelRec.x + (int)panelW, (int)panelRec.y + 30, Color{ 58, 66, 83, 255 });

            float py = panelRec.y + 38;

            auto DrawGodotBtn = [&](Rectangle rec, const char* label, Color bgCol, Color textCol, bool bold = false) -> bool {
                bool hov = CheckCollisionPointRec(mousePos, rec);
                Color fill = hov ? ColorBrightness(bgCol, 0.18f) : bgCol;
                DrawRectangleRounded(rec, 0.2f, 3, fill);
                DrawRectangleRoundedLinesEx(rec, 0.2f, 3, 1.0f, ColorBrightness(bgCol, 0.35f));
                Font f = bold ? fontUIBold : fontUI;
                Vector2 sz = MeasureTextEx(f, label, 13, 0.3f);
                DrawTextEx(f, label, Vector2{ rec.x + (rec.width - sz.x) * 0.5f, rec.y + (rec.height - sz.y) * 0.5f }, 13, 0.3f, textCol);
                return hov && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
            };

            auto DrawSectionHeader = [&](const char* title) {
                DrawRectangleRec(Rectangle{ panelRec.x + 8, py, panelW - 16, 22 }, Color{ 25, 29, 38, 255 });
                DrawLine((int)panelRec.x + 8, (int)py + 22, (int)panelRec.x + (int)panelW - 8, (int)py + 22, Color{ 45, 52, 66, 255 });
                DrawTextEx(fontUIBold, title, Vector2{ panelRec.x + 14, py + 4 }, 12, 0.4f, Color{ 209, 213, 219, 255 });
                py += 28;
            };

            if (selectedObs >= 0 && selectedObs < (int)obsList.size()) {
                auto& o = obsList[selectedObs];
                const char* typeNames[] = { "ROTOR_B5", "TIPPING_B2", "PENDULUM_C2", "GATE_F10", "SWING_F3", "BUMPER_SPRING", "ROCK1", "ROCK2" };
                const char* tName = (o.type >= 0 && o.type <= 7) ? typeNames[o.type] : "OBSTACLE";

                // Object Banner (Godot Node Style)
                Rectangle objBannerRec = { panelRec.x + 8, py, panelW - 16, 32 };
                DrawRectangleRounded(objBannerRec, 0.15f, 3, Color{ 24, 28, 37, 255 });
                DrawRectangleRoundedLinesEx(objBannerRec, 0.15f, 3, 1.0f, Color{ 48, 55, 71, 255 });
                DrawCircle((int)objBannerRec.x + 14, (int)objBannerRec.y + 16, 5.0f, Color{ 71, 140, 191, 255 }); // Godot Node Dot
                DrawTextEx(fontUIBold, TextFormat("KineticObstacle_#%d", selectedObs), Vector2{ objBannerRec.x + 26, objBannerRec.y + 8 }, 13, 0.4f, WHITE);
                
                // Chip on right
                Rectangle chipRec = { objBannerRec.x + objBannerRec.width - 110, objBannerRec.y + 5, 102, 22 };
                DrawRectangleRounded(chipRec, 0.25f, 3, Color{ 39, 46, 61, 255 });
                Vector2 chipSz = MeasureTextEx(fontUI, tName, 11, 0.2f);
                DrawTextEx(fontUI, tName, Vector2{ chipRec.x + (chipRec.width - chipSz.x)*0.5f, chipRec.y + 5 }, 11, 0.2f, Color{ 147, 197, 253, 255 });
                py += 38;

                // 1. Section: Transform
                DrawSectionHeader("v  Transform");

                // Position Row (X / Y / Z Columns with Godot Color Accents)
                DrawTextEx(fontUI, "Position", Vector2{ panelRec.x + 12, py }, 11, 0.3f, Color{ 156, 163, 175, 255 });
                py += 16;

                float colW = 100.0f;
                float startX = panelRec.x + 12;

                // Column X (Red)
                {
                    Rectangle xRec = { startX, py, colW, 26 };
                    DrawRectangleRounded(xRec, 0.15f, 3, Color{ 20, 23, 30, 255 });
                    DrawRectangleRoundedLinesEx(xRec, 0.15f, 3, 1.0f, Color{ 45, 52, 66, 255 });
                    // Red Pill
                    DrawRectangleRec(Rectangle{ xRec.x, xRec.y, 16, xRec.height }, Color{ 214, 69, 69, 255 });
                    DrawTextEx(fontUIBold, "X", Vector2{ xRec.x + 4, xRec.y + 6 }, 11, 0.2f, WHITE);
                    DrawTextEx(fontUI, TextFormat("%.2f", o.position.x), Vector2{ xRec.x + 22, xRec.y + 6 }, 12, 0.3f, Color{ 243, 244, 246, 255 });
                    // Micro +/- step buttons
                    if (DrawGodotBtn(Rectangle{ xRec.x + colW - 24, xRec.y + 2, 11, 22 }, "-", Color{ 32, 37, 49, 255 }, Color{ 156, 163, 175, 255 }) || IsKeyPressed(KEY_J)) o.position.x -= 0.5f;
                    if (DrawGodotBtn(Rectangle{ xRec.x + colW - 12, xRec.y + 2, 11, 22 }, "+", Color{ 32, 37, 49, 255 }, Color{ 156, 163, 175, 255 }) || IsKeyPressed(KEY_L)) o.position.x += 0.5f;
                }

                // Column Y (Green)
                {
                    Rectangle yRec = { startX + colW + 8, py, colW, 26 };
                    DrawRectangleRounded(yRec, 0.15f, 3, Color{ 20, 23, 30, 255 });
                    DrawRectangleRoundedLinesEx(yRec, 0.15f, 3, 1.0f, Color{ 45, 52, 66, 255 });
                    // Green Pill
                    DrawRectangleRec(Rectangle{ yRec.x, yRec.y, 16, yRec.height }, Color{ 46, 160, 67, 255 });
                    DrawTextEx(fontUIBold, "Y", Vector2{ yRec.x + 4, yRec.y + 6 }, 11, 0.2f, WHITE);
                    DrawTextEx(fontUI, TextFormat("%.2f", o.position.y), Vector2{ yRec.x + 22, yRec.y + 6 }, 12, 0.3f, Color{ 243, 244, 246, 255 });
                    if (DrawGodotBtn(Rectangle{ yRec.x + colW - 24, yRec.y + 2, 11, 22 }, "-", Color{ 32, 37, 49, 255 }, Color{ 156, 163, 175, 255 }) || IsKeyPressed(KEY_O)) o.position.y -= 0.2f;
                    if (DrawGodotBtn(Rectangle{ yRec.x + colW - 12, yRec.y + 2, 11, 22 }, "+", Color{ 32, 37, 49, 255 }, Color{ 156, 163, 175, 255 }) || IsKeyPressed(KEY_U)) o.position.y += 0.2f;
                }

                // Column Z (Blue)
                {
                    Rectangle zRec = { startX + (colW + 8) * 2, py, colW, 26 };
                    DrawRectangleRounded(zRec, 0.15f, 3, Color{ 20, 23, 30, 255 });
                    DrawRectangleRoundedLinesEx(zRec, 0.15f, 3, 1.0f, Color{ 45, 52, 66, 255 });
                    // Blue Pill
                    DrawRectangleRec(Rectangle{ zRec.x, zRec.y, 16, zRec.height }, Color{ 59, 130, 246, 255 });
                    DrawTextEx(fontUIBold, "Z", Vector2{ zRec.x + 4, zRec.y + 6 }, 11, 0.2f, WHITE);
                    DrawTextEx(fontUI, TextFormat("%.2f", o.position.z), Vector2{ zRec.x + 22, zRec.y + 6 }, 12, 0.3f, Color{ 243, 244, 246, 255 });
                    if (DrawGodotBtn(Rectangle{ zRec.x + colW - 24, zRec.y + 2, 11, 22 }, "-", Color{ 32, 37, 49, 255 }, Color{ 156, 163, 175, 255 }) || IsKeyPressed(KEY_K)) o.position.z -= 0.5f;
                    if (DrawGodotBtn(Rectangle{ zRec.x + colW - 12, zRec.y + 2, 11, 22 }, "+", Color{ 32, 37, 49, 255 }, Color{ 156, 163, 175, 255 }) || IsKeyPressed(KEY_I)) o.position.z += 0.5f;
                }
                py += 32;

                // Rotation Row
                DrawTextEx(fontUI, "Rotation (Y-Axis)", Vector2{ panelRec.x + 12, py }, 11, 0.3f, Color{ 156, 163, 175, 255 });
                py += 16;
                Rectangle rotBox = { panelRec.x + 12, py, 140, 26 };
                DrawRectangleRounded(rotBox, 0.15f, 3, Color{ 20, 23, 30, 255 });
                DrawRectangleRoundedLinesEx(rotBox, 0.15f, 3, 1.0f, Color{ 45, 52, 66, 255 });
                DrawTextEx(fontUI, TextFormat("%.1f deg", o.rotationY), Vector2{ rotBox.x + 12, rotBox.y + 6 }, 12, 0.3f, Color{ 243, 244, 246, 255 });
                
                if (DrawGodotBtn(Rectangle{ panelRec.x + 160, py, 80, 26 }, "-15 deg", Color{ 40, 47, 62, 255 }, Color{ 229, 231, 235, 255 }) || IsKeyPressed(KEY_R)) o.rotationY -= 15.0f;
                if (DrawGodotBtn(Rectangle{ panelRec.x + 246, py, 80, 26 }, "+15 deg", Color{ 40, 47, 62, 255 }, Color{ 229, 231, 235, 255 }) || IsKeyPressed(KEY_T)) o.rotationY += 15.0f;
                py += 34;

                // 2. Section: KineticObstacle Properties
                DrawSectionHeader("v  Kinetic Properties");

                // Type OptionButton
                DrawTextEx(fontUI, "Obstacle Type", Vector2{ panelRec.x + 12, py }, 11, 0.3f, Color{ 156, 163, 175, 255 });
                py += 16;

                Rectangle optRec = { panelRec.x + 12, py, panelW - 24, 28 };
                DrawRectangleRounded(optRec, 0.15f, 3, Color{ 24, 28, 37, 255 });
                DrawRectangleRoundedLinesEx(optRec, 0.15f, 3, 1.0f, Color{ 58, 66, 83, 255 });

                auto ChangeType = [&](int delta) {
                    o.type = (ObstacleType)(((int)o.type + delta + 8) % 8);
                    if (o.type == OBS_BUMPER_SPRING) { o.radius = 2.5f; o.model = &obsModelA1; }
                    else if (o.type == OBS_ROTOR_B5) { o.radius = 2.4f; o.model = &obsModelB5; }
                    else if (o.type == OBS_TIPPING_B2) { o.radius = 2.2f; o.model = &obsModelB2; }
                    else if (o.type == OBS_PENDULUM_C2) { o.radius = 2.2f; o.model = &obsModelC2; }
                    else if (o.type == OBS_GATE_F10) { o.radius = 2.2f; o.model = &obsModelF10; }
                    else if (o.type == OBS_SWING_F3) { o.radius = 2.2f; o.model = &obsModelF3; }
                    else if (o.type == OBS_ROCK1) { o.radius = 1.6f; o.model = &obsModelRock1; }
                    else if (o.type == OBS_ROCK2) { o.radius = 1.6f; o.model = &obsModelRock2; }
                    editorMsg = TextFormat("Type: %s", typeNames[o.type]);
                    editorMsgTimer = 1.5f;
                };

                if (DrawGodotBtn(Rectangle{ optRec.x + 2, optRec.y + 2, 28, 24 }, "<", Color{ 36, 42, 56, 255 }, WHITE, true)) ChangeType(-1);
                Vector2 tNameSz = MeasureTextEx(fontUIBold, tName, 13, 0.4f);
                DrawTextEx(fontUIBold, tName, Vector2{ optRec.x + (optRec.width - tNameSz.x) * 0.5f, optRec.y + 6 }, 13, 0.4f, Color{ 147, 197, 253, 255 });
                if (DrawGodotBtn(Rectangle{ optRec.x + optRec.width - 30, optRec.y + 2, 28, 24 }, ">", Color{ 36, 42, 56, 255 }, WHITE, true)) ChangeType(1);
                py += 34;

                // Radius display
                DrawTextEx(fontUI, TextFormat("Collision Radius:  %.2f m", o.radius), Vector2{ panelRec.x + 14, py }, 11, 0.3f, Color{ 156, 163, 175, 255 });
                py += 22;

                // 3. Section: Node Actions
                DrawSectionHeader("v  Actions");

                if (DrawGodotBtn(Rectangle{ panelRec.x + 12, py, (panelW - 32) * 0.5f, 28 }, "Duplicate (Ctrl+D)", Color{ 40, 48, 64, 255 }, Color{ 229, 231, 235, 255 }) || (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_D))) {
                    KineticObstacle dup = o;
                    dup.position.z -= 2.0f;
                    obsList.push_back(dup);
                    selectedObs = (int)obsList.size() - 1;
                    editorMsg = "Obstacle Duplicated!";
                    editorMsgTimer = 2.0f;
                }
                if (DrawGodotBtn(Rectangle{ panelRec.x + 16 + (panelW - 32) * 0.5f, py, (panelW - 32) * 0.5f, 28 }, "Delete (Del)", Color{ 110, 38, 38, 255 }, Color{ 254, 202, 202, 255 }) || IsKeyPressed(KEY_DELETE) || IsKeyPressed(KEY_BACKSPACE)) {
                    obsList.erase(obsList.begin() + selectedObs);
                    selectedObs = -1;
                    editorMsg = "Obstacle Deleted!";
                    editorMsgTimer = 2.0f;
                }
                py += 36;
            } else {
                DrawRectangleRec(Rectangle{ panelRec.x + 12, py, panelW - 24, 70 }, Color{ 24, 28, 37, 255 });
                DrawRectangleLinesEx(Rectangle{ panelRec.x + 12, py, panelW - 24, 70 }, 1.0f, Color{ 45, 52, 66, 255 });
                DrawTextEx(fontUI, "No obstacle selected.", Vector2{ panelRec.x + 24, py + 14 }, 13, 0.3f, Color{ 209, 213, 219, 255 });
                DrawTextEx(fontUI, "Left-Click any obstacle in 3D scene to edit.", Vector2{ panelRec.x + 24, py + 36 }, 11, 0.3f, Color{ 148, 156, 172, 255 });
                py += 82;
            }

            // Global Scene Actions
            DrawLine((int)panelRec.x + 10, (int)py, (int)panelRec.x + (int)panelW - 10, (int)py, Color{ 48, 55, 71, 255 });
            py += 10;

            Rectangle btnAddRec = { panelRec.x + 12, py, panelW - 24, 30 };
            if (DrawGodotBtn(btnAddRec, "+  Add New Obstacle (N)", Color{ 42, 50, 68, 255 }, WHITE, true) || IsKeyPressed(KEY_N)) {
                KineticObstacle newO;
                newO.type = OBS_BUMPER_SPRING;
                Vector3 fwd = { -sinf(editorCamYaw) * cosf(editorCamPitch), sinf(editorCamPitch), -cosf(editorCamYaw) * cosf(editorCamPitch) };
                newO.position = Vector3Add(editorCamPos, Vector3Scale(fwd, 6.5f));
                newO.position.y = 0.65f;
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
                editorMsg = "New Obstacle Spawned In View!";
                editorMsgTimer = 2.5f;
            }
            py += 36;

            Rectangle btnSaveRec = { panelRec.x + 12, py, panelW - 24, 34 };
            if (DrawGodotBtn(btnSaveRec, "Save Level to JSON (Ctrl+S)", Color{ 71, 140, 191, 255 }, WHITE, true) || (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_S)) || IsKeyPressed(KEY_F5)) {
                SaveObstaclesJSON(LEVEL_FILE_PATH, obsList);
                editorMsg = "Level Saved to level_obstacles.json";
                editorMsgTimer = 3.5f;
            }
            py += 44;

            // Godot Status & Shortcut Tray
            Rectangle trayRec = { panelRec.x + 8, panelRec.y + panelH - 62, panelW - 16, 54 };
            DrawRectangleRounded(trayRec, 0.15f, 3, Color{ 20, 23, 30, 255 });
            DrawRectangleRoundedLinesEx(trayRec, 0.15f, 3, 1.0f, Color{ 45, 52, 66, 255 });
            DrawTextEx(fontUI, "RMB+WASD: Fly Cam  |  F: Focus Selection", Vector2{ trayRec.x + 10, trayRec.y + 8 }, 11, 0.3f, Color{ 156, 163, 175, 255 });
            DrawTextEx(fontUI, "Hold J/L: X  |  I/K: Z  |  U/O: Y  |  R/T: Rot", Vector2{ trayRec.x + 10, trayRec.y + 24 }, 11, 0.3f, Color{ 156, 163, 175, 255 });
            DrawTextEx(fontUI, "F1: Exit Editor & Resume Game", Vector2{ trayRec.x + 10, trayRec.y + 38 }, 11, 0.3f, Color{ 147, 197, 253, 255 });

            // Godot Style Toast Notification
            if (editorMsgTimer > 0.0f) {
                editorMsgTimer -= dt;
                Vector2 msgSz = MeasureTextEx(fontUIBold, editorMsg.c_str(), 14, 0.4f);
                int toastW = (int)msgSz.x + 40;
                int toastH = 34;
                int toastX = GetScreenWidth() / 2 - toastW / 2;
                int toastY = 70;
                DrawRectangleRounded(Rectangle{ (float)toastX, (float)toastY, (float)toastW, (float)toastH }, 0.35f, 4, Color{ 25, 30, 42, 245 });
                DrawRectangleRoundedLinesEx(Rectangle{ (float)toastX, (float)toastY, (float)toastW, (float)toastH }, 0.35f, 4, 1.2f, Color{ 71, 140, 191, 255 });
                DrawTextEx(fontUIBold, editorMsg.c_str(), Vector2{ (float)toastX + 20, (float)toastY + 9 }, 14, 0.4f, WHITE);
            }
        }
"""
    code = code[:start_idx] + new_panel_code + code[end_idx:]
    print("Replaced inspector panel with Godot 4 theme.")
else:
    print("ERROR: Start or end tag not found!")

with open(path, "w", encoding="utf-8") as f:
    f.write(code)

print("Saved updated GameEngine.cpp successfully!")

