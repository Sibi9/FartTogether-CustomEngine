with open('CustomEngine/native/GameEngine.cpp', 'r', encoding='utf-8') as f:
    code = f.read()

# 1. Update EmitFartBurst
old_emit = """static void EmitFartBurst(Vector3 pos, Vector3 thrustDir, float chargePercent, bool isSuperSync = false) {
    int count = 12 + (int)(chargePercent * 24.0f);
    if (isSuperSync) count *= 2;
    for (int i = 0; i < count; i++) {
        for (int p = 0; p < MAX_PARTICLES; p++) {
            if (!particles[p].active) {
                particles[p].active = true;
                particles[p].position = pos + Vector3{
                    ((float)rand() / RAND_MAX - 0.5f) * 0.7f,
                    ((float)rand() / RAND_MAX - 0.5f) * 0.5f,
                    ((float)rand() / RAND_MAX - 0.5f) * 0.7f
                };

                Vector3 spread = Vector3{
                    ((float)rand() / RAND_MAX - 0.5f) * 2.2f,
                    ((float)rand() / RAND_MAX) * 1.6f,
                    ((float)rand() / RAND_MAX - 0.5f) * 2.2f
                };
                particles[p].velocity = Vector3{ spread.x * 0.7f, 1.1f + spread.y * 0.6f, spread.z * 0.7f + 1.2f };
                particles[p].size = 0.55f + (chargePercent * 0.45f) + ((float)rand() / RAND_MAX) * 0.25f;
                if (isSuperSync) particles[p].size *= 1.25f;
                particles[p].alpha = 1.0f;
                particles[p].rotation = ((float)rand() / RAND_MAX) * 360.0f;
                particles[p].rotSpeed = ((float)rand() / RAND_MAX - 0.5f) * 110.0f;
                particles[p].life = 0.0f;
                particles[p].maxLife = 1.2f + chargePercent * 0.8f;
                particles[p].spriteIndex = rand() % 4;
                break;
            }
        }
    }
}"""

new_emit = """static void EmitFartBurst(Vector3 pos, Vector3 thrustDir, float chargePercent, bool isSuperSync = false) {
    int count = 10 + (int)(chargePercent * 16.0f);
    if (isSuperSync) count *= 2;
    for (int i = 0; i < count; i++) {
        for (int p = 0; p < MAX_PARTICLES; p++) {
            if (!particles[p].active) {
                particles[p].active = true;
                particles[p].position = pos + Vector3{
                    ((float)rand() / RAND_MAX - 0.5f) * 0.35f,
                    0.20f + ((float)rand() / RAND_MAX) * 0.25f,
                    ((float)rand() / RAND_MAX - 0.5f) * 0.35f
                };

                Vector3 spread = Vector3{
                    ((float)rand() / RAND_MAX - 0.5f) * 1.4f,
                    1.2f + ((float)rand() / RAND_MAX) * 1.5f,
                    ((float)rand() / RAND_MAX - 0.5f) * 1.4f
                };
                Vector3 hDir = Vector3Normalize(Vector3{ thrustDir.x + spread.x * 0.35f, 0.0f, thrustDir.z + spread.z * 0.35f });
                float blastSpeed = 2.5f + chargePercent * 4.0f;
                particles[p].velocity = Vector3{ hDir.x * blastSpeed, spread.y, hDir.z * blastSpeed };
                particles[p].size = 0.45f + (chargePercent * 0.35f) + ((float)rand() / RAND_MAX) * 0.20f;
                if (isSuperSync) particles[p].size *= 1.25f;
                particles[p].alpha = 0.95f;
                particles[p].rotation = ((float)rand() / RAND_MAX) * 360.0f;
                particles[p].rotSpeed = ((float)rand() / RAND_MAX - 0.5f) * 90.0f;
                particles[p].life = 0.0f;
                particles[p].maxLife = 1.0f + chargePercent * 0.6f;
                particles[p].spriteIndex = rand() % 4;
                break;
            }
        }
    }
}"""

assert old_emit in code, 'old_emit not found'
code = code.replace(old_emit, new_emit)
print('Replaced EmitFartBurst')

# 2. Add texRaftFoam
old_tex_load = """    Texture2D texFart[4];
    texFart[0] = LoadTexture(ResolveAsset("extracted_assets/textures/Fart1.png").c_str());
    texFart[1] = LoadTexture(ResolveAsset("extracted_assets/textures/Fart4.png").c_str());
    texFart[2] = LoadTexture(ResolveAsset("extracted_assets/textures/Fart7.png").c_str());
    texFart[3] = LoadTexture(ResolveAsset("extracted_assets/textures/Fart8.png").c_str());"""

