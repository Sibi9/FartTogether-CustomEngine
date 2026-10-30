with open(r"CustomEngine\native\GameEngine.cpp", "r", encoding="utf-8") as f:
    code = f.read()

# 1. Fix ApplyMat to iterate over all materials
old_applymat = """    auto ApplyMat = [&](Model& m) {
        if (m.meshCount > 0) {
            m.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texPalette;
            m.materials[0].shader = shaderLighting;
        }
    };"""

new_applymat = """    auto ApplyMat = [&](Model& m) {
        for (int i = 0; i < m.materialCount; i++) {
            m.materials[i].maps[MATERIAL_MAP_DIFFUSE].texture = texPalette;
            m.materials[i].shader = shaderLighting;
        }
    };"""

code = code.replace(old_applymat, new_applymat)

# 2. Fix pirate Y position and scale
old_pirate_draw = """                    // Left Pirate (Player 1) seated facing inward on the dinghy rim
                    float p1Redness = gasP1.redness;
                    SetShaderValue(shaderLighting, uRednessLoc, &p1Redness, SHADER_UNIFORM_FLOAT);

                    rlPushMatrix();
                        // Position on circular rim at rimRadius = 1.25m
                        float p1X = sinf(p1RimAngle) * 1.25f;
                        float p1Z = cosf(p1RimAngle) * 1.25f;
                        rlTranslatef(p1X, 0.22f + gasP1.trembleOffset, p1Z);
                        // Rotate inward towards center of boat
                        float p1FaceYaw = (p1RimAngle * RAD2DEG) + 180.0f;
                        rlRotatef(p1FaceYaw, 0, 1, 0);

                        // Authentic Bone Swell scale (1.0x to 1.4x)
                        rlScalef(1.10f * gasP1.boneScale, 1.10f * gasP1.boneScale, 1.10f * gasP1.boneScale);
                        DrawModel(charA, Vector3{ 0, 0, 0 }, 1.0f, WHITE);
                        DrawModel(shirt, Vector3{ 0, 0, 0 }, 1.0f, WHITE);
                        DrawModel(pant,  Vector3{ 0, 0, 0 }, 1.0f, WHITE);
                        DrawModel(hat1,  Vector3{ 0, 0, 0 }, 1.0f, WHITE);
                    rlPopMatrix();

                    // Right Pirate (Player 2 AI Co-pilot) seated facing inward on the dinghy rim
                    float p2Redness = gasP2.redness;
                    SetShaderValue(shaderLighting, uRednessLoc, &p2Redness, SHADER_UNIFORM_FLOAT);

                    rlPushMatrix();
                        float p2X = sinf(p2RimAngle) * 1.25f;
                        float p2Z = cosf(p2RimAngle) * 1.25f;
                        rlTranslatef(p2X, 0.22f + gasP2.trembleOffset, p2Z);
                        float p2FaceYaw = (p2RimAngle * RAD2DEG) + 180.0f;
                        rlRotatef(p2FaceYaw, 0, 1, 0);

                        rlScalef(1.10f * gasP2.boneScale, 1.10f * gasP2.boneScale, 1.10f * gasP2.boneScale);
                        DrawModel(charB, Vector3{ 0, 0, 0 }, 1.0f, WHITE);
                        DrawModel(shirt, Vector3{ 0, 0, 0 }, 1.0f, WHITE);
                        DrawModel(pant,  Vector3{ 0, 0, 0 }, 1.0f, WHITE);
                    rlPopMatrix();"""

new_pirate_draw = """                    // Left Pirate (Player 1) seated facing inward on the dinghy rim
                    float p1Redness = gasP1.redness;
                    SetShaderValue(shaderLighting, uRednessLoc, &p1Redness, SHADER_UNIFORM_FLOAT);

                    rlPushMatrix();
                        // Position on circular rim at rimRadius = 1.22m, resting on top of tube (Y = 0.44m)
                        float p1X = sinf(p1RimAngle) * 1.22f;
                        float p1Z = cosf(p1RimAngle) * 1.22f;
                        rlTranslatef(p1X, 0.44f + gasP1.trembleOffset, p1Z);
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
                        rlTranslatef(p2X, 0.44f + gasP2.trembleOffset, p2Z);
                        float p2FaceYaw = (p2RimAngle * RAD2DEG) + 180.0f;
                        rlRotatef(p2FaceYaw, 0, 1, 0);

                        float p2Scale = 1.25f * gasP2.boneScale;
                        rlScalef(p2Scale, p2Scale, p2Scale);
                        DrawModel(charB, Vector3{ 0, 0, 0 }, 1.0f, WHITE);
                        DrawModel(shirt, Vector3{ 0, 0, 0 }, 1.0f, WHITE);
                        DrawModel(pant,  Vector3{ 0, 0, 0 }, 1.0f, WHITE);
                        DrawModel(hat1,  Vector3{ 0, 0, 0 }, 1.0f, WHITE);
                    rlPopMatrix();"""

code = code.replace(old_pirate_draw, new_pirate_draw)

# 3. Brighten ambient lighting slightly so character colors pop
code = code.replace(
    "Vector3 aCol = Vector3{ 0.48f, 0.52f, 0.62f };",
    "Vector3 aCol = Vector3{ 0.58f, 0.62f, 0.72f };"
)
code = code.replace(
    "Vector3 lCol = Vector3{ 1.0f, 0.98f, 0.92f };",
    "Vector3 lCol = Vector3{ 1.15f, 1.10f, 1.02f };"
)

with open(r"CustomEngine\native\GameEngine.cpp", "w", encoding="utf-8") as f:
    f.write(code)

print("Updated GameEngine.cpp with material and positioning polish")

