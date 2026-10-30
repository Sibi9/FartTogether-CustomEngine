import re

path = "CustomEngine/native/GameEngine.cpp"
with open(path, "r", encoding="utf-8") as f:
    code = f.read()

print("Initial code length:", len(code))

# -------------------------------------------------------------------------
# 1. Update BUMPER_IMPACT_RADIUS from 5.0f to 1.85f (tight physical contact)
# -------------------------------------------------------------------------
code = code.replace(
    "static const float BUMPER_IMPACT_RADIUS = 5.0f;",
    "static const float BUMPER_IMPACT_RADIUS = 1.85f; // Authentic physical touch (raft radius 1.25m + pad 0.6m)"
)

# -------------------------------------------------------------------------
# 2. Add DEATH_SUICIDE to DeathCause
# -------------------------------------------------------------------------
old_death_cause = """enum DeathCause {
    DEATH_NONE = 0,
    DEATH_CANAL_WALL = 1,
    DEATH_OBSTACLE = 2,
    DEATH_CROCODILE = 3,
    DEATH_SHARK = 4,
    DEATH_VOID = 5
};"""

new_death_cause = """enum DeathCause {
    DEATH_NONE = 0,
    DEATH_CANAL_WALL = 1,
    DEATH_OBSTACLE = 2,
    DEATH_CROCODILE = 3,
    DEATH_SHARK = 4,
    DEATH_VOID = 5,
    DEATH_SUICIDE = 6
};"""

code = code.replace(old_death_cause, new_death_cause)

# -------------------------------------------------------------------------
# 3. Replace ObstacleType and KineticObstacle with unified LevelEntity
# -------------------------------------------------------------------------
old_obs_struct = """// Kinetic Obstacles & Spring Bumpers
enum ObstacleType {
    OBS_ROTOR_B5 = 0,
    OBS_TIPPING_B2,
    OBS_PENDULUM_C2,
    OBS_GATE_F10,
    OBS_SWING_F3,
    OBS_BUMPER_SPRING,// Authentic 3D Spring Bumper (30 deg launch)
    OBS_ROCK1,
    OBS_ROCK2
};

struct KineticObstacle {
    ObstacleType type;
    Vector3 position;
    float rotationY;
    float animTimer;
    float animSpeed;
    float radius;
    Model* model;
    float bumperCooldownTimer;
    float bumperExpandTimer;
    bool isExpanded;
};"""

new_obs_struct = """// Unified Level Entity Architecture (Obstacles, Rocks, Trees, Checkpoints, Props)
enum EntityType {
    ENT_BUMPER = 0,
    ENT_ROTOR,
    ENT_TIPPING,
    ENT_PENDULUM,
    ENT_GATE,
    ENT_SWING,
    ENT_ROCK_DARK1,
    ENT_ROCK_DARK2,
    ENT_TREE_JUNGLE,
    ENT_CHECKPOINT,
    ENT_LILY_PAD,
    ENT_REEDS,
    ENT_TYPE_COUNT
};

static const char* ENTITY_TYPE_NAMES[] = {
    "BUMPER_SPRING",
    "ROTOR_B5",
    "TIPPING_B2",
    "PENDULUM_C2",
    "GATE_F10",
    "SWING_F3",
    "ROCK_DARK1",
    "ROCK_DARK2",
    "TREE_JUNGLE",
    "CHECKPOINT",
    "LILY_PAD",
    "REEDS"
};

struct LevelEntity {
    int id;
    EntityType type;
    Vector3 position;
    float rotationY;
    float scale;
    float radius;
    float animSpeed;
    float animTimer;
    bool isExpanded;
    float bumperExpandTimer;
    float bumperCooldownTimer;
    Model* model;
    bool passed;
};"""

code = code.replace(old_obs_struct, new_obs_struct)

# -------------------------------------------------------------------------
# 4. Replace SaveObstaclesJSON / LoadObstaclesJSON with robust LevelEntity persistence
# -------------------------------------------------------------------------
old_save_load = """static const char* LEVEL_FILE_PATH = "CustomEngine/level_obstacles.json";

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
}"""

new_save_load = """static const char* LEVEL_LAYOUT_PATH = "CustomEngine/level_layout.json";

void AssignEntityModelAndProps(LevelEntity& e, Model* mA1, Model* mB5, Model* mB2, Model* mC2, Model* mF10, Model* mF3, Model* mR1, Model* mR2, Model* mTree, Model* mCP) {
    if (e.scale <= 0.01f) e.scale = 1.0f;
    if (e.radius <= 0.01f) e.radius = 2.0f;
    if (e.animSpeed <= 0.01f) e.animSpeed = 1.0f;

    switch (e.type) {
        case ENT_BUMPER:      e.model = mA1;   e.radius = 1.85f; break;
        case ENT_ROTOR:       e.model = mB5;   e.radius = 2.40f; break;
        case ENT_TIPPING:     e.model = mB2;   e.radius = 2.20f; break;
        case ENT_PENDULUM:    e.model = mC2;   e.radius = 2.20f; break;
        case ENT_GATE:        e.model = mF10;  e.radius = 2.20f; break;
        case ENT_SWING:       e.model = mF3;   e.radius = 2.20f; break;
        case ENT_ROCK_DARK1:  e.model = mR1;   e.radius = 2.20f; break;
        case ENT_ROCK_DARK2:  e.model = mR2;   e.radius = 2.40f; break;
        case ENT_TREE_JUNGLE: e.model = mTree; e.radius = 1.60f; break;
        case ENT_CHECKPOINT:  e.model = mCP;   e.radius = 4.00f; break;
        case ENT_LILY_PAD:    e.model = nullptr; e.radius = 1.0f; break;
        case ENT_REEDS:       e.model = nullptr; e.radius = 0.8f; break;
        default:              e.model = mA1;   e.radius = 2.00f; break;
    }
}

void SaveLevelLayoutJSON(const std::string& path, const std::vector<LevelEntity>& list) {
    FILE* f = fopen(path.c_str(), "w");
    if (!f) return;
    fprintf(f, "[\\n");
    for (size_t i = 0; i < list.size(); i++) {
        const auto& e = list[i];
        fprintf(f, "  { \\"type\\": %d, \\"x\\": %.3f, \\"y\\": %.3f, \\"z\\": %.3f, \\"rotY\\": %.2f, \\"scale\\": %.2f, \\"speed\\": %.2f, \\"radius\\": %.2f }%s\\n",
            (int)e.type, e.position.x, e.position.y, e.position.z, e.rotationY, e.scale, e.animSpeed, e.radius,
            (i + 1 < list.size()) ? "," : "");
    }
    fprintf(f, "]\\n");
    fclose(f);
}

bool LoadLevelLayoutJSON(const std::string& path, std::vector<LevelEntity>& list,
                         Model* mA1, Model* mB5, Model* mB2, Model* mC2, Model* mF10, Model* mF3, Model* mR1, Model* mR2, Model* mTree, Model* mCP) {
    FILE* f = fopen(path.c_str(), "r");
    if (!f) return false;
    list.clear();
    char line[1024];
    while (fgets(line, sizeof(line), f)) {
        if (!strstr(line, "\\"type\\"")) continue;
        LevelEntity e;
        memset(&e, 0, sizeof(e));
        e.scale = 1.0f;
        e.animSpeed = 1.0f;
        e.radius = 2.0f;

        char* p = strstr(line, "\\"type\\":");
        if (p) sscanf(p + 7, "%d", (int*)&e.type);

        p = strstr(line, "\\"x\\":");
        if (p) sscanf(p + 4, "%f", &e.position.x);

        p = strstr(line, "\\"y\\":");
        if (p) sscanf(p + 4, "%f", &e.position.y);

        p = strstr(line, "\\"z\\":");
        if (p) sscanf(p + 4, "%f", &e.position.z);

        p = strstr(line, "\\"rotY\\":");
        if (p) sscanf(p + 7, "%f", &e.rotationY);

        p = strstr(line, "\\"scale\\":");
        if (p) sscanf(p + 8, "%f", &e.scale);

        p = strstr(line, "\\"speed\\":");
        if (p) sscanf(p + 8, "%f", &e.animSpeed);

        p = strstr(line, "\\"radius\\":");
        if (p) sscanf(p + 9, "%f", &e.radius);

        e.id = (int)list.size();
        AssignEntityModelAndProps(e, mA1, mB5, mB2, mC2, mF10, mF3, mR1, mR2, mTree, mCP);
        list.push_back(e);
    }
    fclose(f);
    return !list.empty();
}"""

code = code.replace(old_save_load, new_save_load)

# -------------------------------------------------------------------------
# 5. Load bgr_default_tex.png and GM_3D_JungleTree.obj and apply to env models
# -------------------------------------------------------------------------
old_tex_bgr_load = """    Texture2D texRaftFoam = LoadTexture(ResolveAsset("extracted_assets/textures/Raft-Foam4.png").c_str());
    Texture2D texHipo       = LoadTexture(ResolveAsset("extracted_assets/ui/Character_GameGreen.png").c_str());"""

new_tex_bgr_load = """    Texture2D texRaftFoam   = LoadTexture(ResolveAsset("extracted_assets/textures/Raft-Foam4.png").c_str());
    Texture2D texHipo       = LoadTexture(ResolveAsset("extracted_assets/ui/Character_GameGreen.png").c_str());
    Texture2D texBgrDefault = LoadTexture(ResolveAsset("extracted_assets/textures/bgr_default_tex.png").c_str());
    SetTextureFilter(texBgrDefault, TEXTURE_FILTER_BILINEAR);"""

code = code.replace(old_tex_bgr_load, new_tex_bgr_load)

# Add tree model loading
old_tree_load = """    Model obsModelRock2= LoadModel(ResolveAsset("extracted_assets/meshes/GM_3D_Obstacle_Rock2.obj").c_str());"""
new_tree_load = """    Model obsModelRock2= LoadModel(ResolveAsset("extracted_assets/meshes/GM_3D_Obstacle_Rock2.obj").c_str());
    Model jungleTreeModel = LoadModel(ResolveAsset("extracted_assets/meshes/GM_3D_JungleTree.obj").c_str());"""

code = code.replace(old_tree_load, new_tree_load)

