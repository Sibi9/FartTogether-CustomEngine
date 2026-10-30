import re, pathlib, sys

path = pathlib.Path("CustomEngine/native/GameEngine.cpp")
text = path.read_text(encoding="utf-8")

# 1. Update FS_LIGHTING
fs_lighting_new = '''static const char* FS_LIGHTING = R"(
#version 330
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragNormal;
in vec3 fragPosition;
uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform vec3 lightDir;
uniform vec3 lightColor;
uniform vec3 ambientColor;
uniform float uRedness;   // 0.0 = normal, 1.0 = deep scarlet vascular redness
uniform float uSkinU;     // 0.0 = non-character, >0.0 = character base skin U coordinate
uniform float uSkinV;     // character base skin V coordinate
out vec4 finalColor;

void main() {
    vec2 uv = fragTexCoord;
    // Fix blue/pink UV patch on hippo lower back/butt (UV.x in [0.42, 0.50])
    if (uSkinU > 0.01 && uv.x > 0.42 && uv.x < 0.50) {
        uv = vec2(uSkinU, uSkinV);
    }
    vec4 texColor = texture(texture0, uv);
    vec3 N = normalize(fragNormal);
    vec3 L = normalize(lightDir);
    float diff = max(dot(N, L), 0.0);
    // Stylized Cel-Shading two-tone soft ramp matching Unity Toon Shader
    float celDiff = smoothstep(0.15, 0.55, diff) * 0.45 + diff * 0.55;
    vec3 diffuse = celDiff * lightColor;
    vec3 lighting = ambientColor + diffuse;

    // Authentic Localized Butt Redness Mask (Center: 0.0, 0.40, -0.14)
    vec3 buttCenter = vec3(0.0, 0.40, -0.14);
    vec3 buttScale  = vec3(0.36, 0.28, 0.24);
    float buttDist  = length((fragPosition - buttCenter) / buttScale);
    float buttMask  = clamp(1.0 - smoothstep(0.45, 1.15, buttDist), 0.0, 1.0);

    vec3 redSkin = mix(texColor.rgb, vec3(0.96, 0.12, 0.15), uRedness * buttMask * 0.90);
    vec3 litColor = redSkin * lighting * colDiffuse.rgb;
    finalColor = vec4(litColor, texColor.a * colDiffuse.a);
}
)";'''

text = re.sub(r'static const char\* FS_LIGHTING = R"\([\s\S]*?\);', fs_lighting_new, text, count=1)

# 2. Add QueryActualWaterSurface after FindClosestSplineT
query_water_func = '''static void QueryActualWaterSurface(Vector3 pos, float& outWaterY, float& outDistXZ, Vector3& outNearestWaterPos) {
    float bestDistSq = 1e9f;
    int bestIdx = 0;
    for (int i = 0; i < WATER_SAMPLE_COUNT; i++) {
        float dx = pos.x - AUTHENTIC_WATER_SAMPLES[i].x;
        float dz = pos.z - AUTHENTIC_WATER_SAMPLES[i].z;
        float dSq = dx * dx + dz * dz;
        if (dSq < bestDistSq) {
            bestDistSq = dSq;
            bestIdx = i;
        }
    }
    outDistXZ = sqrtf(bestDistSq);
    outWaterY = AUTHENTIC_WATER_SAMPLES[bestIdx].y;
    outNearestWaterPos = AUTHENTIC_WATER_SAMPLES[bestIdx];
}
'''

spline_t_pat = r'(return bestT;\s*\})'
text = re.sub(spline_t_pat, r'\1\n\n' + query_water_func, text, count=1)

