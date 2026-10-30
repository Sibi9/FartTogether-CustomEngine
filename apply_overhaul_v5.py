with open('CustomEngine/native/GameEngine.cpp', 'r', encoding='utf-8') as f:
    code = f.read()

# 1. Load GM_3D_ObstacleA1 model
old_models = """    Model obsModelB5   = LoadModel(ResolveAsset("extracted_assets/meshes/GM_3D_ObstacleB5.obj").c_str());"""
new_models = """    Model obsModelA1   = LoadModel(ResolveAsset("extracted_assets/meshes/GM_3D_ObstacleA1.obj").c_str());
    Model obsModelB5   = LoadModel(ResolveAsset("extracted_assets/meshes/GM_3D_ObstacleB5.obj").c_str());"""

assert old_models in code, 'old_models not found'
code = code.replace(old_models, new_models, 1)
print('Added obsModelA1')

# 2. ApplyMat for obsModelA1
old_apply_mat = """    ApplyMat(obsModelB5);"""
new_apply_mat = """    ApplyMat(obsModelA1);
    ApplyMat(obsModelB5);"""

assert old_apply_mat in code, 'old_apply_mat not found'
code = code.replace(old_apply_mat, new_apply_mat, 1)
print('Added ApplyMat(obsModelA1)')

# 3. Unload obsModelA1 in cleanup
old_cleanup = """    UnloadModel(obsModelB5);"""
new_cleanup = """    UnloadModel(obsModelB5);
    UnloadModel(obsModelA1);"""

assert old_cleanup in code, 'old_cleanup not found'
code = code.replace(old_cleanup, new_cleanup, 1)
print('Added UnloadModel(obsModelA1)')

# 4. Spiked Bumpers on Canal Walls and Relocated Predators
old_obs_list = """    // Kinetic Obstacles along the river (including Spring Bumpers)
    std::vector<KineticObstacle> obsList = {
        { OBS_BUMPER_SPRING, Vector3{ 24.12f, 0.8f, 62.45f },     0.0f, 0.0f, 1.0f, 3.2f, &obsModelB5, 0.0f, 0.0f, false },
        { OBS_SWING_F3,      Vector3{ 36.48f, 0.5f, 34.44f },     0.0f, 0.0f, 2.5f, 2.8f, &obsModelF3, 0.0f, 0.0f, false },
        { OBS_BUMPER_SPRING, Vector3{ 42.50f, 0.9f, 12.10f },     0.0f, 0.0f, 1.0f, 3.2f, &obsModelB5, 0.0f, 0.0f, false },
        { OBS_GATE_F10,      Vector3{ 11.15f, 0.8f, -10.67f },    0.0f, 0.0f, 1.8f, 3.2f, &obsModelF10, 0.0f, 0.0f, false },
        { OBS_BUMPER_SPRING, Vector3{ -80.12f, 12.5f, -103.11f }, 0.0f, 0.0f, 1.0f, 3.2f, &obsModelB5, 0.0f, 0.0f, false },
        { OBS_ROTOR_B5,      Vector3{ -220.31f, 10.2f, -103.11f },0.0f, 0.0f, 2.2f, 3.8f, &obsModelB5, 0.0f, 0.0f, false },
        { OBS_TIPPING_B2,    Vector3{ -323.30f, 13.6f, -103.11f },0.0f, 0.0f, 1.6f, 3.5f, &obsModelB2, 0.0f, 0.0f, false },
        { OBS_PENDULUM_C2,   Vector3{ -400.18f, 15.3f, -103.11f },0.0f, 0.0f, 2.0f, 4.0f, &obsModelC2, 0.0f, 0.0f, false },
        { OBS_ROCK1,         Vector3{ 22.54f, 0.2f, 96.57f },     0.0f, 0.0f, 0.0f, 2.4f, &obsModelRock1, 0.0f, 0.0f, false },
        { OBS_ROCK2,         Vector3{ 6.42f, 0.2f, 88.49f },      0.0f, 0.0f, 0.0f, 2.4f, &obsModelRock2, 0.0f, 0.0f, false }
    };

    // Authentic River Predators: Crocodiles and Sharks
    std::vector<CrocodileAI> crocodiles = {
        { Vector3{ 17.5f, 0.38f, 68.0f }, 3.14f, Vector3{ 17.5f, 0.38f, 68.0f }, 0.0f, false, CROC_SPEED },
        { Vector3{ 24.0f, 0.38f, 50.0f }, 2.80f, Vector3{ 24.0f, 0.38f, 50.0f }, 0.0f, false, CROC_SPEED },
        { Vector3{ -14.5f, 5.20f, -65.2f }, 2.40f, Vector3{ -14.5f, 5.20f, -65.2f }, 0.0f, false, CROC_SPEED }
    };

    std::vector<SharkAI> sharks = {
        { Vector3{ 22.0f, 0.28f, 56.0f }, 0.0f, Vector3{ 22.0f, 0.28f, 56.0f }, 0.0f, 0.0f, false, SHARK_PATROL_SPEED },
        { Vector3{ 32.5f, 0.25f, 2.8f }, 0.0f, Vector3{ 32.5f, 0.25f, 2.8f }, 0.0f, 0.0f, false, SHARK_PATROL_SPEED },
        { Vector3{ -95.0f, 14.1f, -103.0f }, 1.57f, Vector3{ -95.0f, 14.1f, -103.0f }, 1.57f, 0.0f, false, SHARK_PATROL_SPEED }
    };"""