# Material application: apply texBgrDefault to environmental models (rocks, trees, canyon walls, checkpoints)
old_mat_app = """    ApplyMat(wallModel);
    ApplyMat(startModel);
    ApplyMat(finishModel);
    ApplyMat(cp1Model);
    ApplyMat(cp2Model);
    ApplyMat(cp3Model);
    ApplyCharMat(boatModel);
    ApplyCharMat(charA);
    ApplyCharMat(charB);
    ApplyCharMat(hat1);
    ApplyCharMat(shirt);
    ApplyCharMat(pant);
    ApplyCharMat(shoes);
    ApplyMat(obsModelA1);
    ApplyMat(obsModelB5);
    ApplyMat(obsModelB2);
    ApplyMat(obsModelC2);
    ApplyMat(obsModelF10);
    ApplyMat(obsModelF3);
    ApplyMat(obsModelRock1);
    ApplyMat(obsModelRock2);
    ApplyMat(crocModel);
    ApplyMat(sharkModel);
    ApplyMat(envSector3);
    ApplyMat(envSector2);"""

new_mat_app = """    auto ApplyBgrMat = [&](Model& m) {
        for (int i = 0; i < m.materialCount; i++) {
            m.materials[i].maps[MATERIAL_MAP_DIFFUSE].texture = texBgrDefault;
            m.materials[i].shader = shaderLighting;
        }
    };
    ApplyBgrMat(wallModel);
    ApplyBgrMat(startModel);
    ApplyBgrMat(finishModel);
    ApplyBgrMat(cp1Model);
    ApplyBgrMat(cp2Model);
    ApplyBgrMat(cp3Model);
    ApplyBgrMat(envSector3);
    ApplyBgrMat(envSector2);
    ApplyBgrMat(obsModelRock1); // Dark grey stone (Image 2 Parity)
    ApplyBgrMat(obsModelRock2); // Dark grey stone (Image 2 Parity)
    ApplyBgrMat(jungleTreeModel); // Green foliage & brown bark (Image 3 Parity)

    ApplyCharMat(boatModel);
    ApplyCharMat(charA);
    ApplyCharMat(charB);
    ApplyCharMat(hat1);
    ApplyCharMat(shirt);
    ApplyCharMat(pant);
    ApplyCharMat(shoes);

    ApplyMat(obsModelA1);
    ApplyMat(obsModelB5);
    ApplyMat(obsModelB2);
    ApplyMat(obsModelC2);
    ApplyMat(obsModelF10);
    ApplyMat(obsModelF3);
    ApplyMat(crocModel);
    ApplyMat(sharkModel);"""

code = code.replace(old_mat_app, new_mat_app)

# -------------------------------------------------------------------------
# 6. Initialize default levelEntities list (with rocks, trees, obstacles, checkpoints)
# -------------------------------------------------------------------------
old_obs_init = """    // Authentic Flume Obstacles: Pairs of Purple Spiked Bumpers (GM_3D_ObstacleA1) lining canal walls

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

    // Load custom user obstacle layout from JSON if it exists, otherwise create it
    if (!LoadObstaclesJSON(LEVEL_FILE_PATH, obsList, &obsModelA1, &obsModelB5, &obsModelB2, &obsModelC2, &obsModelF10, &obsModelF3, &obsModelRock1, &obsModelRock2)) {
        SaveObstaclesJSON(LEVEL_FILE_PATH, obsList);
    }"""

new_obs_init = """    // Unified Level Entities (Fully editable in 3D Level Editor)
    std::vector<LevelEntity> levelEntities = {
        // 1. Spiked Bumper Pairs along Canal (Z = 66.6, 59.1, 51.7, 44.2)
        { 0, ENT_BUMPER, Vector3{ 14.45f, 0.65f, 66.65f },    0.0f, 1.0f, 1.85f, 1.0f, 0.0f, false, 0.0f, 0.0f, &obsModelA1, false },
        { 1, ENT_BUMPER, Vector3{ 19.75f, 0.65f, 66.65f },  180.0f, 1.0f, 1.85f, 1.0f, 0.0f, false, 0.0f, 0.0f, &obsModelA1, false },
        { 2, ENT_BUMPER, Vector3{ 16.83f, 0.65f, 59.10f },    0.0f, 1.0f, 1.85f, 1.0f, 0.0f, false, 0.0f, 0.0f, &obsModelA1, false },
        { 3, ENT_BUMPER, Vector3{ 22.13f, 0.65f, 59.10f },  180.0f, 1.0f, 1.85f, 1.0f, 0.0f, false, 0.0f, 0.0f, &obsModelA1, false },
        { 4, ENT_BUMPER, Vector3{ 21.47f, 0.65f, 51.71f },    0.0f, 1.0f, 1.85f, 1.0f, 0.0f, false, 0.0f, 0.0f, &obsModelA1, false },
        { 5, ENT_BUMPER, Vector3{ 26.77f, 0.65f, 51.71f },  180.0f, 1.0f, 1.85f, 1.0f, 0.0f, false, 0.0f, 0.0f, &obsModelA1, false },
        { 6, ENT_BUMPER, Vector3{ 25.91f, 0.65f, 44.20f },    0.0f, 1.0f, 1.85f, 1.0f, 0.0f, false, 0.0f, 0.0f, &obsModelA1, false },
        { 7, ENT_BUMPER, Vector3{ 31.21f, 0.65f, 44.20f },  180.0f, 1.0f, 1.85f, 1.0f, 0.0f, false, 0.0f, 0.0f, &obsModelA1, false },

        // 2. Downstream Mechanical Hazards
        { 8,  ENT_SWING,     Vector3{ 36.48f, 0.5f, 34.44f },     0.0f, 1.0f, 2.20f, 2.5f, 0.0f, false, 0.0f, 0.0f, &obsModelF3, false },
        { 9,  ENT_GATE,      Vector3{ 41.50f, 0.8f, 10.67f },    0.0f, 1.0f, 2.20f, 1.8f, 0.0f, false, 0.0f, 0.0f, &obsModelF10, false },
        { 10, ENT_ROTOR,     Vector3{ -220.31f, 10.2f, -103.11f },0.0f, 1.0f, 2.40f, 2.2f, 0.0f, false, 0.0f, 0.0f, &obsModelB5, false },
        { 11, ENT_TIPPING,   Vector3{ -323.30f, 13.6f, -103.11f },0.0f, 1.0f, 2.20f, 1.6f, 0.0f, false, 0.0f, 0.0f, &obsModelB2, false },
        { 12, ENT_PENDULUM,  Vector3{ -400.18f, 15.3f, -103.11f },0.0f, 1.0f, 2.20f, 2.0f, 0.0f, false, 0.0f, 0.0f, &obsModelC2, false },

        // 3. Authentic Dark Grey Rocks (Image 2 Parity)
        { 13, ENT_ROCK_DARK1, Vector3{ 27.50f, 0.35f, 96.50f },  -35.0f, 1.15f, 2.40f, 0.0f, 0.0f, false, 0.0f, 0.0f, &obsModelRock1, false },
        { 14, ENT_ROCK_DARK2, Vector3{ 4.20f,  0.35f, 92.10f },   40.0f, 1.30f, 2.60f, 0.0f, 0.0f, false, 0.0f, 0.0f, &obsModelRock2, false },
        { 15, ENT_ROCK_DARK1, Vector3{ 3.10f,  0.35f, 82.50f },   15.0f, 1.00f, 2.20f, 0.0f, 0.0f, false, 0.0f, 0.0f, &obsModelRock1, false },
        { 16, ENT_ROCK_DARK2, Vector3{ 26.80f, 0.35f, 79.20f },  -70.0f, 1.25f, 2.50f, 0.0f, 0.0f, false, 0.0f, 0.0f, &obsModelRock2, false },

        // 4. Lush Jungle Trees (Green Foliage & Brown Bark - Image 3 Parity)
        { 17, ENT_TREE_JUNGLE, Vector3{ 29.50f, 0.40f, 98.20f },  20.0f, 1.2f, 1.80f, 0.0f, 0.0f, false, 0.0f, 0.0f, &jungleTreeModel, false },
        { 18, ENT_TREE_JUNGLE, Vector3{ 1.80f,  0.40f, 94.50f }, -45.0f, 1.3f, 1.90f, 0.0f, 0.0f, false, 0.0f, 0.0f, &jungleTreeModel, false },
        { 19, ENT_TREE_JUNGLE, Vector3{ 32.20f, 0.40f, 62.10f },  10.0f, 1.1f, 1.70f, 0.0f, 0.0f, false, 0.0f, 0.0f, &jungleTreeModel, false },
        { 20, ENT_TREE_JUNGLE, Vector3{ 2.50f,  0.40f, 55.40f },  60.0f, 1.4f, 2.00f, 0.0f, 0.0f, false, 0.0f, 0.0f, &jungleTreeModel, false },

        // 5. Water Props: Lily Pads & Reeds in Start Pond (Image 4 Parity)
        { 21, ENT_LILY_PAD,   Vector3{ 10.50f, 0.38f, 97.20f },   0.0f, 1.0f, 1.20f, 0.0f, 0.0f, false, 0.0f, 0.0f, nullptr, false },
        { 22, ENT_LILY_PAD,   Vector3{ 21.00f, 0.38f, 83.50f },   0.0f, 1.2f, 1.30f, 0.0f, 0.0f, false, 0.0f, 0.0f, nullptr, false },
        { 23, ENT_REEDS,      Vector3{ 25.80f, 0.38f, 91.20f },   0.0f, 1.0f, 0.90f, 0.0f, 0.0f, false, 0.0f, 0.0f, nullptr, false },
        { 24, ENT_REEDS,      Vector3{ 5.50f,  0.38f, 87.00f },   0.0f, 1.0f, 0.90f, 0.0f, 0.0f, false, 0.0f, 0.0f, nullptr, false }
    };

    // Load persistent layout from JSON if available
    std::string persistentLayoutPath = ResolveAsset("level_layout.json");
    if (!LoadLevelLayoutJSON(persistentLayoutPath, levelEntities, &obsModelA1, &obsModelB5, &obsModelB2, &obsModelC2, &obsModelF10, &obsModelF3, &obsModelRock1, &obsModelRock2, &jungleTreeModel, &cp1Model)) {
        SaveLevelLayoutJSON(persistentLayoutPath, levelEntities);
    }"""

