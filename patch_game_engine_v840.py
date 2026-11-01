import os

file_path = os.path.join("CustomEngine", "native", "GameEngine.cpp")
with open(file_path, "r", encoding="utf-8") as f:
    content = f.read()

start_marker = "// LAN Co-op 60Hz Synchronization (Plan v8.3.0 Task 4)"
end_marker = '// "FART TOGETHER" Co-op Sync Boost!'

idx1 = content.find(start_marker)
idx2 = content.find(end_marker)
print(f"idx1: {idx1}, idx2: {idx2}")
assert idx1 != -1, "start_marker not found"
assert idx2 != -1, "end_marker not found"

replacement = """        // =====================================================================
        // AAA LAN Co-op Synchronization & Dual-Pirate Input Routing (v8.4.0)
        // User Rim Rule: 1 Player = 360°, 2 Players = 1/2 each, 3 = 1/3, 4 = 1/4
        // =====================================================================
        static uint32_t netSeqHost             = 0;
        static uint32_t netSeqClient           = 0;
        static uint16_t localClientTapSeq      = 0;
        static uint16_t localClientSuicideSeq  = 0;
        static uint16_t hostLastExecClientTap  = 0;
        static uint16_t hostLastExecSuicide    = 0;
        static uint16_t hostP1TapSeq           = 0;
        static uint16_t clientLastSeenP1Tap    = 0;
        static uint16_t clientLastSeenP2Tap    = 0;
        static LanInputPacket gLatchedClientIn = { 0 };
        static float    gClientInputHoldTimer  = 0.0f;
        static bool     s_prevLocalGasDown     = false;

        auto ApplyPirateRimMovement = [](float& angle, float moveInput, float dtStep, int playerIndex, int totalPlayers) {
            if (totalPlayers <= 1) {
                // 1 Player: 360 degrees full rotation around rim
                angle += moveInput * 2.60f * dtStep;
                while (angle >  PI) angle -= 2.0f * PI;
                while (angle < -PI) angle += 2.0f * PI;
                return;
            }
            angle += moveInput * 2.50f * dtStep;
            if (totalPlayers == 2) {
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
            }
        };

        int activeTotalPlayers = (LanGetRole() == LAN_OFFLINE) ? 1 : gConnectedPlayers;
        if (activeTotalPlayers < 1) activeTotalPlayers = 1;

        // 1. Read Local Player Unified Input (Keyboard + Touch HUD + Gamepad)
        float localRimMove = 0.0f;
        if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)  || touchSteerLeft)  localRimMove -= 1.0f;
        if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT) || touchSteerRight) localRimMove += 1.0f;
        if (IsGamepadAvailable(0)) {
            float gpAxis = GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X);
            if (fabsf(gpAxis) > 0.15f) localRimMove += gpAxis;
        }
        localRimMove = Clamp(localRimMove, -1.0f, 1.0f);

        bool gp0Down = IsGamepadAvailable(0) && (IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN) || GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_TRIGGER) > 0.3f);
        bool localGasDown = (!raft.isDestroyed) && (IsKeyDown(KEY_W) || IsKeyDown(KEY_SPACE) || IsKeyDown(KEY_UP) || gp0Down || touchGas);

        // Automated sequence for test screenshot
        if (testScreenshotMode) {
            if (testFrameCount >= 20 && testFrameCount < 52) localGasDown = true;
            else if (testFrameCount == 52) localGasDown = false;
        }

        // True edge detection for Gas Release (Works identically on PC Keyboard & Mobile Touch!)
        bool localGasReleased = (s_prevLocalGasDown && !localGasDown);
        s_prevLocalGasDown = localGasDown;

        float rimRadius = 0.94f;
        Matrix boatRotM = MatrixRotateY(raft.rotationY);

        if (LanGetRole() == LAN_CLIENT) {
            // -----------------------------------------------------------------
            // CLIENT MODE: Local Player controls PIRATE 2 (p2RimAngle, gasP2)
            // -----------------------------------------------------------------
            singlePirateMode = false;

            // Client-Side Prediction for Pirate 2 rim rotation with sector bounds
            if (!raft.isDestroyed) {
                ApplyPirateRimMovement(p2RimAngle, localRimMove, dt, 1, activeTotalPlayers);
            }

            Vector3 p2OutwardLocal = Vector3{ sinf(p2RimAngle), 0.0f, cosf(p2RimAngle) };
            Vector3 p2ThrustLocal  = Vector3Scale(p2OutwardLocal, -1.0f);
            Vector3 p2ThrustWorld  = Vector3Transform(p2ThrustLocal, boatRotM);
            Vector3 p2OutwardWorld = Vector3Transform(p2OutwardLocal, boatRotM);
            Vector3 p2PosWorld     = Vector3Add(raft.position, Vector3Transform(Vector3Scale(p2OutwardLocal, rimRadius), boatRotM));
            p2PosWorld.y += 0.55f;

            // Detect Client Tap Burst BEFORE gasP2.Update resets pressTimer!
            if (localGasReleased && gasP2.pressTimer <= GAS_TAP_THRESHOLD_SECONDS && gasP2.pressTimer > 0.01f && !gasP2.gasDepleted && !raft.isDestroyed) {
                if (gasP2.gasEnergy >= GAS_TAP_COST) {
                    gasP2.gasEnergy -= GAS_TAP_COST;
                    localClientTapSeq++;
                    // Client-Side Immediate Impulse Prediction
                    float tapDt    = Clamp(gasP2.pressTimer, 0.06f, 0.10f);
                    float tapAccel = (gPhysCfg.tapBurstForce / BOAT_MASS) * tapDt;
                    raft.velocity  = Vector3Add(raft.velocity, Vector3Scale(p2ThrustWorld, tapAccel));
                    float steerTq  = (p2OutwardLocal.x * p2ThrustLocal.z - p2OutwardLocal.z * p2ThrustLocal.x);
                    raft.angularVelocityY += steerTq * (gPhysCfg.steerTorque / BOAT_MASS) * 22.0f * tapDt;
                    EmitFartBurst(p2PosWorld, p2OutwardWorld, 0.4f);
                    PlaySound(sndTap[rand() % 3]);
                }
            }

            gasP2.Update(dt, localGasDown, runTime);

            // Client-Side Immediate Sustained Thrust Prediction for Pirate 2
            if (gasP2.isSustained && !raft.isDestroyed) {
                float curForce = gPhysCfg.sustainedMinForce + (gPhysCfg.sustainedMaxForce - gPhysCfg.sustainedMinForce) * gasP2.sustainedCharge;
                float accel    = (curForce / BOAT_MASS) * dt;
                raft.velocity  = Vector3Add(raft.velocity, Vector3Scale(p2ThrustWorld, accel));
                float steerTq  = (p2OutwardLocal.x * p2ThrustLocal.z - p2OutwardLocal.z * p2ThrustLocal.x);
                raft.angularVelocityY += steerTq * (gPhysCfg.steerTorque / BOAT_MASS) * 22.0f * dt;
                if (rand() % 4 == 0) {
                    EmitFartBurst(p2PosWorld, p2OutwardWorld, gasP2.sustainedCharge);
                }
            }

            if (IsKeyPressed(KEY_R)) localClientSuicideSeq++;

            // Send Authoritative Client Input Packet (with Monotonic Event Counters!) to Host
            LanInputPacket inPkt{};
            inPkt.magic          = 0x46415254;
            inPkt.type           = LAN_PKT_INPUT;
            inPkt.seq            = ++netSeqClient;
            inPkt.rimMove        = localRimMove;
            inPkt.clientRimAngle = p2RimAngle;
            inPkt.gasDown        = localGasDown ? 1 : 0;
            inPkt.gasReleased    = localGasReleased ? 1 : 0;
            inPkt.tapBurstSeq    = localClientTapSeq;
            inPkt.suicideSeq     = localClientSuicideSeq;
            LanClientSendInput(inPkt);

            // Poll Authoritative State from Host & Reconcile Smoothly
            LanStatePacket sp{};
            if (LanClientPollState(sp)) {
                Vector3 authPos = Vector3{ sp.raftX, sp.raftY, sp.raftZ };
                float posErr = Vector3Distance(raft.position, authPos);
                if (posErr > 4.5f) {
                    raft.position = authPos;
                } else {
                    raft.position = Vector3Lerp(raft.position, authPos, 0.38f);
                }
                raft.velocity = Vector3Lerp(raft.velocity, Vector3{ sp.velX, sp.velY, sp.velZ }, 0.55f);

                // Shortest-arc angular reconciliation
                float rotDiff = sp.rotY - raft.rotationY;
                while (rotDiff >  PI) rotDiff -= 2.0f * PI;
                while (rotDiff < -PI) rotDiff += 2.0f * PI;
                raft.rotationY += rotDiff * 0.45f;
                raft.angularVelocityY = sp.angVelY;
                raft.pitch = sp.pitch;
                raft.roll  = sp.roll;

                // Replicate Host's Pirate 1 State & VFX
                p1RimAngle             = sp.p1RimAngle;
                gasP1.gasEnergy        = sp.p1Energy;
                gasP1.sustainedCharge  = sp.p1Charge;
                gasP1.isSustained      = (sp.p1Sustained != 0);
                gasP1.isPressing       = (sp.p1Sustained != 0 || sp.p1Eye > 0.05f);
                gasP1.gasDepleted      = (sp.p1Depleted != 0);
                gasP1.redness          = sp.p1Redness;
                gasP1.boneScale        = 1.0f + sp.p1Swell;

                // Reconcile Pirate 2 energy from Host
                gasP2.gasEnergy        = sp.p2Energy;
                gasP2.gasDepleted      = (sp.p2Depleted != 0);

                // Trigger Host P1 Tap Burst VFX/Sound on Client when hostP1TapSeq increments
                if (sp.p1TapSeq != clientLastSeenP1Tap) {
                    clientLastSeenP1Tap = sp.p1TapSeq;
                    Vector3 p1OutL = Vector3{ sinf(p1RimAngle), 0.0f, cosf(p1RimAngle) };
                    Vector3 p1OutW = Vector3Transform(p1OutL, boatRotM);
                    Vector3 p1PosW = Vector3Add(raft.position, Vector3Transform(Vector3Scale(p1OutL, rimRadius), boatRotM));
                    p1PosW.y += 0.55f;
                    EmitFartBurst(p1PosW, p1OutW, 0.4f);
                    PlaySound(sndTap[rand() % 3]);
                }

                if (gasP1.isSustained && !raft.isDestroyed && (rand() % 4 == 0)) {
                    Vector3 p1OutL = Vector3{ sinf(p1RimAngle), 0.0f, cosf(p1RimAngle) };
                    Vector3 p1OutW = Vector3Transform(p1OutL, boatRotM);
                    Vector3 p1PosW = Vector3Add(raft.position, Vector3Transform(Vector3Scale(p1OutL, rimRadius), boatRotM));
                    p1PosW.y += 0.55f;
                    EmitFartBurst(p1PosW, p1OutW, gasP1.sustainedCharge);
                }

                raceTimer      = sp.raceTimer;
                totalDeaths    = sp.totalDeaths;
                raft.deaths    = sp.totalDeaths;
                raft.currentCP = sp.currentCP;
                if (sp.isDestroyed && !raft.isDestroyed) {
                    TriggerBoatDeath(DEATH_OBSTACLE);
                } else if (!sp.isDestroyed && raft.isDestroyed) {
                    raft.isDestroyed = false;
                    ragdollA.active  = false;
                    ragdollB.active  = false;
                }
            }

            // Play sustained hold sound if EITHER Pirate 1 or Pirate 2 is holding gas!
            if ((gasP1.isSustained || gasP2.isSustained) && !raft.isDestroyed) {
                if (!IsSoundPlaying(sndHold[0]) && !IsSoundPlaying(sndHold[1])) {
                    PlaySound(sndHold[rand() % 2]);
                }
            } else {
                if (IsSoundPlaying(sndHold[0])) StopSound(sndHold[0]);
                if (IsSoundPlaying(sndHold[1])) StopSound(sndHold[1]);
            }

        } else {
            // -----------------------------------------------------------------
            // HOST OR SOLO MODE:
            // Local Player controls PIRATE 1 (p1RimAngle, gasP1).
            // In LAN_HOST mode, Remote Client controls PIRATE 2 (p2RimAngle, gasP2)!
            // -----------------------------------------------------------------
            singlePirateMode = (LanGetRole() == LAN_OFFLINE);

            if (!raft.isDestroyed) {
                ApplyPirateRimMovement(p1RimAngle, localRimMove, dt, 0, activeTotalPlayers);
            }

            Vector3 p1OutwardLocal = Vector3{ sinf(p1RimAngle), 0.0f, cosf(p1RimAngle) };
            Vector3 p1ThrustLocal  = Vector3Scale(p1OutwardLocal, -1.0f);
            Vector3 p1ThrustWorld  = Vector3Transform(p1ThrustLocal, boatRotM);
            Vector3 p1OutwardWorld = Vector3Transform(p1OutwardLocal, boatRotM);
            Vector3 p1PosWorld     = Vector3Add(raft.position, Vector3Transform(Vector3Scale(p1OutwardLocal, rimRadius), boatRotM));
            p1PosWorld.y += 0.55f;

            if (localGasReleased && gasP1.pressTimer <= GAS_TAP_THRESHOLD_SECONDS && gasP1.pressTimer > 0.01f && !gasP1.gasDepleted && !raft.isDestroyed) {
                if (gasP1.gasEnergy >= GAS_TAP_COST) {
                    gasP1.gasEnergy -= GAS_TAP_COST;
                    hostP1TapSeq++;
                    float tapDt    = Clamp(gasP1.pressTimer, 0.06f, 0.10f);
                    float tapAccel = (gPhysCfg.tapBurstForce / BOAT_MASS) * tapDt;
                    raft.velocity  = Vector3Add(raft.velocity, Vector3Scale(p1ThrustWorld, tapAccel));
                    float steerTq  = (p1OutwardLocal.x * p1ThrustLocal.z - p1OutwardLocal.z * p1ThrustLocal.x);
                    raft.angularVelocityY += steerTq * (gPhysCfg.steerTorque / BOAT_MASS) * 22.0f * tapDt;
                    EmitFartBurst(p1PosWorld, p1OutwardWorld, 0.4f);
                    PlaySound(sndTap[rand() % 3]);
                }
            }

            gasP1.Update(dt, localGasDown, runTime);

            if (gasP1.isSustained && !raft.isDestroyed) {
                float curForce = gPhysCfg.sustainedMinForce + (gPhysCfg.sustainedMaxForce - gPhysCfg.sustainedMinForce) * gasP1.sustainedCharge;
                float accel    = (curForce / BOAT_MASS) * dt;
                raft.velocity  = Vector3Add(raft.velocity, Vector3Scale(p1ThrustWorld, accel));
                float steerTq  = (p1OutwardLocal.x * p1ThrustLocal.z - p1OutwardLocal.z * p1ThrustLocal.x);
                raft.angularVelocityY += steerTq * (gPhysCfg.steerTorque / BOAT_MASS) * 22.0f * dt;
                if (rand() % 4 == 0) {
                    EmitFartBurst(p1PosWorld, p1OutwardWorld, gasP1.sustainedCharge);
                }
            }

            // Process Remote Client (Pirate 2) in LAN_HOST Mode!
            if (LanGetRole() == LAN_HOST) {
                LanInputPacket newClientIn{};
                if (LanHostPollInput(newClientIn)) {
                    gLatchedClientIn      = newClientIn;
                    gClientInputHoldTimer = 0.18f; // Hold latched input for 180ms across Wi-Fi frame gaps!

                    // Sync Client's exact predicted rim angle smoothly
                    float angDiff = newClientIn.clientRimAngle - p2RimAngle;
                    while (angDiff >  PI) angDiff -= 2.0f * PI;
                    while (angDiff < -PI) angDiff += 2.0f * PI;
                    p2RimAngle += angDiff * 0.65f;
                    ApplyPirateRimMovement(p2RimAngle, 0.0f, dt, 1, activeTotalPlayers);
                } else {
                    if (gClientInputHoldTimer > 0.0f) {
                        gClientInputHoldTimer -= dt;
                    } else {
                        gLatchedClientIn.rimMove = 0.0f;
                        gLatchedClientIn.gasDown = 0;
                    }
                }

                if (!raft.isDestroyed) {
                    ApplyPirateRimMovement(p2RimAngle, Clamp(gLatchedClientIn.rimMove, -1.0f, 1.0f), dt, 1, activeTotalPlayers);
                }

                Vector3 p2OutwardLocal = Vector3{ sinf(p2RimAngle), 0.0f, cosf(p2RimAngle) };
                Vector3 p2ThrustLocal  = Vector3Scale(p2OutwardLocal, -1.0f);
                Vector3 p2ThrustWorld  = Vector3Transform(p2ThrustLocal, boatRotM);
                Vector3 p2OutwardWorld = Vector3Transform(p2OutwardLocal, boatRotM);
                Vector3 p2PosWorld     = Vector3Add(raft.position, Vector3Transform(Vector3Scale(p2OutwardLocal, rimRadius), boatRotM));
                p2PosWorld.y += 0.55f;

                // Execute Reliable Client Tap Burst when tapBurstSeq increments!
                if (gLatchedClientIn.tapBurstSeq != hostLastExecClientTap && !raft.isDestroyed) {
                    hostLastExecClientTap = gLatchedClientIn.tapBurstSeq;
                    if (gasP2.gasEnergy >= GAS_TAP_COST) {
                        gasP2.gasEnergy -= GAS_TAP_COST;
                    }
                    float tapDt    = 0.085f;
                    float tapAccel = (gPhysCfg.tapBurstForce / BOAT_MASS) * tapDt;
                    raft.velocity  = Vector3Add(raft.velocity, Vector3Scale(p2ThrustWorld, tapAccel));
                    float steerTq  = (p2OutwardLocal.x * p2ThrustLocal.z - p2OutwardLocal.z * p2ThrustLocal.x);
                    raft.angularVelocityY += steerTq * (gPhysCfg.steerTorque / BOAT_MASS) * 22.0f * tapDt;
                    EmitFartBurst(p2PosWorld, p2OutwardWorld, 0.4f);
                    PlaySound(sndTap[rand() % 3]);
                }

                // Execute Client Suicide Request if suicideSeq incremented
                if (gLatchedClientIn.suicideSeq != hostLastExecSuicide) {
                    hostLastExecSuicide = gLatchedClientIn.suicideSeq;
                    if (!raft.isDestroyed) TriggerBoatDeath(DEATH_SUICIDE);
                }

                bool clientGasDown = (gLatchedClientIn.gasDown != 0) && !raft.isDestroyed;
                gasP2.Update(dt, clientGasDown, runTime);

                // Apply Client Sustained Thrust to Boat Physics on Host!
                if (gasP2.isSustained && !raft.isDestroyed) {
                    float curForce = gPhysCfg.sustainedMinForce + (gPhysCfg.sustainedMaxForce - gPhysCfg.sustainedMinForce) * gasP2.sustainedCharge;
                    float accel    = (curForce / BOAT_MASS) * dt;
                    raft.velocity  = Vector3Add(raft.velocity, Vector3Scale(p2ThrustWorld, accel));
                    float steerTq  = (p2OutwardLocal.x * p2ThrustLocal.z - p2OutwardLocal.z * p2ThrustLocal.x);
                    raft.angularVelocityY += steerTq * (gPhysCfg.steerTorque / BOAT_MASS) * 22.0f * dt;
                    if (rand() % 4 == 0) {
                        EmitFartBurst(p2PosWorld, p2OutwardWorld, gasP2.sustainedCharge);
                    }
                }
            }

            // Play sustained hold sound if EITHER Pirate 1 or Pirate 2 is holding gas!
            bool anySustained = gasP1.isSustained || (!singlePirateMode && gasP2.isSustained);
            if (anySustained && !raft.isDestroyed) {
                if (!IsSoundPlaying(sndHold[0]) && !IsSoundPlaying(sndHold[1])) {
                    PlaySound(sndHold[rand() % 2]);
                }
            } else {
                if (IsSoundPlaying(sndHold[0])) StopSound(sndHold[0]);
                if (IsSoundPlaying(sndHold[1])) StopSound(sndHold[1]);
            }
        }
\n"""

