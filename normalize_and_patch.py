import re

with open("CustomEngine/native/GameEngine.cpp", "r", encoding="utf-8") as f:
    raw = f.read()

lines = raw.splitlines()
clean = []
prev_empty = False
for l in lines:
    is_emp = (l.strip() == '')
    if is_emp:
        if not prev_empty:
            clean.append('')
            prev_empty = True
    else:
        clean.append(l)
        prev_empty = False

c = '\n'.join(clean)

# 1. UI Textures loading
old_tex = 'Texture2D texRaftFoam = LoadTexture(ResolveAsset("extracted_assets/textures/Raft-Foam4.png").c_str());'
new_tex = '''Texture2D texRaftFoam = LoadTexture(ResolveAsset("extracted_assets/textures/Raft-Foam4.png").c_str());
    Texture2D texHipo       = LoadTexture(ResolveAsset("extracted_assets/ui/Hipo.png").c_str());
    Texture2D texClock      = LoadTexture(ResolveAsset("extracted_assets/ui/Icon_Clock.png").c_str());
    Texture2D texCrashed    = LoadTexture(ResolveAsset("extracted_assets/ui/Icon_RaftCrashed.png").c_str());
    Texture2D texRaftPin    = LoadTexture(ResolveAsset("extracted_assets/ui/Icon_RaftS.png").c_str());
    Texture2D texFinish     = LoadTexture(ResolveAsset("extracted_assets/ui/Icon_Finish.png").c_str());
    Texture2D texEsc        = LoadTexture(ResolveAsset("extracted_assets/ui/keyboard_escape.png").c_str());
    SetTextureFilter(texHipo, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(texClock, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(texCrashed, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(texRaftPin, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(texFinish, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(texEsc, TEXTURE_FILTER_BILINEAR);'''

assert old_tex in c, "old_tex not found"
c = c.replace(old_tex, new_tex, 1)

# 2. JSON Obstacle loading
target_obs = '&obsModelRock2, 0.0f, 0.0f, false }'
idx = c.find(target_obs)
assert idx != -1, "target_obs not found"
idx_close = c.find('};', idx)
assert idx_close != -1, "closing brace of obsList not found"

c = c[:idx_close+2] + '''

    // Load custom user obstacle layout from JSON if it exists, otherwise create it
    if (!LoadObstaclesJSON(LEVEL_FILE_PATH, obsList, &obsModelA1, &obsModelB5, &obsModelB2, &obsModelC2, &obsModelF10, &obsModelF3, &obsModelRock1, &obsModelRock2)) {
        SaveObstaclesJSON(LEVEL_FILE_PATH, obsList);
    }''' + c[idx_close+2:]

# 3. Add Editor state variables
old_vars = 'float raceTimer = 0.0f;'
new_vars = '''float raceTimer = 0.0f;
    int totalDeaths = 0;

    // Interactive Level & Obstacle Editor (Unity / Godot style)
    bool editorMode = false;
    int selectedObs = -1;
    Vector3 editorCamPos = Vector3{ 15.55f, 6.0f, 75.0f };
    float editorCamYaw = 3.14159f;
    float editorCamPitch = 0.35f;
    std::string editorMsg = "";
    float editorMsgTimer = 0.0f;'''

assert old_vars in c, "old_vars not found"
c = c.replace(old_vars, new_vars, 1)

# 4. Replace P2 AI Logic with Dual Manual Controls
p2_ai_pattern = r'// P2 AI Co-pilot Logic \(Right Pirate\)[\s\S]*?gasP2\.Update\(dt, p2InputDown, runTime\);'
p2_manual_code = '''// Player 2 Manual Controls (Right Pirate) - Keyboard Arrow Keys & Mouse Click!
        float p2RimMove = 0.0f;
        if (IsKeyDown(KEY_LEFT))  p2RimMove -= 1.0f;
        if (IsKeyDown(KEY_RIGHT)) p2RimMove += 1.0f;
        p2RimAngle += p2RimMove * 2.44f * dt;
        p2RimAngle = Clamp(p2RimAngle, 0.65f, 2.85f); // Slide along right rim

        bool p2InputDown = (!editorMode) && (IsMouseButtonDown(MOUSE_BUTTON_LEFT) || IsKeyDown(KEY_UP));
        bool p2InputReleased = (!editorMode) && (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) || IsKeyReleased(KEY_UP));

        if (testScreenshotMode) {
            if (testFrameCount >= 20 && testFrameCount < 52) {
                p2InputDown = true;
            }
        }

        gasP2.Update(dt, p2InputDown, runTime);'''

c = re.sub(p2_ai_pattern, p2_manual_code, c, count=1)

# 5. Flume Hard Wall Collision and Abyss Fall
bound_pattern = r'// River Channel Bank Containment[\s\S]*?// Dynamic Wave Buoyancy & Raft Rocking \(42\.0 spring, 7\.5 damping\)[\s\S]*?raft\.velocity\.y \+= \(heave \* 42\.0f - raft\.velocity\.y \* 7\.5f\) \* dt;'
bound_new = '''// Flume Canal Hard Collision vs Open River Containment
        bool inFlumeCanal = (raft.position.z > 36.0f);
        float currentHalfWidth = inFlumeCanal ? 1.42f : 6.8f;

        if (inFlumeCanal) {
            // Hard canal wall collision: solid barrier cannot be penetrated!
            if (fabsf(crossDist) > currentHalfWidth) {
                float pushSign = (crossDist > 0.0f) ? 1.0f : -1.0f;
                raft.position = Vector3Add(riverCenter, Vector3Scale(riverNormal, pushSign * currentHalfWidth));
                Vector3 wallNormal = Vector3Scale(riverNormal, -pushSign);
                float normalVel = Vector3DotProduct(raft.velocity, wallNormal);
                if (normalVel < 0.0f) {
                    raft.velocity = Vector3Subtract(raft.velocity, Vector3Scale(wallNormal, 1.6f * normalVel));
                    raft.velocity = Vector3Scale(raft.velocity, 0.70f);
                    PlaySound(fartSounds[2]);
                    camCtrl.shakeTrauma = fmaxf(camCtrl.shakeTrauma, 0.25f);
                }
            }
        } else {
            // Open river bank soft containment
            if (fabsf(crossDist) > currentHalfWidth) {
                float pen = fabsf(crossDist) - currentHalfWidth;
                float pushSign = (crossDist > 0.0f) ? -1.0f : 1.0f;
                raft.velocity = Vector3Add(raft.velocity, Vector3Scale(riverNormal, pushSign * pen * 36.0f * dt));
            }
        }

        // Dynamic Wave Buoyancy vs Fall Off Cliff Into Abyss
        bool overWater = (fabsf(crossDist) < (inFlumeCanal ? 2.5f : 9.5f));
        if (overWater && !raft.isDestroyed) {
            float waterHeight = riverCenter.y - 0.31f + sinf(raft.position.z * 0.32f - runTime * 1.8f) * 0.05f;
            float heave = waterHeight - raft.position.y;
            raft.velocity.y += (heave * 42.0f - raft.velocity.y * 7.5f) * dt;
        } else {
            // Outside river channel: FALL INTO ABYSS!
            raft.velocity.y -= 32.0f * dt;
            if (raft.position.y < -12.0f && !raft.isDestroyed) {
                totalDeaths++;
                TriggerBoatDeath(DEATH_OBSTACLE);
            }
        }'''

c = re.sub(bound_pattern, bound_new, c, count=1)

with open("CustomEngine/native/GameEngine.cpp", "w", encoding="utf-8") as f:
    f.write(c)

print("Applied normalize_and_patch successfully!")