code = code.replace(old_obs_init, new_obs_init)

# -------------------------------------------------------------------------
# 7. Update TriggerBoatDeath to always increment totalDeaths++
# -------------------------------------------------------------------------
old_trigger_death = """    auto TriggerBoatDeath = [&](DeathCause cause) {

        if (raft.invulnerableTimer > 0.0f || raft.isDestroyed) return;

        raft.isDestroyed = true;

        raft.respawnTimer = 2.0f;

        raft.deaths++;

        raft.lastDeathCause = cause;"""

new_trigger_death = """    auto TriggerBoatDeath = [&](DeathCause cause) {

        if (raft.invulnerableTimer > 0.0f || raft.isDestroyed) return;

        raft.isDestroyed = true;

        raft.respawnTimer = 2.0f;

        raft.deaths++;
        totalDeaths++; // Live tally increments on every death (Fixing Issue #5)

        raft.lastDeathCause = cause;"""

code = code.replace(old_trigger_death, new_trigger_death)

# -------------------------------------------------------------------------
# 8. Start Pond Water & Orange Ring Barrier Physics
# -------------------------------------------------------------------------
old_river_flow_boundary = """        // Flume Canal Hard Collision vs Open River Containment
        Vector3 toCenter = Vector3Subtract(raft.position, riverCenter);
        float crossDist = Vector3DotProduct(toCenter, riverNormal);

        bool inFlumeCanal = (raft.position.z > 36.0f);
        float currentHalfWidth = inFlumeCanal ? 1.42f : 6.8f;

        if (inFlumeCanal) {
            // Hard canal wall collision: solid barrier cannot be penetrated!
            if (fabsf(crossDist) > currentHalfWidth) {
                float pen = fabsf(crossDist) - currentHalfWidth;
                float pushSign = (crossDist > 0.0f) ? -1.0f : 1.0f;
                raft.position = Vector3Add(raft.position, Vector3Scale(riverNormal, pushSign * pen * 1.05f));
                raft.velocity = Vector3Reflect(raft.velocity, riverNormal);
                raft.velocity = Vector3Scale(raft.velocity, 0.45f); // Absorbs energy
                PlaySound(fartSounds[2]);
                camCtrl.shakeTrauma = fmaxf(camCtrl.shakeTrauma, 0.25f);
            }
        } else {
            // Open river bank soft containment
            if (fabsf(crossDist) > currentHalfWidth) {
                float pen = fabsf(crossDist) - currentHalfWidth;
                float pushSign = (crossDist > 0.0f) ? -1.0f : 1.0f;
                raft.velocity = Vector3Add(raft.velocity, Vector3Scale(riverNormal, pushSign * pen * 36.0f * dt));
            }
        }

        // Dynamic Wave Buoyancy vs Fall Off Cliff Into Abyss
        bool overWater = (fabsf(crossDist) < (inFlumeCanal ? 2.5f : 9.5f));
        if (overWater && !raft.isDestroyed) {
            float waterHeight = riverCenter.y - 0.31f + sinf(raft.position.z * 0.32f - runTime * 1.8f) * 0.05f;
            float heave = waterHeight - raft.position.y;
            raft.velocity.y += (heave * 42.0f - raft.velocity.y * 7.5f) * dt;
        } else {
            // Outside river channel: FALL INTO ABYSS!
            raft.velocity.y -= 32.0f * dt;
            if (raft.position.y < -12.0f && !raft.isDestroyed) {
                totalDeaths++;
                TriggerBoatDeath(DEATH_OBSTACLE);
            }
        }"""

new_river_flow_boundary = """        // Start Arena Lake vs Flume Canal vs Open River (Image 4 Parity)
        Vector3 pondCenter = Vector3{ 15.55f, 0.40f, 90.80f };
        float pondRadius = 16.5f;
        Vector3 toPond = Vector3Subtract(raft.position, pondCenter);
        toPond.y = 0.0f;
        float pondDist = Vector3Length(toPond);
        bool inStartPond = (raft.position.z > 74.0f);

        Vector3 toCenter = Vector3Subtract(raft.position, riverCenter);
        float crossDist = Vector3DotProduct(toCenter, riverNormal);
        bool inFlumeCanal = (raft.position.z <= 74.0f && raft.position.z > 36.0f);

        if (inStartPond) {
            // Circular start pond orange bumper tube containment
            if (pondDist > (pondRadius - 1.25f)) {
                Vector3 pondNorm = Vector3Normalize(toPond);
                float pen = pondDist - (pondRadius - 1.25f);
                raft.position = Vector3Subtract(raft.position, Vector3Scale(pondNorm, pen));
                raft.velocity = Vector3Reflect(raft.velocity, pondNorm);
                raft.velocity = Vector3Scale(raft.velocity, 0.5f);
                camCtrl.shakeTrauma = fmaxf(camCtrl.shakeTrauma, 0.2f);
            }
        } else if (inFlumeCanal) {
            // Solid Flume canal wall collision
            float canalHalfW = 1.42f;
            if (fabsf(crossDist) > canalHalfW) {
                float pen = fabsf(crossDist) - canalHalfW;
                float pushSign = (crossDist > 0.0f) ? -1.0f : 1.0f;
                raft.position = Vector3Add(raft.position, Vector3Scale(riverNormal, pushSign * pen * 1.05f));
                raft.velocity = Vector3Reflect(raft.velocity, riverNormal);
                raft.velocity = Vector3Scale(raft.velocity, 0.45f);
                PlaySound(fartSounds[2]);
                camCtrl.shakeTrauma = fmaxf(camCtrl.shakeTrauma, 0.25f);
            }
        }

        // Authentic Abyss Falling Physics: NO artificial magnetic pull!
        bool overWater = inStartPond ? (pondDist < (pondRadius + 2.0f)) : (inFlumeCanal ? (fabsf(crossDist) < 2.5f) : (fabsf(crossDist) < 8.2f));
        if (overWater && !raft.isDestroyed) {
            float waterHeight = inStartPond ? 0.38f : (riverCenter.y - 0.31f + sinf(raft.position.z * 0.32f - runTime * 1.8f) * 0.05f);
            float heave = waterHeight - raft.position.y;
            raft.velocity.y += (heave * 42.0f - raft.velocity.y * 7.5f) * dt;
        } else {
            // Outside track boundaries: Free fall into canyon abyss with gravity!
            raft.velocity.y -= 32.0f * dt;
            if (raft.position.y < -12.0f && !raft.isDestroyed) {
                TriggerBoatDeath(DEATH_VOID);
            }
        }"""

code = code.replace(old_river_flow_boundary, new_river_flow_boundary)

# -------------------------------------------------------------------------
# 9. Realistic Tight Collisions for LevelEntity (No more 5-meter hits!)
# -------------------------------------------------------------------------
old_obs_collision_block = """        // 5. Update Kinetic Obstacles & Collision Reactions

        for (auto& obs : obsList) {

            obs.animTimer += dt * obs.animSpeed;

            // Expand Bumper Reaction

            if (obs.type == OBS_BUMPER_SPRING) {

                if (obs.bumperCooldownTimer > 0.0f) {

                    obs.bumperCooldownTimer -= dt;

                }

                if (obs.isExpanded) {

                    obs.bumperExpandTimer -= dt;

                    if (obs.bumperExpandTimer <= 0.0f) {

                        obs.isExpanded = false;

                        obs.bumperCooldownTimer = BUMPER_COOLDOWN;

                    }

                }

                // Check boat distance for Spring Bumper

                float d = Vector3Distance(raft.position, obs.position);

                if (d < BUMPER_IMPACT_RADIUS && obs.bumperCooldownTimer <= 0.0f && !raft.isDestroyed) {

                    obs.isExpanded = true;

                    obs.bumperExpandTimer = BUMPER_EXPAND_DURATION;

                    // 1:1 Bumper Launch: 18 m/s horizontal + 8 m/s vertical (30 deg launch)

                    Vector3 bumpDir = Vector3Normalize(Vector3Subtract(raft.position, obs.position));

                    bumpDir.y = 0.0f;

                    if (Vector3LengthSqr(bumpDir) < 0.01f) bumpDir = Vector3{ 0, 0, 1 };

                    raft.velocity = Vector3Scale(bumpDir, BUMPER_HORIZ_FORCE);

                    raft.velocity.y = BUMPER_UPWARD_FORCE;

                    raft.health -= 20.0f;

                    camCtrl.shakeTrauma = 0.5f; // 0.5 trauma from BumperObstacle config

                    PlaySound(fartSounds[3]);

                    EmitFartBurst(raft.position, bumpDir, 0.8f);

                    if (raft.health <= 0.0f) {

                        TriggerBoatDeath(DEATH_OBSTACLE);

                    }

                }

            } else {

                // Standard Kinetic Obstacles

                float d = Vector3Distance(raft.position, obs.position);

                if (d < (obs.radius + 1.2f) && raft.invulnerableTimer <= 0.0f && !raft.isDestroyed) {

                    Vector3 bounce = Vector3Normalize(Vector3Subtract(raft.position, obs.position));

                    bounce.y = 0.0f;

                    raft.velocity = Vector3Scale(bounce, 15.0f);

                    raft.health -= 25.0f;

                    raft.invulnerableTimer = 1.5f;

                    camCtrl.shakeTrauma = 0.4f;

                    PlaySound(fartSounds[4]);

                    if (raft.health <= 0.0f) {

                        TriggerBoatDeath(DEATH_OBSTACLE);

                    }

                }

            }

        }"""

