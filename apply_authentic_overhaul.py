import re

with open(r"CustomEngine\native\GameEngine.cpp", "r", encoding="utf-8") as f:
    code = f.read()

# 1. Use seated models
code = code.replace(
    'Model charA        = LoadModel(ResolveAsset("extracted_assets/meshes/GM_3D_Character_MainA.obj").c_str());',
    'Model charA        = LoadModel(ResolveAsset("extracted_assets/meshes/GM_3D_Character_SeatedA.obj").c_str());'
)
code = code.replace(
    'Model charB        = LoadModel(ResolveAsset("extracted_assets/meshes/GM_3D_Character_MainB.obj").c_str());',
    'Model charB        = LoadModel(ResolveAsset("extracted_assets/meshes/GM_3D_Character_SeatedB.obj").c_str());'
)
code = code.replace(
    'Model shirt        = LoadModel(ResolveAsset("extracted_assets/meshes/GM_3D_Character_Shirt.obj").c_str());',
    'Model shirt        = LoadModel(ResolveAsset("extracted_assets/meshes/GM_3D_Character_SeatedShirt.obj").c_str());'
)

# 2. Add rim angles for P1 and P2 in variables
old_vars = """    // Authentic Gas States (P1 & P2)
    PirateGasState gasP1;
    gasP1.Reset();
    PirateGasState gasP2;
    gasP2.Reset();"""

new_vars = """    // Authentic Gas States & Rim Seating (P1 & P2)
    PirateGasState gasP1;
    gasP1.Reset();
    PirateGasState gasP2;
    gasP2.Reset();

    // Authentic Rim Seating Angles (in radians)
    // Left Pirate sits on left-rear rim (-120 deg)
    float p1RimAngle = -2.094f; // -120 deg
    // Right Pirate sits on right-rear rim (+120 deg)
    float p2RimAngle = 2.094f;  // +120 deg"""

code = code.replace(old_vars, new_vars)

# 3. Update Crocodile & Shark starting positions so they are visible right near start gate
old_crocs = """    std::vector<CrocodileAI> crocodiles = {
        { Vector3{ 18.2f, 0.35f, 48.6f }, 3.14f, Vector3{ 18.2f, 0.35f, 48.6f }, 0.0f, false, CROC_SPEED },
        { Vector3{ -14.5f, 5.20f, -65.2f }, 2.40f, Vector3{ -14.5f, 5.20f, -65.2f }, 0.0f, false, CROC_SPEED },
        { Vector3{ -150.2f, 16.8f, -88.4f }, 1.57f, Vector3{ -150.2f, 16.8f, -88.4f }, 0.0f, false, CROC_SPEED }
    };

    std::vector<SharkAI> sharks = {
        { Vector3{ 32.5f, 0.25f, 2.8f }, 0.0f, Vector3{ 32.5f, 0.25f, 2.8f }, 0.0f, 0.0f, false, SHARK_PATROL_SPEED },
        { Vector3{ -95.0f, 14.1f, -103.0f }, 1.57f, Vector3{ -95.0f, 14.1f, -103.0f }, 1.57f, 0.0f, false, SHARK_PATROL_SPEED },
        { Vector3{ -275.0f, 14.5f, -103.0f }, 1.57f, Vector3{ -275.0f, 14.5f, -103.0f }, 3.14f, 0.0f, false, SHARK_PATROL_SPEED }
    };"""

new_crocs = """    std::vector<CrocodileAI> crocodiles = {
        { Vector3{ 17.5f, 0.38f, 68.0f }, 3.14f, Vector3{ 17.5f, 0.38f, 68.0f }, 0.0f, false, CROC_SPEED },
        { Vector3{ 24.0f, 0.38f, 50.0f }, 2.80f, Vector3{ 24.0f, 0.38f, 50.0f }, 0.0f, false, CROC_SPEED },
        { Vector3{ -14.5f, 5.20f, -65.2f }, 2.40f, Vector3{ -14.5f, 5.20f, -65.2f }, 0.0f, false, CROC_SPEED }
    };

    std::vector<SharkAI> sharks = {
        { Vector3{ 22.0f, 0.28f, 56.0f }, 0.0f, Vector3{ 22.0f, 0.28f, 56.0f }, 0.0f, 0.0f, false, SHARK_PATROL_SPEED },
        { Vector3{ 32.5f, 0.25f, 2.8f }, 0.0f, Vector3{ 32.5f, 0.25f, 2.8f }, 0.0f, 0.0f, false, SHARK_PATROL_SPEED },
        { Vector3{ -95.0f, 14.1f, -103.0f }, 1.57f, Vector3{ -95.0f, 14.1f, -103.0f }, 1.57f, 0.0f, false, SHARK_PATROL_SPEED }
    };"""

