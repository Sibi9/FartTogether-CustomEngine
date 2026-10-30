with open("CustomEngine/native/GameEngine.cpp", "r", encoding="utf-8") as f:
    c = f.read()

idx_start = c.find("// Camera Follow with Trauma Shake")
assert idx_start != -1, "idx_start not found"
idx_end = c.find("camera.target   = Vector3Lerp(camera.target, desiredCamTarget, 14.0f * dt);")
assert idx_end != -1, "idx_end not found"
idx_end += len("camera.target   = Vector3Lerp(camera.target, desiredCamTarget, 14.0f * dt);")

cam_replacement = """if (editorMode) {
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
                Ray mRay = GetMouseRay(mousePos, camera);
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
        } else {
            // Camera Follow with Trauma Shake
            float camDistH = camCtrl.distance * cosf(camCtrl.pitch);
            Vector3 camOffset = Vector3{
                -sinf(camCtrl.yaw) * camDistH,
                camCtrl.distance * sinf(camCtrl.pitch) + 1.5f,
                -cosf(camCtrl.yaw) * camDistH
            };

            Vector3 shakeOffset = Vector3{ 0, 0, 0 };
            if (camCtrl.shakeTrauma > 0.0f) {
                float shakeMag = camCtrl.shakeTrauma * camCtrl.shakeTrauma * 0.85f;
                shakeOffset = Vector3{
                    ((float)rand()/RAND_MAX - 0.5f) * shakeMag,
                    ((float)rand()/RAND_MAX - 0.5f) * shakeMag,
                    ((float)rand()/RAND_MAX - 0.5f) * shakeMag
                };
            }

            Vector3 desiredCamPos = Vector3Add(Vector3Add(raft.position, camOffset), shakeOffset);
            Vector3 desiredCamTarget = Vector3Add(raft.position, Vector3{ sinf(camCtrl.yaw) * 3.5f, 0.85f, cosf(camCtrl.yaw) * 3.5f });

            camera.position = Vector3Lerp(camera.position, desiredCamPos, 14.0f * dt);
            camera.target   = Vector3Lerp(camera.target, desiredCamTarget, 14.0f * dt);
        }"""

c = c[:idx_start] + cam_replacement + c[idx_end:]

with open("CustomEngine/native/GameEngine.cpp", "w", encoding="utf-8") as f:
    f.write(c)

print("Camera logic patched successfully!")