new_obs_collision_block = """        // 5. Update Kinetic Obstacles & Realistic Tight Collision Reactions
        for (auto& e : levelEntities) {
            e.animTimer += dt * e.animSpeed;

            if (e.type == ENT_BUMPER) {
                if (e.bumperCooldownTimer > 0.0f) e.bumperCooldownTimer -= dt;
                if (e.isExpanded) {
                    e.bumperExpandTimer -= dt;
                    if (e.bumperExpandTimer <= 0.0f) {
                        e.isExpanded = false;
                        e.bumperCooldownTimer = BUMPER_COOLDOWN;
                    }
                }

                // Tight Physical Contact Check (1.85m touch distance)
                float d = Vector3Distance(raft.position, e.position);
                if (d < BUMPER_IMPACT_RADIUS && e.bumperCooldownTimer <= 0.0f && !raft.isDestroyed) {
                    e.isExpanded = true;
                    e.bumperExpandTimer = BUMPER_EXPAND_DURATION;

                    Vector3 bumpDir = Vector3Normalize(Vector3Subtract(raft.position, e.position));
                    bumpDir.y = 0.0f;
                    if (Vector3LengthSqr(bumpDir) < 0.01f) bumpDir = Vector3{ 0, 0, 1 };

                    raft.velocity = Vector3Scale(bumpDir, BUMPER_HORIZ_FORCE);
                    raft.velocity.y = BUMPER_UPWARD_FORCE;
                    raft.health -= 20.0f;
                    camCtrl.shakeTrauma = 0.5f;

                    PlaySound(fartSounds[3]);
                    EmitFartBurst(raft.position, bumpDir, 0.8f);

                    if (raft.health <= 0.0f) TriggerBoatDeath(DEATH_OBSTACLE);
                }
            } else if (e.type == ENT_ROTOR || e.type == ENT_TIPPING || e.type == ENT_PENDULUM || e.type == ENT_GATE || e.type == ENT_SWING) {
                // Tight physical touch for mechanical hazards
                float d = Vector3Distance(raft.position, e.position);
                float hitDist = (e.radius * 0.42f) + 1.15f;
                if (d < hitDist && raft.invulnerableTimer <= 0.0f && !raft.isDestroyed) {
                    Vector3 bounce = Vector3Normalize(Vector3Subtract(raft.position, e.position));
                    bounce.y = 0.0f;
                    raft.velocity = Vector3Scale(bounce, 15.0f);
                    raft.health -= 25.0f;
                    raft.invulnerableTimer = 1.5f;
                    camCtrl.shakeTrauma = 0.4f;
                    PlaySound(fartSounds[4]);
                    if (raft.health <= 0.0f) TriggerBoatDeath(DEATH_OBSTACLE);
                }
            } else if (e.type == ENT_ROCK_DARK1 || e.type == ENT_ROCK_DARK2) {
                // Solid rock collision: physical deflection
                float d = Vector3Distance(raft.position, e.position);
                float rockDist = (e.radius * 0.45f) + 1.10f;
                if (d < rockDist && !raft.isDestroyed) {
                    Vector3 rBounce = Vector3Normalize(Vector3Subtract(raft.position, e.position));
                    rBounce.y = 0.0f;
                    raft.position = Vector3Add(e.position, Vector3Scale(rBounce, rockDist));
                    raft.velocity = Vector3Reflect(raft.velocity, rBounce);
                    raft.velocity = Vector3Scale(raft.velocity, 0.4f);
                    camCtrl.shakeTrauma = fmaxf(camCtrl.shakeTrauma, 0.25f);
                    PlaySound(fartSounds[1]);
                }
            } else if (e.type == ENT_CHECKPOINT) {
                float d = Vector3Distance(raft.position, e.position);
                if (d < 7.0f && !e.passed) {
                    e.passed = true;
                    raft.currentCP++;
                    PlaySound(fartSounds[0]);
                }
            }
        }"""

code = code.replace(old_obs_collision_block, new_obs_collision_block)

# -------------------------------------------------------------------------
# 10. Fix Predators flying in the sky (Clamp Y strictly to water level)
# -------------------------------------------------------------------------
old_croc_update = """                croc.position = Vector3Add(croc.position, Vector3Scale(toRaft, croc.lungeSpeed * dt));

                croc.rotationY = atan2f(toRaft.x, toRaft.z);"""

new_croc_update = """                croc.position = Vector3Add(croc.position, Vector3Scale(toRaft, croc.lungeSpeed * dt));
                croc.rotationY = atan2f(toRaft.x, toRaft.z);
                // Clamp crocodile Y strictly to water surface (Never fly into sky!)
                float cWaterY = (croc.position.z > 74.0f) ? 0.38f : (GetSplinePoint(FindClosestSplineT(croc.position)).y - 0.12f);
                croc.position.y = cWaterY;"""

code = code.replace(old_croc_update, new_croc_update)

old_shark_update = """                shark.position = Vector3Add(shark.position, Vector3Scale(toRaft, SHARK_CHASE_SPEED * dt));

                shark.rotationY = atan2f(toRaft.x, toRaft.z);"""

new_shark_update = """                shark.position = Vector3Add(shark.position, Vector3Scale(toRaft, SHARK_CHASE_SPEED * dt));
                shark.rotationY = atan2f(toRaft.x, toRaft.z);
                // Clamp shark Y strictly to water surface (Never fly into sky!)
                float sWaterY = (shark.position.z > 74.0f) ? 0.38f : (GetSplinePoint(FindClosestSplineT(shark.position)).y - 0.15f);
                shark.position.y = sWaterY;"""

code = code.replace(old_shark_update, new_shark_update)

# Also clamp during patrol / return:
code = code.replace(
    "croc.position = Vector3Add(croc.position, Vector3Scale(dir, 1.5f * dt));",
    """croc.position = Vector3Add(croc.position, Vector3Scale(dir, 1.5f * dt));
                    float cWaterY = (croc.position.z > 74.0f) ? 0.38f : (GetSplinePoint(FindClosestSplineT(croc.position)).y - 0.12f);
                    croc.position.y = cWaterY;"""
)

code = code.replace(
    "shark.position = Vector3Add(shark.position, Vector3Scale(dir, SHARK_PATROL_SPEED * dt));",
    """shark.position = Vector3Add(shark.position, Vector3Scale(dir, SHARK_PATROL_SPEED * dt));
                    float sWaterY = (shark.position.z > 74.0f) ? 0.38f : (GetSplinePoint(FindClosestSplineT(shark.position)).y - 0.15f);
                    shark.position.y = sWaterY;"""
)

# -------------------------------------------------------------------------
# 11. Add [R] SUICIDE button and handler (Image 4 Parity)
# -------------------------------------------------------------------------
old_leave_btn = """        // 4. Bottom-Left: [ESC] LEAVE Button
        {
            Rectangle btnLeaveRec = { 20, (float)GetScreenHeight() - 48, 115, 34 };
            bool hoverLeave = CheckCollisionPointRec(mousePos, btnLeaveRec);
            DrawRectangleRounded(btnLeaveRec, 0.35f, 4, hoverLeave ? Color{ 30, 41, 59, 240 } : Color{ 15, 23, 42, 210 });
            DrawRectangleRoundedLinesEx(btnLeaveRec, 0.35f, 4, 1.2f, hoverLeave ? Color{ 148, 163, 184, 255 } : Color{ 71, 85, 105, 180 });

            DrawTexturePro(texEsc, Rectangle{ 0, 0, (float)texEsc.width, (float)texEsc.height },
                           Rectangle{ 26, (float)GetScreenHeight() - 43, 24, 24 }, Vector2{ 0, 0 }, 0.0f, WHITE);
            DrawText("LEAVE", 58, GetScreenHeight() - 40, 16, RAYWHITE);
        }"""

new_leave_suicide_btn = """        // 4. Bottom-Left: [ESC] LEAVE & [R] SUICIDE Buttons (Image 4 Parity)
        {
            Rectangle btnLeaveRec = { 20, (float)GetScreenHeight() - 44, 105, 32 };
            bool hoverLeave = CheckCollisionPointRec(mousePos, btnLeaveRec);
            DrawRectangleRounded(btnLeaveRec, 0.3f, 4, hoverLeave ? Color{ 45, 52, 66, 240 } : Color{ 25, 29, 38, 220 });
            DrawRectangleRoundedLinesEx(btnLeaveRec, 0.3f, 4, 1.0f, Color{ 71, 85, 105, 180 });
            DrawTexturePro(texEsc, Rectangle{ 0, 0, (float)texEsc.width, (float)texEsc.height },
                           Rectangle{ 26, (float)GetScreenHeight() - 39, 22, 22 }, Vector2{ 0, 0 }, 0.0f, WHITE);
            DrawTextEx(fontUIBold, "LEAVE", Vector2{ 54, (float)GetScreenHeight() - 36 }, 13, 0.3f, RAYWHITE);

            // [R] SUICIDE Button (Image 4 Parity)
            Rectangle btnSuicideRec = { 135, (float)GetScreenHeight() - 44, 115, 32 };
            bool hoverSuicide = CheckCollisionPointRec(mousePos, btnSuicideRec);
            if (!editorMode && ((hoverSuicide && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) || IsKeyPressed(KEY_R))) {
                TriggerBoatDeath(DEATH_SUICIDE);
            }
            DrawRectangleRounded(btnSuicideRec, 0.3f, 4, hoverSuicide ? Color{ 110, 38, 38, 240 } : Color{ 35, 24, 28, 220 });
            DrawRectangleRoundedLinesEx(btnSuicideRec, 0.3f, 4, 1.0f, Color{ 140, 50, 50, 180 });
            DrawRectangleRounded(Rectangle{ btnSuicideRec.x + 6, btnSuicideRec.y + 5, 22, 22 }, 0.25f, 3, Color{ 70, 30, 35, 255 });
            DrawTextEx(fontUIBold, "R", Vector2{ btnSuicideRec.x + 12, btnSuicideRec.y + 8 }, 13, 0.2f, WHITE);
            DrawTextEx(fontUIBold, "SUICIDE", Vector2{ btnSuicideRec.x + 36, btnSuicideRec.y + 9 }, 12, 0.3f, Color{ 254, 202, 202, 255 });
        }"""

code = code.replace(old_leave_btn, new_leave_suicide_btn)