code = code.replace(old_crocs, new_crocs)

# 4. Update Controls: A/D slides P1 along the circular rim
old_p1_input = """        // P1 Controls: Left Pirate (A key or Left Arrow or Left Click)
        if (!raft.isDestroyed) {
            p1InputDown = IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_W) || IsKeyDown(KEY_SPACE);
            p1InputReleased = IsKeyReleased(KEY_A) || IsKeyReleased(KEY_LEFT) || IsKeyReleased(KEY_W) || IsKeyReleased(KEY_SPACE);
        }"""

new_p1_input = """        // P1 Controls: A/D slides pirate along the dinghy rim, W/SPACE/UP unleashes gas
        if (!raft.isDestroyed) {
            float rimMove = 0.0f;
            if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))  rimMove -= 1.0f;
            if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) rimMove += 1.0f;
            p1RimAngle += rimMove * 2.44f * dt; // 140 deg/s angular slide speed
            p1RimAngle = Clamp(p1RimAngle, -2.85f, -0.65f);

            p1InputDown = IsKeyDown(KEY_W) || IsKeyDown(KEY_SPACE) || IsKeyDown(KEY_UP);
            p1InputReleased = IsKeyReleased(KEY_W) || IsKeyReleased(KEY_SPACE) || IsKeyReleased(KEY_UP);
        }"""

code = code.replace(old_p1_input, new_p1_input)

# 5. Update Fart thrust vector based on pirate's position on the circular dinghy
old_tap = """        // Tap Burst Detection for P1
        if (p1InputReleased && gasP1.pressTimer <= GAS_TAP_THRESHOLD_SECONDS && gasP1.pressTimer > 0.01f && !gasP1.gasDepleted) {
            // TAP BURST! Instant 900N force, 10 cost
            if (gasP1.gasEnergy >= GAS_TAP_COST) {
                gasP1.gasEnergy -= GAS_TAP_COST;
                float tapPower = (GAS_TAP_BURST_FORCE / BOAT_MASS) * 0.35f;

                // Forward thrust + Right Turn Yaw Torque (r_x = -0.75m -> Torque > 0 -> Turn Right)
                Vector3 fwdDir = Vector3{ sinf(raft.rotationY), 0.0f, cosf(raft.rotationY) };
                raft.velocity = Vector3Add(raft.velocity, Vector3Scale(fwdDir, tapPower));
                raft.angularVelocityY += BOAT_STEER_TORQUE * 0.30f;

                Vector3 fartPos = raft.position + Vector3{ -0.75f, 0.40f, 0.8f };
                EmitFartBurst(fartPos, Vector3{ 0,0,-1 }, 0.4f);
                PlaySound(fartSounds[0]);
            }
        }

        gasP1.Update(dt, p1InputDown, runTime);

        // Sustained Thrust Application for P1
        if (gasP1.isSustained && !raft.isDestroyed) {
            float curForce = GAS_MIN_FORCE + (GAS_NORMAL_FORCE - GAS_MIN_FORCE) * gasP1.sustainedCharge;
            float accel = (curForce / BOAT_MASS) * dt * 0.75f;

            Vector3 fwdDir = Vector3{ sinf(raft.rotationY), 0.0f, cosf(raft.rotationY) };
            raft.velocity = Vector3Add(raft.velocity, Vector3Scale(fwdDir, accel));

            // Differential Steering Torque: P1 on left turns raft right!
            raft.angularVelocityY += (BOAT_STEER_TORQUE / BOAT_MASS) * (curForce / GAS_NORMAL_FORCE) * dt * 16.0f;

            if (rand() % 4 == 0) {
                Vector3 fartPos = raft.position + Vector3{ -0.75f, 0.40f, 0.8f };
                EmitFartBurst(fartPos, Vector3{ 0,0,-1 }, gasP1.sustainedCharge);
            }
        }"""