# 3. Replace EntityType, ENTITY_TYPE_NAMES, ObstacleModels, AssignEntityModelAndProps, and LoadLevelLayoutJSON
entity_arch_new = '''// Unified Level Entity Architecture (Obstacles, Rocks, Trees, Checkpoints, Props)
enum EntityType {
    ENT_OBS_A1 = 0,   // GM_3D_ObstacleA1.obj (Spiked wall bumper)
    ENT_OBS_B2 = 1,   // GM_3D_ObstacleB2.obj (Tipping bar)
    ENT_OBS_B3 = 2,   // GM_3D_ObstacleB3.obj (Horizontal spiked spinner)
    ENT_OBS_B5 = 3,   // GM_3D_ObstacleB5.obj (Double sweep rotor)
    ENT_OBS_C1 = 4,   // GM_3D_ObstacleC1.obj (Swinging spiked arc)
    ENT_OBS_C2 = 5,   // GM_3D_ObstacleC2.obj (Pendulum mace)
    ENT_OBS_C4 = 6,   // GM_3D_ObstacleC4.obj (Quad spiked spinner)
    ENT_OBS_C5 = 7,   // GM_3D_ObstacleC5.obj (Vertical spiked wheel)
    ENT_OBS_F3 = 8,   // GM_3D_ObstacleF3.obj (Swinging barrier)
    ENT_OBS_F10 = 9,  // GM_3D_ObstacleF10.obj (Large rotating gate)
    ENT_OBS_F11 = 10, // GM_3D_ObstacleF11.obj (Blue spiked drum cylinder)
    ENT_OBS_F13 = 11, // GM_3D_ObstacleF13.obj (Medium rotating arm)
    ENT_OBS_F14 = 12, // GM_3D_ObstacleF14_P1.obj (Sliding barrier)
    ENT_OBS_F18 = 13, // GM_3D_ObstacleF18.obj (Spike post)
    ENT_OBS_F23 = 14, // GM_3D_ObstacleF23.obj (Double swinging barrier)
    ENT_ROCK_1 = 15,  // GM_3D_Obstacle_Rock1.obj
    ENT_ROCK_2 = 16,  // GM_3D_Obstacle_Rock2.obj
    ENT_GUST = 17,    // Wind Gust zone
    ENT_COUNT = 18
};

static const char* ENTITY_TYPE_NAMES[] = {
    "OBS_A1_BUMPER", "OBS_B2_TIPPING", "OBS_B3_SPINNER", "OBS_B5_ROTOR",
    "OBS_C1_ARC", "OBS_C2_PENDULUM", "OBS_C4_QUAD", "OBS_C5_WHEEL",
    "OBS_F3_SWING", "OBS_F10_GATE", "OBS_F11_DRUM", "OBS_F13_ARM",
    "OBS_F14_SLIDER", "OBS_F18_SPIKE", "OBS_F23_DUAL",
    "ROCK_DARK_1", "ROCK_DARK_2", "WIND_GUST"
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
};

struct ObstacleModels {
    Model mA1;
    Model mB2;
    Model mB3;
    Model mB5;
    Model mC1;
    Model mC2;
    Model mC4;
    Model mC5;
    Model mF3;
    Model mF10;
    Model mF11;
    Model mF13;
    Model mF14;
    Model mF18;
    Model mF23;
    Model mR1;
    Model mR2;
    Model mTree;
};
static ObstacleModels gObsModels;

static const char* LEVEL_LAYOUT_PATH = "level_layout.json";

void AssignEntityModelAndProps(LevelEntity& e) {
    if (e.scale <= 0.01f || !std::isfinite(e.scale)) e.scale = 1.0f;
    if (e.radius <= 0.01f || !std::isfinite(e.radius)) e.radius = 2.0f;
    if (e.animSpeed < 0.0f || !std::isfinite(e.animSpeed)) e.animSpeed = 1.0f;

    switch (e.type) {
        case ENT_OBS_A1:  e.model = &gObsModels.mA1;  e.radius = 1.85f * e.scale; break;
        case ENT_OBS_B2:  e.model = &gObsModels.mB2;  e.radius = 3.20f * e.scale; break;
        case ENT_OBS_B3:  e.model = &gObsModels.mB3;  e.radius = 2.50f * e.scale; break;
        case ENT_OBS_B5:  e.model = &gObsModels.mB5;  e.radius = 3.80f * e.scale; break;
        case ENT_OBS_C1:  e.model = &gObsModels.mC1;  e.radius = 2.50f * e.scale; break;
        case ENT_OBS_C2:  e.model = &gObsModels.mC2;  e.radius = 2.20f * e.scale; break;
        case ENT_OBS_C4:  e.model = &gObsModels.mC4;  e.radius = 2.50f * e.scale; break;
        case ENT_OBS_C5:  e.model = &gObsModels.mC5;  e.radius = 3.80f * e.scale; break;
        case ENT_OBS_F3:  e.model = &gObsModels.mF3;  e.radius = 2.20f * e.scale; break;
        case ENT_OBS_F10: e.model = &gObsModels.mF10; e.radius = 3.50f * e.scale; break;
        case ENT_OBS_F11: e.model = &gObsModels.mF11; e.radius = 1.65f * e.scale; break;
        case ENT_OBS_F13: e.model = &gObsModels.mF13; e.radius = 2.80f * e.scale; break;
        case ENT_OBS_F14: e.model = &gObsModels.mF14; e.radius = 2.40f * e.scale; break;
        case ENT_OBS_F18: e.model = &gObsModels.mF18; e.radius = 1.20f * e.scale; break;
        case ENT_OBS_F23: e.model = &gObsModels.mF23; e.radius = 2.80f * e.scale; break;
        case ENT_ROCK_1:  e.model = &gObsModels.mR1;  e.radius = 2.10f * e.scale; break;
        case ENT_ROCK_2:  e.model = &gObsModels.mR2;  e.radius = 2.30f * e.scale; break;
        case ENT_GUST:    e.model = nullptr;          e.radius = 4.50f * e.scale; break;
        default:          e.type = ENT_OBS_A1; e.model = &gObsModels.mA1; e.radius = 1.85f * e.scale; break;
    }
}

void SaveLevelLayoutJSON(const std::string& path, const std::vector<LevelEntity>& list) {
    std::string resolvedPath = ResolveAsset(path);
    FILE* f = fopen(resolvedPath.c_str(), "w");
    if (!f) f = fopen(path.c_str(), "w");
    if (!f) return;
    fprintf(f, "[\n");
    for (size_t i = 0; i < list.size(); i++) {
        const auto& e = list[i];
        fprintf(f,
            "  {\\n"
            "    \\"id\\": %d,\\n"
            "    \\"type\\": %d,\\n"
            "    \\"x\\": %.3f,\\n"
            "    \\"y\\": %.3f,\\n"
            "    \\"z\\": %.3f,\\n"
            "    \\"rotY\\": %.2f,\\n"
            "    \\"scale\\": %.2f,\\n"
            "    \\"speed\\": %.2f,\\n"
            "    \\"radius\\": %.2f\\n"
            "  }%s\\n",
            (int)i, (int)e.type, e.position.x, e.position.y, e.position.z,
            e.rotationY, e.scale, e.animSpeed, e.radius,
            (i + 1 < list.size()) ? "," : "");
    }
    fprintf(f, "]\\n");
    fclose(f);
}

static bool ParseJsonFloatField(const char* objStart, const char* objEnd, const char* key, float& outVal) {
    const char* p = strstr(objStart, key);
    if (!p || p >= objEnd) return false;
    p += strlen(key);
    while (p < objEnd && (*p == ' ' || *p == '\\t' || *p == ':')) p++;
    return sscanf(p, "%f", &outVal) == 1;
}

static bool ParseJsonIntField(const char* objStart, const char* objEnd, const char* key, int& outVal) {
    const char* p = strstr(objStart, key);
    if (!p || p >= objEnd) return false;
    p += strlen(key);
    while (p < objEnd && (*p == ' ' || *p == '\\t' || *p == ':')) p++;
    return sscanf(p, "%d", &outVal) == 1;
}

bool LoadLevelLayoutJSON(const std::string& path, std::vector<LevelEntity>& list) {
    std::string resolvedPath = ResolveAsset(path);
    char* rawText = LoadFileText(resolvedPath.c_str());
    if (!rawText) return false;

    list.clear();
    int nextId = 0;
    const char* cursor = rawText;
    while (const char* objStart = strchr(cursor, '{')) {
        const char* objEnd = strchr(objStart, '}');
        if (!objEnd) break;

        int t = 0;
        if (ParseJsonIntField(objStart, objEnd, "\\"type\\"", t)) {
            if (t < 0 || t >= (int)ENT_COUNT) t = 0;
            float x = 0.0f, y = 0.0f, z = 0.0f, rotY = 0.0f, scale = 1.0f, spd = 1.0f, rad = 2.0f;
            ParseJsonFloatField(objStart, objEnd, "\\"x\\"", x);
            ParseJsonFloatField(objStart, objEnd, "\\"y\\"", y);
            ParseJsonFloatField(objStart, objEnd, "\\"z\\"", z);
            ParseJsonFloatField(objStart, objEnd, "\\"rotY\\"", rotY);
            ParseJsonFloatField(objStart, objEnd, "\\"scale\\"", scale);
            ParseJsonFloatField(objStart, objEnd, "\\"speed\\"", spd);
            ParseJsonFloatField(objStart, objEnd, "\\"radius\\"", rad);

            LevelEntity e{};
            e.id = nextId++;
            e.type = static_cast<EntityType>(t);
            e.position = Vector3{ x, y, z };
            e.rotationY = rotY;
            e.scale = (scale > 0.01f) ? scale : 1.0f;
            e.animSpeed = (spd >= 0.0f) ? spd : 1.0f;
            e.radius = (rad > 0.01f) ? rad : 2.0f;
            AssignEntityModelAndProps(e);
            list.push_back(e);
        }
        cursor = objEnd + 1;
    }
    UnloadFileText(rawText);
    return !list.empty();
}'''

# Match from enum EntityType up to UnloadFileText(rawText);\n    return !list.empty();\n}
entity_block_pat = r'// Unified Level Entity Architecture[\s\S]*?UnloadFileText\(rawText\);\s*return !list\.empty\(\);\s*\}'
text = re.sub(entity_block_pat, entity_arch_new, text, count=1)

