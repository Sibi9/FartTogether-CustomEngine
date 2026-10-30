import re
import os
import sys

print("Reading CustomEngine/native/GameEngine.cpp.bak_v7...")
with open("CustomEngine/native/GameEngine.cpp.bak_v7", "r", encoding="utf-8") as f:
    code = f.read()

print("Original size:", len(code))

def replace_block(pattern, replacement, desc):
    global code
    new_code, count = re.subn(pattern, replacement, code, count=1, flags=re.DOTALL)
    if count == 0:
        print(f"FAILED to replace: {desc}")
        sys.exit(1)
    else:
        print(f"SUCCESS: {desc} (replaced {count} occurrence)")
        code = new_code

# -----------------------------------------------------------------------------
# 1. BUMPER_IMPACT_RADIUS
# -----------------------------------------------------------------------------
replace_block(
    r'static const float BUMPER_IMPACT_RADIUS = \d+(\.\d+)?f;',
    'static const float BUMPER_IMPACT_RADIUS = 1.85f; // Authentic physical touch (boat 1.25m + pad 0.6m)',
    '1. Update BUMPER_IMPACT_RADIUS'
)

# -----------------------------------------------------------------------------
# 2. DeathCause enum
# -----------------------------------------------------------------------------
death_cause_replacement = """enum DeathCause {
    DEATH_NONE = 0,
    DEATH_WALL = 1,
    DEATH_OBSTACLE = 2,
    DEATH_VOID = 3,
    DEATH_SHARK = 4,
    DEATH_CROCODILE = 5,
    DEATH_FIRE = 6,
    DEATH_SUICIDE = 7
};"""

replace_block(
    r'enum DeathCause\s*\{[^}]+\};',
    death_cause_replacement,
    '2. Update DeathCause enum'
)

# -----------------------------------------------------------------------------
# 3. EntityType and LevelEntity structs
# -----------------------------------------------------------------------------
entity_structs = """// Unified Level Entity Architecture (Obstacles, Rocks, Trees, Checkpoints, Props)
enum EntityType {
    ENT_BUMPER = 0,     // 3D Spring Bumper Launch Pad
    ENT_ROTOR,          // B5 Double Sweep rotor
    ENT_TIPPING,        // B2 Tipping bar
    ENT_PENDULUM,       // C2 Pendulum mace
    ENT_GATE,           // F10 Rotating bamboo gate
    ENT_SWING,          // F3 Swinging bamboo barrier
    ENT_ROCK_DARK1,     // Authentic dark-grey stone boulder 1 (#4A4848)
    ENT_ROCK_DARK2,     // Authentic dark-grey stone boulder 2 (#4A4848)
    ENT_TREE_JUNGLE,    // Authentic jungle tree (green canopy, brown wood)
    ENT_CHECKPOINT,     // Bamboo checkpoint archway
    ENT_LILY_PAD,       // Circular water lily pad
    ENT_REEDS,          // Water reed clusters
    ENT_COUNT
};

static const char* ENTITY_TYPE_NAMES[] = {
    "BUMPER_SPRING",
    "ROTOR_B5",
    "TIPPING_B2",
    "PENDULUM_C2",
    "GATE_F10",
    "SWING_F3",
    "ROCK_DARK_1",
    "ROCK_DARK_2",
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
    float animTimer;
    float animSpeed;
    float radius;
    Model* model;
    // Bumper specific state
    float bumperExpandTimer;
    float bumperCooldownTimer;
    bool isExpanded;
};"""

replace_block(
    r'// Kinetic Obstacles & Spring Bumpers.*?struct KineticObstacle\s*\{[^}]+\};',
    entity_structs,
    '3. Replace ObstacleType & KineticObstacle with LevelEntity'
)

# -----------------------------------------------------------------------------
# 4. SaveLevelLayoutJSON & LoadLevelLayoutJSON
# -----------------------------------------------------------------------------
level_layout_code = """// Level Layout & Entity Persistence (JSON)
static const char* LEVEL_LAYOUT_PATH = "CustomEngine/level_layout.json";

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
        case ENT_TREE_JUNGLE: e.model = mTree; e.radius = 1.50f; break;
        case ENT_CHECKPOINT:  e.model = mCP;   e.radius = 3.50f; break;
        case ENT_LILY_PAD:    e.model = nullptr; e.radius = 0.90f; break;
        case ENT_REEDS:       e.model = nullptr; e.radius = 1.10f; break;
        default: break;
    }
}

void SaveLevelLayoutJSON(const std::string& path, const std::vector<LevelEntity>& list) {
    FILE* f = fopen(path.c_str(), "w");
    if (!f) return;
    fprintf(f, "[\n");
    for (size_t i = 0; i < list.size(); i++) {
        const auto& e = list[i];
        fprintf(f, "  { \"id\": %d, \"type\": %d, \"x\": %.3f, \"y\": %.3f, \"z\": %.3f, \"rotY\": %.2f, \"scale\": %.2f, \"speed\": %.2f, \"radius\": %.2f }%s\\n",
            (int)i, (int)e.type, e.position.x, e.position.y, e.position.z, e.rotationY, e.scale, e.animSpeed, e.radius,
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
    char line[512];
    int nextId = 0;
    while (fgets(line, sizeof(line), f)) {
        if (!strstr(line, "\"type\"")) continue;
        int t = 0;
        float x = 0, y = 0, z = 0, rotY = 0, scale = 1.0f, spd = 1.0f, rad = 2.0f;
        char* pt = strstr(line, "\"type\":");
        if (pt) sscanf(pt + 7, "%d", &t);
        char* px = strstr(line, "\"x\":");
        if (px) sscanf(px + 4, "%f", &x);
        char* py = strstr(line, "\"y\":");
        if (py) sscanf(py + 4, "%f", &y);
        char* pz = strstr(line, "\"z\":");
        if (pz) sscanf(pz + 4, "%f", &z);
        char* pr = strstr(line, "\"rotY\":");
        if (pr) sscanf(pr + 7, "%f", &rotY);
        char* ps = strstr(line, "\"scale\":");
        if (ps) sscanf(ps + 8, "%f", &scale);
        char* pspd = strstr(line, "\"speed\":");
        if (pspd) sscanf(pspd + 8, "%f", &spd);
        char* prad = strstr(line, "\"radius\":");
        if (prad) sscanf(prad + 9, "%f", &rad);

        LevelEntity e;
        memset(&e, 0, sizeof(e));
        e.id = nextId++;
        e.type = (EntityType)t;
        e.position = Vector3{ x, y, z };
        e.rotationY = rotY;
        e.scale = (scale > 0.01f) ? scale : 1.0f;
        e.animSpeed = (spd > 0.01f) ? spd : 1.0f;
        e.radius = (rad > 0.01f) ? rad : 2.0f;
        AssignEntityModelAndProps(e, mA1, mB5, mB2, mC2, mF10, mF3, mR1, mR2, mTree, mCP);
        list.push_back(e);
    }
    fclose(f);
    return !list.empty();
}"""