new_obs_list = """    // Authentic Flume Obstacles: Pairs of Purple Spiked Bumpers (GM_3D_ObstacleA1) lining canal walls
    std::vector<KineticObstacle> obsList = {
        // Flume Spiked Bumper Pairs (Z = 66.6, 59.1, 51.7, 44.2)
        { OBS_BUMPER_SPRING, Vector3{ 14.45f, 0.65f, 66.65f },    0.0f, 0.0f, 1.0f, 2.5f, &obsModelA1, 0.0f, 0.0f, false },
        { OBS_BUMPER_SPRING, Vector3{ 19.75f, 0.65f, 66.65f },  180.0f, 0.0f, 1.0f, 2.5f, &obsModelA1, 0.0f, 0.0f, false },
        { OBS_BUMPER_SPRING, Vector3{ 16.83f, 0.65f, 59.10f },    0.0f, 0.0f, 1.0f, 2.5f, &obsModelA1, 0.0f, 0.0f, false },
        { OBS_BUMPER_SPRING, Vector3{ 22.13f, 0.65f, 59.10f },  180.0f, 0.0f, 1.0f, 2.5f, &obsModelA1, 0.0f, 0.0f, false },
        { OBS_BUMPER_SPRING, Vector3{ 21.47f, 0.65f, 51.71f },    0.0f, 0.0f, 1.0f, 2.5f, &obsModelA1, 0.0f, 0.0f, false },
        { OBS_BUMPER_SPRING, Vector3{ 26.77f, 0.65f, 51.71f },  180.0f, 0.0f, 1.0f, 2.5f, &obsModelA1, 0.0f, 0.0f, false },
        { OBS_BUMPER_SPRING, Vector3{ 25.91f, 0.65f, 44.20f },    0.0f, 0.0f, 1.0f, 2.5f, &obsModelA1, 0.0f, 0.0f, false },
        { OBS_BUMPER_SPRING, Vector3{ 31.21f, 0.65f, 44.20f },  180.0f, 0.0f, 1.0f, 2.5f, &obsModelA1, 0.0f, 0.0f, false },

        // Downstream Mechanical Hazards & Rocks
        { OBS_SWING_F3,      Vector3{ 36.48f, 0.5f, 34.44f },     0.0f, 0.0f, 2.5f, 2.8f, &obsModelF3, 0.0f, 0.0f, false },
        { OBS_GATE_F10,      Vector3{ 41.50f, 0.8f, 10.67f },    0.0f, 0.0f, 1.8f, 3.2f, &obsModelF10, 0.0f, 0.0f, false },
        { OBS_BUMPER_SPRING, Vector3{ -80.12f, 12.5f, -103.11f }, 0.0f, 0.0f, 1.0f, 3.2f, &obsModelB5, 0.0f, 0.0f, false },
        { OBS_ROTOR_B5,      Vector3{ -220.31f, 10.2f, -103.11f },0.0f, 0.0f, 2.2f, 3.8f, &obsModelB5, 0.0f, 0.0f, false },
        { OBS_TIPPING_B2,    Vector3{ -323.30f, 13.6f, -103.11f },0.0f, 0.0f, 1.6f, 3.5f, &obsModelB2, 0.0f, 0.0f, false },
        { OBS_PENDULUM_C2,   Vector3{ -400.18f, 15.3f, -103.11f },0.0f, 0.0f, 2.0f, 4.0f, &obsModelC2, 0.0f, 0.0f, false },
        { OBS_ROCK1,         Vector3{ 22.54f, 0.2f, 96.57f },     0.0f, 0.0f, 0.0f, 2.4f, &obsModelRock1, 0.0f, 0.0f, false },
        { OBS_ROCK2,         Vector3{ 6.42f, 0.2f, 88.49f },      0.0f, 0.0f, 0.0f, 2.4f, &obsModelRock2, 0.0f, 0.0f, false }
    };

    // Authentic River Predators: Relocated to downstream rapids & lakes (zero start obstruction)
    std::vector<CrocodileAI> crocodiles = {
        { Vector3{ 35.0f, 0.38f, 5.0f }, 3.14f, Vector3{ 35.0f, 0.38f, 5.0f }, 0.0f, false, CROC_SPEED },
        { Vector3{ -20.0f, 5.20f, -45.0f }, 2.40f, Vector3{ -20.0f, 5.20f, -45.0f }, 0.0f, false, CROC_SPEED },
        { Vector3{ -100.0f, 14.1f, -90.0f }, 1.57f, Vector3{ -100.0f, 14.1f, -90.0f }, 1.57f, 0.0f, CROC_SPEED }
    };

    std::vector<SharkAI> sharks = {
        { Vector3{ 25.0f, 0.28f, -8.0f }, 0.0f, Vector3{ 25.0f, 0.28f, -8.0f }, 0.0f, 0.0f, false, SHARK_PATROL_SPEED },
        { Vector3{ -60.0f, 12.0f, -70.0f }, 0.0f, Vector3{ -60.0f, 12.0f, -70.0f }, 0.0f, 0.0f, false, SHARK_PATROL_SPEED }
    };"""