new_tap = """        // Tap Burst Detection for P1 (Gas shoots OUTWARD from pirate butt, pushes boat INWARD)
        float rimRadius = 1.25f;
        Vector3 p1OutwardLocal = Vector3{ sinf(p1RimAngle), 0.0f, cosf(p1RimAngle) };
        Vector3 p1ThrustLocal  = Vector3Scale(p1OutwardLocal, -1.0f); // Inward push!

        Matrix boatRotM = MatrixRotateY(raft.rotationY);
        Vector3 p1ThrustWorld = Vector3Transform(p1ThrustLocal, boatRotM);
        Vector3 p1OutwardWorld = Vector3Transform(p1OutwardLocal, boatRotM);
        Vector3 p1PosWorld = Vector3Add(raft.position, Vector3Transform(Vector3Scale(p1OutwardLocal, rimRadius), boatRotM));

        if (p1InputReleased && gasP1.pressTimer <= GAS_TAP_THRESHOLD_SECONDS && gasP1.pressTimer > 0.01f && !gasP1.gasDepleted) {
            if (gasP1.gasEnergy >= GAS_TAP_COST) {
                gasP1.gasEnergy -= GAS_TAP_COST;
                float tapPower = (GAS_TAP_BURST_FORCE / BOAT_MASS) * 0.35f;

                raft.velocity = Vector3Add(raft.velocity, Vector3Scale(p1ThrustWorld, tapPower));
                // Torque = r_x * F_z - r_z * F_x
                float steerTq = (p1OutwardLocal.x * p1ThrustLocal.z - p1OutwardLocal.z * p1ThrustLocal.x);
                raft.angularVelocityY += steerTq * (BOAT_STEER_TORQUE / BOAT_MASS) * 8.0f;

                EmitFartBurst(p1PosWorld, p1OutwardWorld, 0.4f);
                PlaySound(fartSounds[0]);
            }
        }

        gasP1.Update(dt, p1InputDown, runTime);

        // Sustained Thrust Application for P1
        if (gasP1.isSustained && !raft.isDestroyed) {
            float curForce = GAS_MIN_FORCE + (GAS_NORMAL_FORCE - GAS_MIN_FORCE) * gasP1.sustainedCharge;
            float accel = (curForce / BOAT_MASS) * dt * 0.85f;

            raft.velocity = Vector3Add(raft.velocity, Vector3Scale(p1ThrustWorld, accel));
            float steerTq = (p1OutwardLocal.x * p1ThrustLocal.z - p1OutwardLocal.z * p1ThrustLocal.x);
            raft.angularVelocityY += steerTq * (BOAT_STEER_TORQUE / BOAT_MASS) * (curForce / GAS_NORMAL_FORCE) * dt * 22.0f;

            if (rand() % 4 == 0) {
                EmitFartBurst(p1PosWorld, p1OutwardWorld, gasP1.sustainedCharge);
            }
        }"""

code = code.replace(old_tap, new_tap)

# 6. Update P2 thrust with authentic outward direction
old_p2_thrust = """        // Sustained Thrust Application for P2
        if (gasP2.isSustained && !raft.isDestroyed) {
            float curForce = GAS_MIN_FORCE + (GAS_NORMAL_FORCE - GAS_MIN_FORCE) * gasP2.sustainedCharge;
            float accel = (curForce / BOAT_MASS) * dt * 0.75f;

            Vector3 fwdDir = Vector3{ sinf(raft.rotationY), 0.0f, cosf(raft.rotationY) };
            raft.velocity = Vector3Add(raft.velocity, Vector3Scale(fwdDir, accel));

            // Differential Steering Torque: P2 on right turns raft left!
            raft.angularVelocityY -= (BOAT_STEER_TORQUE / BOAT_MASS) * (curForce / GAS_NORMAL_FORCE) * dt * 16.0f;

            if (rand() % 4 == 0) {
                Vector3 fartPos = raft.position + Vector3{ 0.75f, 0.40f, 0.8f };
                EmitFartBurst(fartPos, Vector3{ 0,0,-1 }, gasP2.sustainedCharge);
            }
        }"""

new_p2_thrust = """        // Sustained Thrust Application for P2
        Vector3 p2OutwardLocal = Vector3{ sinf(p2RimAngle), 0.0f, cosf(p2RimAngle) };
        Vector3 p2ThrustLocal  = Vector3Scale(p2OutwardLocal, -1.0f);
        Vector3 p2ThrustWorld = Vector3Transform(p2ThrustLocal, boatRotM);
        Vector3 p2OutwardWorld = Vector3Transform(p2OutwardLocal, boatRotM);
        Vector3 p2PosWorld = Vector3Add(raft.position, Vector3Transform(Vector3Scale(p2OutwardLocal, rimRadius), boatRotM));

        if (gasP2.isSustained && !raft.isDestroyed) {
            float curForce = GAS_MIN_FORCE + (GAS_NORMAL_FORCE - GAS_MIN_FORCE) * gasP2.sustainedCharge;
            float accel = (curForce / BOAT_MASS) * dt * 0.85f;

            raft.velocity = Vector3Add(raft.velocity, Vector3Scale(p2ThrustWorld, accel));
            float steerTq = (p2OutwardLocal.x * p2ThrustLocal.z - p2OutwardLocal.z * p2ThrustLocal.x);
            raft.angularVelocityY += steerTq * (BOAT_STEER_TORQUE / BOAT_MASS) * (curForce / GAS_NORMAL_FORCE) * dt * 22.0f;

            if (rand() % 4 == 0) {
                EmitFartBurst(p2PosWorld, p2OutwardWorld, gasP2.sustainedCharge);
            }
        }"""