replace_block(
    r'static const char\* LEVEL_FILE_PATH = "CustomEngine/level_obstacles\.json";.*?bool LoadObstaclesJSON\([^)]+\)\s*\{.*?\n\}',
    level_layout_code,
    '4. Replace JSON persistence functions'
)

# -----------------------------------------------------------------------------
# 5. Load bgr_default_tex.png and GM_3D_JungleTree.obj and apply materials
# -----------------------------------------------------------------------------
# Let's inspect where texPalette and models are loaded
tex_load_target = r'Texture2D texPalette\s*=\s*LoadTexture\(ResolveAsset\("extracted_assets/textures/Palette\.png"\)\.c_str\(\)\);'
tex_load_replacement = """Texture2D texPalette       = LoadTexture(ResolveAsset("extracted_assets/textures/Palette.png").c_str());
    SetTextureFilter(texPalette, TEXTURE_FILTER_BILINEAR);

    Texture2D texBgrDefault     = LoadTexture(ResolveAsset("extracted_assets/textures/bgr_default_tex.png").c_str());
    SetTextureFilter(texBgrDefault, TEXTURE_FILTER_BILINEAR);"""

replace_block(tex_load_target, tex_load_replacement, '5a. Load texBgrDefault')

tree_load_target = r'Model obsModelRock2=\s*LoadModel\(ResolveAsset\("extracted_assets/meshes/GM_3D_Obstacle_Rock2\.obj"\)\.c_str\(\)\);'
tree_load_replacement = """Model obsModelRock2= LoadModel(ResolveAsset("extracted_assets/meshes/GM_3D_Obstacle_Rock2.obj").c_str());
    Model jungleTreeModel = LoadModel(ResolveAsset("extracted_assets/meshes/GM_3D_JungleTree.obj").c_str());"""

replace_block(tree_load_target, tree_load_replacement, '5b. Load jungleTreeModel')

# Now update ApplyMat so that env models, rocks, and trees use texBgrDefault
apply_mat_target = r'ApplyMat\(obsModelF3\);\s*ApplyMat\(obsModelRock1\);\s*ApplyMat\(obsModelRock2\);'
apply_mat_replacement = """ApplyMat(obsModelF3);

    // Apply authentic bgr_default_tex to rocks, trees, canyon walls and checkpoints
    auto ApplyBgrMat = [&](Model& m) {
        for (int i = 0; i < m.materialCount; i++) {
            m.materials[i].maps[MATERIAL_MAP_DIFFUSE].texture = texBgrDefault;
            m.materials[i].shader = shaderLighting;
        }
    };
    ApplyBgrMat(obsModelRock1);
    ApplyBgrMat(obsModelRock2);
    ApplyBgrMat(jungleTreeModel);
    ApplyBgrMat(wallModel);
    ApplyBgrMat(envSector3);
    ApplyBgrMat(envSector2);"""

replace_block(apply_mat_target, apply_mat_replacement, '5c. Apply texBgrDefault to rocks and trees')

# -----------------------------------------------------------------------------
# 6. Initialize default levelEntities list
# -----------------------------------------------------------------------------
obslist_init_pattern = r'std::vector<KineticObstacle> obsList = \{.*?\n\s*\};\s*if \(!LoadObstaclesJSON\(LEVEL_FILE_PATH, obsList.*?\)\) \{\s*SaveObstaclesJSON\(LEVEL_FILE_PATH, obsList\);\s*\}'