new_content = content[:idx1] + replacement + content[idx2:]

# Now add LanHostBroadcastState(sp) after raft.speed = horizSpeed;
speed_marker = "raft.speed = horizSpeed;"
speed_idx = new_content.find(speed_marker)
assert speed_idx != -1, "raft.speed = horizSpeed; not found"

insert_pos = speed_idx + len(speed_marker)

broadcast_code = """

        // Broadcast authoritative state from Host (Plan v8.4.0)
        if (LanGetRole() == LAN_HOST) {
            LanStatePacket sp{};
            sp.magic       = 0x46415254;
            sp.type        = LAN_PKT_STATE;
            sp.seq         = ++netSeqHost;
            sp.raftX       = raft.position.x;
            sp.raftY       = raft.position.y;
            sp.raftZ       = raft.position.z;
            sp.velX        = raft.velocity.x;
            sp.velY        = raft.velocity.y;
            sp.velZ        = raft.velocity.z;
            sp.rotY        = raft.rotationY;
            sp.angVelY     = raft.angularVelocityY;
            sp.pitch       = raft.pitch;
            sp.roll        = raft.roll;
            sp.p1RimAngle  = p1RimAngle;
            sp.p2RimAngle  = p2RimAngle;
            sp.p1Energy    = gasP1.gasEnergy;
            sp.p2Energy    = gasP2.gasEnergy;
            sp.p1Charge    = gasP1.sustainedCharge;
            sp.p2Charge    = gasP2.sustainedCharge;
            sp.p1Redness   = gasP1.redness;
            sp.p2Redness   = gasP2.redness;
            sp.p1Swell     = gasP1.boneScale - 1.0f;
            sp.p2Swell     = gasP2.boneScale - 1.0f;
            sp.p1Eye       = gasP1.isPressing ? Clamp(gasP1.pressTimer / 0.45f, 0.0f, 1.0f) : 0.0f;
            sp.p2Eye       = gasP2.isPressing ? Clamp(gasP2.pressTimer / 0.45f, 0.0f, 1.0f) : 0.0f;
            sp.p1Sustained = gasP1.isSustained ? 1 : 0;
            sp.p2Sustained = gasP2.isSustained ? 1 : 0;
            sp.p1Depleted  = gasP1.gasDepleted ? 1 : 0;
            sp.p2Depleted  = gasP2.gasDepleted ? 1 : 0;
            sp.p1TapSeq    = hostP1TapSeq;
            sp.p2TapSeq    = hostLastExecClientTap;
            sp.raceTimer   = raceTimer;
            sp.totalDeaths = raft.deaths;
            sp.currentCP   = raft.currentCP;
            sp.isDestroyed = raft.isDestroyed ? 1 : 0;
            LanHostBroadcastState(sp);
        }
"""

new_content = new_content[:insert_pos] + broadcast_code + new_content[insert_pos:]

with open(file_path, "w", encoding="utf-8") as f:
    f.write(new_content)

print("GameEngine.cpp successfully patched for v8.4.0!")