# 4. In main(): Update command line args for --test-cam
main_args_old = r'int main\(int argc, char\*\* argv\) \{[\s\S]*?for \(int i = 1; i < argc; i\+\+\) \{[\s\S]*?if \(strcmp\(argv\[i\], "--shot-frames"\) == 0 && i \+ 1 < argc\) targetTestFrame = atoi\(argv\[\+\+i\]\);\s*\}'
main_args_new = '''int main(int argc, char** argv) {

    bool testScreenshotMode = false;
    bool testEditorMode = false;
    bool useCustomTestCam = false;
    Vector3 customTestCamPos = { 0, 0, 0 };
    Vector3 customTestCamTarget = { 0, 0, 0 };

    std::string testShotName = "CustomEngine/scratch/active_gameplay.png";
    int testAngleMode = 0; // 0=behind, 1=side, 2=front, 3=top
    int targetTestFrame = 60;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--test-screenshot") == 0) testScreenshotMode = true;
        if (strcmp(argv[i], "--test-editor") == 0) testEditorMode = true;
        if (strcmp(argv[i], "--shot-name") == 0 && i + 1 < argc) testShotName = argv[++i];
        if (strcmp(argv[i], "--shot-angle") == 0 && i + 1 < argc) {
            std::string a = argv[++i];
            if (a == "side") testAngleMode = 1;
            else if (a == "front") testAngleMode = 2;
            else if (a == "top") testAngleMode = 3;
            else testAngleMode = 0;
        }
        if (strcmp(argv[i], "--shot-frames") == 0 && i + 1 < argc) targetTestFrame = atoi(argv[++i]);
        if (strcmp(argv[i], "--test-cam") == 0 && i + 6 < argc) {
            useCustomTestCam = true;
            testScreenshotMode = true;
            customTestCamPos.x = (float)atof(argv[++i]);
            customTestCamPos.y = (float)atof(argv[++i]);
            customTestCamPos.z = (float)atof(argv[++i]);
            customTestCamTarget.x = (float)atof(argv[++i]);
            customTestCamTarget.y = (float)atof(argv[++i]);
            customTestCamTarget.z = (float)atof(argv[++i]);
        }
    }'''

text = re.sub(main_args_old, main_args_new, text, count=1)

# 5. Add uSkinU and uSkinV uniform locations
shader_locs_old = r'int uEyeBulgeLoc  = GetShaderLocation\(shaderLighting, "uEyeBulge"\);[\s\S]*?SetShaderValue\(shaderLighting, uEyeBulgeLoc,  &zeroVal, SHADER_UNIFORM_FLOAT\);'
shader_locs_new = '''int uEyeBulgeLoc  = GetShaderLocation(shaderLighting, "uEyeBulge");
    int uSkinULoc     = GetShaderLocation(shaderLighting, "uSkinU");
    int uSkinVLoc     = GetShaderLocation(shaderLighting, "uSkinV");
    float zeroVal = 0.0f;
    SetShaderValue(shaderLighting, uButtSwellLoc, &zeroVal, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shaderLighting, uEyeBulgeLoc,  &zeroVal, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shaderLighting, uSkinULoc,     &zeroVal, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shaderLighting, uSkinVLoc,     &zeroVal, SHADER_UNIFORM_FLOAT);'''

text = re.sub(shader_locs_old, shader_locs_new, text, count=1)

# 6. Replace texture loading & model loading & materials
models_block_old = r'// Textures[\s\S]*?ApplyMat\(crocModel\);\s*ApplyMat\(sharkModel\);'