code = code.replace(old_p2_thrust, new_p2_thrust)

# 7. Update Boat and Pirates Rendering
old_boat_draw = """                    // Authentic Boat Model
                    DrawModel(boatModel, Vector3{ 0, 0, 0 }, 1.0f, WHITE);

                    // Left Pirate (Player 1) with 1:1 Butt Swell, Tremble & Redness
                    float p1Redness = gasP1.redness;
                    SetShaderValue(shaderLighting, uRednessLoc, &p1Redness, SHADER_UNIFORM_FLOAT);

                    rlPushMatrix();
                        rlTranslatef(-0.75f, 0.28f + gasP1.trembleOffset, 0.0f);
                        // Bone Swell scale (1.0x to 1.4x)
                        rlScalef(1.15f * gasP1.boneScale, 1.15f * gasP1.boneScale, 1.15f * gasP1.boneScale);
                        DrawModel(charA, Vector3{ 0, 0, 0 }, 1.0f, WHITE);
                        DrawModel(shirt, Vector3{ 0, 0, 0 }, 1.0f, WHITE);
                        DrawModel(pant,  Vector3{ 0, 0, 0 }, 1.0f, WHITE);
                        DrawModel(hat1,  Vector3{ 0, 0, 0 }, 1.0f, WHITE);
                    rlPopMatrix();

                    // Right Pirate (Player 2 AI Co-pilot) with Butt Swell, Tremble & Redness
                    float p2Redness = gasP2.redness;
                    SetShaderValue(shaderLighting, uRednessLoc, &p2Redness, SHADER_UNIFORM_FLOAT);

                    rlPushMatrix();
                        rlTranslatef(0.75f, 0.28f + gasP2.trembleOffset, 0.0f);
                        rlScalef(1.15f * gasP2.boneScale, 1.15f * gasP2.boneScale, 1.15f * gasP2.boneScale);
                        DrawModel(charB, Vector3{ 0, 0, 0 }, 1.0f, WHITE);
                        DrawModel(shirt, Vector3{ 0, 0, 0 }, 1.0f, WHITE);
                        DrawModel(pant,  Vector3{ 0, 0, 0 }, 1.0f, WHITE);
                    rlPopMatrix();"""

new_boat_draw = """                    // Authentic Boat Model (Scale 1.0f matches character proportions)
                    DrawModel(boatModel, Vector3{ 0, 0, 0 }, 1.0f, WHITE);

                    // Left Pirate (Player 1) seated facing inward on the dinghy rim
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

code = code.replace(old_boat_draw, new_boat_draw)

# 8. Fix Transparent Particle depth mask
old_particles = """            // I. Cartoon Billboard Fart Particles
            for (int p = 0; p < MAX_PARTICLES; p++) {
                if (particles[p].active) {
                    Texture2D tex = texFart[particles[p].spriteIndex];
                    Rectangle srcRec = Rectangle{ 0, 0, (float)tex.width, (float)tex.height };
                    Color pTint = Color{ 255, 255, 255, (unsigned char)(particles[p].alpha * 240) };
                    DrawBillboardPro(camera, tex, srcRec, particles[p].position, Vector3{ 0, 1, 0 },
                                     Vector2{ particles[p].size, particles[p].size * 0.65f },
                                     Vector2{ particles[p].size * 0.5f, particles[p].size * 0.32f },
                                     particles[p].rotation, pTint);
                }
            }"""

new_particles = """            // I. Cartoon Billboard Fart Particles (Proper OpenGL Alpha Blending & Depth Mask Disable)
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

code = code.replace(old_particles, new_particles)

with open(r"CustomEngine\native\GameEngine.cpp", "w", encoding="utf-8") as f:
    f.write(code)

print("Applied authentic overhaul successfully")

