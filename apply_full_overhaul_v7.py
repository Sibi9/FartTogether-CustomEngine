import os

print("Writing apply_full_overhaul_v7.py...")

with open("CustomEngine/native/GameEngine.cpp", "r", encoding="utf-8") as f:
    code = f.read()

# Let's inspect where to insert the JSON helpers and Editor structs
# We can place them right above main() or after KineticObstacle
insert_helpers = """
// =============================================================================
// Level Editor & JSON Obstacle Persistence (Unity / Godot Style)
// =============================================================================
static const char* LEVEL_FILE_PATH = "CustomEngine/level_obstacles.json";

void SaveObstaclesJSON(const std::string& path, const std::vector<KineticObstacle>& list) {
    FILE* f = fopen(path.c_str(), "w");
    if (!f) return;
    fprintf(f, "[\\n");
    for (size_t i = 0; i < list.size(); i++) {
        const auto& o = list[i];
        fprintf(f, "  { \\"type\\": %d, \\"x\\": %.3f, \\"y\\": %.3f, \\"z\\": %.3f, \\"rotY\\": %.2f, \\"speed\\": %.2f, \\"radius\\": %.2f }%s\\n",
            (int)o.type, o.position.x, o.position.y, o.position.z, o.rotationY, o.animSpeed, o.radius,
            (i + 1 < list.size()) ? "," : "");
    }
    fprintf(f, "]\\n");
    fclose(f);
}

bool LoadObstaclesJSON(const std::string& path, std::vector<KineticObstacle>& list,
                       Model* mA1, Model* mB5, Model* mB2, Model* mC2, Model* mF10, Model* mF3, Model* mR1, Model* mR2) {
    FILE* f = fopen(path.c_str(), "r");
    if (!f) return false;
    list.clear();
    char line[512];
    while (fgets(line, sizeof(line), f)) {
        int typeInt = 0;
        float x = 0, y = 0, z = 0, rotY = 0, speed = 1.0f, rad = 2.0f;
        if (strstr(line, "\\"type\\"")) {
            if (sscanf(line, " { \\"type\\": %d, \\"x\\": %f, \\"y\\": %f, \\"z\\": %f, \\"rotY\\": %f, \\"speed\\": %f, \\"radius\\": %f }",
                       &typeInt, &x, &y, &z, &rotY, &speed, &rad) >= 4) {
                KineticObstacle o;
                o.type = (ObstacleType)typeInt;
                o.position = Vector3{ x, y, z };
                o.rotationY = rotY;
                o.animTimer = 0.0f;
                o.animSpeed = speed;
                o.radius = rad;
                o.bumperExpandTimer = 0.0f;
                o.bumperCooldownTimer = 0.0f;
                o.isExpanded = false;

                if (o.type == OBS_BUMPER_SPRING) o.model = mA1;
                else if (o.type == OBS_ROTOR_B5) o.model = mB5;
                else if (o.type == OBS_TIPPING_B2) o.model = mB2;
                else if (o.type == OBS_PENDULUM_C2) o.model = mC2;
                else if (o.type == OBS_GATE_F10) o.model = mF10;
                else if (o.type == OBS_SWING_F3) o.model = mF3;
                else if (o.type == OBS_ROCK1) o.model = mR1;
                else if (o.type == OBS_ROCK2) o.model = mR2;
                else o.model = mA1;

                list.push_back(o);
            }
        }
    }
    fclose(f);
    return !list.empty();
}
"""

# Check if JSON helpers already exist
if "SaveObstaclesJSON" not in code:
    idx_main = code.find("int main(int argc, char** argv)")
    code = code[:idx_main] + insert_helpers + "\n" + code[idx_main:]

with open("CustomEngine/native/GameEngine.cpp", "w", encoding="utf-8") as f:
    f.write(code)

print("Added JSON helpers successfully!")