new_tex_load = """    Texture2D texFart[4];
    texFart[0] = LoadTexture(ResolveAsset("extracted_assets/textures/Fart1.png").c_str());
    texFart[1] = LoadTexture(ResolveAsset("extracted_assets/textures/Fart4.png").c_str());
    texFart[2] = LoadTexture(ResolveAsset("extracted_assets/textures/Fart7.png").c_str());
    texFart[3] = LoadTexture(ResolveAsset("extracted_assets/textures/Fart8.png").c_str());
    Texture2D texRaftFoam = LoadTexture(ResolveAsset("extracted_assets/textures/Raft-Foam4.png").c_str());"""

assert old_tex_load in code, 'old_tex_load not found'
code = code.replace(old_tex_load, new_tex_load)
print('Added texRaftFoam')

# 3. Model loading with SeatedPant and SeatedShoes
old_models = """    Model boatModel    = LoadModel(ResolveAsset("extracted_assets/meshes/GM_3D_BoatMain_ThreeP.obj").c_str());
    Model charA        = LoadModel(ResolveAsset("extracted_assets/meshes/GM_3D_Character_SeatedA.obj").c_str());
    Model charB        = LoadModel(ResolveAsset("extracted_assets/meshes/GM_3D_Character_SeatedB.obj").c_str());
    Model hat1         = LoadModel(ResolveAsset("extracted_assets/meshes/GM_3D_Character_Hat.obj").c_str());
    Model shirt        = LoadModel(ResolveAsset("extracted_assets/meshes/GM_3D_Character_SeatedShirt.obj").c_str());
    Model pant         = LoadModel(ResolveAsset("extracted_assets/meshes/GM_3D_Character_Pant.obj").c_str());"""

new_models = """    Model boatModel    = LoadModel(ResolveAsset("extracted_assets/meshes/GM_3D_BoatMain_ThreeP.obj").c_str());
    Model charA        = LoadModel(ResolveAsset("extracted_assets/meshes/GM_3D_Character_SeatedA.obj").c_str());
    Model charB        = LoadModel(ResolveAsset("extracted_assets/meshes/GM_3D_Character_SeatedB.obj").c_str());
    Model hat1         = LoadModel(ResolveAsset("extracted_assets/meshes/GM_3D_Character_Hat.obj").c_str());
    Model shirt        = LoadModel(ResolveAsset("extracted_assets/meshes/GM_3D_Character_SeatedShirt.obj").c_str());
    Model pant         = LoadModel(ResolveAsset("extracted_assets/meshes/GM_3D_Character_SeatedPant.obj").c_str());
    Model shoes        = LoadModel(ResolveAsset("extracted_assets/meshes/GM_3D_Character_SeatedShoes.obj").c_str());"""

assert old_models in code, 'old_models not found'
code = code.replace(old_models, new_models)
print('Added shoes and seated pant')

# 4. Material assignment with Gradient Palette for all character parts
old_mats = """    ApplyMat(boatModel);
    boatModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texGradientPalette;
    ApplyMat(charA);
    ApplyMat(charB);
    ApplyMat(hat1);
    ApplyMat(shirt);
    ApplyMat(pant);"""

new_mats = """    auto ApplyCharMat = [&](Model& m) {
        for (int i = 0; i < m.materialCount; i++) {
            m.materials[i].maps[MATERIAL_MAP_DIFFUSE].texture = texGradientPalette;
            m.materials[i].shader = shaderLighting;
        }
    };
    ApplyCharMat(boatModel);
    ApplyCharMat(charA);
    ApplyCharMat(charB);
    ApplyCharMat(hat1);
    ApplyCharMat(shirt);
    ApplyCharMat(pant);
    ApplyCharMat(shoes);"""

assert old_mats in code, 'old_mats not found'
code = code.replace(old_mats, new_mats)
print('Assigned Gradient Palette to character parts')