# -------------------------------------------------------------------------
# 12. Draw Circular Start Lake, Lily Pads, Reeds, and All Level Entities
# -------------------------------------------------------------------------
old_draw_scene_entities = """            // C. Kinetic Hazards & Spring Bumpers
            for (size_t i = 0; i < obsList.size(); i++) {
                const auto& obs = obsList[i];
                if (!obs.model) continue;

                rlPushMatrix();
                    rlTranslatef(obs.position.x, obs.position.y, obs.position.z);
                    rlRotatef(obs.rotationY, 0, 1, 0);

                    if (obs.type == OBS_BUMPER_SPRING) {
                        float expandDist = obs.isExpanded ? 1.6f : 0.0f;
                        rlTranslatef(0, 0, expandDist);
                    } else if (obs.type == OBS_ROTOR_B5) {
                        rlRotatef(obs.animTimer * 120.0f, 0, 1, 0);
                    } else if (obs.type == OBS_TIPPING_B2) {
                        float tipAngle = sinf(obs.animTimer * 2.2f) * 28.0f;
                        rlRotatef(tipAngle, 0, 0, 1);
                    } else if (obs.type == OBS_PENDULUM_C2) {
                        float swingAngle = sinf(obs.animTimer * 2.0f) * 45.0f;
                        rlRotatef(swingAngle, 1, 0, 0);
                    } else if (obs.type == OBS_SWING_F3) {
                        float armAngle = sinf(obs.animTimer * 1.8f) * 35.0f;
                        rlRotatef(armAngle, 0, 1, 0);
                    }

                    DrawModel(*obs.model, Vector3{ 0, 0, 0 }, 1.0f, WHITE);
                rlPopMatrix();
            }"""

new_draw_scene_entities = """            // Circular Start Lake Environment (Image 4 Parity)
            {
                Vector3 pCenter = Vector3{ 15.55f, 0.40f, 90.80f };
                float pRad = 16.5f;

                // Circular water pool
                DrawCircle3D(pCenter, pRad, Vector3{ 1, 0, 0 }, 90.0f, Color{ 14, 165, 233, 220 });

                // Outer Green Grass Bank Ring
                for (int seg = 0; seg < 48; seg++) {
                    float a0 = (float)seg / 48.0f * 6.28318f;
                    float a1 = (float)(seg + 1) / 48.0f * 6.28318f;
                    // North canal opening between angle 165 deg and 195 deg
                    if (a0 > 2.75f && a1 < 3.53f) continue;

                    Vector3 p0 = Vector3{ pCenter.x + cosf(a0) * pRad, pCenter.y + 0.15f, pCenter.z + sinf(a0) * pRad };
                    Vector3 p1 = Vector3{ pCenter.x + cosf(a1) * pRad, pCenter.y + 0.15f, pCenter.z + sinf(a1) * pRad };
                    Vector3 p0_out = Vector3{ pCenter.x + cosf(a0) * (pRad + 4.5f), pCenter.y + 0.25f, pCenter.z + sinf(a0) * (pRad + 4.5f) };
                    Vector3 p1_out = Vector3{ pCenter.x + cosf(a1) * (pRad + 4.5f), pCenter.y + 0.25f, pCenter.z + sinf(a1) * (pRad + 4.5f) };

                    DrawTriangle3D(p0, p1, p1_out, Color{ 101, 163, 13, 255 }); // Grass Green (#65a30d)
                    DrawTriangle3D(p0, p1_out, p0_out, Color{ 101, 163, 13, 255 });

                    // Vibrant Orange Rim Bumper Tube
                    DrawLine3D(p0, p1, Color{ 249, 115, 22, 255 }); // Orange Rim (#f97316)
                    DrawLine3D(Vector3Add(p0, Vector3{0, 0.2f, 0}), Vector3Add(p1, Vector3{0, 0.2f, 0}), Color{ 234, 88, 12, 255 });
                }
            }

            // Draw All Level Entities (Obstacles, Dark Rocks, Jungle Trees, Checkpoints, Water Props)
            for (size_t i = 0; i < levelEntities.size(); i++) {
                auto& e = levelEntities[i];

                if (e.type == ENT_LILY_PAD) {
                    // Floating Lily Pad with Pink Lotus Flower (Image 4 Parity)
                    DrawCircle3D(Vector3{ e.position.x, 0.41f, e.position.z }, 0.85f * e.scale, Vector3{ 1, 0, 0 }, 90.0f, Color{ 34, 197, 94, 255 });
                    DrawCircle3D(Vector3{ e.position.x + 0.35f, 0.41f, e.position.z + 0.3f }, 0.55f * e.scale, Vector3{ 1, 0, 0 }, 90.0f, Color{ 22, 163, 74, 255 });
                    // Pink Lotus Flower
                    DrawCircle3D(Vector3{ e.position.x, 0.44f, e.position.z }, 0.22f * e.scale, Vector3{ 1, 0, 0 }, 90.0f, Color{ 244, 114, 182, 255 });
                    DrawCircle3D(Vector3{ e.position.x, 0.45f, e.position.z }, 0.10f * e.scale, Vector3{ 1, 0, 0 }, 90.0f, Color{ 251, 191, 36, 255 });
                    continue;
                }

                if (e.type == ENT_REEDS) {
                    // Cattail Water Reeds (Image 4 Parity)
                    for (int r = -1; r <= 1; r++) {
                        Vector3 rBase = Vector3{ e.position.x + r * 0.25f, 0.40f, e.position.z + r * 0.15f };
                        Vector3 rTop  = Vector3{ rBase.x, rBase.y + 1.4f * e.scale, rBase.z };
                        DrawLine3D(rBase, rTop, Color{ 74, 222, 128, 255 });
                        DrawCylinder(Vector3{ rTop.x, rTop.y - 0.35f, rTop.z }, 0.06f, 0.06f, 0.45f, 6, Color{ 146, 64, 14, 255 });
                    }
                    continue;
                }

                if (!e.model) continue;

                rlPushMatrix();
                    rlTranslatef(e.position.x, e.position.y, e.position.z);
                    rlRotatef(e.rotationY, 0, 1, 0);
                    rlScalef(e.scale, e.scale, e.scale);

                    if (e.type == ENT_BUMPER) {
                        float expandDist = e.isExpanded ? 1.6f : 0.0f;
                        rlTranslatef(0, 0, expandDist);
                    } else if (e.type == ENT_ROTOR) {
                        rlRotatef(e.animTimer * 120.0f, 0, 1, 0);
                    } else if (e.type == ENT_TIPPING) {
                        float tipAngle = sinf(e.animTimer * 2.2f) * 28.0f;
                        rlRotatef(tipAngle, 0, 0, 1);
                    } else if (e.type == ENT_PENDULUM) {
                        float swingAngle = sinf(e.animTimer * 2.0f) * 45.0f;
                        rlRotatef(swingAngle, 1, 0, 0);
                    } else if (e.type == ENT_SWING) {
                        float armAngle = sinf(e.animTimer * 1.8f) * 35.0f;
                        rlRotatef(armAngle, 0, 1, 0);
                    }

                    DrawModel(*e.model, Vector3{ 0, 0, 0 }, 1.0f, WHITE);
                rlPopMatrix();
            }"""

code = code.replace(old_draw_scene_entities, new_draw_scene_entities)

# -------------------------------------------------------------------------
# 13. Update 3D Raycast Selection & Editor Gizmos to use levelEntities
# -------------------------------------------------------------------------
old_editor_raycast = """            // Raycast Click Selection in 3D
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && mousePos.x < (GetScreenWidth() - 340)) {
                Ray mRay = GetScreenToWorldRay(mousePos, camera);
                float bestDist = 9999.0f;
                int hitIdx = -1;
                for (size_t i = 0; i < obsList.size(); i++) {
                    BoundingBox oBox = { Vector3Subtract(obsList[i].position, Vector3{ 1.8f, 1.8f, 1.8f }), Vector3Add(obsList[i].position, Vector3{ 1.8f, 1.8f, 1.8f }) };
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

            // Keyboard Manipulation of Selected Obstacle (Continuous hold support!)
            if (selectedObs >= 0 && selectedObs < (int)obsList.size()) {
                auto& o = obsList[selectedObs];"""

new_editor_raycast = """            // Raycast Click Selection in 3D across ALL Level Entities (Obstacles, Trees, Rocks, Checkpoints)
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && mousePos.x < (GetScreenWidth() - 340)) {
                Ray mRay = GetScreenToWorldRay(mousePos, camera);
                float bestDist = 9999.0f;
                int hitIdx = -1;
                for (size_t i = 0; i < levelEntities.size(); i++) {
                    float bSize = fmaxf(levelEntities[i].radius, 1.8f) * levelEntities[i].scale;
                    BoundingBox oBox = { Vector3Subtract(levelEntities[i].position, Vector3{ bSize, bSize * 1.5f, bSize }), Vector3Add(levelEntities[i].position, Vector3{ bSize, bSize * 1.5f, bSize }) };
                    RayCollision rHit = GetRayCollisionBox(mRay, oBox);
                    if (rHit.hit && rHit.distance < bestDist) {
                        bestDist = rHit.distance;
                        hitIdx = (int)i;
                    }
                }
                if (hitIdx != -1) {
                    selectedObs = hitIdx;
                    editorMsg = TextFormat("Selected: %s #%d", ENTITY_TYPE_NAMES[levelEntities[hitIdx].type], hitIdx);
                    editorMsgTimer = 2.0f;
                }
            }

            // Keyboard Manipulation of Selected Entity (Continuous hold support!)
            if (selectedObs >= 0 && selectedObs < (int)levelEntities.size()) {
                auto& o = levelEntities[selectedObs];"""

code = code.replace(old_editor_raycast, new_editor_raycast)

# -------------------------------------------------------------------------
# 14. Update 3D Gizmo loop to iterate levelEntities
# -------------------------------------------------------------------------
old_gizmo_loop = """            // 3D Level Editor Visual Highlights & Gizmos (Unity / Godot style)
            if (editorMode) {
                for (size_t i = 0; i < obsList.size(); i++) {
                    bool isSel = ((int)i == selectedObs);
                    Vector3 oPos = obsList[i].position;
                    DrawCircle3D(Vector3{ oPos.x, 0.05f, oPos.z }, obsList[i].radius, Vector3{ 1, 0, 0 }, 90.0f, isSel ? Color{ 255, 235, 50, 180 } : Color{ 80, 180, 255, 90 });"""

