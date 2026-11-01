import os

file_path = os.path.join("CustomEngine", "native", "GameEngine.cpp")
with open(file_path, "r", encoding="utf-8") as f:
    content = f.read()

# 1. Update GAME_VERSION_TAG
content = content.replace('static const char* GAME_VERSION_TAG = "v8.4.0";', 'static const char* GAME_VERSION_TAG = "v8.4.1";')

# 2. Update initial declarations at line 2920
old_decl = """    float p1RimAngle = -2.65f;
    float p2RimAngle =  2.65f;"""
new_decl = """    float p1RimAngle =  2.45f;
    float p2RimAngle = -2.45f;"""
content = content.replace(old_decl, new_decl)

# 3. Synchronize Kinetic Obstacles to authoritative raceTimer
old_anim_update = """        // Update Kinetic Obstacles (Runs both in gameplay and in editor when natural motion is toggled ON!)
        if (!editorMode || editorAnimateObstacles) {
            for (auto& ent : levelEntities) {
                ent.animTimer += dt * ent.animSpeed;
            }
        }"""

new_anim_update = """        // Update Kinetic Obstacles (Synchronized strictly to authoritative raceTimer in gameplay!)
        if (editorMode) {
            if (editorAnimateObstacles) {
                for (auto& ent : levelEntities) {
                    ent.animTimer += dt * ent.animSpeed;
                }
            }
        } else {
            for (auto& ent : levelEntities) {
                ent.animTimer = raceTimer * ent.animSpeed;
            }
        }"""
content = content.replace(old_anim_update, new_anim_update)

# 4. Update ApplyPirateRimMovement bounds: P1 left [0.05, 3.10], P2 right [-3.10, -0.05]
old_bounds = """            if (totalPlayers == 2) {
                // 2 Players: Half boat each (P1 left [-3.10f, -0.05f], P2 right [0.05f, 3.10f])
                if (playerIndex == 0) {
                    angle = Clamp(angle, -3.10f, -0.05f);
                } else {
                    angle = Clamp(angle,  0.05f,  3.10f);
                }
            } else if (totalPlayers == 3) {
                // 3 Players: Third of boat each
                if (playerIndex == 0) {
                    angle = Clamp(angle, -3.10f, -1.08f);
                } else if (playerIndex == 1) {
                    angle = Clamp(angle, -1.02f,  1.02f);
                } else {
                    angle = Clamp(angle,  1.08f,  3.10f);
                }
            } else if (totalPlayers >= 4) {
                // 4 Players: Quarter of boat each
                if (playerIndex == 0) {
                    angle = Clamp(angle, -3.10f, -1.60f);
                } else if (playerIndex == 1) {
                    angle = Clamp(angle, -1.54f, -0.05f);
                } else if (playerIndex == 2) {
                    angle = Clamp(angle,  0.05f,  1.54f);
                } else {
                    angle = Clamp(angle,  1.60f,  3.10f);
                }
            }"""

new_bounds = """            if (totalPlayers == 2) {
                // 2 Players: Half boat each (P1 left [0.05f, 3.10f], P2 right [-3.10f, -0.05f])
                if (playerIndex == 0) {
                    angle = Clamp(angle,  0.05f,  3.10f);
                } else {
                    angle = Clamp(angle, -3.10f, -0.05f);
                }
            } else if (totalPlayers == 3) {
                // 3 Players: Third of boat each
                if (playerIndex == 0) {
                    angle = Clamp(angle,  1.08f,  3.10f);
                } else if (playerIndex == 1) {
                    angle = Clamp(angle, -1.02f,  1.02f);
                } else {
                    angle = Clamp(angle, -3.10f, -1.08f);
                }
            } else if (totalPlayers >= 4) {
                // 4 Players: Quarter of boat each
                if (playerIndex == 0) {
                    angle = Clamp(angle,  1.60f,  3.10f);
                } else if (playerIndex == 1) {
                    angle = Clamp(angle,  0.05f,  1.54f);
                } else if (playerIndex == 2) {
                    angle = Clamp(angle, -1.54f, -0.05f);
                } else {
                    angle = Clamp(angle, -3.10f, -1.60f);
                }
            }"""
content = content.replace(old_bounds, new_bounds)

# 5. Tighter Client position & velocity lerp
old_lerp = """                if (posErr > 4.5f) {
                    raft.position = authPos;
                } else {
                    raft.position = Vector3Lerp(raft.position, authPos, 0.38f);
                }
                raft.velocity = Vector3Lerp(raft.velocity, Vector3{ sp.velX, sp.velY, sp.velZ }, 0.55f);"""

new_lerp = """                if (posErr > 3.0f) {
                    raft.position = authPos;
                } else {
                    raft.position = Vector3Lerp(raft.position, authPos, 0.65f);
                }
                raft.velocity = Vector3Lerp(raft.velocity, Vector3{ sp.velX, sp.velY, sp.velZ }, 0.75f);"""
content = content.replace(old_lerp, new_lerp)

# 6. Lethal collision host-authoritative check
old_death = """                } else if (ent.isLethal != 0) {
                    // Lethal Hazard: Explodes boat ONLY on true contact with obstacle mesh triangles
                    if (!raft.isDestroyed && raft.invulnerableTimer <= 0.0f) {
                        TriggerBoatDeath(DEATH_OBSTACLE);
                    }
                }"""

