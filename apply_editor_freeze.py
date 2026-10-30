import re

path = "CustomEngine/native/GameEngine.cpp"
with open(path, "r", encoding="utf-8") as f:
    code = f.read()

# 1. Update texHipo to load Character_GameGreen.png
old_tex = 'Texture2D texHipo       = LoadTexture(ResolveAsset("extracted_assets/ui/Hipo.png").c_str());'
new_tex = 'Texture2D texHipo       = LoadTexture(ResolveAsset("extracted_assets/ui/Character_GameGreen.png").c_str());'
if old_tex in code:
    code = code.replace(old_tex, new_tex)
    print("Replaced texHipo load.")
else:
    print("WARNING: texHipo load line not found exactly!")

# 2. Update raceTimer: only increment when not in editorMode
old_timer = 'if (!raceFinished) raceTimer += dt;'
new_timer = 'if (!raceFinished && !editorMode) raceTimer += dt;'
if old_timer in code:
    code = code.replace(old_timer, new_timer)
    print("Replaced raceTimer increment.")

# 3. Locate the gameplay physics section and wrap in if (!editorMode) { ... } else { ... }
start_tag = "// 3. Player Inputs & Gas Physics"
end_tag = "// Test Camera Overrides"

start_idx = code.find(start_tag)
end_idx = code.find(end_tag)

if start_idx != -1 and end_idx != -1:
    physics_code = code[start_idx:end_idx]
    wrapped_physics = f"""// TOTAL FREEZE CHECK: If in Level Editor Mode, completely freeze raft, characters and physics!
        if (!editorMode) {{
{physics_code}
        }} else {{
            // TOTAL FREEZE OF GAMEPLAY & MOVEMENT DURING LEVEL EDITOR MODE
            raft.velocity = Vector3Zero();
            raft.angularVelocityY = 0.0f;
            gasP1.isSustained = false;
            gasP1.pressTimer = 0.0f;
            gasP1.trembleOffset = 0.0f;
            gasP1.redness = 0.0f;
            gasP1.boneScale = 1.0f;

            gasP2.isSustained = false;
            gasP2.pressTimer = 0.0f;
            gasP2.trembleOffset = 0.0f;
            gasP2.redness = 0.0f;
            gasP2.boneScale = 1.0f;
        }}
"""
    code = code[:start_idx] + wrapped_physics + code[end_idx:]
    print("Wrapped gameplay physics inside if (!editorMode).")
else:
    print("ERROR: Could not locate physics section boundaries!")

# 4. Update the Editor Camera block to add continuous hold for selected obstacle movement and F key focus
old_editor_cam = """        if (editorMode) {
            // Free-fly Editor Camera (Unity / Godot style)
            if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
                Vector2 mDelta = GetMouseDelta();
                editorCamYaw -= mDelta.x * 0.003f;
                editorCamPitch -= mDelta.y * 0.003f;
                editorCamPitch = Clamp(editorCamPitch, -1.4f, 1.4f);
            }

            Vector3 fwd = { -sinf(editorCamYaw) * cosf(editorCamPitch), sinf(editorCamPitch), -cosf(editorCamYaw) * cosf(editorCamPitch) };
            Vector3 right = { cosf(editorCamYaw), 0.0f, -sinf(editorCamYaw) };
            float flySpd = (IsKeyDown(KEY_LEFT_SHIFT) ? 26.0f : 12.0f) * dt;

            if (IsKeyDown(KEY_W)) editorCamPos = Vector3Add(editorCamPos, Vector3Scale(fwd, flySpd));
            if (IsKeyDown(KEY_S)) editorCamPos = Vector3Subtract(editorCamPos, Vector3Scale(fwd, flySpd));
            if (IsKeyDown(KEY_D)) editorCamPos = Vector3Add(editorCamPos, Vector3Scale(right, flySpd));
            if (IsKeyDown(KEY_A)) editorCamPos = Vector3Subtract(editorCamPos, Vector3Scale(right, flySpd));
            if (IsKeyDown(KEY_SPACE)) editorCamPos.y += flySpd;
            if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_C)) editorCamPos.y -= flySpd;

            camera.position = editorCamPos;
            camera.target = Vector3Add(editorCamPos, fwd);

            // Raycast Click Selection in 3D
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && mousePos.x < (GetScreenWidth() - 340)) {
                Ray mRay = GetScreenToWorldRay(mousePos, camera);
                float bestDist = 9999.0f;
                int hitIdx = -1;
                for (size_t i = 0; i < obsList.size(); i++) {
                    BoundingBox oBox = { Vector3Subtract(obsList[i].position, Vector3{ 1.5f, 1.5f, 1.5f }), Vector3Add(obsList[i].position, Vector3{ 1.5f, 1.5f, 1.5f }) };
                    RayCollision rHit = GetRayCollisionBox(mRay, oBox);
                    if (rHit.hit && rHit.distance < bestDist) {
                        bestDist = rHit.distance;
                        hitIdx = (int)i;
                    }
                }
                if (hitIdx != -1) {
                    selectedObs = hitIdx;
                    editorMsg = TextFormat("Selected Obstacle #%d", hitIdx);
                    editorMsgTimer = 2.0f;
                }
            }
        }"""