# 5. Pirate seating drawing on boat rim
old_pirate_draw = """                    // Left Pirate (Player 1) seated facing inward on the dinghy rim
                    float p1Redness = gasP1.redness;
                    SetShaderValue(shaderLighting, uRednessLoc, &p1Redness, SHADER_UNIFORM_FLOAT);

                    rlPushMatrix();
                        // Position on circular rim at rimRadius = 1.22m, resting on top of tube (Y = 0.44m)
                        float p1X = sinf(p1RimAngle) * 1.22f;
                        float p1Z = cosf(p1RimAngle) * 1.22f;
                        rlTranslatef(p1X, 0.38f + gasP1.trembleOffset, p1Z);
                        // Rotate inward towards center of boat
                        float p1FaceYaw = (p1RimAngle * RAD2DEG) + 180.0f;
                        rlRotatef(p1FaceYaw, 0, 1, 0);

                        // Authentic scale matches boat proportions (1.25x * swell)
                        float p1Scale = 1.25f * gasP1.boneScale;
                        rlScalef(p1Scale, p1Scale, p1Scale);
                        DrawModel(charA, Vector3{ 0, 0, 0 }, 1.0f, WHITE);
                        DrawModel(shirt, Vector3{ 0, 0, 0 }, 1.0f, WHITE);
                        DrawModel(pant,  Vector3{ 0, 0, 0 }, 1.0f, WHITE);
                        DrawModel(hat1,  Vector3{ 0, 0, 0 }, 1.0f, WHITE);
                    rlPopMatrix();

                    // Right Pirate (Player 2 AI Co-pilot) seated facing inward on the dinghy rim
                    float p2Redness = gasP2.redness;
                    SetShaderValue(shaderLighting, uRednessLoc, &p2Redness, SHADER_UNIFORM_FLOAT);

                    rlPushMatrix();
                        float p2X = sinf(p2RimAngle) * 1.22f;
                        float p2Z = cosf(p2RimAngle) * 1.22f;
                        rlTranslatef(p2X, 0.38f + gasP2.trembleOffset, p2Z);
                        float p2FaceYaw = (p2RimAngle * RAD2DEG) + 180.0f;
                        rlRotatef(p2FaceYaw, 0, 1, 0);

                        float p2Scale = 1.25f * gasP2.boneScale;
                        rlScalef(p2Scale, p2Scale, p2Scale);
                        DrawModel(charB, Vector3{ 0, 0, 0 }, 1.0f, WHITE);
                        DrawModel(shirt, Vector3{ 0, 0, 0 }, 1.0f, WHITE);
                        DrawModel(pant,  Vector3{ 0, 0, 0 }, 1.0f, WHITE);
                        DrawModel(hat1,  Vector3{ 0, 0, 0 }, 1.0f, WHITE);
                    rlPopMatrix();"""

new_pirate_draw = """                    // Left Pirate (Player 1) seated facing inward on the dinghy rim
                    float p1Redness = gasP1.redness;
                    SetShaderValue(shaderLighting, uRednessLoc, &p1Redness, SHADER_UNIFORM_FLOAT);

                    rlPushMatrix();
                        // 1:1 Parity: Radius = 1.05m on the tube crown, Y = 0.26m resting firmly on the tube
                        float p1X = sinf(p1RimAngle) * 1.05f;
                        float p1Z = cosf(p1RimAngle) * 1.05f;
                        rlTranslatef(p1X, 0.26f + gasP1.trembleOffset, p1Z);
                        float p1FaceYaw = (p1RimAngle * RAD2DEG) + 180.0f;
                        rlRotatef(p1FaceYaw, 0, 1, 0);

                        DrawModel(charA, Vector3{ 0, 0, 0 }, 1.0f, WHITE);
                        DrawModel(shirt, Vector3{ 0, 0, 0 }, 1.0f, WHITE);
                        DrawModel(pant,  Vector3{ 0, 0, 0 }, 1.0f, WHITE);
                        DrawModel(shoes, Vector3{ 0, 0, 0 }, 1.0f, WHITE);
                        DrawModel(hat1,  Vector3{ 0, 0, 0 }, 1.0f, WHITE);
                    rlPopMatrix();

                    // Right Pirate (Player 2 AI Co-pilot) seated facing inward on the dinghy rim
                    float p2Redness = gasP2.redness;
                    SetShaderValue(shaderLighting, uRednessLoc, &p2Redness, SHADER_UNIFORM_FLOAT);

                    rlPushMatrix();
                        float p2X = sinf(p2RimAngle) * 1.05f;
                        float p2Z = cosf(p2RimAngle) * 1.05f;
                        rlTranslatef(p2X, 0.26f + gasP2.trembleOffset, p2Z);
                        float p2FaceYaw = (p2RimAngle * RAD2DEG) + 180.0f;
                        rlRotatef(p2FaceYaw, 0, 1, 0);

                        DrawModel(charB, Vector3{ 0, 0, 0 }, 1.0f, WHITE);
                        DrawModel(shirt, Vector3{ 0, 0, 0 }, 1.0f, WHITE);
                        DrawModel(pant,  Vector3{ 0, 0, 0 }, 1.0f, WHITE);
                        DrawModel(shoes, Vector3{ 0, 0, 0 }, 1.0f, WHITE);
                        DrawModel(hat1,  Vector3{ 0, 0, 0 }, 1.0f, WHITE);
                    rlPopMatrix();"""