new_gizmo_loop = """            // 3D Level Editor Visual Highlights & Gizmos across ALL Entities
            if (editorMode) {
                for (size_t i = 0; i < levelEntities.size(); i++) {
                    bool isSel = ((int)i == selectedObs);
                    Vector3 oPos = levelEntities[i].position;
                    float eRad = levelEntities[i].radius * levelEntities[i].scale;
                    DrawCircle3D(Vector3{ oPos.x, oPos.y + 0.05f, oPos.z }, eRad, Vector3{ 1, 0, 0 }, 90.0f, isSel ? Color{ 255, 235, 50, 200 } : Color{ 71, 140, 191, 100 });"""

code = code.replace(old_gizmo_loop, new_gizmo_loop)

# -------------------------------------------------------------------------
# 15. Update Inspector Panel to support all 12 Entity Types + Save/Duplicate/Add/Delete
# -------------------------------------------------------------------------
old_inspector_content = """            if (selectedObs >= 0 && selectedObs < (int)obsList.size()) {
                auto& o = obsList[selectedObs];
                const char* typeNames[] = { "ROTOR_B5", "TIPPING_B2", "PENDULUM_C2", "GATE_F10", "SWING_F3", "BUMPER_SPRING", "ROCK1", "ROCK2" };
                const char* tName = (o.type >= 0 && o.type <= 7) ? typeNames[o.type] : "OBSTACLE";

                // Object Banner (Godot Node Style)
                Rectangle objBannerRec = { panelRec.x + 8, py, panelW - 16, 32 };
                DrawRectangleRounded(objBannerRec, 0.15f, 3, Color{ 24, 28, 37, 255 });
                DrawRectangleRoundedLinesEx(objBannerRec, 0.15f, 3, 1.0f, Color{ 48, 55, 71, 255 });
                DrawCircle((int)objBannerRec.x + 14, (int)objBannerRec.y + 16, 5.0f, Color{ 71, 140, 191, 255 }); // Godot Node Dot
                DrawTextEx(fontUIBold, TextFormat("KineticObstacle_#%d", selectedObs), Vector2{ objBannerRec.x + 26, objBannerRec.y + 8 }, 13, 0.4f, WHITE);
                
                // Chip on right
                Rectangle chipRec = { objBannerRec.x + objBannerRec.width - 110, objBannerRec.y + 5, 102, 22 };
                DrawRectangleRounded(chipRec, 0.25f, 3, Color{ 39, 46, 61, 255 });
                Vector2 chipSz = MeasureTextEx(fontUI, tName, 11, 0.2f);
                DrawTextEx(fontUI, tName, Vector2{ chipRec.x + (chipRec.width - chipSz.x)*0.5f, chipRec.y + 5 }, 11, 0.2f, Color{ 147, 197, 253, 255 });
                py += 38;

                // 1. Section: Transform
                DrawSectionHeader("v  Transform");

                // Position Row (X / Y / Z Columns with Godot Color Accents)
                DrawTextEx(fontUI, "Position", Vector2{ panelRec.x + 12, py }, 11, 0.3f, Color{ 156, 163, 175, 255 });
                py += 16;

                float colW = 100.0f;
                float startX = panelRec.x + 12;

                // Column X (Red)
                {
                    Rectangle xRec = { startX, py, colW, 26 };
                    DrawRectangleRounded(xRec, 0.15f, 3, Color{ 20, 23, 30, 255 });
                    DrawRectangleRoundedLinesEx(xRec, 0.15f, 3, 1.0f, Color{ 45, 52, 66, 255 });
                    // Red Pill
                    DrawRectangleRec(Rectangle{ xRec.x, xRec.y, 16, xRec.height }, Color{ 214, 69, 69, 255 });
                    DrawTextEx(fontUIBold, "X", Vector2{ xRec.x + 4, xRec.y + 6 }, 11, 0.2f, WHITE);
                    DrawTextEx(fontUI, TextFormat("%.2f", o.position.x), Vector2{ xRec.x + 22, xRec.y + 6 }, 12, 0.3f, Color{ 243, 244, 246, 255 });
                    // Micro +/- step buttons
                    if (DrawGodotBtn(Rectangle{ xRec.x + colW - 24, xRec.y + 2, 11, 22 }, "-", Color{ 32, 37, 49, 255 }, Color{ 156, 163, 175, 255 }) || IsKeyPressed(KEY_J)) o.position.x -= 0.5f;
                    if (DrawGodotBtn(Rectangle{ xRec.x + colW - 12, xRec.y + 2, 11, 22 }, "+", Color{ 32, 37, 49, 255 }, Color{ 156, 163, 175, 255 }) || IsKeyPressed(KEY_L)) o.position.x += 0.5f;
                }

                // Column Y (Green)
                {
                    Rectangle yRec = { startX + colW + 8, py, colW, 26 };
                    DrawRectangleRounded(yRec, 0.15f, 3, Color{ 20, 23, 30, 255 });
                    DrawRectangleRoundedLinesEx(yRec, 0.15f, 3, 1.0f, Color{ 45, 52, 66, 255 });
                    // Green Pill
                    DrawRectangleRec(Rectangle{ yRec.x, yRec.y, 16, yRec.height }, Color{ 46, 160, 67, 255 });
                    DrawTextEx(fontUIBold, "Y", Vector2{ yRec.x + 4, yRec.y + 6 }, 11, 0.2f, WHITE);
                    DrawTextEx(fontUI, TextFormat("%.2f", o.position.y), Vector2{ yRec.x + 22, yRec.y + 6 }, 12, 0.3f, Color{ 243, 244, 246, 255 });
                    if (DrawGodotBtn(Rectangle{ yRec.x + colW - 24, yRec.y + 2, 11, 22 }, "-", Color{ 32, 37, 49, 255 }, Color{ 156, 163, 175, 255 }) || IsKeyPressed(KEY_O)) o.position.y -= 0.2f;
                    if (DrawGodotBtn(Rectangle{ yRec.x + colW - 12, yRec.y + 2, 11, 22 }, "+", Color{ 32, 37, 49, 255 }, Color{ 156, 163, 175, 255 }) || IsKeyPressed(KEY_U)) o.position.y += 0.2f;
                }

                // Column Z (Blue)
                {
                    Rectangle zRec = { startX + (colW + 8) * 2, py, colW, 26 };
                    DrawRectangleRounded(zRec, 0.15f, 3, Color{ 20, 23, 30, 255 });
                    DrawRectangleRoundedLinesEx(zRec, 0.15f, 3, 1.0f, Color{ 45, 52, 66, 255 });
                    // Blue Pill
                    DrawRectangleRec(Rectangle{ zRec.x, zRec.y, 16, zRec.height }, Color{ 59, 130, 246, 255 });
                    DrawTextEx(fontUIBold, "Z", Vector2{ zRec.x + 4, zRec.y + 6 }, 11, 0.2f, WHITE);
                    DrawTextEx(fontUI, TextFormat("%.2f", o.position.z), Vector2{ zRec.x + 22, zRec.y + 6 }, 12, 0.3f, Color{ 243, 244, 246, 255 });
                    if (DrawGodotBtn(Rectangle{ zRec.x + colW - 24, zRec.y + 2, 11, 22 }, "-", Color{ 32, 37, 49, 255 }, Color{ 156, 163, 175, 255 }) || IsKeyPressed(KEY_K)) o.position.z -= 0.5f;
                    if (DrawGodotBtn(Rectangle{ zRec.x + colW - 12, zRec.y + 2, 11, 22 }, "+", Color{ 32, 37, 49, 255 }, Color{ 156, 163, 175, 255 }) || IsKeyPressed(KEY_I)) o.position.z += 0.5f;
                }
                py += 32;

                // Rotation Row
                DrawTextEx(fontUI, "Rotation (Y-Axis)", Vector2{ panelRec.x + 12, py }, 11, 0.3f, Color{ 156, 163, 175, 255 });
                py += 16;
                Rectangle rotBox = { panelRec.x + 12, py, 140, 26 };
                DrawRectangleRounded(rotBox, 0.15f, 3, Color{ 20, 23, 30, 255 });
                DrawRectangleRoundedLinesEx(rotBox, 0.15f, 3, 1.0f, Color{ 45, 52, 66, 255 });
                DrawTextEx(fontUI, TextFormat("%.1f deg", o.rotationY), Vector2{ rotBox.x + 12, rotBox.y + 6 }, 12, 0.3f, Color{ 243, 244, 246, 255 });
                
                if (DrawGodotBtn(Rectangle{ panelRec.x + 160, py, 80, 26 }, "-15 deg", Color{ 40, 47, 62, 255 }, Color{ 229, 231, 235, 255 }) || IsKeyPressed(KEY_R)) o.rotationY -= 15.0f;
                if (DrawGodotBtn(Rectangle{ panelRec.x + 246, py, 80, 26 }, "+15 deg", Color{ 40, 47, 62, 255 }, Color{ 229, 231, 235, 255 }) || IsKeyPressed(KEY_T)) o.rotationY += 15.0f;
                py += 34;

                // 2. Section: KineticObstacle Properties
                DrawSectionHeader("v  Kinetic Properties");

                // Type OptionButton
                DrawTextEx(fontUI, "Obstacle Type", Vector2{ panelRec.x + 12, py }, 11, 0.3f, Color{ 156, 163, 175, 255 });
                py += 16;

                Rectangle optRec = { panelRec.x + 12, py, panelW - 24, 28 };
                DrawRectangleRounded(optRec, 0.15f, 3, Color{ 24, 28, 37, 255 });
                DrawRectangleRoundedLinesEx(optRec, 0.15f, 3, 1.0f, Color{ 58, 66, 83, 255 });

                auto ChangeType = [&](int delta) {
                    o.type = (ObstacleType)(((int)o.type + delta + 8) % 8);
                    if (o.type == OBS_BUMPER_SPRING) { o.radius = 2.5f; o.model = &obsModelA1; }
                    else if (o.type == OBS_ROTOR_B5) { o.radius = 2.4f; o.model = &obsModelB5; }
                    else if (o.type == OBS_TIPPING_B2) { o.radius = 2.2f; o.model = &obsModelB2; }
                    else if (o.type == OBS_PENDULUM_C2) { o.radius = 2.2f; o.model = &obsModelC2; }
                    else if (o.type == OBS_GATE_F10) { o.radius = 2.2f; o.model = &obsModelF10; }
                    else if (o.type == OBS_SWING_F3) { o.radius = 2.2f; o.model = &obsModelF3; }
                    else if (o.type == OBS_ROCK1) { o.radius = 1.6f; o.model = &obsModelRock1; }
                    else if (o.type == OBS_ROCK2) { o.radius = 1.6f; o.model = &obsModelRock2; }
                    editorMsg = TextFormat("Type: %s", typeNames[o.type]);
                    editorMsgTimer = 1.5f;
                };

                if (DrawGodotBtn(Rectangle{ optRec.x + 2, optRec.y + 2, 28, 24 }, "<", Color{ 36, 42, 56, 255 }, WHITE, true)) ChangeType(-1);
                Vector2 tNameSz = MeasureTextEx(fontUIBold, tName, 13, 0.4f);
                DrawTextEx(fontUIBold, tName, Vector2{ optRec.x + (optRec.width - tNameSz.x) * 0.5f, optRec.y + 6 }, 13, 0.4f, Color{ 147, 197, 253, 255 });
                if (DrawGodotBtn(Rectangle{ optRec.x + optRec.width - 30, optRec.y + 2, 28, 24 }, ">", Color{ 36, 42, 56, 255 }, WHITE, true)) ChangeType(1);
                py += 34;

                // Radius display
                DrawTextEx(fontUI, TextFormat("Collision Radius:  %.2f m", o.radius), Vector2{ panelRec.x + 14, py }, 11, 0.3f, Color{ 156, 163, 175, 255 });
                py += 22;

                // 3. Section: Node Actions
                DrawSectionHeader("v  Actions");

                if (DrawGodotBtn(Rectangle{ panelRec.x + 12, py, (panelW - 32) * 0.5f, 28 }, "Duplicate (Ctrl+D)", Color{ 40, 48, 64, 255 }, Color{ 229, 231, 235, 255 }) || (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_D))) {
                    KineticObstacle dup = o;
                    dup.position.z -= 2.0f;
                    obsList.push_back(dup);
                    selectedObs = (int)obsList.size() - 1;
                    editorMsg = "Obstacle Duplicated!";
                    editorMsgTimer = 2.0f;
                }
                if (DrawGodotBtn(Rectangle{ panelRec.x + 16 + (panelW - 32) * 0.5f, py, (panelW - 32) * 0.5f, 28 }, "Delete (Del)", Color{ 110, 38, 38, 255 }, Color{ 254, 202, 202, 255 }) || IsKeyPressed(KEY_DELETE) || IsKeyPressed(KEY_BACKSPACE)) {
                    obsList.erase(obsList.begin() + selectedObs);
                    selectedObs = -1;
                    editorMsg = "Obstacle Deleted!";
                    editorMsgTimer = 2.0f;
                }
                py += 36;
            } else {
                DrawRectangleRec(Rectangle{ panelRec.x + 12, py, panelW - 24, 70 }, Color{ 24, 28, 37, 255 });
                DrawRectangleLinesEx(Rectangle{ panelRec.x + 12, py, panelW - 24, 70 }, 1.0f, Color{ 45, 52, 66, 255 });
                DrawTextEx(fontUI, "No obstacle selected.", Vector2{ panelRec.x + 24, py + 14 }, 13, 0.3f, Color{ 209, 213, 219, 255 });
                DrawTextEx(fontUI, "Left-Click any obstacle in 3D scene to edit.", Vector2{ panelRec.x + 24, py + 36 }, 11, 0.3f, Color{ 148, 156, 172, 255 });
                py += 82;
            }

            // Global Scene Actions
            DrawLine((int)panelRec.x + 10, (int)py, (int)panelRec.x + (int)panelW - 10, (int)py, Color{ 48, 55, 71, 255 });
            py += 10;

            Rectangle btnAddRec = { panelRec.x + 12, py, panelW - 24, 30 };
            if (DrawGodotBtn(btnAddRec, "+  Add New Obstacle (N)", Color{ 42, 50, 68, 255 }, WHITE, true) || IsKeyPressed(KEY_N)) {
                KineticObstacle newO;
                newO.type = OBS_BUMPER_SPRING;
                Vector3 fwd = { -sinf(editorCamYaw) * cosf(editorCamPitch), sinf(editorCamPitch), -cosf(editorCamYaw) * cosf(editorCamPitch) };
                newO.position = Vector3Add(editorCamPos, Vector3Scale(fwd, 6.5f));
                newO.position.y = 0.65f;
                newO.rotationY = 0.0f;
                newO.animTimer = 0.0f;
                newO.animSpeed = 1.0f;
                newO.radius = 2.5f;
                newO.model = &obsModelA1;
                newO.bumperExpandTimer = 0.0f;
                newO.bumperCooldownTimer = 0.0f;
                newO.isExpanded = false;
                obsList.push_back(newO);
                selectedObs = (int)obsList.size() - 1;
                editorMsg = "New Obstacle Spawned In View!";
                editorMsgTimer = 2.5f;
            }
            py += 36;

            Rectangle btnSaveRec = { panelRec.x + 12, py, panelW - 24, 34 };
            if (DrawGodotBtn(btnSaveRec, "Save Level to JSON (Ctrl+S)", Color{ 71, 140, 191, 255 }, WHITE, true) || (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_S)) || IsKeyPressed(KEY_F5)) {
                SaveObstaclesJSON(LEVEL_FILE_PATH, obsList);
                editorMsg = "Level Saved to level_obstacles.json";
                editorMsgTimer = 3.5f;
            }"""