level_entities_init = """std::vector<LevelEntity> levelEntities = {
        // Flume Spiked Bumper Pairs
        { 0, ENT_BUMPER, Vector3{ 14.45f, 0.65f, 66.65f },    0.0f, 1.0f, 0.0f, 1.0f, 1.85f, &obsModelA1, 0, 0, false },
        { 1, ENT_BUMPER, Vector3{ 11.45f, 0.65f, 59.15f },  180.0f, 1.0f, 0.0f, 1.0f, 1.85f, &obsModelA1, 0, 0, false },
        { 2, ENT_BUMPER, Vector3{ 14.45f, 0.65f, 51.72f },    0.0f, 1.0f, 0.0f, 1.0f, 1.85f, &obsModelA1, 0, 0, false },
        { 3, ENT_BUMPER, Vector3{ 11.45f, 0.65f, 44.25f },  180.0f, 1.0f, 0.0f, 1.0f, 1.85f, &obsModelA1, 0, 0, false },
        { 4, ENT_BUMPER, Vector3{ 14.45f, 0.65f, 36.80f },    0.0f, 1.0f, 0.0f, 1.0f, 1.85f, &obsModelA1, 0, 0, false },
        { 5, ENT_BUMPER, Vector3{ 11.45f, 0.65f, 29.35f },  180.0f, 1.0f, 0.0f, 1.0f, 1.85f, &obsModelA1, 0, 0, false },
        { 6, ENT_BUMPER, Vector3{ 14.45f, 0.65f, 21.90f },    0.0f, 1.0f, 0.0f, 1.0f, 1.85f, &obsModelA1, 0, 0, false },
        { 7, ENT_BUMPER, Vector3{ 11.45f, 0.65f, 14.45f },  180.0f, 1.0f, 0.0f, 1.0f, 1.85f, &obsModelA1, 0, 0, false },
        // Kinetic Hazards
        { 8,  ENT_SWING,    Vector3{ 12.95f, 0.5f,  2.0f },     0.0f, 1.0f, 0.0f, 1.0f, 2.2f, &obsModelF3, 0, 0, false },
        { 9,  ENT_GATE,     Vector3{ 12.95f, 0.5f, -15.0f },    0.0f, 1.0f, 0.0f, 1.2f, 2.2f, &obsModelF10, 0, 0, false },
        { 10, ENT_ROTOR,    Vector3{ 12.95f, 0.5f, -32.0f },    0.0f, 1.0f, 0.0f, 1.0f, 2.4f, &obsModelB5, 0, 0, false },
        { 11, ENT_TIPPING,  Vector3{ 12.95f, 0.5f, -48.0f },    0.0f, 1.0f, 0.0f, 1.0f, 2.2f, &obsModelB2, 0, 0, false },
        { 12, ENT_PENDULUM, Vector3{ 12.95f, 0.5f, -65.0f },    0.0f, 1.0f, 0.0f, 1.0f, 2.2f, &obsModelC2, 0, 0, false },
        // Dark-Grey Stone Rocks (#4A4848 Authentic Image 2 Parity)
        { 13, ENT_ROCK_DARK1, Vector3{  5.50f, 0.2f, 78.50f },  25.0f, 1.2f, 0.0f, 0.0f, 2.2f, &obsModelRock1, 0, 0, false },
        { 14, ENT_ROCK_DARK2, Vector3{ 23.40f, 0.2f, 74.20f }, -40.0f, 1.3f, 0.0f, 0.0f, 2.4f, &obsModelRock2, 0, 0, false },
        { 15, ENT_ROCK_DARK1, Vector3{ 19.54f, 0.2f, 96.57f },   0.0f, 1.0f, 0.0f, 0.0f, 2.2f, &obsModelRock1, 0, 0, false },
        { 16, ENT_ROCK_DARK2, Vector3{  6.42f, 0.2f, 88.49f },  60.0f, 1.1f, 0.0f, 0.0f, 2.4f, &obsModelRock2, 0, 0, false },
        // Jungle Trees (Green Foliage, Brown Trunk Authentic Image 3 Parity)
        { 17, ENT_TREE_JUNGLE, Vector3{  3.20f, 0.4f, 72.00f },  15.0f, 1.0f, 0.0f, 0.0f, 1.5f, &jungleTreeModel, 0, 0, false },
        { 18, ENT_TREE_JUNGLE, Vector3{ 25.80f, 0.4f, 68.50f }, -30.0f, 1.1f, 0.0f, 0.0f, 1.5f, &jungleTreeModel, 0, 0, false },
        { 19, ENT_TREE_JUNGLE, Vector3{ 18.20f, 0.4f, 92.40f },  75.0f, 0.9f, 0.0f, 0.0f, 1.5f, &jungleTreeModel, 0, 0, false },
        { 20, ENT_TREE_JUNGLE, Vector3{  8.50f, 0.4f, 94.10f }, -80.0f, 1.0f, 0.0f, 0.0f, 1.5f, &jungleTreeModel, 0, 0, false },
        // Start Pond Decorative Reeds & Lilies
        { 21, ENT_LILY_PAD,    Vector3{ 10.20f, 0.04f, 78.50f },   0.0f, 1.0f, 0.0f, 0.0f, 0.9f, nullptr, 0, 0, false },
        { 22, ENT_LILY_PAD,    Vector3{ 18.80f, 0.04f, 65.20f },   0.0f, 1.0f, 0.0f, 0.0f, 0.9f, nullptr, 0, 0, false },
        { 23, ENT_REEDS,       Vector3{  6.80f, 0.05f, 62.00f },   0.0f, 1.0f, 0.0f, 0.0f, 1.1f, nullptr, 0, 0, false },
        { 24, ENT_REEDS,       Vector3{ 22.10f, 0.05f, 82.50f },   0.0f, 1.0f, 0.0f, 0.0f, 1.1f, nullptr, 0, 0, false }
    };

    if (!LoadLevelLayoutJSON(LEVEL_LAYOUT_PATH, levelEntities, &obsModelA1, &obsModelB5, &obsModelB2, &obsModelC2, &obsModelF10, &obsModelF3, &obsModelRock1, &obsModelRock2, &jungleTreeModel, &cp1Model)) {
        SaveLevelLayoutJSON(LEVEL_LAYOUT_PATH, levelEntities);
    }"""