new_editor_cam = """        if (editorMode) {
            // Free-fly Editor Camera (Unity / Godot style)
            if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
                Vector2 mDelta = GetMouseDelta();
                editorCamYaw -= mDelta.x * 0.003f;
                editorCamPitch -= mDelta.y * 0.003f;
                editorCamPitch = Clamp(editorCamPitch, -1.4f, 1.4f);
            }

            Vector3 fwd = { -sinf(editorCamYaw) * cosf(editorCamPitch), sinf(editorCamPitch), -cosf(editorCamYaw) * cosf(editorCamPitch) };
            Vector3 right = { cosf(editorCamYaw), 0.0f, -sinf(editorCamYaw) };
            float flySpd = (IsKeyDown(KEY_LEFT_SHIFT) ? 28.0f : 12.0f) * dt;

            if (IsKeyDown(KEY_W)) editorCamPos = Vector3Add(editorCamPos, Vector3Scale(fwd, flySpd));
            if (IsKeyDown(KEY_S)) editorCamPos = Vector3Subtract(editorCamPos, Vector3Scale(fwd, flySpd));
            if (IsKeyDown(KEY_D)) editorCamPos = Vector3Add(editorCamPos, Vector3Scale(right, flySpd));
            if (IsKeyDown(KEY_A)) editorCamPos = Vector3Subtract(editorCamPos, Vector3Scale(right, flySpd));
            if (IsKeyDown(KEY_SPACE) || IsKeyDown(KEY_E)) editorCamPos.y += flySpd;
            if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_Q) || IsKeyDown(KEY_C)) editorCamPos.y -= flySpd;

            camera.position = editorCamPos;
            camera.target = Vector3Add(editorCamPos, fwd);

            // Raycast Click Selection in 3D
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && mousePos.x < (GetScreenWidth() - 340)) {
                Ray mRay = GetScreenToWorldRay(mousePos, camera);
                float bestDist = 9999.0f;
                int hitIdx = -1;
                for (size_t i = 0; i < obsList.size(); i++) {
                    BoundingBox oBox = { Vector3Subtract(obsList[i].position, Vector3{ 1.8f, 1.8f, 1.8f }), Vector3Add(obsList[i].position, Vector3{ 1.8f, 1.8f, 1.8f }) };
                    RayCollision rHit = GetRayCollisionBox(mRay, oBox);
                    if (rHit.hit && rHit.distance < bestDist) {
                        bestDist = rHit.distance;
                        hitIdx = (int)i;
                    }
                }
                if (hitIdx != -1) {
                    selectedObs = hitIdx;
                    editorMsg = TextFormat("Selected Obstacle #%d", hitIdx);
                    editorMsgTimer = 2.0f;
                }
            }

            // Keyboard Manipulation of Selected Obstacle (Continuous hold support!)
            if (selectedObs >= 0 && selectedObs < (int)obsList.size()) {
                auto& o = obsList[selectedObs];
                float editSpd = (IsKeyDown(KEY_LEFT_SHIFT) ? 10.0f : 4.0f) * dt;
                
                // J / L = Move X
                if (IsKeyDown(KEY_J)) o.position.x -= editSpd;
                if (IsKeyDown(KEY_L)) o.position.x += editSpd;
                // I / K = Move Z (along stream)
                if (IsKeyDown(KEY_I)) o.position.z += editSpd;
                if (IsKeyDown(KEY_K)) o.position.z -= editSpd;
                // U / O = Move Y (height)
                if (IsKeyDown(KEY_U)) o.position.y += editSpd;
                if (IsKeyDown(KEY_O)) o.position.y -= editSpd;
                // R / T = Rotate Y
                if (IsKeyDown(KEY_R)) o.rotationY -= 65.0f * dt;
                if (IsKeyDown(KEY_T)) o.rotationY += 65.0f * dt;

                // Focus on object (Unity 'F' key)
                if (IsKeyPressed(KEY_F)) {
                    editorCamPos = Vector3Add(o.position, Vector3{ 0.0f, 3.5f, 6.0f });
                    editorCamYaw = 3.14159f;
                    editorCamPitch = 0.35f;
                    editorMsg = "Focused Camera on Selection (F)";
                    editorMsgTimer = 1.5f;
                }
            }
        }"""

if old_editor_cam in code:
    code = code.replace(old_editor_cam, new_editor_cam)
    print("Replaced editor camera & manipulation block.")
else:
    print("WARNING: old_editor_cam block not found directly, will check with regex or normalized whitespace.")

# 5. Update UI toggle and inspector:
old_toggle = """            if ((hoverEditor && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) || IsKeyPressed(KEY_F1) || IsKeyPressed(KEY_E)) {
                editorMode = !editorMode;
                if (editorMode) {
                    editorCamPos = Vector3Add(raft.position, Vector3{ 0.0f, 4.5f, 6.0f });
                    editorCamYaw = raft.rotationY;
                    editorCamPitch = 0.35f;
                    editorMsg = "EDITOR MODE ACTIVE (UNITY / GODOT STYLE)";
                    editorMsgTimer = 3.0f;
                }
            }"""

