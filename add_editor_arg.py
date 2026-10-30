with open("CustomEngine/native/GameEngine.cpp", "r", encoding="utf-8") as f:
    c = f.read()

target = '''    bool editorMode = testEditorMode;
    if (testEditorMode) {
        selectedObs = 0;
        editorCamPos = Vector3{ 14.45f, 3.8f, 73.0f };
        editorCamYaw = 3.14159f;
        editorCamPitch = 0.35f;
    }
    int selectedObs = -1;
    Vector3 editorCamPos = Vector3{ 15.55f, 6.0f, 75.0f };
    float editorCamYaw = 3.14159f;
    float editorCamPitch = 0.35f;'''

replacement = '''    bool editorMode = testEditorMode;
    int selectedObs = testEditorMode ? 0 : -1;
    Vector3 editorCamPos = testEditorMode ? Vector3{ 17.5f, 3.8f, 72.0f } : Vector3{ 15.55f, 6.0f, 75.0f };
    float editorCamYaw = 3.14159f;
    float editorCamPitch = 0.35f;'''

assert target in c, "target not found"
c = c.replace(target, replacement, 1)

with open("CustomEngine/native/GameEngine.cpp", "w", encoding="utf-8") as f:
    f.write(c)

print("Fixed editor declaration order!")