replace_block(obslist_init_pattern, level_entities_init, '6. Initialize default levelEntities list')

# -----------------------------------------------------------------------------
# 7. Update TriggerBoatDeath to always increment totalDeaths++
# -----------------------------------------------------------------------------
trigger_death_pattern = r'void TriggerBoatDeath\(BoatRaft& raft, DeathCause cause\)\s*\{\s*if \(raft\.isDestroyed\) return;\s*raft\.isDestroyed = true;\s*raft\.deathTimer = 0\.0f;\s*raft\.lastDeathCause = cause;\s*raft\.deaths\+\+;'
trigger_death_replacement = """void TriggerBoatDeath(BoatRaft& raft, DeathCause cause) {
    if (raft.isDestroyed) return;
    raft.isDestroyed = true;
    raft.deathTimer = 0.0f;
    raft.lastDeathCause = cause;
    raft.deaths++;
    totalDeaths++; // Live UI Death counter increment"""

replace_block(trigger_death_pattern, trigger_death_replacement, '7. Fix Live Death Counter in TriggerBoatDeath')

# -----------------------------------------------------------------------------
# 8. River Flow & Boundary Physics (Circular Start Arena + Free Abyss Fall)
# -----------------------------------------------------------------------------
river_physics_pattern = r'// Flume Canal Hard Collision vs Open River Containment.*?// Open river bank soft containment.*?raft\.velocity = Vector3Add\(raft\.velocity, Vector3Scale\(riverNormal, pushSign \* 45\.0f \* pen \* dt\)\);\s*\}\s*\}'
river_physics_replacement = """// Circular Start Lake Boundary vs Flume Canal vs Open River Abyss Falls
        Vector3 startPondCenter = { 14.45f, 0.0f, 72.0f };
        float startPondRadius = 16.5f;

        if (raft.position.z >= 68.0f) {
            // Circular Start Arena (Image 4 Parity): Enclosed by orange bouncy rim
            Vector3 dLake = Vector3Subtract(raft.position, startPondCenter);
            dLake.y = 0.0f;
            float distLake = Vector3Length(dLake);
            if (distLake > (startPondRadius - 1.25f)) {
                // Check if exiting into canal (Z < 72.0f and X near 12.95m)
                bool nearCanalMouth = (fabsf(raft.position.x - 12.95f) < 2.0f && raft.position.z < 73.0f);
                if (!nearCanalMouth) {
                    Vector3 nLake = Vector3Normalize(dLake);
                    raft.position = Vector3Add(startPondCenter, Vector3Scale(nLake, startPondRadius - 1.25f));
                    float vn = Vector3DotProduct(raft.velocity, nLake);
                    if (vn > 0.0f) {
                        raft.velocity = Vector3Subtract(raft.velocity, Vector3Scale(nLake, 1.8f * vn));
                        PlaySound(fartSounds[2]);
                    }
                }
            }
        } else if (raft.position.z > 36.0f) {
            // Hard Flume Canal Walls
            Vector3 toCenter = Vector3Subtract(raft.position, riverCenter);
            float crossDist = Vector3DotProduct(toCenter, riverNormal);
            float canalHalfW = 1.42f;
            if (fabsf(crossDist) > canalHalfW) {
                float pushSign = (crossDist > 0.0f) ? 1.0f : -1.0f;
                raft.position = Vector3Add(riverCenter, Vector3Scale(riverNormal, pushSign * canalHalfW));
                Vector3 wallNormal = Vector3Scale(riverNormal, -pushSign);
                float normalVel = Vector3DotProduct(raft.velocity, wallNormal);
                if (normalVel < 0.0f) {
                    raft.velocity = Vector3Subtract(raft.velocity, Vector3Scale(wallNormal, 1.6f * normalVel));
                    raft.velocity = Vector3Scale(raft.velocity, 0.70f);
                    PlaySound(fartSounds[2]);
                    camCtrl.shakeTrauma = fmaxf(camCtrl.shakeTrauma, 0.25f);
                }
            }
        } else {
            // Open River: If boat steers off track, free fall into the void abyss!
            Vector3 toCenter = Vector3Subtract(raft.position, riverCenter);
            float crossDist = Vector3DotProduct(toCenter, riverNormal);
            float riverBankW = 7.5f;
            if (fabsf(crossDist) > riverBankW) {
                // Outside river banks -> free fall into the abyss!
                raft.velocity.y -= 32.0f * dt;
                if (raft.position.y < -12.0f) {
                    TriggerBoatDeath(raft, DEATH_VOID);
                }
            }
        }"""

replace_block(river_physics_pattern, river_physics_replacement, '8. Circular Start Arena & Free Fall Physics')