new_inspector_content = """            if (selectedObs >= 0 && selectedObs < (int)levelEntities.size()) {
                auto& o = levelEntities[selectedObs];
                const char* tName = (o.type >= 0 && o.type < ENT_TYPE_COUNT) ? ENTITY_TYPE_NAMES[o.type] : "ENTITY";

                // Object Banner (Godot Node Style)
                Rectangle objBannerRec = { panelRec.x + 8, py, panelW - 16, 32 };
                DrawRectangleRounded(objBannerRec, 0.15f, 3, Color{ 24, 28, 37, 255 });
                DrawRectangleRoundedLinesEx(objBannerRec, 0.15f, 3, 1.0f, Color{ 48, 55, 71, 255 });
                DrawCircle((int)objBannerRec.x + 14, (int)objBannerRec.y + 16, 5.0f, Color{ 71, 140, 191, 255 });
                DrawTextEx(fontUIBold, TextFormat("%s_#%d", tName, selectedObs), Vector2{ objBannerRec.x + 26, objBannerRec.y + 8 }, 12, 0.3f, WHITE);

                Rectangle chipRec = { objBannerRec.x + objBannerRec.width - 105, objBannerRec.y + 5, 98, 22 };
                DrawRectangleRounded(chipRec, 0.25f, 3, Color{ 39, 46, 61, 255 });
                Vector2 chipSz = MeasureTextEx(fontUI, tName, 10, 0.2f);
                DrawTextEx(fontUI, tName, Vector2{ chipRec.x + (chipRec.width - chipSz.x)*0.5f, chipRec.y + 5 }, 10, 0.2f, Color{ 147, 197, 253, 255 });
                py += 38;

                // 1. Transform Section
                DrawSectionHeader("v  Transform");

                DrawTextEx(fontUI, "Position", Vector2{ panelRec.x + 12, py }, 11, 0.3f, Color{ 156, 163, 175, 255 });
                py += 16;

                float colW = 100.0f;
                float startX = panelRec.x + 12;

                // Column X (Red)
                {
                    Rectangle xRec = { startX, py, colW, 26 };
                    DrawRectangleRounded(xRec, 0.15f, 3, Color{ 20, 23, 30, 255 });
                    DrawRectangleRoundedLinesEx(xRec, 0.15f, 3, 1.0f, Color{ 45, 52, 66, 255 });
                    DrawRectangleRec(Rectangle{ xRec.x, xRec.y, 16, xRec.height }, Color{ 214, 69, 69, 255 });
                    DrawTextEx(fontUIBold, "X", Vector2{ xRec.x + 4, xRec.y + 6 }, 11, 0.2f, WHITE);
                    DrawTextEx(fontUI, TextFormat("%.2f", o.position.x), Vector2{ xRec.x + 22, xRec.y + 6 }, 12, 0.3f, Color{ 243, 244, 246, 255 });
                    if (DrawGodotBtn(Rectangle{ xRec.x + colW - 24, xRec.y + 2, 11, 22 }, "-", Color{ 32, 37, 49, 255 }, Color{ 156, 163, 175, 255 }) || IsKeyPressed(KEY_J)) o.position.x -= 0.5f;
                    if (DrawGodotBtn(Rectangle{ xRec.x + colW - 12, xRec.y + 2, 11, 22 }, "+", Color{ 32, 37, 49, 255 }, Color{ 156, 163, 175, 255 }) || IsKeyPressed(KEY_L)) o.position.x += 0.5f;
                }

                // Column Y (Green)
                {
                    Rectangle yRec = { startX + colW + 8, py, colW, 26 };
                    DrawRectangleRounded(yRec, 0.15f, 3, Color{ 20, 23, 30, 255 });
                    DrawRectangleRoundedLinesEx(yRec, 0.15f, 3, 1.0f, Color{ 45, 52, 66, 255 });
                    DrawRectangleRec(Rectangle{ yRec.x, yRec.y, 16, yRec.height }, Color{ 46, 160, 67, 255 });
                    DrawTextEx(fontUIBold, "Y", Vector2{ yRec.x + 4, yRec.y + 6 }, 11, 0.2f, WHITE);
                    DrawTextEx(fontUI, TextFormat("%.2f", o.position.y), Vector2{ yRec.x + 22, yRec.y + 6 }, 12, 0.3f, Color{ 243, 244, 246, 255 });
                    if (DrawGodotBtn(Rectangle{ yRec.x + colW - 24, yRec.y + 2, 11, 22 }, "-", Color{ 32, 37, 49, 255 }, Color{ 156, 163, 175, 255 }) || IsKeyPressed(KEY_O)) o.position.y -= 0.2f;
                    if (DrawGodotBtn(Rectangle{ yRec.x + colW - 12, yRec.y + 2, 11, 22 }, "+", Color{ 32, 37, 49, 255 }, Color{ 156, 163, 175, 255 }) || IsKeyPressed(KEY_U)) o.position.y += 0.2f;
                }

                // Column Z (Blue)
                {
                    Rectangle zRec = { startX + (colW + 8) * 2, py, colW, 26 };
                    DrawRectangleRounded(zRec, 0.15f, 3, Color{ 20, 23, 30, 255 });
                    DrawRectangleRoundedLinesEx(zRec, 0.15f, 3, 1.0f, Color{ 45, 52, 66, 255 });
                    DrawRectangleRec(Rectangle{ zRec.x, zRec.y, 16, zRec.height }, Color{ 59, 130, 246, 255 });
                    DrawTextEx(fontUIBold, "Z", Vector2{ zRec.x + 4, zRec.y + 6 }, 11, 0.2f, WHITE);
                    DrawTextEx(fontUI, TextFormat("%.2f", o.position.z), Vector2{ zRec.x + 22, zRec.y + 6 }, 12, 0.3f, Color{ 243, 244, 246, 255 });
                    if (DrawGodotBtn(Rectangle{ zRec.x + colW - 24, zRec.y + 2, 11, 22 }, "-", Color{ 32, 37, 49, 255 }, Color{ 156, 163, 175, 255 }) || IsKeyPressed(KEY_K)) o.position.z -= 0.5f;
                    if (DrawGodotBtn(Rectangle{ zRec.x + colW - 12, zRec.y + 2, 11, 22 }, "+", Color{ 32, 37, 49, 255 }, Color{ 156, 163, 175, 255 }) || IsKeyPressed(KEY_I)) o.position.z += 0.5f;
                }
                py += 32;

                // Rotation Row
                DrawTextEx(fontUI, "Rotation (Y-Axis)", Vector2{ panelRec.x + 12, py }, 11, 0.3f, Color{ 156, 163, 175, 255 });
                py += 16;
                Rectangle rotBox = { panelRec.x + 12, py, 140, 26 };
                DrawRectangleRounded(rotBox, 0.15f, 3, Color{ 20, 23, 30, 255 });
                DrawRectangleRoundedLinesEx(rotBox, 0.15f, 3, 1.0f, Color{ 45, 52, 66, 255 });
                DrawTextEx(fontUI, TextFormat("%.1f deg", o.rotationY), Vector2{ rotBox.x + 12, rotBox.y + 6 }, 12, 0.3f, Color{ 243, 244, 246, 255 });
                
                if (DrawGodotBtn(Rectangle{ panelRec.x + 160, py, 80, 26 }, "-15 deg", Color{ 40, 47, 62, 255 }, Color{ 229, 231, 235, 255 }) || IsKeyPressed(KEY_R)) o.rotationY -= 15.0f;
                if (DrawGodotBtn(Rectangle{ panelRec.x + 246, py, 80, 26 }, "+15 deg", Color{ 40, 47, 62, 255 }, Color{ 229, 231, 235, 255 }) || IsKeyPressed(KEY_T)) o.rotationY += 15.0f;
                py += 34;

                // 2. Section: Entity Properties & Type Switcher
                DrawSectionHeader("v  Entity Properties");

                DrawTextEx(fontUI, "Entity Type (12 Types)", Vector2{ panelRec.x + 12, py }, 11, 0.3f, Color{ 156, 163, 175, 255 });
                py += 16;

                Rectangle optRec = { panelRec.x + 12, py, panelW - 24, 28 };
                DrawRectangleRounded(optRec, 0.15f, 3, Color{ 24, 28, 37, 255 });
                DrawRectangleRoundedLinesEx(optRec, 0.15f, 3, 1.0f, Color{ 58, 66, 83, 255 });

                auto ChangeEntityType = [&](int delta) {
                    o.type = (EntityType)(((int)o.type + delta + ENT_TYPE_COUNT) % ENT_TYPE_COUNT);
                    AssignEntityModelAndProps(o, &obsModelA1, &obsModelB5, &obsModelB2, &obsModelC2, &obsModelF10, &obsModelF3, &obsModelRock1, &obsModelRock2, &jungleTreeModel, &cp1Model);
                    editorMsg = TextFormat("Type: %s", ENTITY_TYPE_NAMES[o.type]);
                    editorMsgTimer = 1.5f;
                };

                if (DrawGodotBtn(Rectangle{ optRec.x + 2, optRec.y + 2, 28, 24 }, "<", Color{ 36, 42, 56, 255 }, WHITE, true)) ChangeEntityType(-1);
                Vector2 tNameSz = MeasureTextEx(fontUIBold, tName, 12, 0.3f);
                DrawTextEx(fontUIBold, tName, Vector2{ optRec.x + (optRec.width - tNameSz.x) * 0.5f, optRec.y + 6 }, 12, 0.3f, Color{ 147, 197, 253, 255 });
                if (DrawGodotBtn(Rectangle{ optRec.x + optRec.width - 30, optRec.y + 2, 28, 24 }, ">", Color{ 36, 42, 56, 255 }, WHITE, true)) ChangeEntityType(1);
                py += 34;

                DrawTextEx(fontUI, TextFormat("Radius: %.2f m  |  Scale: %.2f", o.radius, o.scale), Vector2{ panelRec.x + 14, py }, 11, 0.3f, Color{ 156, 163, 175, 255 });
                py += 22;

                // 3. Section: Node Actions
                DrawSectionHeader("v  Actions");

                if (DrawGodotBtn(Rectangle{ panelRec.x + 12, py, (panelW - 32) * 0.5f, 28 }, "Duplicate (Ctrl+D)", Color{ 40, 48, 64, 255 }, Color{ 229, 231, 235, 255 }) || (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_D))) {
                    LevelEntity dup = o;
                    dup.id = (int)levelEntities.size();
                    dup.position.z -= 2.0f;
                    levelEntities.push_back(dup);
                    selectedObs = (int)levelEntities.size() - 1;
                    editorMsg = "Entity Duplicated!";
                    editorMsgTimer = 2.0f;
                }
                if (DrawGodotBtn(Rectangle{ panelRec.x + 16 + (panelW - 32) * 0.5f, py, (panelW - 32) * 0.5f, 28 }, "Delete (Del)", Color{ 110, 38, 38, 255 }, Color{ 254, 202, 202, 255 }) || IsKeyPressed(KEY_DELETE) || IsKeyPressed(KEY_BACKSPACE)) {
                    levelEntities.erase(levelEntities.begin() + selectedObs);
                    selectedObs = -1;
                    editorMsg = "Entity Deleted!";
                    editorMsgTimer = 2.0f;
                }
                py += 36;
            } else {
                DrawRectangleRec(Rectangle{ panelRec.x + 12, py, panelW - 24, 70 }, Color{ 24, 28, 37, 255 });
                DrawRectangleLinesEx(Rectangle{ panelRec.x + 12, py, panelW - 24, 70 }, 1.0f, Color{ 45, 52, 66, 255 });
                DrawTextEx(fontUI, "No entity selected.", Vector2{ panelRec.x + 24, py + 14 }, 13, 0.3f, Color{ 209, 213, 219, 255 });
                DrawTextEx(fontUI, "Left-Click any obstacle, tree, or rock to edit.", Vector2{ panelRec.x + 24, py + 36 }, 11, 0.3f, Color{ 148, 156, 172, 255 });
                py += 82;
            }

            // Global Scene Actions
            DrawLine((int)panelRec.x + 10, (int)py, (int)panelRec.x + (int)panelW - 10, (int)py, Color{ 48, 55, 71, 255 });
            py += 10;

            Rectangle btnAddRec = { panelRec.x + 12, py, panelW - 24, 30 };
            if (DrawGodotBtn(btnAddRec, "+  Add New Entity (N)", Color{ 42, 50, 68, 255 }, WHITE, true) || IsKeyPressed(KEY_N)) {
                LevelEntity newE;
                memset(&newE, 0, sizeof(newE));
                newE.id = (int)levelEntities.size();
                newE.type = ENT_BUMPER;
                Vector3 fwd = { -sinf(editorCamYaw) * cosf(editorCamPitch), sinf(editorCamPitch), -cosf(editorCamYaw) * cosf(editorCamPitch) };
                newE.position = Vector3Add(editorCamPos, Vector3Scale(fwd, 6.5f));
                newE.position.y = 0.50f;
                newE.scale = 1.0f;
                AssignEntityModelAndProps(newE, &obsModelA1, &obsModelB5, &obsModelB2, &obsModelC2, &obsModelF10, &obsModelF3, &obsModelRock1, &obsModelRock2, &jungleTreeModel, &cp1Model);
                levelEntities.push_back(newE);
                selectedObs = (int)levelEntities.size() - 1;
                editorMsg = "New Entity Spawned in View!";
                editorMsgTimer = 2.5f;
            }
            py += 36;

            Rectangle btnSaveRec = { panelRec.x + 12, py, panelW - 24, 34 };
            if (DrawGodotBtn(btnSaveRec, "Save Level Layout (Ctrl+S)", Color{ 71, 140, 191, 255 }, WHITE, true) || (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_S)) || IsKeyPressed(KEY_F5)) {
                std::string p = ResolveAsset("level_layout.json");
                SaveLevelLayoutJSON(p, levelEntities);
                editorMsg = "Saved Layout to level_layout.json!";
                editorMsgTimer = 3.5f;
            }"""

code = code.replace(old_inspector_content, new_inspector_content)

# -------------------------------------------------------------------------
# 16. Unload jungleTreeModel and texBgrDefault
# -------------------------------------------------------------------------
old_unloads = """    UnloadTexture(texEsc);
    if (fontUI.texture.id != GetFontDefault().texture.id) UnloadFont(fontUI);
    if (fontUIBold.texture.id != GetFontDefault().texture.id) UnloadFont(fontUIBold);"""

new_unloads = """    UnloadTexture(texEsc);
    UnloadTexture(texBgrDefault);
    UnloadModel(jungleTreeModel);
    if (fontUI.texture.id != GetFontDefault().texture.id) UnloadFont(fontUI);
    if (fontUIBold.texture.id != GetFontDefault().texture.id) UnloadFont(fontUIBold);"""

code = code.replace(old_unloads, new_unloads)

# Write output file
with open(path, "w", encoding="utf-8") as f:
    f.write(code)

print("Saved updated GameEngine.cpp successfully! New length:", len(code))