new_toggle = """            if ((hoverEditor && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) || IsKeyPressed(KEY_F1)) {
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
            }"""

if old_toggle in code:
    code = code.replace(old_toggle, new_toggle)
    print("Replaced editor toggle.")

# 6. Update top-left icon drawing rect:
old_draw_hipo = """            DrawTexturePro(texHipo, Rectangle{ 0, 0, (float)texHipo.width, (float)texHipo.height },
                           Rectangle{ 14, 6, 56, 36 }, Vector2{ 0, 0 }, 0.0f, WHITE);"""

new_draw_hipo = """            DrawTexturePro(texHipo, Rectangle{ 0, 0, (float)texHipo.width, (float)texHipo.height },
                           Rectangle{ 14, 4, 46, 50 }, Vector2{ 0, 0 }, 0.0f, WHITE);"""

if old_draw_hipo in code:
    code = code.replace(old_draw_hipo, new_draw_hipo)
    print("Replaced DrawTexturePro for texHipo.")

# 7. Update add new obstacle to spawn in front of camera:
old_add = """            Rectangle btnAddRec = { panelRec.x + 16, (float)py, panelW - 32, 32 };
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
            }"""

new_add = """            Rectangle btnAddRec = { panelRec.x + 16, (float)py, panelW - 32, 32 };
            bool hovAdd = CheckCollisionPointRec(mousePos, btnAddRec);
            if ((hovAdd && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) || IsKeyPressed(KEY_N)) {
                KineticObstacle newO;
                newO.type = OBS_BUMPER_SPRING;
                Vector3 fwd = { -sinf(editorCamYaw) * cosf(editorCamPitch), sinf(editorCamPitch), -cosf(editorCamYaw) * cosf(editorCamPitch) };
                newO.position = Vector3Add(editorCamPos, Vector3Scale(fwd, 6.5f));
                newO.position.y = 0.65f; // Clamped to river height
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
                editorMsg = "New Obstacle Spawned in View!";
                editorMsgTimer = 2.5f;
            }"""

if old_add in code:
    code = code.replace(old_add, new_add)
    print("Replaced add obstacle block.")

# 8. Add type switcher to inspector
old_inspector_top = """                DrawText(TextFormat("Selected: #%d (%s)", selectedObs, tName), (int)panelRec.x + 16, py, 14, Color{ 147, 197, 253, 255 }); py += 22;
                DrawText(TextFormat("Position: (%.2f, %.2f, %.2f)", o.position.x, o.position.y, o.position.z), (int)panelRec.x + 16, py, 13, RAYWHITE); py += 18;
                DrawText(TextFormat("Rotation: %.1f deg", o.rotationY), (int)panelRec.x + 16, py, 13, RAYWHITE); py += 24;

                auto DrawBtn = [&](Rectangle rec, const char* label, Color col) -> bool {
                    bool hov = CheckCollisionPointRec(mousePos, rec);
                    DrawRectangleRounded(rec, 0.25f, 3, hov ? ColorBrightness(col, 0.2f) : col);
                    DrawText(label, (int)rec.x + 8, (int)rec.y + 6, 12, RAYWHITE);
                    return hov && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
                };"""

new_inspector_top = """                DrawText(TextFormat("Selected: #%d (%s)", selectedObs, tName), (int)panelRec.x + 16, py, 14, Color{ 147, 197, 253, 255 }); py += 22;
                DrawText(TextFormat("Position: (%.2f, %.2f, %.2f)", o.position.x, o.position.y, o.position.z), (int)panelRec.x + 16, py, 13, RAYWHITE); py += 18;
                DrawText(TextFormat("Rotation: %.1f deg", o.rotationY), (int)panelRec.x + 16, py, 13, RAYWHITE); py += 22;

                auto DrawBtn = [&](Rectangle rec, const char* label, Color col) -> bool {
                    bool hov = CheckCollisionPointRec(mousePos, rec);
                    DrawRectangleRounded(rec, 0.25f, 3, hov ? ColorBrightness(col, 0.2f) : col);
                    DrawText(label, (int)rec.x + 8, (int)rec.y + 6, 12, RAYWHITE);
                    return hov && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
                };

                // Obstacle Type Switcher
                if (DrawBtn(Rectangle{ panelRec.x + 16, (float)py, 135, 26 }, "< PREV TYPE", Color{ 30, 64, 175, 255 })) {
                    o.type = (ObstacleType)(((int)o.type + 7) % 8);
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
                }
                if (DrawBtn(Rectangle{ panelRec.x + 160, (float)py, 135, 26 }, "NEXT TYPE >", Color{ 30, 64, 175, 255 })) {
                    o.type = (ObstacleType)(((int)o.type + 1) % 8);
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
                }
                py += 32;"""

if old_inspector_top in code:
    code = code.replace(old_inspector_top, new_inspector_top)
    print("Replaced inspector top with type switcher.")

with open(path, "w", encoding="utf-8") as f:
    f.write(code)

print("Saved updated GameEngine.cpp successfully!")