new_death = """                } else if (ent.isLethal != 0) {
                    // Lethal Hazard: Explodes boat ONLY on true contact with obstacle mesh triangles
                    if (!raft.isDestroyed && raft.invulnerableTimer <= 0.0f) {
                        if (LanGetRole() != LAN_CLIENT) {
                            TriggerBoatDeath(DEATH_OBSTACLE);
                        }
                    }
                }"""
content = content.replace(old_death, new_death)

# 7. Reset raceTimer and ent.animTimer on Match Start (Host & Client) and Solo Play
old_solo = """                    if (m == 0) { // PLAY SOLO (Single Pirate Mode, 360 degree rotation)
                        LanShutdown();
                        singlePirateMode = true;
                        p1RimAngle = 3.14159f;
                        raft.position = Vector3{ 15.55f, 0.58f, 90.80f };
                        raft.velocity = Vector3{ 0.0f, 0.0f, 0.0f };
                        raft.rotationY = 3.14159f;
                        raft.isDestroyed = false;
                        raft.currentCP = 0;
                        gasP1.gasEnergy = GAS_MAX;
                        gasP1.gasDepleted = false;
                        currentScreen = SCREEN_GAMEPLAY;"""

new_solo = """                    if (m == 0) { // PLAY SOLO (Single Pirate Mode, 360 degree rotation)
                        LanShutdown();
                        singlePirateMode = true;
                        p1RimAngle = -3.14159f;
                        raft.position = Vector3{ 15.55f, 0.58f, 90.80f };
                        raft.velocity = Vector3{ 0.0f, 0.0f, 0.0f };
                        raft.rotationY = 3.14159f;
                        raft.isDestroyed = false;
                        raft.currentCP = 0;
                        gasP1.gasEnergy = GAS_MAX;
                        gasP1.gasDepleted = false;
                        raceTimer = 0.0f;
                        for (auto& ent : levelEntities) {
                            ent.animTimer = 0.0f;
                        }
                        currentScreen = SCREEN_GAMEPLAY;"""
content = content.replace(old_solo, new_solo)

old_lobby_start = """            if (LanGetRole() == LAN_HOST && gConnectedPlayers >= 2 && gHostReady && gClientReady) {
                LanSetMatchStarted(true);
                LanSendStartSignal();
                singlePirateMode = false;
                p1RimAngle = 3.14159f;
                p2RimAngle = 0.0f;
                raft.position = Vector3{ 15.55f, 0.58f, 90.80f };
                raft.velocity = Vector3Zero();
                raft.rotationY = 3.14159f;
                raft.isDestroyed = false;
                raft.currentCP = 0;
                gasP1.gasEnergy = GAS_MAX;
                gasP2.gasEnergy = GAS_MAX;
                currentScreen = SCREEN_GAMEPLAY;
            } else if (LanGetRole() == LAN_CLIENT && LanIsMatchStarted()) {
                singlePirateMode = false;
                p1RimAngle = 3.14159f;
                p2RimAngle = 0.0f;
                raft.position = Vector3{ 15.55f, 0.58f, 90.80f };
                raft.velocity = Vector3Zero();
                raft.rotationY = 3.14159f;
                raft.isDestroyed = false;
                raft.currentCP = 0;
                gasP1.gasEnergy = GAS_MAX;
                gasP2.gasEnergy = GAS_MAX;
                currentScreen = SCREEN_GAMEPLAY;
            }"""

new_lobby_start = """            if (LanGetRole() == LAN_HOST && gConnectedPlayers >= 2 && gHostReady && gClientReady) {
                LanSetMatchStarted(true);
                LanSendStartSignal();
                singlePirateMode = false;
                p1RimAngle =  2.45f;
                p2RimAngle = -2.45f;
                raft.position = Vector3{ 15.55f, 0.58f, 90.80f };
                raft.velocity = Vector3Zero();
                raft.rotationY = 3.14159f;
                raft.isDestroyed = false;
                raft.currentCP = 0;
                gasP1.gasEnergy = GAS_MAX;
                gasP2.gasEnergy = GAS_MAX;
                raceTimer = 0.0f;
                for (auto& ent : levelEntities) {
                    ent.animTimer = 0.0f;
                }
                currentScreen = SCREEN_GAMEPLAY;
            } else if (LanGetRole() == LAN_CLIENT && LanIsMatchStarted()) {
                singlePirateMode = false;
                p1RimAngle =  2.45f;
                p2RimAngle = -2.45f;
                raft.position = Vector3{ 15.55f, 0.58f, 90.80f };
                raft.velocity = Vector3Zero();
                raft.rotationY = 3.14159f;
                raft.isDestroyed = false;
                raft.currentCP = 0;
                gasP1.gasEnergy = GAS_MAX;
                gasP2.gasEnergy = GAS_MAX;
                raceTimer = 0.0f;
                for (auto& ent : levelEntities) {
                    ent.animTimer = 0.0f;
                }
                currentScreen = SCREEN_GAMEPLAY;
            }"""
content = content.replace(old_lobby_start, new_lobby_start)

with open(file_path, "w", encoding="utf-8") as f:
    f.write(content)

print("GameEngine.cpp successfully patched for v8.4.1!")