models_block_new = '''// Textures
    Texture2D texPalette = LoadTexture(ResolveAsset("extracted_assets/textures/Palette.png").c_str());
    SetTextureFilter(texPalette, TEXTURE_FILTER_BILINEAR);

    Image imgPalFlip = LoadImage(ResolveAsset("extracted_assets/textures/Palette.png").c_str());
    ImageFlipVertical(&imgPalFlip);
    Texture2D texPaletteFlipped = LoadTextureFromImage(imgPalFlip);
    UnloadImage(imgPalFlip);
    SetTextureFilter(texPaletteFlipped, TEXTURE_FILTER_BILINEAR);

    // Flipped bgr_default_tex for authentic green trees & warm brown rocks in scene_2..15 and wallModel
    Image imgBgrFlip = LoadImage(ResolveAsset("extracted_assets/textures/bgr_default_tex.png").c_str());
    ImageFlipVertical(&imgBgrFlip);
    Texture2D texBgrDefault = LoadTextureFromImage(imgBgrFlip);
    UnloadImage(imgBgrFlip);
    SetTextureFilter(texBgrDefault, TEXTURE_FILTER_BILINEAR);

    // Unflipped bgr_default_tex for GM_3D_JungleTree.obj (whose UVs were pre-inverted)
    Texture2D texBgrTree = LoadTexture(ResolveAsset("extracted_assets/textures/bgr_default_tex.png").c_str());
    SetTextureFilter(texBgrTree, TEXTURE_FILTER_BILINEAR);

    Texture2D texSkyFront = LoadTexture(ResolveAsset("extracted_assets/textures/Sky_Day_BlueSky_Nothing_Cam_0_Front+Z.png").c_str());
    SetTextureFilter(texSkyFront, TEXTURE_FILTER_BILINEAR);

    Texture2D texRockDark       = LoadTexture(ResolveAsset("extracted_assets/textures/tex_rock_dark.png").c_str());
    SetTextureFilter(texRockDark, TEXTURE_FILTER_BILINEAR);

    Texture2D texGradientPalette = LoadTexture(ResolveAsset("extracted_assets/textures/Gradient_Pallete.png").c_str());

    Texture2D texCaustic         = LoadTexture(ResolveAsset("extracted_assets/textures/Caustic 1.png").c_str());
    Texture2D texFoam1           = LoadTexture(ResolveAsset("extracted_assets/textures/Foam 1.png").c_str());
    Texture2D texFoam2           = LoadTexture(ResolveAsset("extracted_assets/textures/Foam2.png").c_str());
    SetTextureFilter(texCaustic, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(texFoam1,   TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(texFoam2,   TEXTURE_FILTER_BILINEAR);
    SetTextureWrap(texCaustic, TEXTURE_WRAP_REPEAT);
    SetTextureWrap(texFoam1,   TEXTURE_WRAP_REPEAT);
    SetTextureWrap(texFoam2,   TEXTURE_WRAP_REPEAT);

    int causticLoc = GetShaderLocation(shaderWater, "causticTex");
    int foam1Loc   = GetShaderLocation(shaderWater, "foam1Tex");
    int foam2Loc   = GetShaderLocation(shaderWater, "foam2Tex");
    shaderWater.locs[SHADER_LOC_MAP_DIFFUSE]  = causticLoc;
    shaderWater.locs[SHADER_LOC_MAP_NORMAL]   = foam1Loc;
    shaderWater.locs[SHADER_LOC_MAP_SPECULAR] = foam2Loc;

    Texture2D texFart[4];
    texFart[0] = LoadTexture(ResolveAsset("extracted_assets/textures/Fart1.png").c_str());
    texFart[1] = LoadTexture(ResolveAsset("extracted_assets/textures/Fart4.png").c_str());
    texFart[2] = LoadTexture(ResolveAsset("extracted_assets/textures/Fart7.png").c_str());
    texFart[3] = LoadTexture(ResolveAsset("extracted_assets/textures/Fart8.png").c_str());

    Texture2D texRaftFoam = LoadTexture(ResolveAsset("extracted_assets/textures/Raft-Foam4.png").c_str());
    Texture2D texHipo       = LoadTexture(ResolveAsset("extracted_assets/ui/Character_GameGreen.png").c_str());
    Texture2D texClock      = LoadTexture(ResolveAsset("extracted_assets/ui/Icon_Clock.png").c_str());
    Texture2D texCrashed    = LoadTexture(ResolveAsset("extracted_assets/ui/Icon_RaftCrashed.png").c_str());
    Texture2D texRaftPin    = LoadTexture(ResolveAsset("extracted_assets/ui/Icon_RaftS.png").c_str());
    Texture2D texFinish     = LoadTexture(ResolveAsset("extracted_assets/ui/Icon_Finish.png").c_str());
    Texture2D texEsc        = LoadTexture(ResolveAsset("extracted_assets/ui/keyboard_escape.png").c_str());
    SetTextureFilter(texHipo, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(texClock, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(texCrashed, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(texRaftPin, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(texFinish, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(texEsc, TEXTURE_FILTER_BILINEAR);

    Font fontUI     = LoadFontEx(ResolveAsset("extracted_assets/fonts/ui_font.ttf").c_str(), 18, 0, 0);
    Font fontUIBold = LoadFontEx(ResolveAsset("extracted_assets/fonts/ui_font_bold.ttf").c_str(), 18, 0, 0);
    if (fontUI.texture.id == 0) fontUI = GetFontDefault();
    else SetTextureFilter(fontUI.texture, TEXTURE_FILTER_BILINEAR);
    if (fontUIBold.texture.id == 0) fontUIBold = GetFontDefault();
    else SetTextureFilter(fontUIBold.texture, TEXTURE_FILTER_BILINEAR);

    // 3D Models
    Model wallModel    = LoadModel(ResolveAsset("extracted_assets/meshes/GM_3D_Demo_Wall.obj").c_str());
    Model waterModel   = LoadModel(ResolveAsset("extracted_assets/meshes/GM_3D_Demo_Water.obj").c_str());
    Model startModel   = LoadModel(ResolveAsset("extracted_assets/meshes/GM_3D_Start.obj").c_str());
    Model finishModel  = LoadModel(ResolveAsset("extracted_assets/meshes/GM_3D_Finish.obj").c_str());
    Model cp1Model     = LoadModel(ResolveAsset("extracted_assets/meshes/GM_3D_CheckPointV1.obj").c_str());
    Model cp2Model     = LoadModel(ResolveAsset("extracted_assets/meshes/GM_3D_CheckPointV2.obj").c_str());
    Model cp3Model     = LoadModel(ResolveAsset("extracted_assets/meshes/GM_3D_CheckPointV3.obj").c_str());
    Model boatModel    = LoadModel(ResolveAsset("extracted_assets/meshes/GM_3D_BoatMain_ThreeP.obj").c_str());
    Model charA        = LoadModel(ResolveAsset("extracted_assets/meshes/GM_3D_Character_SeatedA.obj").c_str());
    Model charB        = LoadModel(ResolveAsset("extracted_assets/meshes/GM_3D_Character_SeatedB.obj").c_str());

    // Authentic Obstacle Models (All 16 Types + Tree)
    gObsModels.mA1  = LoadModel(ResolveAsset("extracted_assets/meshes/GM_3D_ObstacleA1.obj").c_str());
    gObsModels.mB2  = LoadModel(ResolveAsset("extracted_assets/meshes/GM_3D_ObstacleB2.obj").c_str());
    gObsModels.mB3  = LoadModel(ResolveAsset("extracted_assets/meshes/GM_3D_ObstacleB3.obj").c_str());
    gObsModels.mB5  = LoadModel(ResolveAsset("extracted_assets/meshes/GM_3D_ObstacleB5.obj").c_str());
    gObsModels.mC1  = LoadModel(ResolveAsset("extracted_assets/meshes/GM_3D_ObstacleC1.obj").c_str());
    gObsModels.mC2  = LoadModel(ResolveAsset("extracted_assets/meshes/GM_3D_ObstacleC2.obj").c_str());
    gObsModels.mC4  = LoadModel(ResolveAsset("extracted_assets/meshes/GM_3D_ObstacleC4.obj").c_str());
    gObsModels.mC5  = LoadModel(ResolveAsset("extracted_assets/meshes/GM_3D_ObstacleC5.obj").c_str());
    gObsModels.mF3  = LoadModel(ResolveAsset("extracted_assets/meshes/GM_3D_ObstacleF3.obj").c_str());
    gObsModels.mF10 = LoadModel(ResolveAsset("extracted_assets/meshes/GM_3D_ObstacleF10.obj").c_str());
    gObsModels.mF11 = LoadModel(ResolveAsset("extracted_assets/meshes/GM_3D_ObstacleF11.obj").c_str());
    gObsModels.mF13 = LoadModel(ResolveAsset("extracted_assets/meshes/GM_3D_ObstacleF13.obj").c_str());
    gObsModels.mF14 = LoadModel(ResolveAsset("extracted_assets/meshes/GM_3D_ObstacleF14_P1.obj").c_str());
    gObsModels.mF18 = LoadModel(ResolveAsset("extracted_assets/meshes/GM_3D_ObstacleF18.obj").c_str());
    gObsModels.mF23 = LoadModel(ResolveAsset("extracted_assets/meshes/GM_3D_ObstacleF23.obj").c_str());
    gObsModels.mR1  = LoadModel(ResolveAsset("extracted_assets/meshes/GM_3D_Obstacle_Rock1.obj").c_str());
    gObsModels.mR2  = LoadModel(ResolveAsset("extracted_assets/meshes/GM_3D_Obstacle_Rock2.obj").c_str());
    gObsModels.mTree= LoadModel(ResolveAsset("extracted_assets/meshes/GM_3D_JungleTree.obj").c_str());

    Model crocModel = LoadModel(ResolveAsset("extracted_assets/meshes/GM_3D_Crocodile.obj").c_str());
    Model sharkModel= LoadModel(ResolveAsset("extracted_assets/meshes/Shark.obj").c_str());

    // Canyon Environment Clustered Sectors (scene_2..15, excluding scene 0 with flying shards)
    static const int ENV_SECTOR_COUNT = 14;
    Model envSectors[ENV_SECTOR_COUNT];
    for (int s = 0; s < ENV_SECTOR_COUNT; s++) {
        std::string secName = "extracted_assets/meshes/Combined_Mesh_root__scene_" + std::to_string(s + 2) + ".obj";
        envSectors[s] = LoadModel(ResolveAsset(secName).c_str());
    }

    auto ApplyMat = [&](Model& m) {
        for (int i = 0; i < m.materialCount; i++) {
            m.materials[i].maps[MATERIAL_MAP_DIFFUSE].texture = texPalette;
            m.materials[i].shader = shaderLighting;
        }
    };
    auto ApplyCharMat = [&](Model& m) {
        for (int i = 0; i < m.materialCount; i++) {
            m.materials[i].maps[MATERIAL_MAP_DIFFUSE].texture = texGradientPalette;
            m.materials[i].shader = shaderLighting;
        }
    };
    auto ApplyPaletteFlippedMat = [&](Model& m) {
        for (int i = 0; i < m.materialCount; i++) {
            m.materials[i].maps[MATERIAL_MAP_DIFFUSE].texture = texPaletteFlipped;
            m.materials[i].shader = shaderLighting;
        }
    };
    auto ApplyRockMat = [&](Model& m) {
        for (int i = 0; i < m.materialCount; i++) {
            m.materials[i].maps[MATERIAL_MAP_DIFFUSE].texture = texRockDark;
            m.materials[i].shader = shaderLighting;
        }
    };
    auto ApplyBgrMat = [&](Model& m) {
        for (int i = 0; i < m.materialCount; i++) {
            m.materials[i].maps[MATERIAL_MAP_DIFFUSE].texture = texBgrDefault;
            m.materials[i].shader = shaderLighting;
        }
    };
    auto ApplyBgrTreeMat = [&](Model& m) {
        for (int i = 0; i < m.materialCount; i++) {
            m.materials[i].maps[MATERIAL_MAP_DIFFUSE].texture = texBgrTree;
            m.materials[i].shader = shaderLighting;
        }
    };

    ApplyMat(wallModel);
    ApplyMat(startModel);
    ApplyMat(finishModel);
    ApplyMat(cp1Model);
    ApplyMat(cp2Model);
    ApplyMat(cp3Model);

    ApplyCharMat(boatModel);
    ApplyCharMat(charA);
    ApplyCharMat(charB);

    // Crocodile uses Gradient_Pallete -> Authentic Olive Green!
    ApplyCharMat(crocModel);

    // Shark and F11 use Palette flipped -> Ocean Blue & Metallic Blue/Purple!
    ApplyPaletteFlippedMat(sharkModel);
    ApplyPaletteFlippedMat(gObsModels.mF11);

    // Standard Obstacles
    ApplyMat(gObsModels.mA1);
    ApplyMat(gObsModels.mB2);
    ApplyMat(gObsModels.mB3);
    ApplyMat(gObsModels.mB5);
    ApplyMat(gObsModels.mC1);
    ApplyMat(gObsModels.mC2);
    ApplyMat(gObsModels.mC4);
    ApplyMat(gObsModels.mC5);
    ApplyMat(gObsModels.mF3);
    ApplyMat(gObsModels.mF10);
    ApplyMat(gObsModels.mF13);
    ApplyMat(gObsModels.mF14);
    ApplyMat(gObsModels.mF18);
    ApplyMat(gObsModels.mF23);

    ApplyRockMat(gObsModels.mR1);
    ApplyRockMat(gObsModels.mR2);

    // Trees use unflipped bgr texture
    ApplyBgrTreeMat(gObsModels.mTree);

    // Wall and Environment Sectors use vertically flipped bgr texture -> Authentic Green Foliage & Brown Cliffs!
    ApplyBgrMat(wallModel);
    for (int s = 0; s < ENV_SECTOR_COUNT; s++) {
        ApplyBgrMat(envSectors[s]);
    }'''