# -----------------------------------------------------------------------------
# 9. Realistic Tight Collisions for LevelEntity (No more 5-meter hits!)
# -----------------------------------------------------------------------------
collision_pattern = r'// 5\. Update Kinetic Obstacles & Spring Bumpers.*?for \(auto& obs : obsList\) \{.*?float d = Vector3Distance\(raft\.position, obs\.position\);.*?TriggerBoatDeath\(raft, DEATH_OBSTACLE\);\s*\}\s*\}\s*\}'
collision_replacement = """// 5. Update Kinetic Obstacles & Spring Bumpers
        for (auto& ent : levelEntities) {
            ent.animTimer += dt * ent.animSpeed;
            if (ent.type == ENT_ROTOR) ent.rotationY += dt * 90.0f * ent.animSpeed;
            if (ent.type == ENT_TIPPING) ent.rotationY = sinf(ent.animTimer) * 45.0f;
            if (ent.type == ENT_PENDULUM) ent.rotationY = cosf(ent.animTimer) * 55.0f;
            if (ent.type == ENT_GATE) ent.rotationY = sinf(ent.animTimer * 1.5f) * 75.0f;
            if (ent.type == ENT_SWING) ent.rotationY = sinf(ent.animTimer * 2.0f) * 60.0f;

            if (ent.type == ENT_BUMPER) {
                if (ent.bumperCooldownTimer > 0.0f) ent.bumperCooldownTimer -= dt;
                if (ent.isExpanded) {
                    ent.bumperExpandTimer -= dt;
                    if (ent.bumperExpandTimer <= 0.0f) {
                        ent.isExpanded = false;
                        ent.bumperCooldownTimer = BUMPER_COOLDOWN;
                    }
                }

                // Tight Physical Contact (BUMPER_IMPACT_RADIUS = 1.85m)
                float d = Vector3Distance(raft.position, ent.position);
                if (d < BUMPER_IMPACT_RADIUS && ent.bumperCooldownTimer <= 0.0f && !raft.isDestroyed) {
                    ent.isExpanded = true;
                    ent.bumperExpandTimer = BUMPER_EXPAND_DURATION;
                    // 1:1 Bumper Launch: 18 m/s horizontal + 8 m/s vertical
                    float rad = ent.rotationY * DEG2RAD;
                    Vector3 launchDir = { sinf(rad), 0.5f, cosf(rad) };
                    launchDir = Vector3Normalize(launchDir);
                    raft.velocity = Vector3Scale(launchDir, 18.0f);
                    raft.angularVel.y += 15.0f;
                    PlaySound(fartSounds[1]);
                    camCtrl.shakeTrauma = fmaxf(camCtrl.shakeTrauma, 0.65f);
                }
            } else if (ent.type >= ENT_ROTOR && ent.type <= ENT_SWING) {
                // Moving kinetic hazard collision (tight touch only)
                float d = Vector3Distance(raft.position, ent.position);
                if (d < ent.radius && !raft.isDestroyed) {
                    TriggerBoatDeath(raft, DEATH_OBSTACLE);
                }
            } else if (ent.type == ENT_ROCK_DARK1 || ent.type == ENT_ROCK_DARK2) {
                // Rock obstacle collision (tight physical boundary)
                float d = Vector3Distance(raft.position, ent.position);
                if (d < ent.radius && !raft.isDestroyed) {
                    Vector3 pushOut = Vector3Normalize(Vector3Subtract(raft.position, ent.position));
                    raft.position = Vector3Add(ent.position, Vector3Scale(pushOut, ent.radius + 0.1f));
                    raft.velocity = Vector3Scale(pushOut, 5.0f);
                    TriggerBoatDeath(raft, DEATH_OBSTACLE);
                }
            }
        }"""

replace_block(collision_pattern, collision_replacement, '9. Tight LevelEntity Collision & Physical Launches')

# -----------------------------------------------------------------------------
# 10. Clamp Predators Y strictly to water level
# -----------------------------------------------------------------------------
croc_pattern = r'c\.position = Vector3Add\(c\.position, Vector3Scale\(vel, dt\)\);\s*c\.rotationY = atan2f\(dir\.x, dir\.z\) \* RAD2DEG;'
croc_replacement = """c.position = Vector3Add(c.position, Vector3Scale(vel, dt));
            c.position.y = 0.05f; // Clamp strictly to water surface (no flying!)
            c.rotationY = atan2f(dir.x, dir.z) * RAD2DEG;"""

replace_block(croc_pattern, croc_replacement, '10a. Clamp Crocodile Y to water surface')

shark_pattern = r's\.position = Vector3Add\(s\.position, Vector3Scale\(vel, dt\)\);\s*s\.rotationY = atan2f\(dir\.x, dir\.z\) \* RAD2DEG;'
shark_replacement = """s.position = Vector3Add(s.position, Vector3Scale(vel, dt));
            s.position.y = 0.02f; // Clamp strictly to water surface (no flying!)
            s.rotationY = atan2f(dir.x, dir.z) * RAD2DEG;"""

replace_block(shark_pattern, shark_replacement, '10b. Clamp Shark Y to water surface')