assert old_obs_list in code, 'old_obs_list not found'
code = code.replace(old_obs_list, new_obs_list, 1)
print('Updated obsList and relocated predators')

# 5. Remove fake river flow velocity (Zero-motion start)
old_flow = """        float flowSpeed = 5.2f;
        if (currentSplineT > 25.0f && currentSplineT < 60.0f) flowSpeed = 7.8f;
        if (currentSplineT > 75.0f && currentSplineT < 95.0f) flowSpeed = 8.6f;

        Vector3 riverFlowVelocity = Vector3Scale(riverTangent, flowSpeed);
        Vector3 velRelToWater = Vector3Subtract(riverFlowVelocity, raft.velocity);
        raft.velocity = Vector3Add(raft.velocity, Vector3Scale(velRelToWater, 2.2f * dt));"""

new_flow = """        // Authentic Hydrodynamics: ZERO automatic current at start.
        // The boat only moves when propelled by fart thrust!
        // Linear damping gently brings drifting boat to a complete stop when gas stops.
        (void)riverTangent;"""

assert old_flow in code, 'old_flow not found'
code = code.replace(old_flow, new_flow, 1)
print('Removed fake river current')

# 6. Toxic Neon Lime Green Fart Color
old_p_color = """                    Color pTint = Color{ 255, 255, 255, (unsigned char)(particles[p].alpha * 220) };"""
new_p_color = """                    // Authentic Toxic Neon Lime Green Fart Gas (1:1 with reference gameplay video)
                    unsigned char greenVal = (unsigned char)(235.0f + 20.0f * (1.0f - particles[p].alpha));
                    unsigned char redVal   = (unsigned char)(75.0f + 40.0f * (1.0f - particles[p].alpha));
                    unsigned char blueVal  = (unsigned char)(25.0f + 20.0f * (1.0f - particles[p].alpha));
                    Color pTint = Color{ redVal, greenVal, blueVal, (unsigned char)(particles[p].alpha * 230) };"""

assert old_p_color in code, 'old_p_color not found'
code = code.replace(old_p_color, new_p_color, 1)
print('Updated particle color to Toxic Neon Lime Green')

# 7. Magenta tint for Spiked Bumper Obstacles
old_obs_draw = """                    DrawModel(*obs.model, Vector3{ 0, 0, 0 }, 1.0f, WHITE);"""
new_obs_draw = """                    Color obsTint = (obs.type == OBS_BUMPER_SPRING && obs.model == &obsModelA1) ? Color{ 215, 38, 180, 255 } : WHITE;
                    DrawModel(*obs.model, Vector3{ 0, 0, 0 }, 1.0f, obsTint);"""

assert old_obs_draw in code, 'old_obs_draw not found'
code = code.replace(old_obs_draw, new_obs_draw, 1)
print('Updated obstacle drawing with magenta tint')

# 8. Add Player Name Tags ("tisk" and "Echos")
old_hud_start = """        // 2D HUD (Authentic Typography, 1:1 Gas Tanks & Gauges)"""
new_hud_start = """        // Authentic Player Overhead Name Tags (Projected 3D Billboards matching video)
        Vector3 p1WorldPos = raft.position + Vector3{ sinf(p1RimAngle) * 1.05f, 0.95f, cosf(p1RimAngle) * 1.05f };
        Vector2 p1Screen = GetWorldToScreen(p1WorldPos, camera);
        if (p1Screen.x > 50 && p1Screen.x < GetScreenWidth() - 50 && p1Screen.y > 50 && p1Screen.y < GetScreenHeight() - 50) {
            DrawText("tisk", (int)p1Screen.x - MeasureText("tisk", 20) / 2, (int)p1Screen.y, 20, RAYWHITE);
        }

        Vector3 p2WorldPos = raft.position + Vector3{ sinf(p2RimAngle) * 1.05f, 0.95f, cosf(p2RimAngle) * 1.05f };
        Vector2 p2Screen = GetWorldToScreen(p2WorldPos, camera);
        if (p2Screen.x > 50 && p2Screen.x < GetScreenWidth() - 50 && p2Screen.y > 50 && p2Screen.y < GetScreenHeight() - 50) {
            DrawText("Echos", (int)p2Screen.x - MeasureText("Echos", 20) / 2, (int)p2Screen.y, 20, RAYWHITE);
        }

        // 2D HUD (Authentic Typography, 1:1 Gas Tanks & Gauges)"""

assert old_hud_start in code, 'old_hud_start not found'
code = code.replace(old_hud_start, new_hud_start, 1)
print('Added player nametags')

with open('CustomEngine/native/GameEngine.cpp', 'w', encoding='utf-8') as f:
    f.write(code)
print('Successfully written GameEngine.cpp!')