text = re.sub(models_block_old, models_block_new, text, count=1)

# 7. Checkpoints & levelEntities loading & Predators
checkpoints_block_old = r'std::vector<RouteCheckpoint> checkpoints = \{[\s\S]*?std::vector<SharkAI> sharks = \{[\s\S]*?\};\s*\}'
checkpoints_block_new = '''std::vector<RouteCheckpoint> checkpoints = {
        { startGatePos, 0.0f, &startModel, false },
        { Vector3{ 41.88f, 0.55f, 19.79f }, -35.0f, &cp1Model, false },
        { Vector3{ 30.38f, 0.55f, -10.64f }, 90.0f, &cp1Model, false },
        { Vector3{ -49.10f, 9.85f, -103.86f }, 90.0f, &cp2Model, false },
        { Vector3{ -122.48f, 8.95f, -91.45f }, 90.0f, &cp2Model, false },
        { Vector3{ -174.31f, 8.95f, -58.82f }, 0.0f, &cp2Model, false },
        { Vector3{ -198.09f, 8.95f, -103.12f }, 90.0f, &cp2Model, false },
        { Vector3{ -336.19f, 8.95f, -103.23f }, 90.0f, &cp2Model, false },
        { finishGatePos, 90.0f, &finishModel, false }
    };

    std::vector<LevelEntity> levelEntities;
    if (!LoadLevelLayoutJSON(LEVEL_LAYOUT_PATH, levelEntities)) {
        TraceLog(LOG_WARNING, "Failed to load level_layout.json");
    }

    // Authentic River Predators: Relocated to downstream rapids & lakes
    std::vector<CrocodileAI> crocodiles = {
        { Vector3{ 42.5f, 0.55f, 22.0f }, 3.14f, Vector3{ 42.5f, 0.55f, 22.0f }, 0.0f, false, CROC_SPEED },
        { Vector3{ -54.5f, 9.85f, -85.0f }, 1.57f, Vector3{ -54.5f, 9.85f, -85.0f }, 0.0f, false, CROC_SPEED },
        { Vector3{ -174.0f, 8.95f, -65.0f }, 1.57f, Vector3{ -174.0f, 8.95f, -65.0f }, 0.0f, false, CROC_SPEED }
    };

    std::vector<SharkAI> sharks = {
        { Vector3{ 18.0f, 0.55f, -10.6f }, 0.0f, Vector3{ 18.0f, 0.55f, -10.6f }, 0.0f, 0.0f, false, SHARK_PATROL_SPEED },
        { Vector3{ -230.0f, 9.82f, -103.5f }, 0.0f, Vector3{ -230.0f, 9.82f, -103.5f }, 0.0f, 0.0f, false, SHARK_PATROL_SPEED }
    };'''

text = re.sub(checkpoints_block_old, checkpoints_block_new, text, count=1)

# 8. Pirate initial angles
angles_old = r'float p1RimAngle = -2\.094f;\s*//\s*-120 deg[\s\S]*?float p2RimAngle = 2\.094f;\s*//\s*\+120 deg'
angles_new = '''// Left Pirate starts at rear-left (-152 deg), Right Pirate starts at rear-right (+152 deg)
    float p1RimAngle = -2.65f;
    float p2RimAngle =  2.65f;'''
text = re.sub(angles_old, angles_new, text, count=1)

# 9. P1 / P2 rim angles clamp and rimRadius
text = text.replace('p1RimAngle = Clamp(p1RimAngle, -2.85f, -0.65f);', 'p1RimAngle = Clamp(p1RimAngle, -3.10f, -0.15f);')
text = text.replace('p2RimAngle = Clamp(p2RimAngle, 0.65f, 2.85f);', 'p2RimAngle = Clamp(p2RimAngle, 0.15f, 3.10f);')
text = text.replace('float rimRadius = 1.25f;', 'float rimRadius = 0.94f;')