# -----------------------------------------------------------------------------
# 11. Add [R] SUICIDE button next to [ESC] LEAVE (Image 4 Parity)
# -----------------------------------------------------------------------------
leave_btn_pattern = r'Rectangle btnEscRec = \{ 16, \(float\)GetScreenHeight\(\) - 44, 96, 28 \};\s*if \(DrawGodotBtn\(btnEscRec, "\[ESC\] LEAVE", Color\{ 35, 40, 50, 240 \}, WHITE, true\) \|\| IsKeyPressed\(KEY_ESCAPE\)\) \{\s*if \(inEditorMode\) \{\s*inEditorMode = false;\s*\} else \{\s*shouldClose = true;\s*\}\s*\}'
leave_btn_replacement = """Rectangle btnEscRec = { 16, (float)GetScreenHeight() - 44, 96, 28 };
        if (DrawGodotBtn(btnEscRec, "[ESC] LEAVE", Color{ 35, 40, 50, 240 }, WHITE, true) || IsKeyPressed(KEY_ESCAPE)) {
            if (inEditorMode) {
                inEditorMode = false;
            } else {
                shouldClose = true;
            }
        }

        // [R] SUICIDE button (Image 4 Parity)
        Rectangle btnSuicideRec = { 120, (float)GetScreenHeight() - 44, 104, 28 };
        if (DrawGodotBtn(btnSuicideRec, "[R] SUICIDE", Color{ 48, 55, 68, 240 }, Color{ 220, 225, 235, 255 }, true) || IsKeyPressed(KEY_R)) {
            if (!inEditorMode && !raft.isDestroyed) {
                TriggerBoatDeath(raft, DEATH_SUICIDE);
            }
        }"""

replace_block(leave_btn_pattern, leave_btn_replacement, '11. Add [R] SUICIDE button and handler')

# -----------------------------------------------------------------------------
# 12. Draw Circular Start Lake, Lily Pads, Reeds, and All Level Entities in 3D
# -----------------------------------------------------------------------------
draw_entities_pattern = r'// C\. Kinetic Hazards & Spring Bumpers\s*for \(const auto& obs : obsList\) \{.*?rlPopMatrix\(\);\s*\}'
draw_entities_replacement = """// A. Circular Start Lake Enclosure (Image 4 Parity)
        // Center: (14.45, 0, 72.0), Radius = 16.5m
        DrawCylinder(Vector3{ 14.45f, -0.05f, 72.0f }, 16.5f, 16.5f, 0.1f, 48, Color{ 30, 160, 235, 140 });
        // Outer Grassy Bank Ring
        DrawRing(Vector3{ 14.45f, 0.15f, 72.0f }, 16.4f, 18.5f, 0.0f, 360.0f, 48, Color{ 76, 175, 40, 255 });
        // Orange Bouncy Bumper Tube Rim
        DrawRing(Vector3{ 14.45f, 0.28f, 72.0f }, 16.1f, 16.6f, 0.0f, 360.0f, 48, Color{ 245, 120, 20, 255 });

        // C. Draw All Level Entities (Obstacles, Rocks, Trees, Lilies, Reeds)
        for (const auto& ent : levelEntities) {
            if (ent.model != nullptr) {
                rlPushMatrix();
                    rlTranslatef(ent.position.x, ent.position.y, ent.position.z);
                    rlRotatef(ent.rotationY, 0.0f, 1.0f, 0.0f);
                    float s = (ent.scale > 0.01f) ? ent.scale : 1.0f;
                    rlScalef(s, s, s);
                    DrawModel(*ent.model, Vector3{ 0, 0, 0 }, 1.0f, WHITE);
                rlPopMatrix();
            } else if (ent.type == ENT_LILY_PAD) {
                // Floating Water Lily with Pink Lotus
                DrawCircle3D(ent.position, ent.radius, Vector3{ 1, 0, 0 }, 90.0f, Color{ 34, 139, 34, 230 });
                DrawCircle3D(Vector3{ ent.position.x, ent.position.y + 0.03f, ent.position.z }, 0.28f, Vector3{ 1, 0, 0 }, 90.0f, Color{ 255, 105, 180, 255 });
            } else if (ent.type == ENT_REEDS) {
                // Water Reeds
                for (int r = 0; r < 5; r++) {
                    float roffX = sinf((float)r * 1.3f) * 0.4f;
                    float roffZ = cosf((float)r * 1.3f) * 0.4f;
                    DrawCylinder(Vector3{ ent.position.x + roffX, ent.position.y + 0.45f, ent.position.z + roffZ }, 0.04f, 0.04f, 0.9f, 6, Color{ 46, 125, 50, 255 });
                    DrawCylinder(Vector3{ ent.position.x + roffX, ent.position.y + 0.95f, ent.position.z + roffZ }, 0.07f, 0.07f, 0.25f, 6, Color{ 160, 82, 45, 255 });
                }
            }
        }"""

replace_block(draw_entities_pattern, draw_entities_replacement, '12. Draw Circular Start Lake & Level Entities')

