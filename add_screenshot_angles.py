with open(r"CustomEngine\native\GameEngine.cpp", "r", encoding="utf-8") as f:
    code = f.read()

old_args = """int main(int argc, char** argv) {
    bool testScreenshotMode = false;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--test-screenshot") == 0) testScreenshotMode = true;
    }"""

new_args = """int main(int argc, char** argv) {
    bool testScreenshotMode = false;
    std::string testShotName = "CustomEngine/scratch/active_gameplay.png";
    int testAngleMode = 0; // 0=behind, 1=side, 2=front, 3=top
    int targetTestFrame = 60;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--test-screenshot") == 0) testScreenshotMode = true;
        if (strcmp(argv[i], "--shot-name") == 0 && i + 1 < argc) testShotName = argv[++i];
        if (strcmp(argv[i], "--shot-angle") == 0 && i + 1 < argc) {
            std::string a = argv[++i];
            if (a == "side") testAngleMode = 1;
            else if (a == "front") testAngleMode = 2;
            else if (a == "top") testAngleMode = 3;
            else testAngleMode = 0;
        }
        if (strcmp(argv[i], "--shot-frames") == 0 && i + 1 < argc) targetTestFrame = atoi(argv[++i]);
    }"""

code = code.replace(old_args, new_args)

old_cam_override = """        // Camera Follow with Trauma Shake"""

new_cam_override = """        // Test Camera Overrides
        if (testScreenshotMode) {
            if (testAngleMode == 1) { // Side Profile
                camCtrl.yaw = raft.rotationY + 1.45f;
                camCtrl.pitch = 0.22f;
                camCtrl.distance = 5.2f;
            } else if (testAngleMode == 2) { // Front Facing
                camCtrl.yaw = raft.rotationY + 3.14159f;
                camCtrl.pitch = 0.18f;
                camCtrl.distance = 5.6f;
            } else if (testAngleMode == 3) { // Top / Action view
                camCtrl.yaw = raft.rotationY;
                camCtrl.pitch = 0.58f;
                camCtrl.distance = 7.5f;
            }
        }

        // Camera Follow with Trauma Shake"""

code = code.replace(old_cam_override, new_cam_override)

old_shot_take = """        testFrameCount++;
        if (testScreenshotMode && testFrameCount == 56) {
            TakeScreenshot("CustomEngine/scratch/active_gameplay.png");
            break;
        }"""

new_shot_take = """        testFrameCount++;
        if (testScreenshotMode && testFrameCount >= targetTestFrame) {
            TakeScreenshot(testShotName.c_str());
            break;
        }"""

code = code.replace(old_shot_take, new_shot_take)

with open(r"CustomEngine\native\GameEngine.cpp", "w", encoding="utf-8") as f:
    f.write(code)

print("Added multi-angle camera support to GameEngine.cpp")