# 10. Replace Hydrodynamics and Raft physics
hydro_old = r'// Authentic Hydrodynamics:[\s\S]*?raft\.speed = horizSpeed;'
hydro_new = '''// 4. Authentic Water Mesh Surface & Spline Hydrodynamics
        float currentSplineT = FindClosestSplineT(raft.position);
        Vector3 riverCenter  = GetSplinePoint(currentSplineT);
        Vector3 riverTangent = GetSplineTangent(currentSplineT);
        Vector3 riverNormal  = Vector3{ riverTangent.z, 0.0f, -riverTangent.x };

        float actualWaterY = 0.55f;
        float distToWaterMesh = 0.0f;
        Vector3 nearestWaterPt = raft.position;
        QueryActualWaterSurface(raft.position, actualWaterY, distToWaterMesh, nearestWaterPt);

        bool inStartPond = (raft.position.z >= 73.0f && raft.position.x > 2.0f && raft.position.x < 28.0f);
        bool overWater   = inStartPond || (distToWaterMesh < 5.8f);

        // Downstream slope acceleration on water ramps & waterfalls
        float maxAllowedSpeed = BOAT_MAX_SPEED;
        if (!inStartPond && overWater && !raft.isDestroyed) {
            float downSlope = Clamp(-riverTangent.y, -0.35f, 0.65f);
            float streamPush = 0.65f + fmaxf(0.0f, downSlope * 18.0f);
            raft.velocity.x += riverTangent.x * streamPush * dt;
            raft.velocity.z += riverTangent.z * streamPush * dt;
            if (downSlope > 0.08f) maxAllowedSpeed = 22.0f; // Fast waterfall slide!
        }

        // Authentic Start Basin & Canal Wall Elastic Containment
        if (inStartPond) {
            Vector3 pondCenter = { 15.43f, 0.55f, 87.67f };
            Vector3 dPond = { raft.position.x - pondCenter.x, 0.0f, raft.position.z - pondCenter.z };
            float rPond = Vector3Length(dPond);
            bool nearExitGate = (fabsf(raft.position.x - 15.51f) < 3.6f && raft.position.z < 79.5f);
            if (rPond > 10.5f && !nearExitGate) {
                Vector3 nPond = Vector3Normalize(dPond);
                raft.position.x = pondCenter.x + nPond.x * 10.5f;
                raft.position.z = pondCenter.z + nPond.z * 10.5f;
                float vn = raft.velocity.x * nPond.x + raft.velocity.z * nPond.z;
                if (vn > 0.0f) {
                    raft.velocity.x -= 1.65f * vn * nPond.x;
                    raft.velocity.z -= 1.65f * vn * nPond.z;
                }
            }
        } else if (distToWaterMesh > 2.6f && distToWaterMesh < 5.8f && raft.position.z > -45.0f) {
            // Elastic bounce off raised flume/canyon walls
            Vector3 pushIn = Vector3Normalize(Vector3{ nearestWaterPt.x - raft.position.x, 0.0f, nearestWaterPt.z - raft.position.z });
            raft.position.x += pushIn.x * (distToWaterMesh - 2.6f) * 0.85f;
            raft.position.z += pushIn.z * (distToWaterMesh - 2.6f) * 0.85f;
            float vn = raft.velocity.x * pushIn.x + raft.velocity.z * pushIn.z;
            if (vn < 0.0f) {
                raft.velocity.x -= 1.55f * vn * pushIn.x;
                raft.velocity.z -= 1.55f * vn * pushIn.z;
                camCtrl.shakeTrauma = fmaxf(camCtrl.shakeTrauma, 0.20f);
            }
        }

        // Buoyancy on Actual Water Mesh vs Free-Fall in Abyss
        if (overWater && !raft.isDestroyed) {
            float waveY = actualWaterY + 0.04f + sinf(raft.position.z * 0.32f - runTime * 1.8f) * 0.04f;
            float heave = waveY - raft.position.y;
            raft.velocity.y += (heave * 45.0f - raft.velocity.y * 8.0f) * dt;
        } else if (!overWater) {
            raft.velocity.y -= 32.0f * dt;
            if (raft.position.y < (actualWaterY - 12.0f) && !raft.isDestroyed) {
                TriggerBoatDeath(DEATH_VOID);
            }
        }

        // Linear Damping & Position Integration
        raft.velocity.x *= (1.0f - BOAT_LINEAR_DAMPING * dt);
        raft.velocity.z *= (1.0f - BOAT_LINEAR_DAMPING * dt);
        raft.position = Vector3Add(raft.position, Vector3Scale(raft.velocity, dt));

        // Clamp Horizontal Speed
        float horizSpeed = sqrtf(raft.velocity.x * raft.velocity.x + raft.velocity.z * raft.velocity.z);
        if (horizSpeed > maxAllowedSpeed) {
            float ratio = maxAllowedSpeed / horizSpeed;
            raft.velocity.x *= ratio;
            raft.velocity.z *= ratio;
        }

        // Boat Tilt / Pitch / Roll
        float fwdAccel = Vector3DotProduct(raft.velocity, riverTangent);
        Vector3 toNearest = Vector3Subtract(raft.position, nearestWaterPt);
        float crossWaterDist = Vector3DotProduct(toNearest, riverNormal);
        raft.pitch = Lerp(raft.pitch, -fwdAccel * 0.035f, 6.0f * dt);
        raft.roll  = Lerp(raft.roll, (crossWaterDist * 0.045f) + (raft.angularVelocityY * 0.12f), 6.0f * dt);
        raft.speed = horizSpeed;'''

text = re.sub(hydro_old, hydro_new, text, count=1)

# 11. Kinetic obstacles & collision handling
hazards_block_old = r'// 5\. Update Kinetic Obstacles & Spring Bumpers[\s\S]*?// 6\. Update Crocodile Predator AI'
hazards_block_new = '''// 5. Update Kinetic Obstacles & Hazards
        for (auto& ent : levelEntities) {
            ent.animTimer += dt * ent.animSpeed;
            if (ent.type == ENT_OBS_B2) ent.rotationY = sinf(ent.animTimer) * 45.0f;
            else if (ent.type == ENT_OBS_B3) ent.rotationY += dt * 90.0f * ent.animSpeed;
            else if (ent.type == ENT_OBS_B5) ent.rotationY += dt * 90.0f * ent.animSpeed;
            else if (ent.type == ENT_OBS_C1) ent.rotationY = sinf(ent.animTimer * 1.3f) * 60.0f;
            else if (ent.type == ENT_OBS_C2) ent.rotationY = cosf(ent.animTimer) * 55.0f;
            else if (ent.type == ENT_OBS_C4) ent.rotationY += dt * 110.0f * ent.animSpeed;
            else if (ent.type == ENT_OBS_C5) ent.rotationY += dt * 80.0f * ent.animSpeed;
            else if (ent.type == ENT_OBS_F3) ent.rotationY = sinf(ent.animTimer * 1.5f) * 75.0f;
            else if (ent.type == ENT_OBS_F10) ent.rotationY = sinf(ent.animTimer * 1.2f) * 75.0f;
            else if (ent.type == ENT_OBS_F11) ent.rotationY += dt * 120.0f * ent.animSpeed;
            else if (ent.type == ENT_OBS_F13) ent.rotationY += dt * 75.0f * ent.animSpeed;
            else if (ent.type == ENT_OBS_F23) ent.rotationY = sinf(ent.animTimer * 1.4f) * 65.0f;

            if (ent.type == ENT_OBS_A1) {
                if (ent.bumperCooldownTimer > 0.0f) ent.bumperCooldownTimer -= dt;
                if (ent.isExpanded) {
                    ent.bumperExpandTimer -= dt;
                    if (ent.bumperExpandTimer <= 0.0f) {
                        ent.isExpanded = false;
                        ent.bumperCooldownTimer = BUMPER_COOLDOWN;
                    }
                }
                float d = Vector3Distance(raft.position, ent.position);
                if (d < ent.radius && ent.bumperCooldownTimer <= 0.0f && !raft.isDestroyed) {
                    ent.isExpanded = true;
                    ent.bumperExpandTimer = BUMPER_EXPAND_DURATION;
                    float rad = ent.rotationY * DEG2RAD;
                    Vector3 launchDir = { sinf(rad), 0.5f, cosf(rad) };
                    launchDir = Vector3Normalize(launchDir);
                    raft.velocity = Vector3Scale(launchDir, 18.0f);
                    raft.angularVelocityY += 15.0f;
                    PlaySound(fartSounds[1]);
                    camCtrl.shakeTrauma = fmaxf(camCtrl.shakeTrauma, 0.65f);
                }
            } else if (ent.type == ENT_GUST) {
                float d = Vector3Distance(raft.position, ent.position);
                if (d < ent.radius && !raft.isDestroyed) {
                    float rad = ent.rotationY * DEG2RAD;
                    Vector3 gustDir = { sinf(rad), 0.0f, cosf(rad) };
                    raft.velocity.x += gustDir.x * 24.0f * dt;
                    raft.velocity.z += gustDir.z * 24.0f * dt;
                    camCtrl.shakeTrauma = fmaxf(camCtrl.shakeTrauma, 0.20f);
                }
            } else if (ent.type == ENT_ROCK_1 || ent.type == ENT_ROCK_2) {
                float d = Vector3Distance(raft.position, ent.position);
                if (d < ent.radius && !raft.isDestroyed) {
                    Vector3 pushOut = Vector3Normalize(Vector3Subtract(raft.position, ent.position));
                    raft.position = Vector3Add(ent.position, Vector3Scale(pushOut, ent.radius + 0.1f));
                    raft.velocity = Vector3Scale(pushOut, 5.0f);
                    TriggerBoatDeath(DEATH_OBSTACLE);
                }
            } else {
                // Moving kinetic hazard collision
                float d = Vector3Distance(raft.position, ent.position);
                if (d < ent.radius && !raft.isDestroyed) {
                    TriggerBoatDeath(DEATH_OBSTACLE);
                }
            }
        }

        // 6. Update Crocodile Predator AI'''