# -----------------------------------------------------------------------------
# 13. Update 3D Raycast Selection & Editor Gizmos to use levelEntities
# -----------------------------------------------------------------------------
editor_raycast_pattern = r'if \(IsMouseButtonPressed\(MOUSE_BUTTON_LEFT\) && !CheckCollisionPointRec\(mPos, panelRec\)\) \{.*?for \(size_t i = 0; i < obsList\.size\(\); i\+\+\) \{.*?selectedObs = \(int\)i;\s*break;\s*\}\s*\}\s*\}'
editor_raycast_replacement = """if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !CheckCollisionPointRec(mPos, panelRec)) {
                Ray mRay = GetMouseRay(mPos, editorCam);
                int hitIdx = -1;
                float closestDist = 99999.0f;
                for (size_t i = 0; i < levelEntities.size(); i++) {
                    float r = (levelEntities[i].radius > 0.5f) ? levelEntities[i].radius : 1.5f;
                    BoundingBox oBox = { Vector3Subtract(levelEntities[i].position, Vector3{ r, 2.0f, r }), Vector3Add(levelEntities[i].position, Vector3{ r, 2.0f, r }) };
                    RayCollision rHit = GetRayCollisionBox(mRay, oBox);
                    if (rHit.hit && rHit.distance < closestDist) {
                        closestDist = rHit.distance;
                        hitIdx = (int)i;
                    }
                }
                if (hitIdx >= 0) {
                    selectedObs = hitIdx;
                    editorMsg = TextFormat("Selected: #%d (%s)", selectedObs, ENTITY_TYPE_NAMES[levelEntities[selectedObs].type]);
                    editorMsgTimer = 2.0f;
                }
            }

            // Keyboard Fine Positioning & Rotation for Selected Entity
            if (selectedObs >= 0 && selectedObs < (int)levelEntities.size()) {
                auto& o = levelEntities[selectedObs];
                float editSpd = (IsKeyDown(KEY_LEFT_SHIFT) ? 10.0f : 4.0f) * dt;
                if (IsKeyDown(KEY_J)) o.position.x -= editSpd;
                if (IsKeyDown(KEY_L)) o.position.x += editSpd;
                if (IsKeyDown(KEY_I)) o.position.z += editSpd;
                if (IsKeyDown(KEY_K)) o.position.z -= editSpd;
                if (IsKeyDown(KEY_U)) o.position.y += editSpd;
                if (IsKeyDown(KEY_O)) o.position.y -= editSpd;
                if (IsKeyDown(KEY_R)) o.rotationY += 45.0f * dt;
                if (IsKeyDown(KEY_T)) o.rotationY -= 45.0f * dt;
            }"""

replace_block(editor_raycast_pattern, editor_raycast_replacement, '13. Raycasting and Key Controls for levelEntities')

# -----------------------------------------------------------------------------
# 14. Update 3D Gizmo loop to iterate levelEntities
# -----------------------------------------------------------------------------
gizmo_pattern = r'for \(size_t i = 0; i < obsList\.size\(\); i\+\+\) \{.*?bool isSel = \(\(int\)i == selectedObs\);.*?DrawLine3D\(Vector3\{ oPos\.x, oPos\.y, oPos\.z \}, Vector3\{ oPos\.x, oPos\.y \+ 2\.5f, oPos\.z \}, GREEN\);\s*\}\s*\}'
gizmo_replacement = """for (size_t i = 0; i < levelEntities.size(); i++) {
                    bool isSel = ((int)i == selectedObs);
                    Vector3 oPos = levelEntities[i].position;
                    float r = (levelEntities[i].radius > 0.5f) ? levelEntities[i].radius : 1.5f;
                    DrawCircle3D(Vector3{ oPos.x, 0.05f, oPos.z }, r, Vector3{ 1, 0, 0 }, 90.0f, isSel ? Color{ 255, 235, 50, 180 } : Color{ 80, 180, 255, 90 });
                    if (isSel) {
                        DrawBoundingBox(BoundingBox{ Vector3Subtract(oPos, Vector3{ r, 1.5f, r }), Vector3Add(oPos, Vector3{ r, 1.5f, r }) }, Color{ 255, 215, 0, 255 });
                        DrawLine3D(Vector3{ oPos.x, oPos.y, oPos.z }, Vector3{ oPos.x + 2.0f, oPos.y, oPos.z }, RED);
                        DrawLine3D(Vector3{ oPos.x, oPos.y, oPos.z }, Vector3{ oPos.x, oPos.y + 2.5f, oPos.z }, GREEN);
                        DrawLine3D(Vector3{ oPos.x, oPos.y, oPos.z }, Vector3{ oPos.x, oPos.y, oPos.z + 2.0f }, BLUE);
                    }
                }"""

replace_block(gizmo_pattern, gizmo_replacement, '14. 3D Editor Gizmos for levelEntities')

