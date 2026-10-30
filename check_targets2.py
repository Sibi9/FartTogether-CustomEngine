import pathlib

text = pathlib.Path("CustomEngine/native/GameEngine.cpp").read_text(encoding="utf-8")

def check(name, s):
    c = text.count(s)
    print(f"[{'OK' if c == 1 else 'FAIL'}] {name}: count={c}")

s_tex_start = "    // Textures\n\n    Texture2D texPalette"
s_tex_end = "    ApplyMat(sharkModel);\n\n    // Water Material setup"
check("tex start", s_tex_start)
check("tex end", s_tex_end)

s_checkpoints_start = "    std::vector<RouteCheckpoint> checkpoints = {"
s_checkpoints_end = "    std::vector<SharkAI> sharks = {\n        { Vector3{ 25.0f, 0.02f, -8.0f }, 0.0f, Vector3{ 25.0f, 0.02f, -8.0f }, 0.0f, 0.0f, false, SHARK_PATROL_SPEED },\n        { Vector3{ -60.0f, 0.02f, -70.0f }, 0.0f, Vector3{ -60.0f, 0.02f, -70.0f }, 0.0f, 0.0f, false, SHARK_PATROL_SPEED }\n    };"
check("checkpoints start", s_checkpoints_start)
check("checkpoints end", s_checkpoints_end)

s_hydro_start = "        // Authentic Hydrodynamics:"
s_hydro_end = "        raft.speed = horizSpeed;\n\n        // 5. Update Kinetic Obstacles & Spring Bumpers"
check("hydro start", s_hydro_start)
check("hydro end", s_hydro_end)

s_kinetics_start = "        // 5. Update Kinetic Obstacles & Spring Bumpers"
s_kinetics_end = "        // 6. Update Crocodile Predator AI"
check("kinetics start", s_kinetics_start)
check("kinetics end", s_kinetics_end)

s_croc_elev = "            float crocSplineT = FindClosestSplineT(croc.position);\n            float crocWaterY  = GetSplinePoint(crocSplineT).y - 0.26f;\n            croc.position.y   = crocWaterY;"
check("croc elev", s_croc_elev)

s_shark_elev = "            float sharkSplineT = FindClosestSplineT(shark.position);\n            float sharkWaterY  = GetSplinePoint(sharkSplineT).y - 0.29f;\n            shark.position.y   = sharkWaterY;"
check("shark elev", s_shark_elev)

s_sky_start = "        // 1. Cartoon Sky Gradient\n\n        ClearBackground(Color{ 194, 217, 242, 255 });\n\n        DrawRectangleGradientV(0, 0, GetScreenWidth(), GetScreenHeight(), Color{ 87, 133, 222, 255 }, Color{ 194, 217, 242, 255 });\n\n        BeginMode3D(camera);"
check("sky start", s_sky_start)

s_pirates_draw_start = "                    // Left Pirate (Player 1: Green Hippo - Naked Cartoon)"
s_pirates_draw_end = "                    // Reset uniforms for raft and all subsequent models\n                    float zeroReset = 0.0f;\n                    SetShaderValue(shaderLighting, uRednessLoc,   &zeroReset, SHADER_UNIFORM_FLOAT);\n                    SetShaderValue(shaderLighting, uButtSwellLoc, &zeroReset, SHADER_UNIFORM_FLOAT);\n                    SetShaderValue(shaderLighting, uEyeBulgeLoc,  &zeroReset, SHADER_UNIFORM_FLOAT);\n\n                rlPopMatrix();\n            }"
check("pirates draw start", s_pirates_draw_start)
check("pirates draw end", s_pirates_draw_end)

s_unload_start = "    UnloadTexture(texCaustic);"
s_unload_end = "    UnloadModel(crocModel);\n\n    CloseAudioDevice();"
check("unload start", s_unload_start)
check("unload end", s_unload_end)