text = re.sub(hazards_block_old, hazards_block_new, text, count=1)

# 12. Predator AI elevation query
croc_elev_old = r'float crocSplineT = FindClosestSplineT\(croc\.position\);\s*float crocWaterY\s*=\s*GetSplinePoint\(crocSplineT\)\.y\s*-\s*0\.26f;\s*croc\.position\.y\s*=\s*crocWaterY;'
croc_elev_new = '''float crocWaterY = 0.55f, crocDist = 0.0f;
            Vector3 crocPt = croc.position;
            QueryActualWaterSurface(croc.position, crocWaterY, crocDist, crocPt);
            croc.position.y = crocWaterY - 0.20f;'''
text = re.sub(croc_elev_old, croc_elev_new, text, count=1)

shark_elev_old = r'float sharkSplineT = FindClosestSplineT\(shark\.position\);\s*float sharkWaterY\s*=\s*GetSplinePoint\(sharkSplineT\)\.y\s*-\s*0\.29f;\s*shark\.position\.y\s*=\s*sharkWaterY;'
shark_elev_new = '''float sharkWaterY = 0.55f, sharkDist = 0.0f;
            Vector3 sharkPt = shark.position;
            QueryActualWaterSurface(shark.position, sharkWaterY, sharkDist, sharkPt);
            shark.position.y = sharkWaterY - 0.22f;'''
text = re.sub(shark_elev_old, shark_elev_new, text, count=1)

# 13. Camera override for --test-cam
cam_override_old = r'// Pass uniforms to Water Shader'
cam_override_new = '''if (useCustomTestCam) {
            camera.position = customTestCamPos;
            camera.target   = customTestCamTarget;
            camera.up       = Vector3{ 0.0f, 1.0f, 0.0f };
            camera.fovy     = 55.0f;
        }

        // Pass uniforms to Water Shader'''
text = text.replace(cam_override_old, cam_override_new)

# 14. Panoramic sky rendering and removal of 3 DrawCylinder lake calls
sky_draw_old = r'// 1\. Cartoon Sky Gradient[\s\S]*?BeginMode3D\(camera\);'
sky_draw_new = '''// 1. Authentic Cartoon Sky Backdrop
        ClearBackground(Color{ 194, 217, 242, 255 });
        if (texSkyFront.id != 0) {
            float skyW = (float)GetScreenWidth();
            float skyH = (float)GetScreenHeight();
            float yawNorm = fmodf(-camCtrl.yaw / (2.0f * 3.14159f), 1.0f);
            if (yawNorm < 0.0f) yawNorm += 1.0f;
            float offset = yawNorm * skyW;
            DrawTexturePro(texSkyFront, Rectangle{ 0, 0, (float)texSkyFront.width, (float)texSkyFront.height },
                           Rectangle{ -offset, 0, skyW, skyH }, Vector2{0, 0}, 0.0f, WHITE);
            DrawTexturePro(texSkyFront, Rectangle{ 0, 0, (float)texSkyFront.width, (float)texSkyFront.height },
                           Rectangle{ skyW - offset, 0, skyW, skyH }, Vector2{0, 0}, 0.0f, WHITE);
        } else {
            DrawRectangleGradientV(0, 0, GetScreenWidth(), GetScreenHeight(), Color{ 87, 133, 222, 255 }, Color{ 194, 217, 242, 255 });
        }

        BeginMode3D(camera);'''

text = re.sub(sky_draw_old, sky_draw_new, text, count=1)

# Remove the three DrawCylinder calls for start lake enclosure
cylinders_pat = r'// A\. Circular Start Lake Enclosure[\s\S]*?DrawCylinder\(Vector3\{\s*14\.45f,\s*0\.18f,\s*72\.0f\s*\}[\s\S]*?\);'
text = re.sub(cylinders_pat, '// A. Authentic Start Basin in Canyon Walls & Water Mesh (Zero fake cylinders)', text, count=1)