assert old_pirate_draw in code, 'old_pirate_draw not found'
code = code.replace(old_pirate_draw, new_pirate_draw)
print('Updated pirate drawing')

# 6. Raft foam wake and particle updates
old_particles_render = """            // I. Cartoon Billboard Fart Particles (Proper OpenGL Alpha Blending & Depth Mask Disable)
            rlDisableDepthMask();
            BeginBlendMode(BLEND_ALPHA);
            for (int p = 0; p < MAX_PARTICLES; p++) {
                if (particles[p].active) {
                    Texture2D tex = texFart[particles[p].spriteIndex];
                    Rectangle srcRec = Rectangle{ 0, 0, (float)tex.width, (float)tex.height };
                    Color pTint = Color{ 255, 255, 255, (unsigned char)(particles[p].alpha * 220) };
                    DrawBillboardPro(camera, tex, srcRec, particles[p].position, Vector3{ 0, 1, 0 },
                                     Vector2{ particles[p].size, particles[p].size * 0.65f },
                                     Vector2{ particles[p].size * 0.5f, particles[p].size * 0.32f },
                                     particles[p].rotation, pTint);
                }
            }
            EndBlendMode();
            rlEnableDepthMask();"""

new_particles_render = """            // Authentic Raft Foam Wake on Water Surface
            rlDisableDepthMask();
            BeginBlendMode(BLEND_ALPHA);
            rlPushMatrix();
                rlTranslatef(raft.position.x, raft.position.y + 0.02f, raft.position.z);
                rlRotatef(raft.rotationY * RAD2DEG, 0, 1, 0);
                float foamRad = 1.65f + (raft.speed / 14.0f) * 0.35f;
                Color foamTint = Color{ 255, 255, 255, 200 };
                rlSetTexture(texRaftFoam.id);
                rlBegin(RL_QUADS);
                    rlColor4ub(foamTint.r, foamTint.g, foamTint.b, foamTint.a);
                    rlTexCoord2f(0.0f, 0.0f); rlVertex3f(-foamRad, 0.0f, -foamRad);
                    rlTexCoord2f(1.0f, 0.0f); rlVertex3f( foamRad, 0.0f, -foamRad);
                    rlTexCoord2f(1.0f, 1.0f); rlVertex3f( foamRad, 0.0f,  foamRad);
                    rlTexCoord2f(0.0f, 1.0f); rlVertex3f(-foamRad, 0.0f,  foamRad);
                rlEnd();
                rlSetTexture(0);
            rlPopMatrix();

            // I. Cartoon Billboard Fart Particles (Proper OpenGL Alpha Blending & Depth Mask Disable)
            for (int p = 0; p < MAX_PARTICLES; p++) {
                if (particles[p].active) {
                    Texture2D tex = texFart[particles[p].spriteIndex];
                    Rectangle srcRec = Rectangle{ 0, 0, (float)tex.width, (float)tex.height };
                    Color pTint = Color{ 255, 255, 255, (unsigned char)(particles[p].alpha * 220) };
                    DrawBillboardPro(camera, tex, srcRec, particles[p].position, Vector3{ 0, 1, 0 },
                                     Vector2{ particles[p].size, particles[p].size * 0.65f },
                                     Vector2{ particles[p].size * 0.5f, particles[p].size * 0.32f },
                                     particles[p].rotation, pTint);
                }
            }
            EndBlendMode();
            rlEnableDepthMask();"""

assert old_particles_render in code, 'old_particles_render not found'
code = code.replace(old_particles_render, new_particles_render)
print('Added Raft Foam Wake')

# 7. Unload texRaftFoam and shoes
old_cleanup = """    UnloadModel(pant);
    UnloadModel(shirt);
    UnloadModel(hat1);
    UnloadModel(charB);
    UnloadModel(charA);"""

new_cleanup = """    UnloadTexture(texRaftFoam);
    UnloadModel(shoes);
    UnloadModel(pant);
    UnloadModel(shirt);
    UnloadModel(hat1);
    UnloadModel(charB);
    UnloadModel(charA);"""

assert old_cleanup in code, 'old_cleanup not found'
code = code.replace(old_cleanup, new_cleanup)
print('Updated cleanup')

with open('CustomEngine/native/GameEngine.cpp', 'w', encoding='utf-8') as f:
    f.write(code)
print('Successfully wrote CustomEngine/native/GameEngine.cpp')

