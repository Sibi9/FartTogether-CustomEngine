import pathlib

text = pathlib.Path("CustomEngine/native/GameEngine.cpp").read_text(encoding="utf-8")

def check(name, s):
    c = text.count(s)
    print(f"[{'OK' if c == 1 else 'FAIL'}] {name}: count={c}")

check("FS_LIGHTING", 'static const char* FS_LIGHTING = R"(\n#version 330')
check("FindClosestSplineT return", '    return bestT;\n\n}\n\n// =============================================================================\n\n// 4. Asset Helper')
check("Entity Architecture start", '// Unified Level Entity Architecture (Obstacles, Rocks, Trees, Checkpoints, Props)\nenum EntityType {')
check("Entity Architecture end", '    UnloadFileText(rawText);\n    return !list.empty();\n}')
check("main args", '    bool testScreenshotMode = false;\n    bool testEditorMode = false;\n\n    std::string testShotName = "CustomEngine/scratch/active_gameplay.png";\n\n    int testAngleMode = 0; // 0=behind, 1=side, 2=front, 3=top\n\n    int targetTestFrame = 60;')
check("uRedness uniform", '    int uRednessLoc   = GetShaderLocation(shaderLighting, "uRedness");\n    int uButtSwellLoc = GetShaderLocation(shaderLighting, "uButtSwell");\n    int uEyeBulgeLoc  = GetShaderLocation(shaderLighting, "uEyeBulge");\n    float zeroVal = 0.0f;\n    SetShaderValue(shaderLighting, uButtSwellLoc, &zeroVal, SHADER_UNIFORM_FLOAT);\n    SetShaderValue(shaderLighting, uEyeBulgeLoc,  &zeroVal, SHADER_UNIFORM_FLOAT);')
check("P1/P2 rim initial angles", '    // Left Pirate sits on left-rear rim (-120 deg)\n\n    float p1RimAngle = -2.094f; // -120 deg\n\n    // Right Pirate sits on right-rear rim (+120 deg)\n\n    float p2RimAngle = 2.094f;  // +120 deg')
check("P1 clamp", 'p1RimAngle = Clamp(p1RimAngle, -2.85f, -0.65f);')
check("P2 clamp", 'p2RimAngle = Clamp(p2RimAngle, 0.65f, 2.85f);')
check("rimRadius", 'float rimRadius = 1.25f;')
check("Cylinder enclosure", '            // A. Circular Start Lake Enclosure (Image 4 Parity)\n            // Outer Grassy Bank Ring\n            DrawCylinder(Vector3{ 14.45f, 0.02f, 72.0f }, 18.2f, 18.2f, 0.15f, 48, Color{ 76, 175, 40, 255 });\n            // Orange Bouncy Bumper Tube Rim\n            DrawCylinder(Vector3{ 14.45f, 0.12f, 72.0f }, 16.7f, 16.7f, 0.22f, 48, Color{ 245, 120, 20, 255 });\n            // Blue Lake Water\n            DrawCylinder(Vector3{ 14.45f, 0.18f, 72.0f }, 16.2f, 16.2f, 0.24f, 48, Color{ 40, 165, 240, 255 });')