# 15. Pirate rendering on raft (Task 3)
pirates_draw_old = r'// Left Pirate \(Player 1: Green Hippo[\s\S]*?// H\. Ragdoll Pirates Ejected'
pirates_draw_new = '''// Left Pirate (Player 1: Green Hippo - charB is green in Gradient_Pallete!)
                    float p1Redness = gasP1.redness;
                    float p1ButtSwell = (gasP1.boneScale - 1.0f) * 2.5f;
                    float p1EyeBulge  = gasP1.isSustained ? gasP1.sustainedCharge : 0.0f;
                    float p1SkinU = 0.320f, p1SkinV = 0.735f;
                    SetShaderValue(shaderLighting, uRednessLoc,   &p1Redness,   SHADER_UNIFORM_FLOAT);
                    SetShaderValue(shaderLighting, uButtSwellLoc, &p1ButtSwell, SHADER_UNIFORM_FLOAT);
                    SetShaderValue(shaderLighting, uEyeBulgeLoc,  &p1EyeBulge,  SHADER_UNIFORM_FLOAT);
                    SetShaderValue(shaderLighting, uSkinULoc,     &p1SkinU,     SHADER_UNIFORM_FLOAT);
                    SetShaderValue(shaderLighting, uSkinVLoc,     &p1SkinV,     SHADER_UNIFORM_FLOAT);

                    rlPushMatrix();
                        float p1X = sinf(p1RimAngle) * 0.94f;
                        float p1Z = cosf(p1RimAngle) * 0.94f;
                        rlTranslatef(p1X, 0.34f + gasP1.trembleOffset, p1Z);
                        float p1FaceYaw = (p1RimAngle * RAD2DEG) + 180.0f;
                        rlRotatef(p1FaceYaw, 0, 1, 0);
                        DrawModel(charB, Vector3{ 0, 0, 0 }, 1.0f, WHITE);
                    rlPopMatrix();

                    // Right Pirate (Player 2: Brown/Grey Hippo - charA)
                    float p2Redness = gasP2.redness;
                    float p2ButtSwell = (gasP2.boneScale - 1.0f) * 2.5f;
                    float p2EyeBulge  = gasP2.isSustained ? gasP2.sustainedCharge : 0.0f;
                    float p2SkinU = 0.068f, p2SkinV = 0.585f;
                    SetShaderValue(shaderLighting, uRednessLoc,   &p2Redness,   SHADER_UNIFORM_FLOAT);
                    SetShaderValue(shaderLighting, uButtSwellLoc, &p2ButtSwell, SHADER_UNIFORM_FLOAT);
                    SetShaderValue(shaderLighting, uEyeBulgeLoc,  &p2EyeBulge,  SHADER_UNIFORM_FLOAT);
                    SetShaderValue(shaderLighting, uSkinULoc,     &p2SkinU,     SHADER_UNIFORM_FLOAT);
                    SetShaderValue(shaderLighting, uSkinVLoc,     &p2SkinV,     SHADER_UNIFORM_FLOAT);

                    rlPushMatrix();
                        float p2X = sinf(p2RimAngle) * 0.94f;
                        float p2Z = cosf(p2RimAngle) * 0.94f;
                        rlTranslatef(p2X, 0.34f + gasP2.trembleOffset, p2Z);
                        float p2FaceYaw = (p2RimAngle * RAD2DEG) + 180.0f;
                        rlRotatef(p2FaceYaw, 0, 1, 0);
                        DrawModel(charA, Vector3{ 0, 0, 0 }, 1.0f, WHITE);
                    rlPopMatrix();

                    // Reset uniforms for subsequent models
                    float zeroReset = 0.0f;
                    SetShaderValue(shaderLighting, uRednessLoc,   &zeroReset, SHADER_UNIFORM_FLOAT);
                    SetShaderValue(shaderLighting, uButtSwellLoc, &zeroReset, SHADER_UNIFORM_FLOAT);
                    SetShaderValue(shaderLighting, uEyeBulgeLoc,  &zeroReset, SHADER_UNIFORM_FLOAT);
                    SetShaderValue(shaderLighting, uSkinULoc,     &zeroReset, SHADER_UNIFORM_FLOAT);
                    SetShaderValue(shaderLighting, uSkinVLoc,     &zeroReset, SHADER_UNIFORM_FLOAT);

                rlPopMatrix();
            }

            // H. Ragdoll Pirates Ejected'''

text = re.sub(pirates_draw_old, pirates_draw_new, text, count=1)

# 16. Editor UI type switcher and new entity
editor_switcher_old = r'if \(DrawGodotBtn\(btnPrevType[\s\S]*?editorMsgTimer = 2\.0f;\s*\}'
editor_switcher_new = '''if (DrawGodotBtn(btnPrevType, "< Prev Type", Color{ 36, 43, 58, 255 }, WHITE, true)) {
                    o.type = (EntityType)(((int)o.type - 1 + ENT_COUNT) % ENT_COUNT);
                    AssignEntityModelAndProps(o);
                    editorMsg = TextFormat("Type: %s", ENTITY_TYPE_NAMES[o.type]);
                    editorMsgTimer = 2.0f;
                }
                if (DrawGodotBtn(btnNextType, "Next Type >", Color{ 36, 43, 58, 255 }, WHITE, true)) {
                    o.type = (EntityType)(((int)o.type + 1) % ENT_COUNT);
                    AssignEntityModelAndProps(o);
                    editorMsg = TextFormat("Type: %s", ENTITY_TYPE_NAMES[o.type]);
                    editorMsgTimer = 2.0f;
                }'''
text = re.sub(editor_switcher_old, editor_switcher_new, text, count=1)

editor_add_old = r'newE\.type = ENT_BUMPER;[\s\S]*?AssignEntityModelAndProps\(newE[\s\S]*?\);'
editor_add_new = '''newE.type = ENT_OBS_A1;
                Vector3 fwd = { -sinf(editorCamYaw) * cosf(editorCamPitch), sinf(editorCamPitch), -cosf(editorCamYaw) * cosf(editorCamPitch) };
                newE.position = Vector3Add(editorCamPos, Vector3Scale(fwd, 6.5f));
                newE.position.y = 0.50f;
                newE.scale = 1.0f;
                AssignEntityModelAndProps(newE);'''
text = re.sub(editor_add_old, editor_add_new, text, count=1)

# 17. Unload newly created models and textures at exit
unload_old = r'UnloadTexture\(texCaustic\);[\s\S]*?UnloadModel\(crocModel\);'
unload_new = '''UnloadTexture(texCaustic);
    UnloadTexture(texFoam1);
    UnloadTexture(texFoam2);
    UnloadTexture(texPalette);
    UnloadTexture(texPaletteFlipped);
    UnloadTexture(texBgrDefault);
    UnloadTexture(texBgrTree);
    UnloadTexture(texSkyFront);
    UnloadTexture(texRockDark);
    UnloadTexture(texGradientPalette);
    UnloadTexture(texRaftFoam);
    for (int p = 0; p < 4; p++) UnloadTexture(texFart[p]);
    UnloadTexture(texHipo);
    UnloadTexture(texClock);
    UnloadTexture(texCrashed);
    UnloadTexture(texRaftPin);
    UnloadTexture(texFinish);
    UnloadTexture(texEsc);

    for (int s = 0; s < ENV_SECTOR_COUNT; s++) {
        UnloadModel(envSectors[s]);
    }
    UnloadModel(wallModel);
    UnloadModel(waterModel);
    UnloadModel(startModel);
    UnloadModel(finishModel);
    UnloadModel(cp1Model);
    UnloadModel(cp2Model);
    UnloadModel(cp3Model);
    UnloadModel(boatModel);
    UnloadModel(charA);
    UnloadModel(charB);
    UnloadModel(crocModel);
    UnloadModel(sharkModel);

    UnloadModel(gObsModels.mA1);
    UnloadModel(gObsModels.mB2);
    UnloadModel(gObsModels.mB3);
    UnloadModel(gObsModels.mB5);
    UnloadModel(gObsModels.mC1);
    UnloadModel(gObsModels.mC2);
    UnloadModel(gObsModels.mC4);
    UnloadModel(gObsModels.mC5);
    UnloadModel(gObsModels.mF3);
    UnloadModel(gObsModels.mF10);
    UnloadModel(gObsModels.mF11);
    UnloadModel(gObsModels.mF13);
    UnloadModel(gObsModels.mF14);
    UnloadModel(gObsModels.mF18);
    UnloadModel(gObsModels.mF23);
    UnloadModel(gObsModels.mR1);
    UnloadModel(gObsModels.mR2);
    UnloadModel(gObsModels.mTree);'''

text = re.sub(unload_old, unload_new, text, count=1)

path.write_text(text, encoding="utf-8")
print("Successfully patched CustomEngine/native/GameEngine.cpp")