# -----------------------------------------------------------------------------
# 15. Update Inspector Panel to support all 12 Entity Types + Save/Duplicate/Add/Delete
# -----------------------------------------------------------------------------
inspector_pattern = r'if \(selectedObs >= 0 && selectedObs < \(int\)obsList\.size\(\)\) \{.*?if \(DrawGodotBtn\(btnSaveRec, "Save Level Layout \(Ctrl\+S\)", Color\{ 71, 140, 191, 255 \}, WHITE, true\).*?editorMsgTimer = 3\.5f;\s*\}'
inspector_replacement = """if (selectedObs >= 0 && selectedObs < (int)levelEntities.size()) {
                auto& o = levelEntities[selectedObs];
                DrawTextEx(fontUIBold, TextFormat("Selected: #%d (%s)", selectedObs, ENTITY_TYPE_NAMES[o.type]), Vector2{ panelRec.x + 14, py }, 13, 0.5f, Color{ 142, 202, 230, 255 });
                py += 24;

                // Entity Type Switcher
                DrawTextEx(fontUI, "Entity Type:", Vector2{ panelRec.x + 14, py }, 11, 0.3f, Color{ 180, 190, 205, 255 });
                py += 16;
                Rectangle btnPrevType = { panelRec.x + 12, py, 120, 24 };
                Rectangle btnNextType = { panelRec.x + 140, py, 120, 24 };
                if (DrawGodotBtn(btnPrevType, "< Prev Type", Color{ 36, 43, 58, 255 }, WHITE, true)) {
                    o.type = (EntityType)(((int)o.type - 1 + ENT_COUNT) % ENT_COUNT);
                    AssignEntityModelAndProps(o, &obsModelA1, &obsModelB5, &obsModelB2, &obsModelC2, &obsModelF10, &obsModelF3, &obsModelRock1, &obsModelRock2, &jungleTreeModel, &cp1Model);
                    editorMsg = TextFormat("Type: %s", ENTITY_TYPE_NAMES[o.type]);
                    editorMsgTimer = 2.0f;
                }
                if (DrawGodotBtn(btnNextType, "Next Type >", Color{ 36, 43, 58, 255 }, WHITE, true)) {
                    o.type = (EntityType)(((int)o.type + 1) % ENT_COUNT);
                    AssignEntityModelAndProps(o, &obsModelA1, &obsModelB5, &obsModelB2, &obsModelC2, &obsModelF10, &obsModelF3, &obsModelRock1, &obsModelRock2, &jungleTreeModel, &cp1Model);
                    editorMsg = TextFormat("Type: %s", ENTITY_TYPE_NAMES[o.type]);
                    editorMsgTimer = 2.0f;
                }
                py += 30;

                // Position X/Y/Z Controls
                DrawTextEx(fontUI, TextFormat("Position: (%.2f, %.2f, %.2f)", o.position.x, o.position.y, o.position.z), Vector2{ panelRec.x + 14, py }, 11, 0.3f, Color{ 220, 225, 235, 255 });
                py += 18;
                Rectangle btnXm = { panelRec.x + 12, py, 58, 22 };
                Rectangle btnXp = { panelRec.x + 74, py, 58, 22 };
                Rectangle btnZm = { panelRec.x + 140, py, 58, 22 };
                Rectangle btnZp = { panelRec.x + 202, py, 58, 22 };
                if (DrawGodotBtn(btnXm, "X -", Color{ 36, 43, 58, 255 }, WHITE, true)) o.position.x -= 0.5f;
                if (DrawGodotBtn(btnXp, "X +", Color{ 36, 43, 58, 255 }, WHITE, true)) o.position.x += 0.5f;
                if (DrawGodotBtn(btnZm, "Z -", Color{ 36, 43, 58, 255 }, WHITE, true)) o.position.z -= 0.5f;
                if (DrawGodotBtn(btnZp, "Z +", Color{ 36, 43, 58, 255 }, WHITE, true)) o.position.z += 0.5f;
                py += 26;

                Rectangle btnYm = { panelRec.x + 12, py, 58, 22 };
                Rectangle btnYp = { panelRec.x + 74, py, 58, 22 };
                Rectangle btnRm = { panelRec.x + 140, py, 58, 22 };
                Rectangle btnRp = { panelRec.x + 202, py, 58, 22 };
                if (DrawGodotBtn(btnYm, "Y -", Color{ 36, 43, 58, 255 }, WHITE, true)) o.position.y -= 0.25f;
                if (DrawGodotBtn(btnYp, "Y +", Color{ 36, 43, 58, 255 }, WHITE, true)) o.position.y += 0.25f;
                if (DrawGodotBtn(btnRm, "Rot -15", Color{ 36, 43, 58, 255 }, WHITE, true)) o.rotationY -= 15.0f;
                if (DrawGodotBtn(btnRp, "Rot +15", Color{ 36, 43, 58, 255 }, WHITE, true)) o.rotationY += 15.0f;
                py += 32;

                // Duplicate & Delete Entity
                Rectangle btnDup = { panelRec.x + 12, py, 120, 26 };
                Rectangle btnDel = { panelRec.x + 140, py, 120, 26 };
                if (DrawGodotBtn(btnDup, "Duplicate", Color{ 48, 85, 120, 255 }, WHITE, true) || (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_D))) {
                    LevelEntity dup = o;
                    dup.id = (int)levelEntities.size();
                    dup.position.x += 1.5f;
                    levelEntities.push_back(dup);
                    selectedObs = (int)levelEntities.size() - 1;
                    editorMsg = "Entity Duplicated!";
                    editorMsgTimer = 2.0f;
                }
                if (DrawGodotBtn(btnDel, "Delete", Color{ 145, 45, 45, 255 }, WHITE, true) || IsKeyPressed(KEY_DELETE)) {
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
                SaveLevelLayoutJSON(LEVEL_LAYOUT_PATH, levelEntities);
                editorMsg = "Saved Layout to level_layout.json!";
                editorMsgTimer = 3.5f;
            }"""

replace_block(inspector_pattern, inspector_replacement, '15. Inspector Panel for 12 Entity Types + Save/Duplicate/Add/Delete')

# -----------------------------------------------------------------------------
# 16. Unload jungleTreeModel and texBgrDefault
# -----------------------------------------------------------------------------
unloads_pattern = r'UnloadTexture\(texEsc\);'
unloads_replacement = """UnloadTexture(texEsc);
    UnloadTexture(texBgrDefault);
    UnloadModel(jungleTreeModel);"""

replace_block(unloads_pattern, unloads_replacement, '16. Unload models and textures')

with open("CustomEngine/native/GameEngine.cpp", "w", encoding="utf-8") as f:
    f.write(code)

print("Saved updated CustomEngine/native/GameEngine.cpp successfully! New length:", len(code))

