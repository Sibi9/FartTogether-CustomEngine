// =============================================================================

// Fart Together - Authentic Native C++17 Game Engine

// Complete 1:1 Parity: Authentic 3D Canyon Mesh, Water Shader with Caustics,

// Dual Foam Maps, Kinetic Obstacles, Billboard Comic Fart VFX, Mouse Camera,

// AND 1:1 Authentic Game Mechanics Overhaul (Assembly-CSharp.dll Parity):

// - Pirate Gas Tank (100 pts, 12/s drain, 1s delay, 18/s regen, 35% recover)

// - Dual Mode Fart: Tap Burst (<0.1s: 900N burst, 10 cost) vs Hold Ramp (100N->900N in 3s)

// - Pirate Butt Physics: 1.4x Bone Swell, 12Hz Tremble, Dynamic Scarlet Redness

// - Differential Steering Torque (6 N*m per pirate) & "Fart Together" Sync Boost

// - Spring Bumpers (30 deg launch, 18 m/s horiz, 8 m/s vert, screen trauma)

// - Crocodile Predator AI (14m detect, 5 m/s lunge, 3m bite, 1.5s cooldown)

// - Shark Predator AI (25m patrol, 6 m/s chase, 2.2m bite)

// - Death Causes, Boat Destruction, Ragdoll Ejection & 2s Invulnerable Respawn

// =============================================================================

#include "raylib.h"
#include "LanNetwork.h"

#include "raymath.h"

#include "rlgl.h"

#include <vector>

#include <string>

#include <cmath>

#include <cstdlib>

#include <algorithm>

#include <iostream>

// =============================================================================

// 1. Constants & Configurations (Authentic 1:1 from Assembly-CSharp.dll)

// =============================================================================

static const int WINDOW_WIDTH = 1280;

static const int WINDOW_HEIGHT = 720;

static const float GRAVITY = 9.81f;

static const Color COLOR_TITLEBAR_BG = Color{ 15, 23, 42, 245 };

static const Color COLOR_HUD_BG      = Color{ 15, 23, 42, 210 };

static const Color COLOR_HUD_BORDER  = Color{ 51, 65, 85, 255 };

static const Color COLOR_ACCENT_GREEN= Color{ 34, 197, 94, 255 };

static const Color COLOR_ACCENT_BLUE = Color{ 56, 189, 248, 255 };

static const Color COLOR_ACCENT_GOLD = Color{ 234, 179, 8, 255 };

static const Color COLOR_ACCENT_RED  = Color{ 239, 68, 68, 255 };

// BoatPhysicsConfig & BoatDefinition

static const float BOAT_MASS = 40.0f;

static const float BOAT_LINEAR_DAMPING = 0.40f;

static const float BOAT_ANGULAR_DAMPING = 2.0f;

static const float BOAT_MAX_SPEED = 14.0f;

static const float BOAT_STEER_TORQUE = 6.0f;

static const float BOAT_ATTACHMENT_RADIUS = 1.40f;

// Gas Physics

static const float GAS_NORMAL_FORCE = 900.0f;

static const float GAS_MIN_FORCE = 100.0f;

static const float GAS_RAMP_SECONDS = 3.0f;

static const float GAS_RELEASE_SECONDS = 0.50f;

static const float GAS_TAP_THRESHOLD_SECONDS = 0.10f;

static const float GAS_TAP_BURST_FORCE = 900.0f;

static const float GAS_TAP_COST = 10.0f;

static const float GAS_MAX = 100.0f;

static const float GAS_DRAIN_PER_SEC = 12.0f;

static const float GAS_REGEN_PER_SEC = 18.0f;

static const float GAS_REGEN_DELAY = 1.0f;

static const float GAS_RECOVER_PERCENT = 0.35f;

// Pirate Butt Jiggle & Swelling

static const float BUTT_PEAK_SCALE = 1.30f;

static const float BUTT_TREMBLE_FREQ = 12.0f;

static const float BUTT_TREMBLE_AMP = 0.05f;

static const float BUTT_MIN_TREMBLE_THRESHOLD = 0.55f;

// Bumper Obstacle Physics

static const float BUMPER_DETECTION_RADIUS = 8.0f;

static const float BUMPER_IMPACT_RADIUS = 1.85f; // Authentic physical touch (boat 1.25m + pad 0.6m)

static const float BUMPER_HORIZ_FORCE = 18.0f;

static const float BUMPER_UPWARD_FORCE = 8.0f;

static const float BUMPER_UPWARD_ANGLE_DEG = 30.0f;

static const float BUMPER_EXPAND_DURATION = 0.18f;

static const float BUMPER_COOLDOWN = 1.0f;

// Crocodile AI

static const float CROC_SPEED = 5.0f;

static const float CROC_DETECT_RANGE = 14.0f;

static const float CROC_BITE_RANGE = 3.0f;

static const float CROC_BITE_COOLDOWN = 1.5f;

// Shark AI

static const float SHARK_PATROL_SPEED = 3.5f;

static const float SHARK_CHASE_SPEED = 6.0f;

static const float SHARK_VIEW_LENGTH = 14.0f;

static const float SHARK_VIEW_HALF_WIDTH = 6.0f;

static const float SHARK_BITE_RANGE = 2.2f;

static const float SHARK_BITE_COOLDOWN = 3.0f;

static const float SHARK_WANDER_RADIUS = 25.0f;

// Death Causes

enum DeathCause {
    DEATH_NONE = 0,
    DEATH_WALL = 1,
    DEATH_OBSTACLE = 2,
    DEATH_VOID = 3,
    DEATH_SHARK = 4,
    DEATH_CROCODILE = 5,
    DEATH_FIRE = 6,
    DEATH_SUICIDE = 7
};

// =============================================================================

// 2. Shaders: Lighting with Dynamic Skin Redness & Multi-Texture Water Shader

// =============================================================================

static const char* VS_LIGHTING = R"(
#version 330
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
in vec4 vertexColor;
uniform mat4 mvp;
uniform float uButtSwell; // 0.0 to 0.30 (1.0x to 1.3x butt inflation)
uniform float uEyeBulge;  // 0.0 to 1.0  (Funny cartoon eyeball pop)
out vec2 fragTexCoord;
out vec4 fragColor;
out vec3 fragNormal;
out vec3 fragPosition;

void main() {
    fragTexCoord = vertexTexCoord;
    fragColor    = vertexColor;
    fragNormal   = vertexNormal;
    fragPosition = vertexPosition;

    vec3 pos = vertexPosition;

    // A. Butt Swell (*1.3 max scale when uButtSwell = 0.30): Local butt cheeks Z < -0.01, Y in [0.18, 0.52]
    if (pos.z < -0.01 && pos.y >= 0.18 && pos.y <= 0.52 && uButtSwell > 0.001) {
        float wy = sin(clamp((pos.y - 0.18) / 0.34, 0.0, 1.0) * 3.14159265);
        float wz = clamp((-pos.z - 0.01) / 0.12, 0.0, 1.0);
        float wButt = wy * wz;
        vec3 buttPivot = vec3(0.0, 0.34, -0.02);
        pos = mix(pos, buttPivot + (pos - buttPivot) * (1.0 + uButtSwell), wButt);
        pos.z -= uButtSwell * 0.14 * wButt;
    }

    // B. Exact Eyeball-Only Bulging (Matches ONLY the 322 sclera + pupil vertices, ZERO snout/jaw/teeth vertices!)
    bool isPupil  = (vertexTexCoord.x < 0.08) && (vertexTexCoord.y < 0.12 || vertexTexCoord.y > 0.88);
    bool isSclera = (vertexTexCoord.x > 0.68 && vertexTexCoord.x < 0.75) && (vertexTexCoord.y < 0.05 || vertexTexCoord.y > 0.95);
    if (pos.y > 0.88 && (isPupil || isSclera) && uEyeBulge > 0.001) {
        float eyeSign = (pos.x >= 0.0) ? 1.0 : -1.0;
        vec2 eyeCenterXY = vec2(eyeSign * 0.0456, 0.935);
        pos.xy = eyeCenterXY + (pos.xy - eyeCenterXY) * (1.0 + uEyeBulge * 0.45);
        pos.x += eyeSign * uEyeBulge * 0.012;
        pos.z += uEyeBulge * 0.055;
    }

    gl_Position = mvp * vec4(pos, 1.0);
})";

static const char* FS_LIGHTING = R"(
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
uniform float uRedness;     // 0.0 below 60% charge, ramps 0.0 -> 1.0 from 60% to 100% charge
uniform int uIsJungleTree;  // 1 = procedural two-tone brown trunk + lush green canopy
out vec4 finalColor;

void main() {
    vec4 texColor = texture(texture0, fragTexCoord);
    vec3 baseRgb = texColor.rgb;

    if (uIsJungleTree == 1) {
        if (fragPosition.y < 3.05) {
            baseRgb = mix(vec3(0.46, 0.28, 0.14), vec3(0.60, 0.39, 0.21), clamp(fragPosition.y / 3.05, 0.0, 1.0));
        } else {
            float hFactor = clamp((fragPosition.y - 3.05) / 4.2, 0.0, 1.0);
            baseRgb = mix(vec3(0.13, 0.50, 0.20), vec3(0.29, 0.76, 0.33), hFactor);
        }
    }

    vec3 N = normalize(fragNormal);
    vec3 L = normalize(lightDir);
    float diff = max(dot(N, L), 0.0);
    float celDiff = smoothstep(0.18, 0.58, diff) * 0.65 + diff * 0.35;
    vec3 lighting = mix(ambientColor, lightColor, celDiff);

    // Localized Butt Cheek Redness Mask
    vec3 buttCenter = vec3(0.0, 0.34, -0.12);
    vec3 buttScale  = vec3(0.26, 0.20, 0.16);
    float buttDist  = length((fragPosition - buttCenter) / buttScale);
    float buttMask  = (fragPosition.z < -0.01) ? clamp(1.0 - smoothstep(0.35, 1.05, buttDist), 0.0, 1.0) : 0.0;

    vec3 redSkin = mix(baseRgb, vec3(0.96, 0.12, 0.12), uRedness * buttMask * 0.92);
    vec3 litColor = redSkin * lighting * colDiffuse.rgb;
    finalColor = vec4(litColor, texColor.a * colDiffuse.a);
})";

static const char* VS_WATER = R"(

#version 330

in vec3 vertexPosition;

in vec2 vertexTexCoord;

in vec3 vertexNormal;

uniform mat4 mvp;

uniform vec3 trackOffset;

uniform float uTime;

out vec2 fragTexCoord;

out vec3 fragNormal;

out vec3 fragPos;

void main() {

    fragTexCoord = vertexTexCoord;

    vec3 pos = vertexPosition;

    // Gentle physical wave undulation matching Unity WaterSurface (_WaveHeight = 0.05, _WaveSpeed = 1.15)

    float w1 = sin(pos.z * 0.32 - uTime * 1.8 + pos.x * 0.12) * 0.05;

    float w2 = cos(pos.x * 0.40 + pos.z * 0.22 - uTime * 2.2) * 0.04;

    pos.y += w1 + w2;

    fragPos = pos + trackOffset;

    float dHdx = cos(pos.x * 0.40 + pos.z * 0.22 - uTime * 2.2) * 0.40 * 0.04;

    float dHdz = cos(pos.z * 0.32 - uTime * 1.8 + pos.x * 0.12) * 0.32 * 0.05;

    fragNormal = normalize(vec3(-dHdx, 1.0, -dHdz));

    gl_Position = mvp * vec4(pos, 1.0);

}

)";

static const char* FS_WATER = R"(

#version 330

in vec2 fragTexCoord;

in vec3 fragNormal;

in vec3 fragPos;

uniform sampler2D causticTex;

uniform sampler2D foam1Tex;

uniform sampler2D foam2Tex;

uniform float uTime;

uniform vec3 lightDir;

uniform vec3 camPos;

out vec4 finalColor;

void main() {

    // Perturb normal with smooth scrolling ripple derivative to eliminate polygonal triangulation seams

    vec2 ripUV = fragPos.xz * 0.12 + vec2(uTime * 0.02, uTime * 0.025);

    float hL = texture(causticTex, ripUV - vec2(0.01, 0.0)).r;

    float hR = texture(causticTex, ripUV + vec2(0.01, 0.0)).r;

    float hD = texture(causticTex, ripUV - vec2(0.0, 0.01)).r;

    float hU = texture(causticTex, ripUV + vec2(0.0, 0.01)).r;

    vec3 rippleN = normalize(vec3((hL - hR) * 0.45, 1.0, (hD - hU) * 0.45));

    vec3 N = normalize(fragNormal * 0.60 + rippleN * 0.40);

    vec3 V = normalize(camPos - fragPos);

    // 1. Dual-tone Water Palette extracted 1:1 from Unity WaterSurface Material

    vec3 deepCol    = vec3(0.02, 0.16, 0.42);

    vec3 shallowCol = vec3(0.05, 0.62, 0.86);

    float depthMix  = clamp(N.y * 0.85 - 0.10, 0.0, 1.0);

    vec3 baseCol    = mix(deepCol, shallowCol, depthMix);

    // 2. Animated Caustics Map (dual-layer scrolling ripples)

    vec2 uvC1 = fragPos.xz * 0.10 + vec2(uTime * 0.025, uTime * 0.035);

    vec2 uvC2 = fragPos.xz * 0.13 + vec2(-uTime * 0.030, uTime * 0.020);

    float c1 = texture(causticTex, uvC1).r;

    float c2 = texture(causticTex, uvC2).r;

    float causticVal = (c1 * 0.6 + c2 * 0.4) * 0.65;

    vec3 causticCol = shallowCol * (causticVal * 1.3);

    // 3. Flow Foam Map (FoamColor = 0.72, 0.96, 1.00)

    vec2 uvF1 = fragPos.xz * 0.22 + vec2(0.0, -uTime * 0.10);

    vec2 uvF2 = fragPos.xz * 0.28 + vec2(-uTime * 0.06, -uTime * 0.14);

    float f1 = texture(foam1Tex, uvF1).r;

    float f2 = texture(foam2Tex, uvF2).r;

    float foamSample = f1 * 0.65 + f2 * 0.35;

    float foamMask = smoothstep(0.64, 0.82, foamSample) * 0.85;

    vec3 foamCol = vec3(0.72, 0.96, 1.00);

    // 4. Tight Stylized Sun Glint

    vec3 sunDir = normalize(vec3(-0.35, 0.80, -0.45));

    vec3 R = reflect(-sunDir, N);

    float RdotV = max(dot(R, V), 0.0);

    float spec = pow(RdotV, 64.0) * 0.45;

    vec3 sunGlint = vec3(1.0, 0.95, 0.85) * spec;

    // 5. Subtle Sky Glint on Grazing Angles (Fresnel)

    float fresnel = pow(1.0 - max(dot(N, V), 0.0), 3.0) * 0.20;

    vec3 skyGlint = vec3(0.60, 0.82, 0.98) * fresnel;

    // Combine

    vec3 waterBody = baseCol + causticCol + skyGlint;

    vec3 rgb = mix(waterBody, foamCol, foamMask) + sunGlint;

    finalColor = vec4(rgb, 0.94);

}

)";

// =============================================================================

// 3. Authentic River Course Spline (113 Points from Start to Finish)

// =============================================================================

#include "spline_code.inl"

static inline Vector3 GetSplinePoint(float t) {

    if (t <= 0.0f) return AUTHENTIC_RIVER_SPLINE[0];

    if (t >= (float)(SPLINE_POINT_COUNT - 1)) return AUTHENTIC_RIVER_SPLINE[SPLINE_POINT_COUNT - 1];

    int idx = (int)t;

    float frac = t - (float)idx;

    return Vector3Lerp(AUTHENTIC_RIVER_SPLINE[idx], AUTHENTIC_RIVER_SPLINE[idx + 1], frac);

}

static inline Vector3 GetSplineTangent(float t) {

    float tA = fmaxf(0.0f, t - 0.5f);

    float tB = fminf((float)(SPLINE_POINT_COUNT - 1), t + 0.5f);

    Vector3 pA = GetSplinePoint(tA);

    Vector3 pB = GetSplinePoint(tB);

    return Vector3Normalize(Vector3Subtract(pB, pA));

}

static float FindClosestSplineT(Vector3 pos) {

    float bestT = 0.0f;

    float bestDistSq = 1e9f;

    for (int i = 0; i < SPLINE_POINT_COUNT; i++) {

        float dSq = Vector3DistanceSqr(pos, AUTHENTIC_RIVER_SPLINE[i]);

        if (dSq < bestDistSq) {

            bestDistSq = dSq;

            bestT = (float)i;

        }

    }

    float startT = fmaxf(0.0f, bestT - 1.0f);

    float endT   = fminf((float)(SPLINE_POINT_COUNT - 1), bestT + 1.0f);

    for (float t = startT; t <= endT; t += 0.1f) {

        float dSq = Vector3DistanceSqr(pos, GetSplinePoint(t));

        if (dSq < bestDistSq) {

            bestDistSq = dSq;

            bestT = t;

        }

    }

    return bestT;

}

static void QueryActualWaterSurface(Vector3 pos, float& outWaterY, float& outDistXZ, Vector3& outNearestWaterPos) {
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

// =============================================================================

// 4. Asset Helper

// =============================================================================

static inline std::string ResolveAsset(const std::string& relPath) {

    if (FileExists(relPath.c_str())) return relPath;

    std::string customPath = "CustomEngine/" + relPath;

    if (FileExists(customPath.c_str())) return customPath;

    return relPath;

}

static std::string ResolveAudioFile(const std::string& fileName) {
    std::string p1 = ResolveAsset("extracted_assets/audio/" + fileName);
    if (FileExists(p1.c_str())) return p1;
    std::string p2 = ResolveAsset("extracted_assets/audio/" + fileName + ".wav");
    if (FileExists(p2.c_str())) return p2;
    return p1;
}

static bool ParseJsonFloatField(const char* objStart, const char* objEnd, const char* key, float& outVal);

struct GasPhysicsConfig {
    float tapBurstForce     = 480.0f; // Force of quick tap burst (equalized with sustained)
    float sustainedMaxForce = 480.0f; // Max force of sustained gas hold
    float sustainedMinForce = 480.0f; // Min force of sustained gas hold
    float steerTorque       = 6.0f;   // Steering torque
    float boatMaxSpeed      = 14.0f;  // Maximum boat speed
    float waterLinearDrag   = 0.40f;  // Water linear drag/damping
    float gasDrainRate      = 12.0f;  // Gas energy consumed per second
    float gasRegenRate      = 18.0f;  // Gas energy regenerated per second
} gPhysCfg;

static void SavePhysicsConfigJSON(const std::string& path = "physics_config.json") {
    std::string resolved = ResolveAsset(path);
    FILE* f = fopen(resolved.c_str(), "w");
    if (!f) f = fopen(path.c_str(), "w");
    if (!f) return;
    fprintf(f,
        "{\n"
        "  \"tapBurstForce\": %.2f,\n"
        "  \"sustainedMaxForce\": %.2f,\n"
        "  \"sustainedMinForce\": %.2f,\n"
        "  \"steerTorque\": %.2f,\n"
        "  \"boatMaxSpeed\": %.2f,\n"
        "  \"waterLinearDrag\": %.3f,\n"
        "  \"gasDrainRate\": %.2f,\n"
        "  \"gasRegenRate\": %.2f\n"
        "}\n",
        gPhysCfg.tapBurstForce, gPhysCfg.sustainedMaxForce, gPhysCfg.sustainedMinForce,
        gPhysCfg.steerTorque, gPhysCfg.boatMaxSpeed, gPhysCfg.waterLinearDrag,
        gPhysCfg.gasDrainRate, gPhysCfg.gasRegenRate);
    fclose(f);
}

static void LoadPhysicsConfigJSON(const std::string& path = "physics_config.json") {
    std::string resolved = ResolveAsset(path);
    char* txt = LoadFileText(resolved.c_str());
    if (!txt) { SavePhysicsConfigJSON(path); return; }
    const char* end = txt + strlen(txt);
    ParseJsonFloatField(txt, end, "\"tapBurstForce\"", gPhysCfg.tapBurstForce);
    ParseJsonFloatField(txt, end, "\"sustainedMaxForce\"", gPhysCfg.sustainedMaxForce);
    ParseJsonFloatField(txt, end, "\"sustainedMinForce\"", gPhysCfg.sustainedMinForce);
    ParseJsonFloatField(txt, end, "\"steerTorque\"", gPhysCfg.steerTorque);
    ParseJsonFloatField(txt, end, "\"boatMaxSpeed\"", gPhysCfg.boatMaxSpeed);
    ParseJsonFloatField(txt, end, "\"waterLinearDrag\"", gPhysCfg.waterLinearDrag);
    ParseJsonFloatField(txt, end, "\"gasDrainRate\"", gPhysCfg.gasDrainRate);
    ParseJsonFloatField(txt, end, "\"gasRegenRate\"", gPhysCfg.gasRegenRate);
    UnloadFileText(txt);
}

enum AppScreenState {
    SCREEN_MAIN_MENU = 0,
    SCREEN_LAN_LOBBY = 1,
    SCREEN_SETTINGS  = 2,
    SCREEN_GAMEPLAY  = 3
};


// =============================================================================

// 5. Particle System (Animated Comic Billboard Fart VFX)

// =============================================================================

struct ComicParticle {

    Vector3 position;

    Vector3 velocity;

    float size;

    float alpha;

    float rotation;

    float rotSpeed;

    float life;

    float maxLife;

    int spriteIndex;

    bool active;

};

static const int MAX_PARTICLES = 400;

static ComicParticle particles[MAX_PARTICLES];

static void EmitFartBurst(Vector3 pos, Vector3 thrustDir, float chargePercent, bool isSuperSync = false) {

    int count = 10 + (int)(chargePercent * 16.0f);

    if (isSuperSync) count *= 2;

    for (int i = 0; i < count; i++) {

        for (int p = 0; p < MAX_PARTICLES; p++) {

            if (!particles[p].active) {

                particles[p].active = true;

                particles[p].position = pos + Vector3{

                    ((float)rand() / RAND_MAX - 0.5f) * 0.35f,

                    0.20f + ((float)rand() / RAND_MAX) * 0.25f,

                    ((float)rand() / RAND_MAX - 0.5f) * 0.35f

                };

                Vector3 spread = Vector3{

                    ((float)rand() / RAND_MAX - 0.5f) * 1.4f,

                    1.2f + ((float)rand() / RAND_MAX) * 1.5f,

                    ((float)rand() / RAND_MAX - 0.5f) * 1.4f

                };

                Vector3 hDir = Vector3Normalize(Vector3{ thrustDir.x + spread.x * 0.35f, 0.0f, thrustDir.z + spread.z * 0.35f });

                float blastSpeed = 2.5f + chargePercent * 4.0f;

                particles[p].velocity = Vector3{ hDir.x * blastSpeed, spread.y, hDir.z * blastSpeed };

                particles[p].size = 0.45f + (chargePercent * 0.35f) + ((float)rand() / RAND_MAX) * 0.20f;

                if (isSuperSync) particles[p].size *= 1.25f;

                particles[p].alpha = 0.95f;

                particles[p].rotation = ((float)rand() / RAND_MAX) * 360.0f;

                particles[p].rotSpeed = ((float)rand() / RAND_MAX - 0.5f) * 90.0f;

                particles[p].life = 0.0f;

                particles[p].maxLife = 1.0f + chargePercent * 0.6f;

                particles[p].spriteIndex = rand() % 4;

                break;

            }

        }

    }

}

// =============================================================================

// 6. Camera Controller with Shake Trauma

// =============================================================================

struct CameraController {

    float yaw;

    float pitch;

    float distance;

    float targetDist;

    bool freeLookMode;

    float autoRecenterTimer;

    float shakeTrauma; // 0.0 to 1.0 (Bumper / bite explosion)

};

// =============================================================================

// 7. Dynamic Game Mechanics Structures

// =============================================================================

// Gas Energy State (Per Pirate)

struct PirateGasState {

    float gasEnergy;

    bool  gasDepleted;

    float regenDelayTimer;

    float pressTimer;

    bool  isPressing;

    bool  isSustained;

    float sustainedCharge; // 0.0 to 1.0

    float releaseTimer;

    float releaseStartCharge;

    // Visual Butt State

    float boneScale;

    float trembleOffset;

    float redness;

    float jigglePhase;

    void Reset() {

        gasEnergy = GAS_MAX;

        gasDepleted = false;

        regenDelayTimer = 0.0f;

        pressTimer = 0.0f;

        isPressing = false;

        isSustained = false;

        sustainedCharge = 0.0f;

        releaseTimer = 0.0f;

        releaseStartCharge = 0.0f;

        boneScale = 1.0f;

        trembleOffset = 0.0f;

        redness = 0.0f;

        jigglePhase = 0.0f;

    }

    void Update(float dt, bool inputDown, float runTime) {

        jigglePhase += dt * 10.0f;

        if (inputDown && !gasDepleted) {

            isPressing = true;

            pressTimer += dt;

            regenDelayTimer = GAS_REGEN_DELAY;

            if (pressTimer > GAS_TAP_THRESHOLD_SECONDS) {

                // Sustained Hold

                isSustained = true;

                sustainedCharge = Clamp((pressTimer - GAS_TAP_THRESHOLD_SECONDS) / GAS_RAMP_SECONDS, 0.0f, 1.0f);

                gasEnergy -= GAS_DRAIN_PER_SEC * dt;

                if (gasEnergy <= 0.0f) {

                    gasEnergy = 0.0f;

                    gasDepleted = true;

                    isSustained = false;

                    isPressing = false;

                    pressTimer = 0.0f;

                }

            }

        } else {

            // Released or depleted

            if (isPressing) {

                // If released after sustained hold, start release decay

                if (isSustained) {

                    releaseTimer = GAS_RELEASE_SECONDS;

                    releaseStartCharge = sustainedCharge;

                }

                isPressing = false;

                isSustained = false;

                pressTimer = 0.0f;

            }

            if (releaseTimer > 0.0f) {

                releaseTimer -= dt;

                if (releaseTimer < 0.0f) releaseTimer = 0.0f;

            }

            // Gas Regeneration

            if (regenDelayTimer > 0.0f) {

                regenDelayTimer -= dt;

            } else {

                gasEnergy += GAS_REGEN_PER_SEC * dt;

                if (gasEnergy >= GAS_MAX) {

                    gasEnergy = GAS_MAX;

                    gasDepleted = false;

                } else if (gasDepleted && (gasEnergy / GAS_MAX) >= GAS_RECOVER_PERCENT) {

                    gasDepleted = false;

                }

            }

            sustainedCharge = 0.0f;

        }

        // Compute effective visual charge (starts immediately on press!)
        float immediatePressCharge = isPressing ? Clamp(pressTimer / 1.2f, 0.18f, 1.0f) : 0.0f;
        float visualCharge = fmaxf(sustainedCharge, immediatePressCharge);
        if (releaseTimer > 0.0f) {
            visualCharge = fmaxf(visualCharge, releaseStartCharge * (releaseTimer / GAS_RELEASE_SECONDS));
        }

        // Gradual Butt Bone Swell (1.0x to 1.30x max)
        boneScale = 1.0f + (BUTT_PEAK_SCALE - 1.0f) * visualCharge;

        // Tremble vibration at 12Hz above 25% charge
        if (visualCharge > 0.25f) {
            float strength = (visualCharge - 0.25f) / 0.75f;
            trembleOffset = sinf(2.0f * PI * BUTT_TREMBLE_FREQ * runTime) * BUTT_TREMBLE_AMP * strength;
        } else {
            trembleOffset = 0.0f;
        }

        // Smooth Gradual Vascular Redness (starts strictly at 60% charge!)
        if (visualCharge >= 0.60f) {
            float rFrac = Clamp((visualCharge - 0.60f) / 0.40f, 0.0f, 1.0f);
            redness = rFrac * (0.65f + 0.35f * rFrac);
        } else {
            redness = 0.0f;
        }

    }

};

// Unified Level Entity Architecture (Obstacles, Rocks, Trees, Checkpoints, Props, Custom Colliders)
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
    ENT_OBS_F10 = 9,  // GM_3D_ObstacleF10.obj (Split: Arms & Downward Hammer)
    ENT_OBS_F11 = 10, // GM_3D_ObstacleF11.obj (Blue spiked drum cylinder)
    ENT_OBS_F13 = 11, // GM_3D_ObstacleF13.obj (Medium rotating arm)
    ENT_OBS_F14 = 12, // GM_3D_ObstacleF14_P1.obj (Sliding barrier)
    ENT_OBS_F18 = 13, // GM_3D_ObstacleF18.obj (Spike post)
    ENT_OBS_F23 = 14, // GM_3D_ObstacleF23.obj (Double swinging barrier)
    ENT_ROCK_1 = 15,  // GM_3D_Obstacle_Rock1.obj
    ENT_ROCK_2 = 16,  // GM_3D_Obstacle_Rock2.obj
    ENT_GUST = 17,    // Wind Gust zone
    ENT_CHECKPOINT = 18,  // GM_3D_CheckPointV1.obj (Unified editable Checkpoint)
    ENT_TREE_JUNGLE = 19, // GM_3D_JungleTree.obj
    // 10 Custom Collision Shapes (Plan v6.0 Task 4 & 5)
    ENT_COL_SQUARE = 20,   // Square base collision box
    ENT_COL_RECT = 21,     // Rectangle / Wall collision box
    ENT_COL_CIRCLE = 22,   // Circle horizontal collision
    ENT_COL_CUBE = 23,     // 3D Cube
    ENT_COL_CYLINDER = 24, // 3D Cylinder
    ENT_COL_CAPSULE = 25,  // 3D Capsule
    ENT_COL_SPHERE = 26,   // 3D Sphere
    ENT_COL_WEDGE = 27,    // Wedge / Ramp
    ENT_COL_FALL_ZONE = 28,// Fall into abyss zone
    ENT_COL_KILL_ZONE = 29,// Explode / Kill boat zone
    ENT_COUNT = 30
};

static const char* ENTITY_TYPE_NAMES[] = {
    "OBS_A1_BUMPER", "OBS_B2_TIPPING", "OBS_B3_SPINNER", "OBS_B5_ROTOR",
    "OBS_C1_ARC", "OBS_C2_PENDULUM", "OBS_C4_QUAD", "OBS_C5_WHEEL",
    "OBS_F3_SWING", "OBS_F10_GATE", "OBS_F11_DRUM", "OBS_F13_ARM",
    "OBS_F14_SLIDER", "OBS_F18_SPIKE", "OBS_F23_DUAL",
    "ROCK_DARK_1", "ROCK_DARK_2", "WIND_GUST",
    "CHECKPOINT_GATE", "TREE_JUNGLE",
    "COL_SQUARE", "COL_RECT_WALL", "COL_CIRCLE", "COL_CUBE",
    "COL_CYLINDER", "COL_CAPSULE", "COL_SPHERE", "COL_WEDGE",
    "COL_FALL_ZONE", "COL_KILL_ZONE"
};

struct LevelEntity {
    int id;
    EntityType type;
    Vector3 position;
    float rotationX;
    float rotationY;
    float rotationZ;
    int flipX;
    int flipY;
    int flipZ;
    float scale;
    float animTimer;
    float animSpeed;
    float radius;
    float sizeX;
    float sizeY;
    float sizeZ;
    int colBehavior; // 0=Solid Wall, 1=Fall/Abyss, 2=Kill/Explode, 3=Bounce Pad
    bool cpPassed;
    Model* model;
    // Bumper specific state
    float bumperExpandTimer;
    float bumperCooldownTimer;
    bool isExpanded;
};

// Crocodile AI State
struct CrocodileAI {
    Vector3 position;
    float rotationY;
    Vector3 startPosition;
    float biteCooldownTimer;
    bool isChasing;
    float lungeSpeed;
};

// Shark AI State
struct SharkAI {
    Vector3 position;
    float rotationY;
    Vector3 centerPoint;
    float patrolAngle;
    float biteCooldownTimer;
    bool isChasing;
    float speed;
};

// Ragdoll Pirate on Death
struct RagdollPirate {
    Vector3 position;
    Vector3 velocity;
    Vector3 rotation;
    Vector3 rotVelocity;
    bool active;
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
    Model mF10_Arms;
    Model mF10_Hammer;
    Model mF11;
    Model mF13;
    Model mF14;
    Model mF18;
    Model mF23;
    Model mR1;
    Model mR2;
    Model mTree;
    Model mCP;
};

// Real Water Mesh Triangle Grid for accurate surface elevation and cliff-edge falling
struct WaterTri2D {
    float x0, z0, x1, z1, x2, z2;
    float minX, maxX, minZ, maxZ, avgY;
};
static std::vector<WaterTri2D> gWaterTris;

static void BuildWaterTriangleGrid(const Model& waterModel, Vector3 offset) {
    gWaterTris.clear();
    for (int mi = 0; mi < waterModel.meshCount; mi++) {
        const Mesh& m = waterModel.meshes[mi];
        const float* v = m.vertices;
        if (!v) continue;
        if (m.indices && m.triangleCount > 0) {
            for (int t = 0; t < m.triangleCount; t++) {
                int i0 = m.indices[t * 3 + 0];
                int i1 = m.indices[t * 3 + 1];
                int i2 = m.indices[t * 3 + 2];
                float x0 = v[i0*3+0] + offset.x, y0 = v[i0*3+1] + offset.y, z0 = v[i0*3+2] + offset.z;
                float x1 = v[i1*3+0] + offset.x, y1 = v[i1*3+1] + offset.y, z1 = v[i1*3+2] + offset.z;
                float x2 = v[i2*3+0] + offset.x, y2 = v[i2*3+1] + offset.y, z2 = v[i2*3+2] + offset.z;
                WaterTri2D wt;
                wt.x0 = x0; wt.z0 = z0; wt.x1 = x1; wt.z1 = z1; wt.x2 = x2; wt.z2 = z2;
                wt.minX = fminf(x0, fminf(x1, x2)) - 0.50f;
                wt.maxX = fmaxf(x0, fmaxf(x1, x2)) + 0.50f;
                wt.minZ = fminf(z0, fminf(z1, z2)) - 0.50f;
                wt.maxZ = fmaxf(z0, fmaxf(z1, z2)) + 0.50f;
                wt.avgY = (y0 + y1 + y2) / 3.0f;
                gWaterTris.push_back(wt);
            }
        }
    }
}

static bool IsPointOverWaterMesh(float px, float pz, float& outWaterY) {
    // 1. Check circular start pond
    float dxPond = px - 15.43f, dzPond = pz - 84.0f;
    if (pz >= 74.0f && (dxPond * dxPond + dzPond * dzPond) <= (15.8f * 15.8f)) {
        outWaterY = 0.55f;
        return true;
    }
    // 2. Check water triangles of GM_3D_Demo_Water.obj
    for (const auto& t : gWaterTris) {
        if (px < t.minX || px > t.maxX || pz < t.minZ || pz > t.maxZ) continue;
        float dX = px - t.x2, dZ = pz - t.z2;
        float dX21 = t.x2 - t.x1, dZ12 = t.z1 - t.z2;
        float D = dZ12 * (t.x0 - t.x2) + dX21 * (t.z0 - t.z2);
        if (fabsf(D) < 1e-5f) continue;
        float s = (dZ12 * dX + dX21 * dZ) / D;
        float tVal = ((t.z2 - t.z0) * dX + (t.x0 - t.x2) * dZ) / D;
        if (s >= -0.08f && tVal >= -0.08f && (s + tVal) <= 1.08f) {
            outWaterY = t.avgY;
            return true;
        }
    }
    return false;
}

// 2D distance helper for Gizmo ray interaction
static float DistPointToSegment2D(Vector2 p, Vector2 a, Vector2 b) {
    Vector2 ab = { b.x - a.x, b.y - a.y };
    Vector2 ap = { p.x - a.x, p.y - a.y };
    float lenSq = ab.x * ab.x + ab.y * ab.y;
    if (lenSq < 1.0f) return Vector2Distance(p, a);
    float t = Clamp((ap.x * ab.x + ap.y * ab.y) / lenSq, 0.0f, 1.0f);
    Vector2 proj = { a.x + ab.x * t, a.y + ab.y * t };
    return Vector2Distance(p, proj);
}

static void GetEntityAnimatedTransform(const LevelEntity& ent, Vector3& outPos, float& outYaw, float& outPitch, float& outRoll) {
    outPos   = ent.position;
    outYaw   = ent.rotationY;
    outPitch = ent.rotationX;
    outRoll  = ent.rotationZ;

    switch (ent.type) {
        case ENT_OBS_B2:
        case ENT_OBS_C1:
            // Semicircle 180 deg swing around local X axis
            outPitch = -90.0f + sinf(ent.animTimer * 1.85f + (float)ent.id) * 90.0f;
            break;

        case ENT_OBS_F10:
            // Video 2: Arms rotate along symmetrical arc from left bank to right bank (-73.07° ± 73.07°)
            outRoll = -73.07f + sinf(ent.animTimer * 1.85f + (float)ent.id) * 73.07f;
            break;

        case ENT_OBS_F13:
            // Video 5: Full 360 deg horizontal circle in XZ plane around local Y axis!
            outYaw   = fmodf(ent.rotationY - ent.animTimer * 120.0f, 360.0f);
            outPitch = 0.0f;
            outRoll  = 0.0f;
            break;

        case ENT_OBS_F11:
            // Blue spiked drum: stationary
            break;

        case ENT_OBS_B3:
        case ENT_OBS_B5:
        case ENT_OBS_C4:
            outYaw = fmodf(ent.rotationY + ent.animTimer * 95.0f, 360.0f);
            break;

        case ENT_OBS_C5:
            outRoll = fmodf(ent.animTimer * 80.0f, 360.0f);
            break;

        case ENT_OBS_C2:
            outRoll = sinf(ent.animTimer * 2.0f + (float)ent.id) * 48.0f;
            break;

        case ENT_OBS_F3:
        case ENT_OBS_F23:
            // Video 1 (OBS_F3_SWING): Full 180 deg swing (-90° to +90°)!
            outYaw = ent.rotationY + sinf(ent.animTimer * 1.85f) * 90.0f;
            break;

        case ENT_OBS_F14: {
            float slide = sinf(ent.animTimer * 2.0f + (float)ent.id) * 1.8f;
            float rad = ent.rotationY * DEG2RAD;
            outPos.x += cosf(rad) * slide;
            outPos.z -= sinf(rad) * slide;
            break;
        }

        default:
            break;
    }
}

static Vector3 GetObstacleHazardWorldPos(const LevelEntity& ent) {
    Vector3 animPos; float animYaw, animPitch, animRoll;
    GetEntityAnimatedTransform(ent, animPos, animYaw, animPitch, animRoll);
    float s = (ent.scale > 0.01f) ? ent.scale : 1.0f;

    Vector3 localHazard = { 0.0f, 0.0f, 0.0f };
    if (ent.type == ENT_OBS_B2)  localHazard = { 0.0f, 0.0f,  4.4f * s };
    if (ent.type == ENT_OBS_C1)  localHazard = { 0.0f, 0.0f,  3.7f * s };
    if (ent.type == ENT_OBS_F10) {
        float rad = animRoll * DEG2RAD;
        const float wristX = -7.23f, wristY = 2.20f;
        float newWristX = wristX * cosf(rad) - wristY * sinf(rad);
        float newWristY = wristX * sinf(rad) + wristY * cosf(rad);
        localHazard = Vector3{ newWristX * s, (newWristY - 2.20f) * s, 0.0f };
    }
    if (ent.type == ENT_OBS_F13) localHazard = { -4.6f * s, 0.0f, 0.0f };

    Matrix m = MatrixIdentity();
    if (fabsf(animRoll)  > 0.01f) m = MatrixMultiply(m, MatrixRotateZ(animRoll  * DEG2RAD));
    if (fabsf(animPitch) > 0.01f) m = MatrixMultiply(m, MatrixRotateX(animPitch * DEG2RAD));
    m = MatrixMultiply(m, MatrixRotateY(animYaw * DEG2RAD));
    return Vector3Add(animPos, Vector3Transform(localHazard, m));
}

static ObstacleModels gObsModels;
static const char* LEVEL_LAYOUT_PATH = "level_layout.json";

static bool CheckObstacleOBBCollision(Vector3 raftPos, const LevelEntity& ent, Vector3 animPos, float animYawDeg, float animRollDeg) {
    if (ent.type == ENT_TREE_JUNGLE || ent.type == ENT_GUST || ent.type == ENT_ROCK_1 || ent.type == ENT_ROCK_2 || ent.type == ENT_OBS_F18 || ent.type == ENT_CHECKPOINT) return false;
    if (ent.type >= ENT_COL_SQUARE) return false; // Handled by custom collision shapes

    const float boatR = 0.88f;
    float s = (ent.scale > 0.01f) ? ent.scale : 1.0f;

    // For elevated swinging arm hazards (B2, C1, F10, F13), check exact 3D hazard head position
    if (ent.type == ENT_OBS_B2 || ent.type == ENT_OBS_C1 || ent.type == ENT_OBS_F10 || ent.type == ENT_OBS_F13) {
        Vector3 hPos = GetObstacleHazardWorldPos(ent);
        if (fabsf((raftPos.y + 0.35f) - hPos.y) < 1.4f) {
            float dx = raftPos.x - hPos.x;
            float dz = raftPos.z - hPos.z;
            float hRadius = (ent.type == ENT_OBS_F10) ? (0.95f * s) : (0.80f * s);
            if ((dx * dx + dz * dz) < ((hRadius + boatR) * (hRadius + boatR))) {
                return true;
            }
        }
        float dyBase = fabsf((raftPos.y + 0.35f) - animPos.y);
        if (dyBase < 1.5f) {
            float dxBase = raftPos.x - animPos.x;
            float dzBase = raftPos.z - animPos.z;
            if ((dxBase * dxBase + dzBase * dzBase) < ((0.65f * s + boatR) * (0.65f * s + boatR))) {
                return true;
            }
        }
        return false;
    }

    if (ent.type == ENT_OBS_A1) {
        if (fabsf(raftPos.y - animPos.y) > 2.0f) return false;
        float dx = raftPos.x - animPos.x;
        float dz = raftPos.z - animPos.z;
        float rad = -animYawDeg * DEG2RAD;
        float lx = dx * cosf(rad) - dz * sinf(rad);
        float lz = dx * sinf(rad) + dz * cosf(rad);
        return (fabsf(lx) < (0.48f * s + boatR)) && (fabsf(lz) < (2.40f * s + 0.65f));
    }

    float dy = fabsf((raftPos.y + 0.35f) - animPos.y);
    if (ent.type == ENT_OBS_C5) {
        if (dy > 4.6f * s) return false;
    } else if (dy > 2.2f * s) {
        return false;
    }

    float rad = -animYawDeg * DEG2RAD;
    float dx = raftPos.x - animPos.x;
    float dz = raftPos.z - animPos.z;
    float localX = dx * cosf(rad) - dz * sinf(rad);
    float localZ = dx * sinf(rad) + dz * cosf(rad);

    float hx = 1.0f * s, hz = 1.0f * s;
    switch (ent.type) {
        case ENT_OBS_B3:  hx = 2.45f * s; hz = 0.48f * s; break;
        case ENT_OBS_B5:  hx = 4.90f * s; hz = 0.50f * s; break;
        case ENT_OBS_C2:  hx = 1.15f * s; hz = 1.15f * s; break;
        case ENT_OBS_C4:  hx = 2.50f * s; hz = 1.25f * s; break;
        case ENT_OBS_C5:  hx = 4.30f * s; hz = 0.55f * s; break;
        case ENT_OBS_F3:  hx = 2.30f * s; hz = 0.38f * s; break;
        case ENT_OBS_F11: hx = 0.95f * s; hz = 0.95f * s; break;
        case ENT_OBS_F14: hx = 2.80f * s; hz = 0.45f * s; break;
        case ENT_OBS_F23: hx = 2.80f * s; hz = 0.40f * s; break;
        default: break;
    }
    return (fabsf(localX) < (hx + boatR)) && (fabsf(localZ) < (hz + boatR));
}

void AssignEntityModelAndProps(LevelEntity& e) {
    if (e.scale <= 0.01f || !std::isfinite(e.scale)) e.scale = 1.0f;
    if (e.radius <= 0.01f || !std::isfinite(e.radius)) e.radius = 2.0f;
    if (e.animSpeed < 0.0f || !std::isfinite(e.animSpeed)) e.animSpeed = 1.0f;
    if (e.flipX == 0) e.flipX = 1;
    if (e.flipY == 0) e.flipY = 1;
    if (e.flipZ == 0) e.flipZ = 1;
    if (e.sizeX <= 0.05f) e.sizeX = e.radius;
    if (e.sizeY <= 0.05f) e.sizeY = 2.0f;
    if (e.sizeZ <= 0.05f) e.sizeZ = e.radius;

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
        case ENT_GUST:         e.model = nullptr;         e.radius = 4.50f * e.scale; break;
        case ENT_CHECKPOINT:   e.model = &gObsModels.mCP;  e.radius = 3.50f * e.scale; break;
        case ENT_TREE_JUNGLE:  e.model = &gObsModels.mTree;e.radius = 1.35f * e.scale; break;
        // Custom Collision Shapes
        case ENT_COL_SQUARE:
        case ENT_COL_RECT:
        case ENT_COL_CIRCLE:
        case ENT_COL_CUBE:
        case ENT_COL_CYLINDER:
        case ENT_COL_CAPSULE:
        case ENT_COL_SPHERE:
        case ENT_COL_WEDGE:
        case ENT_COL_FALL_ZONE:
        case ENT_COL_KILL_ZONE:
            e.model = nullptr;
            e.radius = (e.sizeX > 0.1f) ? e.sizeX : 2.0f;
            break;
        default: e.type = ENT_OBS_A1; e.model = &gObsModels.mA1; e.radius = 1.85f * e.scale; break;
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
            "  {\n"
            "    \"id\": %d,\n"
            "    \"type\": %d,\n"
            "    \"x\": %.3f,\n"
            "    \"y\": %.3f,\n"
            "    \"z\": %.3f,\n"
            "    \"rotX\": %.2f,\n"
            "    \"rotY\": %.2f,\n"
            "    \"rotZ\": %.2f,\n"
            "    \"flipX\": %d,\n"
            "    \"flipY\": %d,\n"
            "    \"flipZ\": %d,\n"
            "    \"scale\": %.2f,\n"
            "    \"speed\": %.2f,\n"
            "    \"radius\": %.2f,\n"
            "    \"sizeX\": %.2f,\n"
            "    \"sizeY\": %.2f,\n"
            "    \"sizeZ\": %.2f,\n"
            "    \"colBehavior\": %d\n"
            "  }%s\n",
            (int)i, (int)e.type, e.position.x, e.position.y, e.position.z,
            e.rotationX, e.rotationY, e.rotationZ,
            e.flipX, e.flipY, e.flipZ,
            e.scale, e.animSpeed, e.radius,
            e.sizeX, e.sizeY, e.sizeZ, e.colBehavior,
            (i + 1 < list.size()) ? "," : "");
    }
    fprintf(f, "]\n");
    fclose(f);
}

static bool ParseJsonFloatField(const char* objStart, const char* objEnd, const char* key, float& outVal) {
    const char* p = strstr(objStart, key);
    if (!p || p >= objEnd) return false;
    p += strlen(key);
    while (p < objEnd && (*p == ' ' || *p == '\t' || *p == ':')) p++;
    return sscanf(p, "%f", &outVal) == 1;
}

static bool ParseJsonIntField(const char* objStart, const char* objEnd, const char* key, int& outVal) {
    const char* p = strstr(objStart, key);
    if (!p || p >= objEnd) return false;
    p += strlen(key);
    while (p < objEnd && (*p == ' ' || *p == '\t' || *p == ':')) p++;
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
        if (ParseJsonIntField(objStart, objEnd, "\"type\"", t)) {
            if (t < 0 || t >= (int)ENT_COUNT) t = 0;
            float x = 0.0f, y = 0.0f, z = 0.0f;
            float rotX = 0.0f, rotY = 0.0f, rotZ = 0.0f;
            int flipX = 1, flipY = 1, flipZ = 1;
            float scale = 1.0f, spd = 1.0f, rad = 2.0f;
            float szX = 2.0f, szY = 2.0f, szZ = 2.0f;
            int colBeh = 0;

            ParseJsonFloatField(objStart, objEnd, "\"x\"", x);
            ParseJsonFloatField(objStart, objEnd, "\"y\"", y);
            ParseJsonFloatField(objStart, objEnd, "\"z\"", z);
            ParseJsonFloatField(objStart, objEnd, "\"rotX\"", rotX);
            ParseJsonFloatField(objStart, objEnd, "\"rotY\"", rotY);
            ParseJsonFloatField(objStart, objEnd, "\"rotZ\"", rotZ);
            ParseJsonIntField(objStart, objEnd, "\"flipX\"", flipX);
            ParseJsonIntField(objStart, objEnd, "\"flipY\"", flipY);
            ParseJsonIntField(objStart, objEnd, "\"flipZ\"", flipZ);
            ParseJsonFloatField(objStart, objEnd, "\"scale\"", scale);
            ParseJsonFloatField(objStart, objEnd, "\"speed\"", spd);
            ParseJsonFloatField(objStart, objEnd, "\"radius\"", rad);
            ParseJsonFloatField(objStart, objEnd, "\"sizeX\"", szX);
            ParseJsonFloatField(objStart, objEnd, "\"sizeY\"", szY);
            ParseJsonFloatField(objStart, objEnd, "\"sizeZ\"", szZ);
            ParseJsonIntField(objStart, objEnd, "\"colBehavior\"", colBeh);

            LevelEntity e{};
            e.id = nextId++;
            e.type = static_cast<EntityType>(t);
            e.position = Vector3{ x, y, z };
            e.rotationX = rotX;
            e.rotationY = rotY;
            e.rotationZ = rotZ;
            e.flipX = (flipX != 0) ? flipX : 1;
            e.flipY = (flipY != 0) ? flipY : 1;
            e.flipZ = (flipZ != 0) ? flipZ : 1;
            e.scale = (scale > 0.01f) ? scale : 1.0f;
            e.animSpeed = (spd >= 0.0f) ? spd : 1.0f;
            e.radius = (rad > 0.01f) ? rad : 2.0f;
            e.sizeX = (szX > 0.05f) ? szX : e.radius;
            e.sizeY = (szY > 0.05f) ? szY : 2.0f;
            e.sizeZ = (szZ > 0.05f) ? szZ : e.radius;
            e.colBehavior = colBeh;
            e.cpPassed = false;
            AssignEntityModelAndProps(e);
            list.push_back(e);
        }
        cursor = objEnd + 1;
    }
    UnloadFileText(rawText);
    return !list.empty();
}
int main(int argc, char** argv) {

    bool testScreenshotMode = false;
    bool testEditorMode = false;
    bool testDeathMode = false;
    bool testLanMode = false;
    bool testCatalogMode = false;
    int testSelectIdx = -1;
    bool useCustomTestCam = false;
    Vector3 customTestCamPos = { 0, 0, 0 };
    Vector3 customTestCamTarget = { 0, 0, 0 };

    std::string testShotName = "CustomEngine/scratch/active_gameplay.png";

    int testAngleMode = 0; // 0=behind, 1=side, 2=front, 3=top

    int targetTestFrame = 60;

    std::string testScreenArg = "";
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--test-screen") == 0 && i + 1 < argc) testScreenArg = argv[++i];
        if (strcmp(argv[i], "--test-screenshot") == 0) testScreenshotMode = true;
        if (strcmp(argv[i], "--test-editor") == 0) testEditorMode = true;
        if (strcmp(argv[i], "--test-catalog") == 0) { testCatalogMode = true; testEditorMode = true; }
        if (strcmp(argv[i], "--test-death") == 0) testDeathMode = true;
        if (strcmp(argv[i], "--test-lan") == 0) testLanMode = true;
        if (strcmp(argv[i], "--test-select") == 0 && i + 1 < argc) testSelectIdx = atoi(argv[++i]);

        if (strcmp(argv[i], "--shot-name") == 0 && i + 1 < argc) { testShotName = argv[++i]; testScreenshotMode = true; }

        if (strcmp(argv[i], "--shot-angle") == 0 && i + 1 < argc) {

            std::string a = argv[++i];

            if (a == "side") testAngleMode = 1;

            else if (a == "front") testAngleMode = 2;

            else if (a == "top") testAngleMode = 3;

            else testAngleMode = 0;

        }

        if (strcmp(argv[i], "--shot-frames") == 0 && i + 1 < argc) { targetTestFrame = atoi(argv[++i]); testScreenshotMode = true; }

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
    }

    SetConfigFlags(FLAG_WINDOW_UNDECORATED | FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);

    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Fart Together - Authentic 1:1 Engine");
    SetExitKey(KEY_NULL); // ESC key handled by in-game UI menu

    InitAudioDevice();

    SetTargetFPS(60);
    // 3D Model Live Preview and Editor Inspector state
    RenderTexture2D previewRT = LoadRenderTexture(280, 125);
    bool inspectorCollapsed = false;
    bool leftDockCollapsed = false;
    bool visualCatalogOpen = testCatalogMode;
    int editorGizmoMode = 0; // 0=Select, 1=Move, 2=Rotate, 3=Scale
    int gActiveGizmoAxis = -1; // -1=none, 0=X, 1=Y, 2=Z
    int hierarchyScroll = 0;
    int catalogScroll = 0;

    TraceLog(LOG_INFO, "Loading authentic shaders, textures, and 3D meshes...");

    // Shaders

    Shader shaderLighting = LoadShaderFromMemory(VS_LIGHTING, FS_LIGHTING);

    int lightDirLoc   = GetShaderLocation(shaderLighting, "lightDir");

    int lightColLoc   = GetShaderLocation(shaderLighting, "lightColor");

    int ambColLoc     = GetShaderLocation(shaderLighting, "ambientColor");

    int uRednessLoc   = GetShaderLocation(shaderLighting, "uRedness");
    int uButtSwellLoc = GetShaderLocation(shaderLighting, "uButtSwell");
    int uEyeBulgeLoc  = GetShaderLocation(shaderLighting, "uEyeBulge");
    int uSkinULoc     = GetShaderLocation(shaderLighting, "uSkinU");
    int uSkinVLoc     = GetShaderLocation(shaderLighting, "uSkinV");
    int uIsJungleTreeLoc = GetShaderLocation(shaderLighting, "uIsJungleTree");
    int zeroTree = 0;
    SetShaderValue(shaderLighting, uIsJungleTreeLoc, &zeroTree, SHADER_UNIFORM_INT);
    float zeroVal = 0.0f;
    SetShaderValue(shaderLighting, uButtSwellLoc, &zeroVal, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shaderLighting, uEyeBulgeLoc,  &zeroVal, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shaderLighting, uSkinULoc,     &zeroVal, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shaderLighting, uSkinVLoc,     &zeroVal, SHADER_UNIFORM_FLOAT);

    Vector3 lDir = Vector3Normalize(Vector3{ -0.35f, 0.95f, 0.40f });

    Vector3 lCol = Vector3{ 1.02f, 0.99f, 0.94f };

    Vector3 aCol = Vector3{ 0.62f, 0.65f, 0.72f };

    float initRed = 0.0f;

    SetShaderValue(shaderLighting, lightDirLoc, &lDir, SHADER_UNIFORM_VEC3);

    SetShaderValue(shaderLighting, lightColLoc, &lCol, SHADER_UNIFORM_VEC3);

    SetShaderValue(shaderLighting, ambColLoc,   &aCol, SHADER_UNIFORM_VEC3);

    SetShaderValue(shaderLighting, uRednessLoc, &initRed, SHADER_UNIFORM_FLOAT);

    Shader shaderWater = LoadShaderFromMemory(VS_WATER, FS_WATER);

    int uTimeLoc  = GetShaderLocation(shaderWater, "uTime");

    int camPosLoc = GetShaderLocation(shaderWater, "camPos");

    int wLightLoc = GetShaderLocation(shaderWater, "lightDir");

    SetShaderValue(shaderWater, wLightLoc, &lDir, SHADER_UNIFORM_VEC3);

    // Textures
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
    Model wallColModel = LoadModel(ResolveAsset("extracted_assets/meshes/GM_3D_Demo_WallCol.obj").c_str());
    Texture2D texSkullGas = LoadTexture(ResolveAsset("extracted_assets/textures/SkullGasCloud.png").c_str());
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
    gObsModels.mF10        = LoadModel(ResolveAsset("extracted_assets/meshes/GM_3D_ObstacleF10.obj").c_str());
    gObsModels.mF10_Arms   = LoadModel(ResolveAsset("extracted_assets/meshes/GM_3D_ObstacleF10_Arms.obj").c_str());
    gObsModels.mF10_Hammer = LoadModel(ResolveAsset("extracted_assets/meshes/GM_3D_ObstacleF10_Hammer.obj").c_str());
    gObsModels.mF11 = LoadModel(ResolveAsset("extracted_assets/meshes/GM_3D_ObstacleF11.obj").c_str());
    gObsModels.mF13 = LoadModel(ResolveAsset("extracted_assets/meshes/GM_3D_ObstacleF13.obj").c_str());
    gObsModels.mF14 = LoadModel(ResolveAsset("extracted_assets/meshes/GM_3D_ObstacleF14_P1.obj").c_str());
    gObsModels.mF18 = LoadModel(ResolveAsset("extracted_assets/meshes/GM_3D_ObstacleF18.obj").c_str());
    gObsModels.mF23 = LoadModel(ResolveAsset("extracted_assets/meshes/GM_3D_ObstacleF23.obj").c_str());
    gObsModels.mR1  = LoadModel(ResolveAsset("extracted_assets/meshes/GM_3D_Obstacle_Rock1.obj").c_str());
    gObsModels.mR2  = LoadModel(ResolveAsset("extracted_assets/meshes/GM_3D_Obstacle_Rock2.obj").c_str());
    gObsModels.mTree= LoadModel(ResolveAsset("extracted_assets/meshes/GM_3D_JungleTree.obj").c_str());
    gObsModels.mCP  = cp1Model;

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
    ApplyMat(gObsModels.mF10_Arms);
    ApplyMat(gObsModels.mF10_Hammer);
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
    }

    // Water Material setup

    waterModel.materials[0].shader = shaderWater;

    waterModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texCaustic;

    waterModel.materials[0].maps[MATERIAL_MAP_NORMAL].texture  = texFoam1;

    waterModel.materials[0].maps[MATERIAL_MAP_SPECULAR].texture= texFoam2;

    // Audio

    Sound fartSounds[6];
    for (int s = 0; s < 6; s++) {
        std::string aPath = ResolveAsset("extracted_assets/audio/fart_" + std::to_string(s + 1) + ".wav");
        fartSounds[s] = LoadSound(aPath.c_str());
    }

    Sound sndTap[3];
    sndTap[0] = LoadSound(ResolveAudioFile("fart_1.wav").c_str());
    sndTap[1] = LoadSound(ResolveAudioFile("fart_2.wav").c_str());
    sndTap[2] = LoadSound(ResolveAudioFile("fart_5.wav").c_str());

    Sound sndHold[2];
    sndHold[0] = LoadSound(ResolveAudioFile("freesound_community-fart-3-89567.wav").c_str());
    sndHold[1] = LoadSound(ResolveAudioFile("freesound_community-fart-sound-effect-reed-82976.wav").c_str());

    Sound sndExplode = LoadSound(ResolveAudioFile("boat_explode.wav").c_str());
    Sound sndBounce  = LoadSound(ResolveAudioFile("fart_4.wav").c_str());

    // Load authentic UI textures for Main Menu and Lobby
    Texture2D texLogoSplash = LoadTexture(ResolveAsset("extracted_assets/textures/Logo_Splash.png").c_str());
    Texture2D texMenuBg     = LoadTexture(ResolveAsset("extracted_assets/textures/Background.png").c_str());
    Texture2D texModeFartRun = LoadTexture(ResolveAsset("extracted_assets/textures/Mode_FartRun.png").c_str());
    Texture2D texLobbyChars[4] = {
        LoadTexture(ResolveAsset("extracted_assets/textures/Character_LobbyGreen.png").c_str()),
        LoadTexture(ResolveAsset("extracted_assets/textures/Character_LobbyOrange.png").c_str()),
        LoadTexture(ResolveAsset("extracted_assets/textures/Character_LobbyBlue.png").c_str()),
        LoadTexture(ResolveAsset("extracted_assets/textures/Character_LobbyGrey.png").c_str())
    };
    int selectedLobbyChar = 0;
    float masterVolume = 0.85f;
    SetMasterVolume(masterVolume);

    LoadPhysicsConfigJSON("physics_config.json");

    // World Transforms

    Vector3 trackOffset  = Vector3{ 38.44f, 0.57f, 156.14f };
    // Task 4: Build actual 2D water triangle grid from GM_3D_Demo_Water.obj + trackOffset
    BuildWaterTriangleGrid(waterModel, trackOffset);

    int trackOffLoc = GetShaderLocation(shaderWater, "trackOffset");

    SetShaderValue(shaderWater, trackOffLoc, &trackOffset, SHADER_UNIFORM_VEC3);

    Vector3 startGatePos = Vector3{ 15.51f, 1.07f, 77.51f };

    Vector3 finishGatePos= Vector3{ -439.21f, 7.98f, -103.12f };

    // Raft State (Authentic Boat Physics)

    struct RaftState {

        Vector3 position;

        Vector3 velocity;

        float rotationY;

        float angularVelocityY;

        float pitch;

        float roll;

        float speed;

        float health;

        int deaths;

        int currentCP;

        float invulnerableTimer;

        bool isDestroyed;

        float respawnTimer;

        DeathCause lastDeathCause;
        Vector3 lastCheckpointPos;
        float lastCheckpointRotY;
        bool hasCheckpoint;

    } raft;

    raft.position = Vector3{ 15.55f, 0.58f, 90.80f };

    raft.velocity = Vector3{ 0.0f, 0.0f, 0.0f };

    raft.rotationY = 3.14159f; // Facing downriver towards -Z

    raft.angularVelocityY = 0.0f;

    raft.pitch = 0.0f;

    raft.roll = 0.0f;

    raft.speed = 0.0f;

    raft.health = 100.0f;

    raft.deaths = 0;

    raft.currentCP = 0;

    raft.invulnerableTimer = 0.0f;

    raft.isDestroyed = false;

    raft.respawnTimer = 0.0f;

    raft.lastDeathCause = DEATH_NONE;
    raft.lastCheckpointPos = Vector3{ 15.55f, 0.58f, 90.80f };
    raft.lastCheckpointRotY = 3.14159f;
    raft.hasCheckpoint = false;

    std::vector<LevelEntity> levelEntities;
    if (!LoadLevelLayoutJSON(LEVEL_LAYOUT_PATH, levelEntities)) {
        TraceLog(LOG_WARNING, "Failed to load level_layout.json");
    }

    // Authentic River Predators (Empty in authentic level)
    std::vector<CrocodileAI> crocodiles;
    std::vector<SharkAI> sharks;

    // Ragdoll Pirates

    RagdollPirate ragdollA = { Vector3{ 0,0,0 }, Vector3{ 0,0,0 }, Vector3{ 0,0,0 }, Vector3{ 0,0,0 }, false };

    RagdollPirate ragdollB = { Vector3{ 0,0,0 }, Vector3{ 0,0,0 }, Vector3{ 0,0,0 }, Vector3{ 0,0,0 }, false };

    // Camera

    Camera3D camera = { 0 };

    camera.position = Vector3{ 15.55f, 2.3f, 90.80f + 5.8f };

    camera.target   = Vector3{ 15.55f, 1.1f, 90.80f - 3.5f };

    camera.up = Vector3{ 0.0f, 1.0f, 0.0f };

    camera.fovy = 50.0f;

    camera.projection = CAMERA_PERSPECTIVE;

    CameraController camCtrl;

    camCtrl.yaw = 3.14159f;

    camCtrl.pitch = 0.26f;

    camCtrl.distance = 5.8f;

    camCtrl.targetDist = 5.8f;

    camCtrl.freeLookMode = false;

    camCtrl.autoRecenterTimer = 0.0f;

    camCtrl.shakeTrauma = 0.0f;

    // Authentic Gas States & Rim Seating (P1 & P2)

    PirateGasState gasP1;

    gasP1.Reset();

    PirateGasState gasP2;

    gasP2.Reset();

    // Authentic Rim Seating Angles (in radians)

    // Left Pirate starts at rear-left (-152 deg), Right Pirate starts at rear-right (+152 deg)
    float p1RimAngle = -2.65f;
    float p2RimAngle =  2.65f;

    float raceTimer = 0.0f;
    int totalDeaths = 0;

    // Interactive Level & Obstacle Editor (Unity / Godot style)
    bool singlePirateMode = (LanGetRole() == LAN_OFFLINE);
    AppScreenState currentScreen = SCREEN_MAIN_MENU;
    if (testScreenshotMode || testEditorMode || testDeathMode || testLanMode || useCustomTestCam) {
        currentScreen = SCREEN_GAMEPLAY;
    }
    if (testScreenArg == "menu") {
        currentScreen = SCREEN_MAIN_MENU;
        testScreenshotMode = true;
    } else if (testScreenArg == "lobby") {
        currentScreen = SCREEN_LAN_LOBBY;
        testScreenshotMode = true;
    } else if (testScreenArg == "settings") {
        currentScreen = SCREEN_SETTINGS;
        testScreenshotMode = true;
    }

    bool editorMode = testEditorMode;
    bool editorAnimateObstacles = true; // Natural obstacle motion enabled by default!
    int selectedObs = (testSelectIdx >= 0) ? testSelectIdx : (testEditorMode ? 0 : -1);
    Vector3 editorCamPos = testEditorMode ? Vector3{ 17.2f, 2.8f, 70.8f } : Vector3{ 15.55f, 6.0f, 75.0f };
    float editorCamYaw = 3.14159f;
    float editorCamPitch = 0.35f;
    std::string editorMsg = "";
    float editorMsgTimer = 0.0f;

    bool raceFinished = false;

    int testFrameCount = 0;

    float runTime = 0.0f;

    // Window controls

    Rectangle btnMinRec   = Rectangle{ (float)GetScreenWidth() - 65, 5, 25, 25 };

    Rectangle btnCloseRec = Rectangle{ (float)GetScreenWidth() - 35, 5, 25, 25 };

    // Trigger Boat Destruction & Respawn function

    auto TriggerBoatDeath = [&](DeathCause cause) {
        if (raft.invulnerableTimer > 0.0f || raft.isDestroyed) return;
        raft.isDestroyed = true;
        raft.respawnTimer = 2.0f;
        raft.deaths++;
        totalDeaths++; // Live UI Death counter increment

        raft.lastDeathCause = cause;

        camCtrl.shakeTrauma = 1.0f; // maximum trauma shake

        // Eject Pirates in comedic ragdoll trajectories

        ragdollA.active = true;

        ragdollA.position = raft.position + Vector3{ -0.75f, 0.8f, 0.0f };

        ragdollA.velocity = Vector3{ ((float)rand()/RAND_MAX - 0.5f) * 12.0f, 14.0f, ((float)rand()/RAND_MAX - 0.5f) * 12.0f };

        ragdollA.rotation = Vector3{ 0, 0, 0 };

        ragdollA.rotVelocity = Vector3{ 360.0f, 180.0f, 270.0f };

        ragdollB.active = true;

        ragdollB.position = raft.position + Vector3{ 0.75f, 0.8f, 0.0f };

        ragdollB.velocity = Vector3{ ((float)rand()/RAND_MAX - 0.5f) * 12.0f, 15.0f, ((float)rand()/RAND_MAX - 0.5f) * 12.0f };

        ragdollB.rotation = Vector3{ 0, 0, 0 };

        ragdollB.rotVelocity = Vector3{ -320.0f, 210.0f, 310.0f };

        // Huge fart & water explosion

        EmitFartBurst(raft.position, Vector3{ 0, 1, 0 }, 1.0f, true);
        for (int p = 0; p < MAX_PARTICLES; p++) {
            if (!particles[p].active) {
                particles[p].active = true;
                particles[p].position = raft.position + Vector3{ 0.0f, 1.5f, 0.0f };
                particles[p].velocity = Vector3{ 0.0f, 0.95f, 0.0f };
                particles[p].size = 3.8f;
                particles[p].alpha = 1.0f;
                particles[p].rotation = 0.0f;
                particles[p].rotSpeed = 0.0f;
                particles[p].life = 2.4f;
                particles[p].maxLife = 2.4f;
                particles[p].spriteIndex = 4; // SkullGasCloud.png
                break;
            }
        }
        PlaySound(sndExplode);

    };

    // =========================================================================

    // Game Loop

    // =========================================================================

    while (!WindowShouldClose()) {

        float dt = GetFrameTime();

        if (dt > 0.05f) dt = 0.05f;
        if (testDeathMode && testFrameCount == 20) {
            TriggerBoatDeath(DEATH_OBSTACLE);
        }
        if (testLanMode && testFrameCount == 2) {
            LanInitHost(7777);
        }

        runTime += dt;

                // LAN Networking Hotkeys (F2: Host | F3: Join | F4: Offline)
        if (IsKeyPressed(KEY_F2)) {
            if (LanGetRole() == LAN_HOST) LanShutdown();
            else LanInitHost(7777);
        }
        if (IsKeyPressed(KEY_F3)) {
            if (LanGetRole() == LAN_CLIENT) LanShutdown();
            else LanInitClient(7777);
        }
        if (IsKeyPressed(KEY_F4)) {
            LanShutdown();
        }

        if (!raceFinished && !editorMode) raceTimer += dt;

        Vector2 mousePos = GetMousePosition();

        // 1. Mouse Drag Window

        if (mousePos.y < 38 && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {

            Vector2 delta = GetMouseDelta();

            Vector2 winPos = GetWindowPosition();

            SetWindowPosition((int)(winPos.x + delta.x), (int)(winPos.y + delta.y));

        }

        if (CheckCollisionPointRec(mousePos, btnCloseRec) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) break;

        if (CheckCollisionPointRec(mousePos, btnMinRec) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) MinimizeWindow();

                if (currentScreen == SCREEN_GAMEPLAY) {
// 2. Mouse Camera Orbit Controls

        if (IsKeyPressed(KEY_TAB) || IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE)) {

            camCtrl.freeLookMode = !camCtrl.freeLookMode;

            if (camCtrl.freeLookMode) DisableCursor();

            else EnableCursor();

        }

        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT) || camCtrl.freeLookMode) {

            Vector2 mouseDelta = GetMouseDelta();

            camCtrl.yaw   -= mouseDelta.x * 0.0055f;

            camCtrl.pitch += mouseDelta.y * 0.0055f;

            camCtrl.pitch = Clamp(camCtrl.pitch, -0.10f, 1.30f);

            camCtrl.autoRecenterTimer = 0.0f;

        }

        float wheel = GetMouseWheelMove();

        if (wheel != 0.0f) {

            camCtrl.targetDist -= wheel * 1.5f;

            camCtrl.targetDist = Clamp(camCtrl.targetDist, 5.0f, 22.0f);

        }

        camCtrl.distance = Lerp(camCtrl.distance, camCtrl.targetDist, 10.0f * dt);

        if (!editorMode && IsKeyPressed(KEY_C)) {

            camCtrl.yaw = raft.rotationY;

            camCtrl.pitch = 0.32f;

            camCtrl.targetDist = 9.5f;

            camCtrl.autoRecenterTimer = 0.0f;

        }

        // Camera Shake Trauma Decay

        if (camCtrl.shakeTrauma > 0.0f) {

            camCtrl.shakeTrauma -= dt * 1.8f;

            if (camCtrl.shakeTrauma < 0.0f) camCtrl.shakeTrauma = 0.0f;

        }

        // Update Kinetic Obstacles (Runs both in gameplay and in editor when natural motion is toggled ON!)
        if (!editorMode || editorAnimateObstacles) {
            for (auto& ent : levelEntities) {
                ent.animTimer += dt * ent.animSpeed;
            }
        }

        // TOTAL FREEZE CHECK: If in Level Editor Mode, completely freeze raft, characters and physics!
        if (!editorMode) {
// 3. Player Inputs & Gas Physics

        bool p1InputDown = false;

        bool p1InputReleased = false;

        // P1 Controls: A/D (or Gamepad 0 Left Stick) slides left pirate along rim, W/SPACE (or Gamepad 0 South Face Button) unleashes gas
        if (!raft.isDestroyed) {
            float rimMove1 = 0.0f;
            if (IsKeyDown(KEY_A)) rimMove1 -= 1.0f;
            if (IsKeyDown(KEY_D)) rimMove1 += 1.0f;
            if (IsGamepadAvailable(0)) {
                float gpAxis = GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X);
                if (fabsf(gpAxis) > 0.15f) rimMove1 += gpAxis;
            }
            if (singlePirateMode) {
                // Free 360 degree rotation around the boat rim in Solo Mode!
                p1RimAngle += Clamp(rimMove1, -1.0f, 1.0f) * 2.60f * dt;
                while (p1RimAngle >  PI) p1RimAngle -= 2.0f * PI;
                while (p1RimAngle < -PI) p1RimAngle += 2.0f * PI;
            } else {
                p1RimAngle += Clamp(rimMove1, -1.0f, 1.0f) * 2.44f * dt;
                p1RimAngle = Clamp(p1RimAngle, -3.10f, -0.15f);
            }

            bool gp1Down = IsGamepadAvailable(0) && (IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN) || GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_TRIGGER) > 0.3f);
            bool gp1Rel  = IsGamepadAvailable(0) && IsGamepadButtonReleased(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN);
            p1InputDown     = IsKeyDown(KEY_W) || IsKeyDown(KEY_SPACE) || gp1Down;
            p1InputReleased = IsKeyReleased(KEY_W) || IsKeyReleased(KEY_SPACE) || gp1Rel;
        }

        // Automated sequence for test screenshot
        if (testScreenshotMode) {
            if (testFrameCount >= 20 && testFrameCount < 52) {
                p1InputDown = true;
                p1InputReleased = false;
            } else if (testFrameCount == 52) {
                p1InputDown = false;
                p1InputReleased = true;
            }
        }

        // Tap Burst Detection for P1 (Gas shoots OUTWARD from pirate butt, pushes boat INWARD)
        float rimRadius = 0.94f;
        Vector3 p1OutwardLocal = Vector3{ sinf(p1RimAngle), 0.0f, cosf(p1RimAngle) };
        Vector3 p1ThrustLocal  = Vector3Scale(p1OutwardLocal, -1.0f);
        Matrix boatRotM        = MatrixRotateY(raft.rotationY);
        Vector3 p1ThrustWorld  = Vector3Transform(p1ThrustLocal, boatRotM);
        Vector3 p1OutwardWorld = Vector3Transform(p1OutwardLocal, boatRotM);
        Vector3 p1PosWorld     = Vector3Add(raft.position, Vector3Transform(Vector3Scale(p1OutwardLocal, rimRadius), boatRotM));
        p1PosWorld.y += 0.55f;

        if (p1InputReleased && gasP1.pressTimer <= GAS_TAP_THRESHOLD_SECONDS && gasP1.pressTimer > 0.01f && !gasP1.gasDepleted && !raft.isDestroyed) {
            if (gasP1.gasEnergy >= GAS_TAP_COST) {
                gasP1.gasEnergy -= GAS_TAP_COST;
                float tapDt    = Clamp(gasP1.pressTimer, 0.06f, 0.10f);
                float tapAccel = (gPhysCfg.tapBurstForce / BOAT_MASS) * tapDt;
                raft.velocity  = Vector3Add(raft.velocity, Vector3Scale(p1ThrustWorld, tapAccel));
                float steerTq  = (p1OutwardLocal.x * p1ThrustLocal.z - p1OutwardLocal.z * p1ThrustLocal.x);
                raft.angularVelocityY += steerTq * (gPhysCfg.steerTorque / BOAT_MASS) * 22.0f * tapDt;
                EmitFartBurst(p1PosWorld, p1OutwardWorld, 0.4f);
                PlaySound(sndTap[rand() % 3]);
            }
        }

        gasP1.Update(dt, p1InputDown, runTime);

        // Sustained Thrust Application for P1
        if (gasP1.isSustained && !raft.isDestroyed) {
            float curForce = gPhysCfg.sustainedMinForce + (gPhysCfg.sustainedMaxForce - gPhysCfg.sustainedMinForce) * gasP1.sustainedCharge;
            float accel    = (curForce / BOAT_MASS) * dt;
            raft.velocity  = Vector3Add(raft.velocity, Vector3Scale(p1ThrustWorld, accel));
            float steerTq  = (p1OutwardLocal.x * p1ThrustLocal.z - p1OutwardLocal.z * p1ThrustLocal.x);
            raft.angularVelocityY += steerTq * (gPhysCfg.steerTorque / BOAT_MASS) * 22.0f * dt;
            if (rand() % 4 == 0) {
                EmitFartBurst(p1PosWorld, p1OutwardWorld, gasP1.sustainedCharge);
            }
            if (!IsSoundPlaying(sndHold[0]) && !IsSoundPlaying(sndHold[1])) {
                PlaySound(sndHold[rand() % 2]);
            }
        } else {
            if (IsSoundPlaying(sndHold[0])) StopSound(sndHold[0]);
            if (IsSoundPlaying(sndHold[1])) StopSound(sndHold[1]);
        }

        // Player 2 Independent Controls (Active only in Multiplayer LAN!)
        bool p2InputDown = false;
        if (!singlePirateMode) {
        bool p2InputReleased = false;
        if (!raft.isDestroyed) {
            float p2RimMove = 0.0f;
            if (IsKeyDown(KEY_LEFT))  p2RimMove -= 1.0f;
            if (IsKeyDown(KEY_RIGHT)) p2RimMove += 1.0f;
            if (IsGamepadAvailable(1)) {
                float gp2Axis = GetGamepadAxisMovement(1, GAMEPAD_AXIS_LEFT_X);
                if (fabsf(gp2Axis) > 0.15f) p2RimMove += gp2Axis;
            }
            p2RimAngle += Clamp(p2RimMove, -1.0f, 1.0f) * 2.44f * dt;
            p2RimAngle = Clamp(p2RimAngle, 0.15f, 3.10f);

            bool mouseSafe = (mousePos.y > 55.0f && mousePos.y < (GetScreenHeight() - 55.0f));
            bool gp2Down = IsGamepadAvailable(1) && (IsGamepadButtonDown(1, GAMEPAD_BUTTON_RIGHT_FACE_DOWN) || GetGamepadAxisMovement(1, GAMEPAD_AXIS_RIGHT_TRIGGER) > 0.3f);
            bool gp2Rel  = IsGamepadAvailable(1) && IsGamepadButtonReleased(1, GAMEPAD_BUTTON_RIGHT_FACE_DOWN);

            p2InputDown     = (mouseSafe && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) || IsKeyDown(KEY_UP) || IsKeyDown(KEY_RIGHT_CONTROL) || gp2Down;
            p2InputReleased = (mouseSafe && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) || IsKeyReleased(KEY_UP) || IsKeyReleased(KEY_RIGHT_CONTROL) || gp2Rel;
        }

        if (testScreenshotMode) {
            if (testFrameCount >= 20 && testFrameCount < 52) {
                p2InputDown = true;
                p2InputReleased = false;
            } else if (testFrameCount == 52) {
                p2InputDown = false;
                p2InputReleased = true;
            }
        }

        Vector3 p2OutwardLocal = Vector3{ sinf(p2RimAngle), 0.0f, cosf(p2RimAngle) };
        Vector3 p2ThrustLocal  = Vector3Scale(p2OutwardLocal, -1.0f);
        Vector3 p2ThrustWorld  = Vector3Transform(p2ThrustLocal, boatRotM);
        Vector3 p2OutwardWorld = Vector3Transform(p2OutwardLocal, boatRotM);
        Vector3 p2PosWorld     = Vector3Add(raft.position, Vector3Transform(Vector3Scale(p2OutwardLocal, rimRadius), boatRotM));
        p2PosWorld.y += 0.55f;

        // Tap Burst Detection for P2 (Previously missing!)
        if (p2InputReleased && gasP2.pressTimer <= GAS_TAP_THRESHOLD_SECONDS && gasP2.pressTimer > 0.01f && !gasP2.gasDepleted && !raft.isDestroyed) {
            if (gasP2.gasEnergy >= GAS_TAP_COST) {
                gasP2.gasEnergy -= GAS_TAP_COST;
                float tapDt    = Clamp(gasP2.pressTimer, 0.06f, 0.10f);
                float tapAccel = (gPhysCfg.tapBurstForce / BOAT_MASS) * tapDt;
                raft.velocity  = Vector3Add(raft.velocity, Vector3Scale(p2ThrustWorld, tapAccel));
                float steerTq  = (p2OutwardLocal.x * p2ThrustLocal.z - p2OutwardLocal.z * p2ThrustLocal.x);
                raft.angularVelocityY += steerTq * (gPhysCfg.steerTorque / BOAT_MASS) * 22.0f * tapDt;
                EmitFartBurst(p2PosWorld, p2OutwardWorld, 0.4f);
                PlaySound(sndTap[rand() % 3]);
            }
        }

        gasP2.Update(dt, p2InputDown, runTime);

        // Sustained Thrust Application for P2
        if (gasP2.isSustained && !raft.isDestroyed) {

            float curForce = GAS_MIN_FORCE + (GAS_NORMAL_FORCE - GAS_MIN_FORCE) * gasP2.sustainedCharge;

            float accel = (curForce / BOAT_MASS) * dt * 0.85f;

            raft.velocity = Vector3Add(raft.velocity, Vector3Scale(p2ThrustWorld, accel));

            float steerTq = (p2OutwardLocal.x * p2ThrustLocal.z - p2OutwardLocal.z * p2ThrustLocal.x);

            raft.angularVelocityY += steerTq * (BOAT_STEER_TORQUE / BOAT_MASS) * (curForce / GAS_NORMAL_FORCE) * dt * 22.0f;

            if (rand() % 4 == 0) {

                EmitFartBurst(p2PosWorld, p2OutwardWorld, gasP2.sustainedCharge);

            }

        }

                }

// "FART TOGETHER" Co-op Sync Boost!

        bool isSuperSync = (gasP1.isSustained && gasP2.isSustained && gasP1.sustainedCharge > 0.5f && gasP2.sustainedCharge > 0.5f);

        if (isSuperSync && !raft.isDestroyed) {

            // Torques cancel out, double forward boost!

            Vector3 fwdDir = Vector3{ sinf(raft.rotationY), 0.0f, cosf(raft.rotationY) };

            raft.velocity = Vector3Add(raft.velocity, Vector3Scale(fwdDir, 8.5f * dt));

            camCtrl.shakeTrauma = fmaxf(camCtrl.shakeTrauma, 0.15f);

            if (rand() % 3 == 0) {

                EmitFartBurst(raft.position + Vector3{ 0, 0.4f, 0.9f }, Vector3{ 0,0,-1 }, 1.0f, true);

            }

        }

        // Angular Damping & Rotation update

        raft.angularVelocityY *= (1.0f - BOAT_ANGULAR_DAMPING * dt);

        raft.rotationY += raft.angularVelocityY * dt;

        // 4. Authentic Water Mesh Surface & Spline Hydrodynamics
        float currentSplineT = FindClosestSplineT(raft.position);
        Vector3 riverCenter  = GetSplinePoint(currentSplineT);
        Vector3 riverTangent = GetSplineTangent(currentSplineT);
        Vector3 riverNormal  = Vector3{ riverTangent.z, 0.0f, -riverTangent.x };
        Vector3 toCenter     = Vector3Subtract(raft.position, riverCenter);
        float crossDist      = Vector3DotProduct(toCenter, riverNormal);

        // ZERO automatic sideways or downstream push — boat moves ONLY by pirate gas thrust!
        bool inStartPond  = (raft.position.z >= 75.5f && raft.position.x > -5.0f && raft.position.x < 35.0f);
        bool inStartCanal = (raft.position.z >= 36.5f && raft.position.z < 75.5f && raft.position.x > 10.0f && raft.position.x < 21.0f);
        bool overWater    = true;

        if (inStartPond) {
            Vector3 startPondCenter = { 15.43f, 0.0f, 84.0f };
            float startPondRadius   = 15.5f;
            Vector3 dLake = { raft.position.x - startPondCenter.x, 0.0f, raft.position.z - startPondCenter.z };
            float distLake = Vector3Length(dLake);
            bool nearCanalMouth = (fabsf(raft.position.x - 15.43f) < 2.8f && raft.position.z < 78.5f);
            if (distLake > startPondRadius && !nearCanalMouth) {
                Vector3 nLake = Vector3Normalize(dLake);
                raft.position.x = startPondCenter.x + nLake.x * startPondRadius;
                raft.position.z = startPondCenter.z + nLake.z * startPondRadius;
                float vn = raft.velocity.x * nLake.x + raft.velocity.z * nLake.z;
                if (vn > 0.0f) {
                    raft.velocity.x -= 1.5f * vn * nLake.x;
                    raft.velocity.z -= 1.5f * vn * nLake.z;
                }
            }
        } else if (inStartCanal) {
            // Straight concrete walls of the first canal (from GM_3D_Demo_WallCol.obj: X = 12.65 to 18.21)
            const float canalMinX = 13.62f; // 12.65 + boatRadius(0.97)
            const float canalMaxX = 17.24f; // 18.21 - boatRadius(0.97)
            if (raft.position.x < canalMinX) {
                raft.position.x = canalMinX;
                if (raft.velocity.x < 0.0f) raft.velocity.x *= -0.4f;
            } else if (raft.position.x > canalMaxX) {
                raft.position.x = canalMaxX;
                if (raft.velocity.x > 0.0f) raft.velocity.x *= -0.4f;
            }
        } else {
            // Task 4: Accurate water mesh boundary check — stops buoyancy on dry cliffs / air!
            float dummyY = 0.55f;
            overWater = IsPointOverWaterMesh(raft.position.x, raft.position.z, dummyY);
        }

        // Downstream slope acceleration ONLY on waterfalls/ramps (Z < 30)
        float maxAllowedSpeed = gPhysCfg.boatMaxSpeed;
        if (!inStartPond && !inStartCanal && overWater && !raft.isDestroyed && raft.position.z < 30.0f) {
            float downSlope = Clamp(-riverTangent.y, -0.35f, 0.65f);
            if (downSlope > 0.08f) {
                float streamPush = downSlope * 16.0f;
                raft.velocity.x += riverTangent.x * streamPush * dt;
                raft.velocity.z += riverTangent.z * streamPush * dt;
                maxAllowedSpeed = 22.0f;
            }
        }

        // Buoyancy on Actual Water Mesh vs Free-Fall in Abyss (Plan v6.0 Task 4)
        float actualWaterY = 0.55f;
        float distToWaterMesh = 0.0f;
        Vector3 nearestWaterPt = raft.position;
        QueryActualWaterSurface(raft.position, actualWaterY, distToWaterMesh, nearestWaterPt);

        if (overWater && !raft.isDestroyed) {
            float waveY = actualWaterY + 0.04f + sinf(raft.position.z * 0.32f - runTime * 1.8f) * 0.04f;
            float heave = waveY - raft.position.y;
            raft.velocity.y += (heave * 45.0f - raft.velocity.y * 8.0f) * dt;
        } else if (!overWater) {
            raft.velocity.y -= 32.0f * dt;
            if (raft.position.y < (actualWaterY - 6.5f) && !raft.isDestroyed) {
                TriggerBoatDeath(DEATH_VOID);
            }
        }

        // Linear Damping & Position Integration
        raft.velocity.x *= (1.0f - gPhysCfg.waterLinearDrag * dt);
        raft.velocity.z *= (1.0f - gPhysCfg.waterLinearDrag * dt);
        raft.position = Vector3Add(raft.position, Vector3Scale(raft.velocity, dt));

        // Clamp Horizontal Speed
        float horizSpeed = sqrtf(raft.velocity.x * raft.velocity.x + raft.velocity.z * raft.velocity.z);
        if (horizSpeed > maxAllowedSpeed) {
            float ratio = maxAllowedSpeed / horizSpeed;
            raft.velocity.x *= ratio;
            raft.velocity.z *= ratio;
        }

        // Boat is 100% Flat on Water (Fixes Image 1 Tilt!)
        raft.pitch = Lerp(raft.pitch, 0.0f, 10.0f * dt);
        raft.roll  = Lerp(raft.roll, Clamp(raft.angularVelocityY * 0.025f, -0.08f, 0.08f), 10.0f * dt);
        raft.speed = horizSpeed;

        // 5. Check Custom Collision Shapes (10 types) & Obstacles
        for (const auto& ent : levelEntities) {
            if (ent.type >= ENT_COL_SQUARE && ent.type <= ENT_COL_KILL_ZONE) {
                float rad = -ent.rotationY * DEG2RAD;
                float dx = raft.position.x - ent.position.x;
                float dz = raft.position.z - ent.position.z;
                float dy = raft.position.y - ent.position.y;
                float lx = dx * cosf(rad) - dz * sinf(rad);
                float lz = dx * sinf(rad) + dz * cosf(rad);
                float hx = (ent.sizeX > 0.05f) ? ent.sizeX : ent.radius;
                float hz = (ent.sizeZ > 0.05f) ? ent.sizeZ : ent.radius;
                float hy = (ent.sizeY > 0.05f) ? ent.sizeY : 2.0f;
                const float boatR = 0.88f;

                bool inside = false;
                if (ent.type == ENT_COL_CIRCLE || ent.type == ENT_COL_CYLINDER || ent.type == ENT_COL_SPHERE) {
                    inside = (dx*dx + dz*dz) < ((hx + boatR) * (hx + boatR)) && (fabsf(dy) < (hy + 0.6f));
                } else {
                    inside = (fabsf(lx) < (hx + boatR)) && (fabsf(lz) < (hz + boatR)) && (fabsf(dy) < (hy + 0.6f));
                }

                if (inside) {
                    if (ent.colBehavior == 1 || ent.type == ENT_COL_FALL_ZONE) {
                        overWater = false;
                    } else if (ent.colBehavior == 2 || ent.type == ENT_COL_KILL_ZONE) {
                        if (!raft.isDestroyed && raft.invulnerableTimer <= 0.0f) {
                            TriggerBoatDeath(DEATH_OBSTACLE);
                        }
                    } else if (ent.colBehavior == 3) {
                        Vector3 push = Vector3Normalize(Vector3Subtract(raft.position, ent.position));
                        raft.velocity = Vector3Scale(push, 18.0f);
                        PlaySound(fartSounds[3]);
                    } else { // 0 = Solid Wall
                        float penX = (hx + boatR) - fabsf(lx);
                        float penZ = (hz + boatR) - fabsf(lz);
                        if (penX < penZ) {
                            float sign = (lx >= 0.0f) ? 1.0f : -1.0f;
                            float wpx = sign * penX * cosf(-rad);
                            float wpz = -sign * penX * sinf(-rad);
                            raft.position.x += wpx;
                            raft.position.z += wpz;
                            raft.velocity.x *= -0.3f;
                        } else {
                            float sign = (lz >= 0.0f) ? 1.0f : -1.0f;
                            float wpx = sign * penZ * sinf(-rad);
                            float wpz = sign * penZ * cosf(-rad);
                            raft.position.x += wpx;
                            raft.position.z += wpz;
                            raft.velocity.z *= -0.3f;
                        }
                    }
                }
            }
        }

        // 5. Check Obstacle & Hazard Collisions (Rocks & F18 Rebound are Non-Fatal!)
        for (auto& ent : levelEntities) {
            Vector3 animPos;
            float animYaw, animPitch, animRoll;
            GetEntityAnimatedTransform(ent, animPos, animYaw, animPitch, animRoll);

            if (ent.type == ENT_ROCK_1 || ent.type == ENT_ROCK_2) {
                // الصخور: تصادم صلب منزلق فقط بدون انفجار القارب!
                float rockR = 0.65f * ent.scale + 1.05f;
                float dx = raft.position.x - ent.position.x;
                float dz = raft.position.z - ent.position.z;
                float dXZ = sqrtf(dx * dx + dz * dz);
                if (dXZ < rockR && dXZ > 0.001f && fabsf(raft.position.y - ent.position.y) < 2.5f) {
                    float nx = dx / dXZ, nz = dz / dXZ;
                    raft.position.x = ent.position.x + nx * rockR;
                    raft.position.z = ent.position.z + nz * rockR;
                    float vn = raft.velocity.x * nx + raft.velocity.z * nz;
                    if (vn < 0.0f) {
                        raft.velocity.x -= vn * nx;
                        raft.velocity.z -= vn * nz;
                    }
                }
            } else if (ent.type == ENT_OBS_F18) {
                // الصورة 5 (OBS_F18_SPIKE): عمود ارتدادي يدفع القارب عكس اتجاه قدومه وبحسب سرعة ارتطامه!
                float postR = 0.45f * ent.scale + 1.05f;
                float dx = raft.position.x - ent.position.x;
                float dz = raft.position.z - ent.position.z;
                float dXZ = sqrtf(dx * dx + dz * dz);
                if (dXZ < postR && dXZ > 0.001f && fabsf(raft.position.y - ent.position.y) < 3.0f && !raft.isDestroyed) {
                    Vector3 radialOut = { dx / dXZ, 0.0f, dz / dXZ };
                    Vector3 horizVel  = { raft.velocity.x, 0.0f, raft.velocity.z };
                    float impactSpeed = Vector3Length(horizVel);
                    Vector3 reverseDir = (impactSpeed > 0.5f)
                        ? Vector3Normalize(Vector3Add(Vector3Scale(Vector3Normalize(horizVel), -0.7f), Vector3Scale(radialOut, 0.3f)))
                        : radialOut;
                    raft.position.x = ent.position.x + radialOut.x * (postR + 0.08f);
                    raft.position.z = ent.position.z + radialOut.z * (postR + 0.08f);
                    float pushSpeed = Clamp(impactSpeed * 1.65f, 7.5f, 22.0f);
                    raft.velocity.x = reverseDir.x * pushSpeed;
                    raft.velocity.z = reverseDir.z * pushSpeed;
                    camCtrl.shakeTrauma = fmaxf(camCtrl.shakeTrauma, Clamp(impactSpeed / 14.0f, 0.25f, 0.65f));
                    PlaySound(sndBounce);
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
            } else if (CheckObstacleOBBCollision(raft.position, ent, animPos, animYaw, animRoll)) {
                if (!raft.isDestroyed) {
                    TriggerBoatDeath(DEATH_OBSTACLE);
                }
            }
        }

        // 6. Update Crocodile Predator AI (Spline-aware water surface elevation)
        for (auto& croc : crocodiles) {
            if (croc.biteCooldownTimer > 0.0f) croc.biteCooldownTimer -= dt;
float crocWaterY = 0.55f, crocDist = 0.0f;
            Vector3 crocPt = croc.position;
            QueryActualWaterSurface(croc.position, crocWaterY, crocDist, crocPt);
            croc.position.y = crocWaterY - 0.20f;

            float dx = raft.position.x - croc.position.x;
            float dz = raft.position.z - croc.position.z;
            float distToRaft = sqrtf(dx * dx + dz * dz);

            if (distToRaft < CROC_DETECT_RANGE && !raft.isDestroyed) {
                croc.isChasing = true;
                Vector3 toRaft = Vector3Normalize(Vector3{ dx, 0.0f, dz });
                croc.position = Vector3Add(croc.position, Vector3Scale(toRaft, croc.lungeSpeed * dt));
                croc.position.y = crocWaterY;
                croc.rotationY = atan2f(toRaft.x, toRaft.z);

                if (distToRaft < CROC_BITE_RANGE && croc.biteCooldownTimer <= 0.0f && raft.invulnerableTimer <= 0.0f) {
                    croc.biteCooldownTimer = CROC_BITE_COOLDOWN;
                    raft.health -= 40.0f;
                    camCtrl.shakeTrauma = 0.7f;
                    PlaySound(fartSounds[1]);
                    EmitFartBurst(croc.position, toRaft, 0.9f);
                    if (raft.health <= 0.0f) {
                        TriggerBoatDeath(DEATH_CROCODILE);
                    }
                }
            } else {
                croc.isChasing = false;
                Vector3 toStart = Vector3{ croc.startPosition.x - croc.position.x, 0.0f, croc.startPosition.z - croc.position.z };
                if (Vector3Length(toStart) > 0.5f) {
                    Vector3 dir = Vector3Normalize(toStart);
                    croc.position = Vector3Add(croc.position, Vector3Scale(dir, 1.5f * dt));
                    croc.position.y = crocWaterY;
                    croc.rotationY = atan2f(dir.x, dir.z);
                }
            }
        }

        // 7. Update Shark Predator AI (Spline-aware water surface elevation)
        for (auto& shark : sharks) {
            if (shark.biteCooldownTimer > 0.0f) shark.biteCooldownTimer -= dt;
float sharkWaterY = 0.55f, sharkDist = 0.0f;
            Vector3 sharkPt = shark.position;
            QueryActualWaterSurface(shark.position, sharkWaterY, sharkDist, sharkPt);
            shark.position.y = sharkWaterY - 0.22f;

            float dx = raft.position.x - shark.position.x;
            float dz = raft.position.z - shark.position.z;
            float distToRaft = sqrtf(dx * dx + dz * dz);

            if (distToRaft < SHARK_VIEW_LENGTH && !raft.isDestroyed) {
                shark.isChasing = true;
                Vector3 toRaft = Vector3Normalize(Vector3{ dx, 0.0f, dz });
                shark.position = Vector3Add(shark.position, Vector3Scale(toRaft, SHARK_CHASE_SPEED * dt));
                shark.position.y = sharkWaterY;
                shark.rotationY = atan2f(toRaft.x, toRaft.z);

                if (distToRaft < SHARK_BITE_RANGE && shark.biteCooldownTimer <= 0.0f && raft.invulnerableTimer <= 0.0f) {
                    shark.biteCooldownTimer = SHARK_BITE_COOLDOWN;
                    raft.health -= 50.0f;
                    camCtrl.shakeTrauma = 0.8f;
                    PlaySound(fartSounds[2]);
                    EmitFartBurst(shark.position, toRaft, 1.0f);
                    if (raft.health <= 0.0f) {
                        TriggerBoatDeath(DEATH_SHARK);
                    }
                }
            } else {
                shark.isChasing = false;
                shark.patrolAngle += (SHARK_PATROL_SPEED / SHARK_WANDER_RADIUS) * dt;
                Vector3 targetPatrol = shark.centerPoint + Vector3{
                    cosf(shark.patrolAngle) * SHARK_WANDER_RADIUS * 0.4f,
                    0.0f,
                    sinf(shark.patrolAngle) * SHARK_WANDER_RADIUS * 0.4f
                };
                Vector3 toPatrol = Vector3Normalize(Vector3{ targetPatrol.x - shark.position.x, 0.0f, targetPatrol.z - shark.position.z });
                shark.position = Vector3Add(shark.position, Vector3Scale(toPatrol, SHARK_PATROL_SPEED * dt));
                shark.position.y = sharkWaterY;
                shark.rotationY = atan2f(toPatrol.x, toPatrol.z);
            }
        }

        // 8. Respawn System & Ragdoll Updates

        if (raft.isDestroyed) {

            raft.respawnTimer -= dt;

            // Ragdoll physics

            if (ragdollA.active) {

                ragdollA.velocity.y -= GRAVITY * 2.0f * dt;

                ragdollA.position = Vector3Add(ragdollA.position, Vector3Scale(ragdollA.velocity, dt));

                ragdollA.rotation = Vector3Add(ragdollA.rotation, Vector3Scale(ragdollA.rotVelocity, dt));

            }

            if (ragdollB.active) {

                ragdollB.velocity.y -= GRAVITY * 2.0f * dt;

                ragdollB.position = Vector3Add(ragdollB.position, Vector3Scale(ragdollB.velocity, dt));

                ragdollB.rotation = Vector3Add(ragdollB.rotation, Vector3Scale(ragdollB.rotVelocity, dt));

            }

            if (raft.respawnTimer <= 0.0f) {
                // Task 2: Respawn directly at the unified editable checkpoint entity!
                if (raft.hasCheckpoint) {
                    raft.position = raft.lastCheckpointPos;
                    raft.rotationY = raft.lastCheckpointRotY;
                } else {
                    raft.position = Vector3{ 15.55f, 0.58f, 90.80f };
                    raft.rotationY = 3.14159f;
                }
                raft.velocity = Vector3{ 0.0f, 0.0f, 0.0f };
                raft.angularVelocityY = 0.0f;
                raft.pitch = 0.0f;
                raft.roll = 0.0f;
                raft.health = 100.0f;
                raft.invulnerableTimer = 2.0f;
                raft.isDestroyed = false;
                ragdollA.active = false;
                ragdollB.active = false;
                gasP1.Reset();
                gasP2.Reset();
                camCtrl.yaw = raft.rotationY;
            }
        }

        if (raft.invulnerableTimer > 0.0f) raft.invulnerableTimer -= dt;

        // Checkpoints & Finish Check (Plan v6.0 Task 2)
        if (!editorMode && !raft.isDestroyed) {
            for (auto& ent : levelEntities) {
                if (ent.type == ENT_CHECKPOINT) {
                    float dist = Vector3Distance(raft.position, ent.position);
                    if (dist < 6.5f * ent.scale && !ent.cpPassed) {
                        ent.cpPassed = true;
                        raft.lastCheckpointPos  = Vector3{ ent.position.x, ent.position.y + 0.25f, ent.position.z };
                        float cpSplineT = FindClosestSplineT(ent.position);
                        Vector3 cpTan   = GetSplineTangent(cpSplineT);
                        raft.lastCheckpointRotY = atan2f(cpTan.x, cpTan.z);
                        raft.hasCheckpoint      = true;
                        raft.currentCP++;
                        PlaySound(fartSounds[0]);
                    }
                }
            }
            if (Vector3Distance(raft.position, finishGatePos) < 12.0f && !raceFinished) {
                raceFinished = true;
                PlaySound(fartSounds[0]);
            }
        }

        // Particle Updates

        for (int p = 0; p < MAX_PARTICLES; p++) {

            if (particles[p].active) {

                particles[p].life += dt;

                if (particles[p].life >= particles[p].maxLife) {

                    particles[p].active = false;

                } else {

                    particles[p].position = Vector3Add(particles[p].position, Vector3Scale(particles[p].velocity, dt));

                    particles[p].rotation += particles[p].rotSpeed * dt;

                    particles[p].alpha = 1.0f - (particles[p].life / particles[p].maxLife);

                    particles[p].size += dt * 0.6f;

                }

            }

        }

        
        } else {
            // TOTAL FREEZE OF GAMEPLAY & MOVEMENT DURING LEVEL EDITOR MODE
            raft.velocity = Vector3Zero();
            raft.angularVelocityY = 0.0f;
            gasP1.isSustained = false;
            gasP1.pressTimer = 0.0f;
            gasP1.trembleOffset = 0.0f;
            gasP1.redness = 0.0f;
            gasP1.boneScale = 1.0f;

            gasP2.isSustained = false;
            gasP2.pressTimer = 0.0f;
            gasP2.trembleOffset = 0.0f;
            gasP2.redness = 0.0f;
            gasP2.boneScale = 1.0f;
        }
// Test Camera Overrides

        if (testScreenshotMode) {

            if (testAngleMode == 1) { // Side Profile

                camCtrl.yaw = raft.rotationY + 1.45f;

                camCtrl.pitch = 0.22f;

                camCtrl.distance = 5.2f;

            } else if (testAngleMode == 2) { // Front Facing

                camCtrl.yaw = raft.rotationY + 3.14159f;

                camCtrl.pitch = 0.18f;

                camCtrl.distance = 5.6f;

            } else if (testAngleMode == 3) { // Authentic Top-Down Lake View (Image 4 Parity)

                camCtrl.yaw = 0.0f;

                camCtrl.pitch = 1.48f;

                camCtrl.distance = 32.0f;

            }

        }

        if (editorMode) {
            // Free-fly Editor Camera (Unity / Godot style)
            if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
                Vector2 mDelta = GetMouseDelta();
                editorCamYaw -= mDelta.x * 0.003f;
                editorCamPitch -= mDelta.y * 0.003f;
                editorCamPitch = Clamp(editorCamPitch, -1.4f, 1.4f);
            }

            Vector3 fwd = { -sinf(editorCamYaw) * cosf(editorCamPitch), sinf(editorCamPitch), -cosf(editorCamYaw) * cosf(editorCamPitch) };
            Vector3 right = { cosf(editorCamYaw), 0.0f, -sinf(editorCamYaw) };
            float flySpd = (IsKeyDown(KEY_LEFT_SHIFT) ? 28.0f : 12.0f) * dt;

            if (IsKeyDown(KEY_W)) editorCamPos = Vector3Add(editorCamPos, Vector3Scale(fwd, flySpd));
            if (IsKeyDown(KEY_S)) editorCamPos = Vector3Subtract(editorCamPos, Vector3Scale(fwd, flySpd));
            if (IsKeyDown(KEY_D)) editorCamPos = Vector3Add(editorCamPos, Vector3Scale(right, flySpd));
            if (IsKeyDown(KEY_A)) editorCamPos = Vector3Subtract(editorCamPos, Vector3Scale(right, flySpd));
            if (IsKeyDown(KEY_SPACE) || IsKeyDown(KEY_E)) editorCamPos.y += flySpd;
            if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_Q) || IsKeyDown(KEY_C)) editorCamPos.y -= flySpd;

            camera.position = editorCamPos;
            camera.target = Vector3Add(editorCamPos, fwd);

            // Compute panel bounds to prevent clicking through UI
            float leftDockW = leftDockCollapsed ? 0.0f : 270.0f;
            float rightDockW = inspectorCollapsed ? 0.0f : 320.0f;
            float rightDockX = (float)GetScreenWidth() - rightDockW;
            bool mouseOverUI = (mousePos.y < 44.0f) || (mousePos.x < leftDockW) || (mousePos.x > rightDockX);
            if (visualCatalogOpen && mousePos.x >= (leftDockW + 10.0f) && mousePos.x <= (leftDockW + 250.0f) && mousePos.y >= 44.0f) {
                mouseOverUI = true;
            }

            // 3D Gizmo and Raycast Selection
            if (selectedObs >= 0 && selectedObs < (int)levelEntities.size()) {
                LevelEntity& sel = levelEntities[selectedObs];
                Vector3 oPos = sel.position;
                Vector2 sOrig  = GetWorldToScreen(oPos, camera);
                Vector2 sXEnd  = GetWorldToScreen(Vector3Add(oPos, Vector3{ 3.2f, 0.0f, 0.0f }), camera);
                Vector2 sYEnd  = GetWorldToScreen(Vector3Add(oPos, Vector3{ 0.0f, 3.2f, 0.0f }), camera);
                Vector2 sZEnd  = GetWorldToScreen(Vector3Add(oPos, Vector3{ 0.0f, 0.0f, 3.2f }), camera);

                Vector2 sFlipX = GetWorldToScreen(Vector3Add(oPos, Vector3{ -1.8f, 0.0f, 0.0f }), camera);
                Vector2 sFlipY = GetWorldToScreen(Vector3Add(oPos, Vector3{ 0.0f, -1.8f, 0.0f }), camera);
                Vector2 sFlipZ = GetWorldToScreen(Vector3Add(oPos, Vector3{ 0.0f, 0.0f, -1.8f }), camera);

                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !mouseOverUI) {
                    if (Vector2Distance(mousePos, sFlipX) < 18.0f) {
                        sel.flipX *= -1;
                        sel.rotationY = fmodf(-sel.rotationY + 360.0f, 360.0f);
                        editorMsg = "Flipped Selected Object along X Axis!";
                        editorMsgTimer = 2.0f;
                    } else if (Vector2Distance(mousePos, sFlipY) < 18.0f) {
                        sel.flipY *= -1;
                        sel.rotationZ = fmodf(sel.rotationZ + 180.0f, 360.0f);
                        editorMsg = "Flipped Selected Object along Y Axis!";
                        editorMsgTimer = 2.0f;
                    } else if (Vector2Distance(mousePos, sFlipZ) < 18.0f) {
                        sel.flipZ *= -1;
                        sel.rotationY = fmodf(sel.rotationY + 180.0f, 360.0f);
                        editorMsg = "Flipped Selected Object along Z Axis!";
                        editorMsgTimer = 2.0f;
                    } else {
                        float dX = DistPointToSegment2D(mousePos, sOrig, sXEnd);
                        float dY = DistPointToSegment2D(mousePos, sOrig, sYEnd);
                        float dZ = DistPointToSegment2D(mousePos, sOrig, sZEnd);
                        float minD = fminf(dX, fminf(dY, dZ));
                        if (minD < 20.0f) {
                            if (minD == dX) gActiveGizmoAxis = 0;
                            else if (minD == dY) gActiveGizmoAxis = 1;
                            else gActiveGizmoAxis = 2;
                        }
                    }
                }

                if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
                    gActiveGizmoAxis = -1;
                }

                if (gActiveGizmoAxis >= 0 && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
                    Vector2 mDelta = GetMouseDelta();
                    Vector2 axisEnd = (gActiveGizmoAxis == 0) ? sXEnd : ((gActiveGizmoAxis == 1) ? sYEnd : sZEnd);
                    Vector2 screenDir = { axisEnd.x - sOrig.x, axisEnd.y - sOrig.y };
                    float screenLen = Vector2Length(screenDir);
                    if (screenLen > 1.0f) {
                        screenDir.x /= screenLen;
                        screenDir.y /= screenLen;
                        float pixelsMoved = mDelta.x * screenDir.x + mDelta.y * screenDir.y;
                        float worldPerPixel = 3.2f / screenLen;
                        float deltaWorld = pixelsMoved * worldPerPixel;
                        if (gActiveGizmoAxis == 0) sel.position.x += deltaWorld;
                        if (gActiveGizmoAxis == 1) sel.position.y += deltaWorld;
                        if (gActiveGizmoAxis == 2) sel.position.z += deltaWorld;
                    }
                }
            }

            // Raycast click selection when not dragging gizmo
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !mouseOverUI && gActiveGizmoAxis == -1) {
                Ray mRay = GetScreenToWorldRay(mousePos, camera);
                float bestDist = 9999.0f;
                int hitIdx = -1;
                for (size_t i = 0; i < levelEntities.size(); i++) {
                    float r = (levelEntities[i].radius > 0.5f) ? levelEntities[i].radius : 1.5f;
                    BoundingBox oBox = { Vector3Subtract(levelEntities[i].position, Vector3{ r, 2.0f, r }), Vector3Add(levelEntities[i].position, Vector3{ r, 2.0f, r }) };
                    RayCollision rHit = GetRayCollisionBox(mRay, oBox);
                    if (rHit.hit && rHit.distance < bestDist) {
                        bestDist = rHit.distance;
                        hitIdx = (int)i;
                    }
                }
                if (hitIdx != -1) {
                    selectedObs = hitIdx;
                    editorMsg = TextFormat("Selected: #%d (%s)", hitIdx, ENTITY_TYPE_NAMES[levelEntities[hitIdx].type]);
                    editorMsgTimer = 2.0f;
                }
            }

            // Keyboard Manipulation of Selected Entity
            if (selectedObs >= 0 && selectedObs < (int)levelEntities.size()) {
                auto& o = levelEntities[selectedObs];
                float editSpd = (IsKeyDown(KEY_LEFT_SHIFT) ? 10.0f : 4.0f) * dt;
                
                if (IsKeyDown(KEY_J)) o.position.x -= editSpd;
                if (IsKeyDown(KEY_L)) o.position.x += editSpd;
                if (IsKeyDown(KEY_I)) o.position.z += editSpd;
                if (IsKeyDown(KEY_K)) o.position.z -= editSpd;
                if (IsKeyDown(KEY_U)) o.position.y += editSpd;
                if (IsKeyDown(KEY_O)) o.position.y -= editSpd;
                if (IsKeyDown(KEY_R)) o.rotationY -= 65.0f * dt;
                if (IsKeyDown(KEY_T)) o.rotationY += 65.0f * dt;

                if (IsKeyPressed(KEY_F)) {
                    editorCamPos = Vector3Add(o.position, Vector3{ 0.0f, 3.5f, 6.0f });
                    editorCamYaw = 3.14159f;
                    editorCamPitch = 0.35f;
                    editorMsg = "Focused Camera on Selection (F)";
                    editorMsgTimer = 1.5f;
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
        }

        // Auto Recenter

        if (!IsMouseButtonDown(MOUSE_BUTTON_RIGHT) && !camCtrl.freeLookMode) {

            camCtrl.autoRecenterTimer += dt;

            if (camCtrl.autoRecenterTimer > 3.0f && raft.speed > 2.0f) {

                float targetYaw = atan2f(-raft.velocity.x, -raft.velocity.z);

                float diff = targetYaw - camCtrl.yaw;

                while (diff >  PI) diff -= 2.0f * PI;

                while (diff < -PI) diff += 2.0f * PI;

                camCtrl.yaw += diff * 1.5f * dt;

            }

        }

        if (useCustomTestCam) {
            camera.position = customTestCamPos;
            camera.target   = customTestCamTarget;
            camera.up       = Vector3{ 0.0f, 1.0f, 0.0f };
            camera.fovy     = 55.0f;
        }

        // Pass uniforms to Water Shader

        SetShaderValue(shaderWater, uTimeLoc, &runTime, SHADER_UNIFORM_FLOAT);

        SetShaderValue(shaderWater, camPosLoc, &camera.position, SHADER_UNIFORM_VEC3);

        // =====================================================================

        // Rendering: 3D Scene + Authentic Assets

        // =====================================================================

                // Update Live 3D Model Preview for Godot Level Editor
        if (editorMode && !inspectorCollapsed) {
            EntityType pType = (selectedObs >= 0 && selectedObs < (int)levelEntities.size())
                ? levelEntities[selectedObs].type : ENT_OBS_A1;
            Model* pModel = (selectedObs >= 0 && selectedObs < (int)levelEntities.size() && levelEntities[selectedObs].model)
                ? levelEntities[selectedObs].model : &gObsModels.mA1;

            BeginTextureMode(previewRT);
                ClearBackground(Color{ 18, 22, 30, 255 });
                Camera3D pCam = { 0 };
                pCam.position = Vector3{ 3.6f, 2.6f, 3.6f };
                pCam.target   = Vector3{ 0.0f, 0.4f, 0.0f };
                pCam.up       = Vector3{ 0.0f, 1.0f, 0.0f };
                pCam.fovy     = 42.0f;
                pCam.projection = CAMERA_PERSPECTIVE;
                BeginMode3D(pCam);
                    DrawGrid(6, 1.0f);
                    if (pModel) {
                        int isTree = (pType == ENT_TREE_JUNGLE) ? 1 : 0;
                        SetShaderValue(shaderLighting, uIsJungleTreeLoc, &isTree, SHADER_UNIFORM_INT);
                        rlPushMatrix();
                            rlRotatef(runTime * 45.0f, 0, 1, 0);
                            float sc = (pType == ENT_OBS_F10 || pType == ENT_OBS_B5 || pType == ENT_OBS_C5) ? 0.32f : 0.65f;
                            rlScalef(sc, sc, sc);
                            DrawModel(*pModel, Vector3{ 0, 0, 0 }, 1.0f, WHITE);
                        rlPopMatrix();
                        if (isTree) {
                            int zero = 0;
                            SetShaderValue(shaderLighting, uIsJungleTreeLoc, &zero, SHADER_UNIFORM_INT);
                        }
                    }
                EndMode3D();
            EndTextureMode();
        }

                }

                auto DrawMenuBtn = [&](Rectangle rec, const char* label, Color bgCol, Color textCol, int fontSize = 16, bool bold = true) -> bool {
            bool hov = CheckCollisionPointRec(mousePos, rec);
            Color fill = hov ? ColorBrightness(bgCol, 0.22f) : bgCol;
            DrawRectangleRounded(rec, 0.25f, 4, fill);
            DrawRectangleRoundedLinesEx(rec, 0.25f, 4, 1.5f, hov ? WHITE : ColorBrightness(bgCol, 0.45f));
            Font f = bold ? fontUIBold : fontUI;
            Vector2 sz = MeasureTextEx(f, label, (float)fontSize, 0.4f);
            DrawTextEx(f, label, Vector2{ rec.x + (rec.width - sz.x) * 0.5f, rec.y + (rec.height - sz.y) * 0.5f }, (float)fontSize, 0.4f, textCol);
            return hov && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
        };

        BeginDrawing();

        if (currentScreen == SCREEN_GAMEPLAY) {
        // 1. Authentic Cartoon Sky Backdrop
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

        BeginMode3D(camera);

            // A. Environment Foliage & Cliffs

            float zeroRed = 0.0f;

            SetShaderValue(shaderLighting, uRednessLoc, &zeroRed, SHADER_UNIFORM_FLOAT);

            for (int s = 0; s < ENV_SECTOR_COUNT; s++) {
                DrawModel(envSectors[s], Vector3{ 0, 0, 0 }, 1.0f, WHITE);
            }

            // B. Authentic Canyon Walls

            DrawModel(wallModel, trackOffset, 1.0f, WHITE);

            // C. Authentic Water Surface

            DrawModel(waterModel, trackOffset, 1.0f, WHITE);

            // D. Start Gate & Finish Gate (Intermediate checkpoints rendered from levelEntities!)
            rlPushMatrix();
                rlTranslatef(startGatePos.x, startGatePos.y, startGatePos.z);
                rlRotatef(0.0f, 0, 1, 0);
                DrawModel(startModel, Vector3{ 0, 0, 0 }, 1.0f, WHITE);
            rlPopMatrix();

            rlPushMatrix();
                rlTranslatef(finishGatePos.x, finishGatePos.y, finishGatePos.z);
                rlRotatef(90.0f, 0, 1, 0);
                DrawModel(finishModel, Vector3{ 0, 0, 0 }, 1.0f, raceFinished ? COLOR_ACCENT_GREEN : WHITE);
            rlPopMatrix();

            // A. Authentic Start Basin inside Canyon Walls & Water Mesh (Zero fake cylinders)

            // E. Draw All Level Entities (Obstacles, Rocks, Trees, Checkpoints)
        for (const auto& ent : levelEntities) {
            Vector3 animPos;
            float animYaw, animRoll;
            float animPitch = 0.0f;
            GetEntityAnimatedTransform(ent, animPos, animYaw, animPitch, animRoll);

            // Task 3: ENT_OBS_F10 split rendering with hammer ALWAYS pointing down towards ground!
            if (ent.type == ENT_OBS_F10) {
                rlPushMatrix();
                rlTranslatef(animPos.x, animPos.y, animPos.z);
                rlRotatef(animYaw, 0.0f, 1.0f, 0.0f);
                float s = (ent.scale > 0.01f) ? ent.scale : 1.0f;
                float sx = s * (float)ent.flipX;
                float sy = s * (float)ent.flipY;
                float sz = s * (float)ent.flipZ;
                rlScalef(sx, sy, sz);

                // 1. Draw arms rotating by animRoll around pivot (0,0,0)
                rlPushMatrix();
                    rlRotatef(animRoll, 0.0f, 0.0f, 1.0f);
                    DrawModel(gObsModels.mF10_Arms, Vector3{ 0, 0, 0 }, 1.0f, WHITE);
                rlPopMatrix();

                // 2. Draw hammer hanging from wrist (-7.23, 2.20) with roll = 0.0f (always pointing down towards ground)
                float rad = animRoll * DEG2RAD;
                const float wristX = -7.23f, wristY = 2.20f;
                float newWristX = wristX * cosf(rad) - wristY * sinf(rad);
                float newWristY = wristX * sinf(rad) + wristY * cosf(rad);
                rlPushMatrix();
                    rlTranslatef(newWristX - wristX, newWristY - wristY, 0.0f);
                    DrawModel(gObsModels.mF10_Hammer, Vector3{ 0, 0, 0 }, 1.0f, WHITE);
                rlPopMatrix();

                rlPopMatrix();
                continue;
            }

            // Task 2: Checkpoint gate rendering with active green tint
            if (ent.type == ENT_CHECKPOINT) {
                Color tint = ent.cpPassed ? COLOR_ACCENT_GREEN : WHITE;
                rlPushMatrix();
                rlTranslatef(animPos.x, animPos.y, animPos.z);
                rlRotatef(animYaw, 0.0f, 1.0f, 0.0f);
                float s = (ent.scale > 0.01f) ? ent.scale : 1.0f;
                rlScalef(s * (float)ent.flipX, s * (float)ent.flipY, s * (float)ent.flipZ);
                DrawModel(gObsModels.mCP, Vector3{ 0, 0, 0 }, 1.0f, tint);
                rlPopMatrix();
                continue;
            }

            // Task 4: Custom Collision Shapes rendering in editor mode
            if (ent.type >= ENT_COL_SQUARE && ent.type <= ENT_COL_KILL_ZONE) {
                if (editorMode) {
                    Color colTint = Color{ 0, 180, 255, 140 }; // Solid Wall
                    if (ent.colBehavior == 1 || ent.type == ENT_COL_FALL_ZONE) colTint = Color{ 255, 160, 0, 140 }; // Fall Zone
                    else if (ent.colBehavior == 2 || ent.type == ENT_COL_KILL_ZONE) colTint = Color{ 240, 40, 40, 160 }; // Kill Zone
                    else if (ent.colBehavior == 3) colTint = Color{ 220, 50, 220, 150 }; // Bounce Pad

                    rlPushMatrix();
                    rlTranslatef(animPos.x, animPos.y, animPos.z);
                    rlRotatef(animYaw, 0.0f, 1.0f, 0.0f);
                    float hx = (ent.sizeX > 0.05f) ? ent.sizeX : ent.radius;
                    float hz = (ent.sizeZ > 0.05f) ? ent.sizeZ : ent.radius;
                    float hy = (ent.sizeY > 0.05f) ? ent.sizeY : 2.0f;
                    if (ent.type == ENT_COL_CIRCLE || ent.type == ENT_COL_CYLINDER) {
                        DrawCylinderWires(Vector3{ 0, 0, 0 }, hx, hx, hy * 2.0f, 16, colTint);
                    } else if (ent.type == ENT_COL_SPHERE) {
                        DrawSphereWires(Vector3{ 0, 0, 0 }, hx, 12, 12, colTint);
                    } else {
                        DrawCubeWires(Vector3{ 0, 0, 0 }, hx * 2.0f, hy * 2.0f, hz * 2.0f, colTint);
                    }
                    rlPopMatrix();
                }
                continue;
            }

            if (ent.model != nullptr) {
                rlPushMatrix();
                rlTranslatef(animPos.x, animPos.y, animPos.z);
                rlRotatef(animYaw, 0.0f, 1.0f, 0.0f);
                if (fabsf(animPitch) > 0.01f) rlRotatef(animPitch, 1.0f, 0.0f, 0.0f);
                if (fabsf(animRoll)  > 0.01f) rlRotatef(animRoll,  0.0f, 0.0f, 1.0f);
                float s = (ent.scale > 0.01f) ? ent.scale : 1.0f;
                rlScalef(s * (float)ent.flipX, s * (float)ent.flipY, s * (float)ent.flipZ);
                int isTreeFlag = (ent.type == ENT_TREE_JUNGLE) ? 1 : 0;
                SetShaderValue(shaderLighting, uIsJungleTreeLoc, &isTreeFlag, SHADER_UNIFORM_INT);
                DrawModel(*ent.model, Vector3{ 0, 0, 0 }, 1.0f, WHITE);
                if (isTreeFlag) {
                    int zeroFlag = 0;
                    SetShaderValue(shaderLighting, uIsJungleTreeLoc, &zeroFlag, SHADER_UNIFORM_INT);
                }
                rlPopMatrix();
            }
        }

        // F. River Predators (Crocodiles & Sharks)

            for (const auto& croc : crocodiles) {

                rlPushMatrix();

                    rlTranslatef(croc.position.x, croc.position.y, croc.position.z);

                    rlRotatef(croc.rotationY * RAD2DEG, 0, 1, 0);

                    rlScalef(1.35f, 1.35f, 1.35f);

                    DrawModel(crocModel, Vector3{ 0, 0, 0 }, 1.0f, WHITE);

                rlPopMatrix();

            }

            for (const auto& shark : sharks) {

                rlPushMatrix();

                    rlTranslatef(shark.position.x, shark.position.y, shark.position.z);

                    rlRotatef(shark.rotationY * RAD2DEG, 0, 1, 0);

                    rlScalef(1.20f, 1.20f, 1.20f);

                    DrawModel(sharkModel, Vector3{ 0, 0, 0 }, 1.0f, WHITE);

                rlPopMatrix();

            }

            // G. The Shared Raft & Crew

            bool flashInvuln = (raft.invulnerableTimer > 0.0f && ((int)(runTime * 14.0f) % 2 == 0));

            if (!raft.isDestroyed && !flashInvuln) {

                rlPushMatrix();

                    rlTranslatef(raft.position.x, raft.position.y + 0.15f, raft.position.z);

                    rlRotatef(raft.rotationY * RAD2DEG, 0, 1, 0);

                    rlRotatef(raft.pitch * RAD2DEG, 1, 0, 0);

                    rlRotatef(raft.roll * RAD2DEG, 0, 0, 1);

                    // Authentic Boat Model (Scale 1.0f matches character proportions)

                    DrawModel(boatModel, Vector3{ 0, 0, 0 }, 1.0f, WHITE);

                    // Left Pirate (Player 1: Green Hippo - SeatedB)
            float p1Redness = gasP1.redness;
            float p1ButtSwell = gasP1.boneScale - 1.0f; // 0.0 to 0.30 (*1.3 scale)
            float p1EyeBulge = (gasP1.isPressing || gasP1.releaseTimer > 0.0f) ? Clamp(gasP1.pressTimer / 0.45f, 0.0f, 1.0f) : 0.0f;
            SetShaderValue(shaderLighting, uRednessLoc,   &p1Redness,   SHADER_UNIFORM_FLOAT);
            SetShaderValue(shaderLighting, uButtSwellLoc, &p1ButtSwell, SHADER_UNIFORM_FLOAT);
            SetShaderValue(shaderLighting, uEyeBulgeLoc,  &p1EyeBulge,  SHADER_UNIFORM_FLOAT);

            rlPushMatrix();
            // Image 6 Authentic Outer Rim Seating (Legs and buttocks dangle outside over water!)
            float rimSeatRadius = 1.10f;
            float rimSeatHeight = 0.40f;
            float p1X = sinf(p1RimAngle) * rimSeatRadius;
            float p1Z = cosf(p1RimAngle) * rimSeatRadius;
            rlTranslatef(p1X, rimSeatHeight + gasP1.trembleOffset, p1Z);
            float p1FaceYaw = (p1RimAngle * RAD2DEG) + 180.0f;
            rlRotatef(p1FaceYaw, 0, 1, 0);
            DrawModel(charB, Vector3{ 0, 0, 0 }, 1.0f, WHITE);
            rlPopMatrix();

            if (!singlePirateMode) {
                // Right Pirate (Player 2: Orange Hippo - Active in LAN mode!)
                float p2Redness = gasP2.redness;
                float p2ButtSwell = gasP2.boneScale - 1.0f; // 0.0 to 0.30 (*1.3 scale)
                float p2EyeBulge = (gasP2.isPressing || gasP2.releaseTimer > 0.0f) ? Clamp(gasP2.pressTimer / 0.45f, 0.0f, 1.0f) : 0.0f;
                SetShaderValue(shaderLighting, uRednessLoc,   &p2Redness,   SHADER_UNIFORM_FLOAT);
                SetShaderValue(shaderLighting, uButtSwellLoc, &p2ButtSwell, SHADER_UNIFORM_FLOAT);
                SetShaderValue(shaderLighting, uEyeBulgeLoc,  &p2EyeBulge,  SHADER_UNIFORM_FLOAT);

                rlPushMatrix();
                float p2X = sinf(p2RimAngle) * rimSeatRadius;
                float p2Z = cosf(p2RimAngle) * rimSeatRadius;
                rlTranslatef(p2X, rimSeatHeight + gasP2.trembleOffset, p2Z);
                float p2FaceYaw = (p2RimAngle * RAD2DEG) + 180.0f;
                rlRotatef(p2FaceYaw, 0, 1, 0);
                DrawModel(charA, Vector3{ 0, 0, 0 }, 1.0f, WHITE);
                rlPopMatrix();
            }

            // Reset uniforms for raft and all subsequent models
                    float zeroReset = 0.0f;
                    SetShaderValue(shaderLighting, uRednessLoc,   &zeroReset, SHADER_UNIFORM_FLOAT);
                    SetShaderValue(shaderLighting, uButtSwellLoc, &zeroReset, SHADER_UNIFORM_FLOAT);
                    SetShaderValue(shaderLighting, uEyeBulgeLoc,  &zeroReset, SHADER_UNIFORM_FLOAT);
                    SetShaderValue(shaderLighting, uSkinULoc,     &zeroReset, SHADER_UNIFORM_FLOAT);
                    SetShaderValue(shaderLighting, uSkinVLoc,     &zeroReset, SHADER_UNIFORM_FLOAT);

                rlPopMatrix();
            }

            // H. Ragdoll Pirates Ejected during Boat Destruction

            if (raft.isDestroyed) {

                SetShaderValue(shaderLighting, uRednessLoc, &zeroRed, SHADER_UNIFORM_FLOAT);

                if (ragdollA.active) {

                    rlPushMatrix();

                        rlTranslatef(ragdollA.position.x, ragdollA.position.y, ragdollA.position.z);

                        rlRotatef(ragdollA.rotation.x, 1, 0, 0);

                        rlRotatef(ragdollA.rotation.y, 0, 1, 0);

                        DrawModel(charA, Vector3{ 0, 0, 0 }, 1.15f, WHITE);

                    rlPopMatrix();

                }

                if (ragdollB.active) {

                    rlPushMatrix();

                        rlTranslatef(ragdollB.position.x, ragdollB.position.y, ragdollB.position.z);

                        rlRotatef(ragdollB.rotation.x, 1, 0, 0);

                        rlRotatef(ragdollB.rotation.y, 0, 1, 0);

                        DrawModel(charB, Vector3{ 0, 0, 0 }, 1.15f, WHITE);

                    rlPopMatrix();

                }

            }

            // Authentic Raft Foam Wake on Water Surface

            rlDisableDepthMask();

            BeginBlendMode(BLEND_ALPHA);

            rlPushMatrix();

                rlTranslatef(raft.position.x, raft.position.y + 0.02f, raft.position.z);

                rlRotatef(raft.rotationY * RAD2DEG, 0, 1, 0);

                float foamRad = 1.65f + (raft.speed / 14.0f) * 0.35f;

                Color foamTint = Color{ 255, 255, 255, 200 };

                rlSetTexture(texRaftFoam.id);

                rlBegin(RL_QUADS);

                    rlColor4ub(foamTint.r, foamTint.g, foamTint.b, foamTint.a);

                    rlTexCoord2f(0.0f, 0.0f); rlVertex3f(-foamRad, 0.0f, -foamRad);

                    rlTexCoord2f(1.0f, 0.0f); rlVertex3f( foamRad, 0.0f, -foamRad);

                    rlTexCoord2f(1.0f, 1.0f); rlVertex3f( foamRad, 0.0f,  foamRad);

                    rlTexCoord2f(0.0f, 1.0f); rlVertex3f(-foamRad, 0.0f,  foamRad);

                rlEnd();

                rlSetTexture(0);

            rlPopMatrix();

            // I. Cartoon Billboard Fart Particles (Buttery Smooth Billows, Zero Water Clipping)

            rlDisableDepthTest();

            for (int p = 0; p < MAX_PARTICLES; p++) {

                if (particles[p].active) {

                    Texture2D tex = (particles[p].spriteIndex == 4) ? texSkullGas : texFart[particles[p].spriteIndex % 4];
            Rectangle srcRec = { 0, 0, (float)tex.width, (float)tex.height };
            Color pTint = (particles[p].spriteIndex == 4) ? ColorAlpha(WHITE, particles[p].alpha)
                                                         : ColorAlpha(Color{ 140, 255, 60, 255 }, particles[p].alpha);
            DrawBillboardPro(camera, tex, srcRec, particles[p].position, Vector3{ 0, 1, 0 },
                             Vector2{ particles[p].size, particles[p].size }, Vector2{ particles[p].size * 0.5f, particles[p].size * 0.5f },
                             particles[p].rotation, pTint);

                }

            }

            EndBlendMode();

            rlEnableDepthTest();

            rlEnableDepthMask();

        
            // 3D Level Editor Visual Highlights & Gizmos (Unity / Godot style)
            if (editorMode) {
                for (size_t i = 0; i < levelEntities.size(); i++) {
                    bool isSel = ((int)i == selectedObs);
                    Vector3 oPos = levelEntities[i].position;
                    float r = (levelEntities[i].radius > 0.5f) ? levelEntities[i].radius : 1.5f;
                    DrawCircle3D(Vector3{ oPos.x, 0.05f, oPos.z }, r, Vector3{ 1, 0, 0 }, 90.0f, isSel ? Color{ 255, 235, 50, 180 } : Color{ 80, 180, 255, 90 });

                    if (isSel) {
                        BoundingBox selBBox = { Vector3Subtract(oPos, Vector3{ r, 1.4f, r }), Vector3Add(oPos, Vector3{ r, 1.4f, r }) };
                        DrawBoundingBox(selBBox, YELLOW);

                        // Task 5: 3D Interactive Gizmo with X/Y/Z arrows and negative Flip cubes!
                        DrawLine3D(oPos, Vector3Add(oPos, Vector3{ 3.2f, 0, 0 }), RED);
                        DrawLine3D(oPos, Vector3Add(oPos, Vector3{ 0, 3.2f, 0 }), GREEN);
                        DrawLine3D(oPos, Vector3Add(oPos, Vector3{ 0, 0, 3.2f }), BLUE);
                        DrawSphere(Vector3Add(oPos, Vector3{ 3.2f, 0, 0 }), 0.22f, RED);
                        DrawSphere(Vector3Add(oPos, Vector3{ 0, 3.2f, 0 }), 0.22f, GREEN);
                        DrawSphere(Vector3Add(oPos, Vector3{ 0, 0, 3.2f }), 0.22f, BLUE);

                        // Negative Flip handles (-X, -Y, -Z)
                        DrawCube(Vector3Add(oPos, Vector3{ -1.8f, 0, 0 }), 0.35f, 0.35f, 0.35f, Color{ 220, 50, 50, 220 });
                        DrawCube(Vector3Add(oPos, Vector3{ 0, -1.8f, 0 }), 0.35f, 0.35f, 0.35f, Color{ 50, 200, 50, 220 });
                        DrawCube(Vector3Add(oPos, Vector3{ 0, 0, -1.8f }), 0.35f, 0.35f, 0.35f, Color{ 50, 100, 220, 220 });
                    }
                }
            }

        EndMode3D();

        // =====================================================================

        // Authentic Game UI (1:1 Matching Image 2 & Disassembled Sprites)
        // =====================================================================

        // 1. Top-Left: Authentic Dual Hippo Gas Tank Level Bars (P1 & P2)
        {
            float gasFrac1 = Clamp(gasP1.gasEnergy / GAS_MAX, 0.0f, 1.0f);
            Color barFillCol1 = gasP1.gasDepleted ? Color{ 239, 68, 68, 255 } : Color{ 74, 222, 50, 255 };

            DrawRectangleRounded(Rectangle{ 65, 10, 184, 18 }, 0.5f, 6, Color{ 15, 23, 42, 210 });
            DrawRectangleRoundedLinesEx(Rectangle{ 65, 10, 184, 18 }, 0.5f, 6, 1.5f, Color{ 255, 255, 255, 160 });
            if (gasFrac1 > 0.02f) {
                DrawRectangleRounded(Rectangle{ 67, 12, 180.0f * gasFrac1, 14 }, 0.45f, 6, barFillCol1);
            }

            if (!singlePirateMode) {
                float gasFrac2 = Clamp(gasP2.gasEnergy / GAS_MAX, 0.0f, 1.0f);
                Color barFillCol2 = gasP2.gasDepleted ? Color{ 239, 68, 68, 255 } : Color{ 56, 189, 248, 255 };

                DrawRectangleRounded(Rectangle{ 65, 32, 184, 18 }, 0.5f, 6, Color{ 15, 23, 42, 210 });
                DrawRectangleRoundedLinesEx(Rectangle{ 65, 32, 184, 18 }, 0.5f, 6, 1.5f, Color{ 255, 255, 255, 160 });
                if (gasFrac2 > 0.02f) {
                    DrawRectangleRounded(Rectangle{ 67, 34, 180.0f * gasFrac2, 14 }, 0.45f, 6, barFillCol2);
                }
            }

            DrawTexturePro(texHipo, Rectangle{ 0, 0, (float)texHipo.width, (float)texHipo.height },
                           Rectangle{ 14, 4, 46, 50 }, Vector2{ 0, 0 }, 0.0f, WHITE);
        }

        // 2. Top-Center: Full Course Progress Bar Track & Lap Stopwatch (Spline-accurate!)
        {
            int trackStartX = 290;
            int trackEndX = GetScreenWidth() - 290;
            int trackWidth = trackEndX - trackStartX;
            int trackY = 18;

            DrawRectangle(trackStartX, trackY, trackWidth, 5, Color{ 255, 255, 255, 230 });
            DrawRectangleLines(trackStartX, trackY, trackWidth, 5, Color{ 15, 23, 42, 200 });

            for (const auto& ent : levelEntities) {
                if (ent.type == ENT_CHECKPOINT) {
                    float cpT = FindClosestSplineT(ent.position);
                    float frac = Clamp(cpT / (float)(SPLINE_POINT_COUNT - 1), 0.0f, 1.0f);
                    int tickX = trackStartX + (int)(trackWidth * frac);
                    DrawRectangle(tickX - 1, trackY - 3, 3, 11, ent.cpPassed ? COLOR_ACCENT_GREEN : Color{ 30, 41, 59, 255 });
                }
            }

            DrawTexturePro(texFinish, Rectangle{ 0, 0, (float)texFinish.width, (float)texFinish.height },
                           Rectangle{ (float)trackEndX - 8, (float)trackY - 14, 26, 28 }, Vector2{ 0, 0 }, 0.0f, WHITE);

            float raftSplineT = FindClosestSplineT(raft.position);
            float courseFrac = Clamp(raftSplineT / (float)(SPLINE_POINT_COUNT - 1), 0.0f, 1.0f);
            float pinX = (float)trackStartX + (float)trackWidth * courseFrac;
            DrawTexturePro(texRaftPin, Rectangle{ 0, 0, (float)texRaftPin.width, (float)texRaftPin.height },
                           Rectangle{ pinX - 14, (float)trackY - 13, 28, 28 }, Vector2{ 0, 0 }, 0.0f, WHITE);

            int mins = (int)raceTimer / 60;
            int secs = (int)raceTimer % 60;
            int hundredths = (int)((raceTimer - (int)raceTimer) * 100);

            int clockBadgeW = 150;
            int clockBadgeX = GetScreenWidth() / 2 - clockBadgeW / 2;
            DrawRectangleRounded(Rectangle{ (float)clockBadgeX, 29, (float)clockBadgeW, 26 }, 0.4f, 4, Color{ 15, 23, 42, 210 });
            DrawTexturePro(texClock, Rectangle{ 0, 0, (float)texClock.width, (float)texClock.height },
                           Rectangle{ (float)clockBadgeX + 8, 32, 20, 20 }, Vector2{ 0, 0 }, 0.0f, WHITE);
            DrawText(TextFormat("%02d:%02d.%02d", mins, secs, hundredths), clockBadgeX + 36, 34, 18, WHITE);
        }

        // 3. Top-Right: Death / Attempt Counter
        {
            int deathBadgeW = 110;
            int deathBadgeX = GetScreenWidth() - deathBadgeW - 24;
            DrawRectangleRounded(Rectangle{ (float)deathBadgeX, 14, (float)deathBadgeW, 30 }, 0.4f, 4, Color{ 15, 23, 42, 210 });
            DrawRectangleRoundedLinesEx(Rectangle{ (float)deathBadgeX, 14, (float)deathBadgeW, 30 }, 0.4f, 4, 1.0f, Color{ 255, 255, 255, 120 });

            DrawTexturePro(texCrashed, Rectangle{ 0, 0, (float)texCrashed.width, (float)texCrashed.height },
                           Rectangle{ (float)deathBadgeX + 8, 17, 30, 22 }, Vector2{ 0, 0 }, 0.0f, WHITE);
            DrawText(TextFormat("%d", totalDeaths), deathBadgeX + 46, 20, 18, WHITE);
        }

        // 4. Bottom-Left: [ESC] LEAVE Button (Now functional on click!)
        {
            Rectangle btnLeaveRec = { 20, (float)GetScreenHeight() - 48, 115, 34 };
            bool hoverLeave = CheckCollisionPointRec(mousePos, btnLeaveRec);
            DrawRectangleRounded(btnLeaveRec, 0.35f, 4, hoverLeave ? Color{ 30, 41, 59, 240 } : Color{ 15, 23, 42, 210 });
            DrawRectangleRoundedLinesEx(btnLeaveRec, 0.35f, 4, 1.2f, hoverLeave ? Color{ 148, 163, 184, 255 } : Color{ 71, 85, 105, 180 });

            DrawTexturePro(texEsc, Rectangle{ 0, 0, (float)texEsc.width, (float)texEsc.height },
                           Rectangle{ 26, (float)GetScreenHeight() - 43, 24, 24 }, Vector2{ 0, 0 }, 0.0f, WHITE);
            DrawText("LEAVE", 58, GetScreenHeight() - 40, 16, RAYWHITE);
            if ((hoverLeave && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) || IsKeyPressed(KEY_ESCAPE)) {
                currentScreen = SCREEN_MAIN_MENU;
            }
        }

        // Bottom-Left: [R] SUICIDE Button (Authentic Image 4 Parity)
        {
            Rectangle btnSuicideRec = { 145, (float)GetScreenHeight() - 48, 125, 34 };
            bool hoverSuicide = CheckCollisionPointRec(mousePos, btnSuicideRec);
            DrawRectangleRounded(btnSuicideRec, 0.35f, 4, hoverSuicide ? Color{ 45, 25, 30, 240 } : Color{ 25, 15, 20, 210 });
            DrawRectangleRoundedLinesEx(btnSuicideRec, 0.35f, 4, 1.2f, hoverSuicide ? Color{ 239, 68, 68, 255 } : Color{ 153, 27, 27, 180 });

            DrawText("[R]", 155, GetScreenHeight() - 40, 16, Color{ 248, 113, 113, 255 });
            DrawText("SUICIDE", 185, GetScreenHeight() - 40, 16, RAYWHITE);

            if ((hoverSuicide && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) || IsKeyPressed(KEY_R)) {
                if (!editorMode && !raft.isDestroyed) {
                    TriggerBoatDeath(DEATH_SUICIDE);
                }
            }
        }

                // 5. Bottom-Right: [F1] LEVEL EDITOR Toggle Button (Godot Style)
        {
            Rectangle btnEditorRec = { (float)GetScreenWidth() - 175, (float)GetScreenHeight() - 44, 160, 32 };
            bool hoverEditor = CheckCollisionPointRec(mousePos, btnEditorRec);
            if ((hoverEditor && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) || IsKeyPressed(KEY_F1)) {
                editorMode = !editorMode;
                if (editorMode) {
                    editorCamPos = Vector3Add(raft.position, Vector3{ 0.0f, 4.5f, 6.0f });
                    editorCamYaw = raft.rotationY;
                    editorCamPitch = 0.35f;
                    editorMsg = "LEVEL EDITOR ACTIVE - GAMEPLAY FROZEN";
                    editorMsgTimer = 3.0f;
                    raft.velocity = Vector3Zero();
                    raft.angularVelocityY = 0.0f;
                    gasP1.isSustained = false; gasP1.pressTimer = 0.0f;
                    gasP2.isSustained = false; gasP2.pressTimer = 0.0f;
                }
            }

            Color btnBg = editorMode ? Color{ 71, 140, 191, 245 } : (hoverEditor ? Color{ 48, 54, 70, 240 } : Color{ 32, 37, 49, 230 });
            Color btnBorder = editorMode ? Color{ 110, 180, 230, 255 } : Color{ 68, 76, 95, 200 };
            DrawRectangleRounded(btnEditorRec, 0.25f, 4, btnBg);
            DrawRectangleRoundedLinesEx(btnEditorRec, 0.25f, 4, 1.0f, btnBorder);
            const char* toggleLbl = editorMode ? "Play Mode (F1)" : "Level Editor (F1)";
            Vector2 tSz = MeasureTextEx(fontUIBold, toggleLbl, 14, 0.4f);
            DrawTextEx(fontUIBold, toggleLbl, Vector2{ btnEditorRec.x + (btnEditorRec.width - tSz.x) * 0.5f, btnEditorRec.y + 8 }, 14, 0.4f, WHITE);

            // Prominent Top Bar Natural Motion Toggle Button (Accessible anytime in Editor!)
            if (editorMode) {
                Rectangle btnTopMotionRec = Rectangle{ btnEditorRec.x - 225, btnEditorRec.y, 215, 34 };
                bool hovMotion = CheckCollisionPointRec(mousePos, btnTopMotionRec);
                Color mBg = editorAnimateObstacles ? Color{ 46, 128, 76, 245 } : Color{ 75, 40, 50, 240 };
                Color mBorder = editorAnimateObstacles ? Color{ 85, 205, 125, 255 } : Color{ 170, 75, 80, 255 };
                DrawRectangleRounded(btnTopMotionRec, 0.25f, 4, mBg);
                DrawRectangleRoundedLinesEx(btnTopMotionRec, 0.25f, 4, 1.0f, mBorder);
                const char* topMLbl = editorAnimateObstacles ? "â–¶ Motion: NATURAL (P)" : "â¸ Motion: FROZEN (P)";
                Vector2 mSz = MeasureTextEx(fontUIBold, topMLbl, 13, 0.3f);
                DrawTextEx(fontUIBold, topMLbl, Vector2{ btnTopMotionRec.x + (btnTopMotionRec.width - mSz.x) * 0.5f, btnTopMotionRec.y + 9 }, 13, 0.3f, WHITE);
                if ((hovMotion && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) || IsKeyPressed(KEY_P)) {
                    editorAnimateObstacles = !editorAnimateObstacles;
                    editorMsg = editorAnimateObstacles ? "Obstacle Motion: PLAYING (Natural)" : "Obstacle Motion: FROZEN";
                    editorMsgTimer = 2.5f;
                }
            }
        }

        // 6. Professional Godot 4-Dock Editor Architecture (Plan v6.0 Task 5)
        if (editorMode) {
            float screenW = (float)GetScreenWidth();
            float screenH = (float)GetScreenHeight();

            auto DrawGButton = [&](Rectangle rec, const char* label, Color bgCol, Color textCol, bool bold = false) -> bool {
                bool hov = CheckCollisionPointRec(mousePos, rec);
                Color fill = hov ? ColorBrightness(bgCol, 0.20f) : bgCol;
                DrawRectangleRounded(rec, 0.2f, 3, fill);
                DrawRectangleRoundedLinesEx(rec, 0.2f, 3, 1.0f, ColorBrightness(bgCol, 0.35f));
                Font f = bold ? fontUIBold : fontUI;
                Vector2 sz = MeasureTextEx(f, label, 12, 0.3f);
                DrawTextEx(f, label, Vector2{ rec.x + (rec.width - sz.x) * 0.5f, rec.y + (rec.height - sz.y) * 0.5f }, 12, 0.3f, textCol);
                return hov && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
            };

            // -------------------------------------------------------------
            // A. TOP TOOLBAR (Height 42px)
            // -------------------------------------------------------------
            Rectangle topBarRec = { 0, 0, screenW, 42.0f };
            DrawRectangleRec(topBarRec, Color{ 24, 28, 38, 252 });
            DrawLine(0, 42, (int)screenW, 42, Color{ 52, 60, 78, 255 });

            float bx = 12.0f;
            // Tool modes: Select, Move, Rotate, Scale
            const char* tools[] = { "[Q] Select", "[W] Move", "[E] Rotate", "[R] Scale" };
            for (int tm = 0; tm < 4; tm++) {
                Rectangle bRec = { bx, 7.0f, 75.0f, 28.0f };
                bool active = (editorGizmoMode == tm);
                Color bCol = active ? Color{ 48, 120, 200, 255 } : Color{ 36, 43, 58, 255 };
                if (DrawGButton(bRec, tools[tm], bCol, WHITE, active)) {
                    editorGizmoMode = tm;
                }
                bx += 79.0f;
            }
            if (IsKeyPressed(KEY_Q)) editorGizmoMode = 0;
            if (IsKeyPressed(KEY_W)) editorGizmoMode = 1;
            if (IsKeyPressed(KEY_E)) editorGizmoMode = 2;
            if (IsKeyPressed(KEY_R)) editorGizmoMode = 3;

            DrawLine((int)bx + 4, 8, (int)bx + 4, 34, Color{ 60, 70, 90, 255 });
            bx += 12.0f;

            // Instant Axis Flip Buttons (Flip X, Flip Y, Flip Z)
            Rectangle bFlipX = { bx, 7.0f, 62.0f, 28.0f }; bx += 66.0f;
            Rectangle bFlipY = { bx, 7.0f, 62.0f, 28.0f }; bx += 66.0f;
            Rectangle bFlipZ = { bx, 7.0f, 62.0f, 28.0f }; bx += 70.0f;

            if (selectedObs >= 0 && selectedObs < (int)levelEntities.size()) {
                auto& sel = levelEntities[selectedObs];
                if (DrawGButton(bFlipX, "Flip X", Color{ 180, 50, 50, 255 }, WHITE, true)) {
                    sel.flipX *= -1;
                    sel.rotationY = fmodf(-sel.rotationY + 360.0f, 360.0f);
                    editorMsg = "Flipped Object along X Axis!";
                    editorMsgTimer = 2.0f;
                }
                if (DrawGButton(bFlipY, "Flip Y", Color{ 45, 150, 60, 255 }, WHITE, true)) {
                    sel.flipY *= -1;
                    sel.rotationZ = fmodf(sel.rotationZ + 180.0f, 360.0f);
                    editorMsg = "Flipped Object along Y Axis!";
                    editorMsgTimer = 2.0f;
                }
                if (DrawGButton(bFlipZ, "Flip Z", Color{ 45, 90, 180, 255 }, WHITE, true)) {
                    sel.flipZ *= -1;
                    sel.rotationY = fmodf(sel.rotationY + 180.0f, 360.0f);
                    editorMsg = "Flipped Object along Z Axis!";
                    editorMsgTimer = 2.0f;
                }
            } else {
                DrawGButton(bFlipX, "Flip X", Color{ 50, 40, 40, 180 }, Color{ 150, 150, 150, 255 });
                DrawGButton(bFlipY, "Flip Y", Color{ 40, 50, 40, 180 }, Color{ 150, 150, 150, 255 });
                DrawGButton(bFlipZ, "Flip Z", Color{ 40, 40, 50, 180 }, Color{ 150, 150, 150, 255 });
            }

            DrawLine((int)bx + 4, 8, (int)bx + 4, 34, Color{ 60, 70, 90, 255 });
            bx += 12.0f;

            // Visual Obstacle Catalog Toggle Button
            Rectangle bCat = { bx, 7.0f, 130.0f, 28.0f }; bx += 136.0f;
            Color catBg = visualCatalogOpen ? Color{ 160, 90, 30, 255 } : Color{ 36, 43, 58, 255 };
            if (DrawGButton(bCat, visualCatalogOpen ? "* Catalog: OPEN" : "+ Visual Catalog", catBg, WHITE, true)) {
                visualCatalogOpen = !visualCatalogOpen;
            }

            // Purge Hidden Obstacles Button
            Rectangle bPurge = { bx, 7.0f, 105.0f, 28.0f }; bx += 112.0f;
            if (DrawGButton(bPurge, "Purge Hidden", Color{ 120, 50, 50, 255 }, WHITE, true)) {
                int purged = 0;
                for (int i = (int)levelEntities.size() - 1; i >= 0; i--) {
                    if (levelEntities[i].position.y < -1.0f) {
                        levelEntities.erase(levelEntities.begin() + i);
                        purged++;
                    }
                }
                editorMsg = TextFormat("Purged %d hidden entities!", purged);
                editorMsgTimer = 2.5f;
            }

            // Right side of top toolbar: Motion toggle, Save, Play Mode
            float rbx = screenW - 130.0f;
            Rectangle bPlay = { rbx, 7.0f, 118.0f, 28.0f }; rbx -= 150.0f;
            if (DrawGButton(bPlay, "Play Mode (F1)", Color{ 37, 99, 235, 255 }, WHITE, true) || IsKeyPressed(KEY_F1)) {
                editorMode = false;
            }

            Rectangle bSave = { rbx, 7.0f, 142.0f, 28.0f }; rbx -= 160.0f;
            if (DrawGButton(bSave, "Save All (Ctrl+S)", Color{ 34, 139, 34, 255 }, WHITE, true) || (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_S))) {
                SaveLevelLayoutJSON(LEVEL_LAYOUT_PATH, levelEntities);
                SavePhysicsConfigJSON("physics_config.json");
                editorMsg = "Saved Level & Physics to JSON!";
                editorMsgTimer = 3.0f;
            }

            Rectangle bMotion = { rbx, 7.0f, 152.0f, 28.0f };
            Color motCol = editorAnimateObstacles ? Color{ 46, 128, 76, 255 } : Color{ 120, 48, 56, 255 };
            const char* motLbl = editorAnimateObstacles ? "Motion: NATURAL (P)" : "Motion: FROZEN (P)";
            if (DrawGButton(bMotion, motLbl, motCol, WHITE, true) || IsKeyPressed(KEY_P)) {
                editorAnimateObstacles = !editorAnimateObstacles;
                editorMsg = editorAnimateObstacles ? "Obstacle Motion: NATURAL (1.0x)" : "Obstacle Motion: FROZEN";
                editorMsgTimer = 2.0f;
            }

            // -------------------------------------------------------------
            // B. LEFT DOCK: COLLISION SHAPES & SCENE HIERARCHY (Width 270px)
            // -------------------------------------------------------------
            float leftDockW = 270.0f;
            Rectangle leftCollapseBtn = { leftDockCollapsed ? 0.0f : leftDockW, 46.0f, 22.0f, 40.0f };
            if (DrawGButton(leftCollapseBtn, leftDockCollapsed ? ">" : "<", Color{ 32, 38, 52, 255 }, WHITE, true)) {
                leftDockCollapsed = !leftDockCollapsed;
            }

            if (!leftDockCollapsed) {
                Rectangle leftRec = { 0, 42.0f, leftDockW, screenH - 42.0f };
                DrawRectangleRec(leftRec, Color{ 28, 33, 44, 252 });
                DrawLine((int)leftDockW, 42, (int)leftDockW, (int)screenH, Color{ 52, 60, 78, 255 });

                float ly = 50.0f;
                // Header
                DrawTextEx(fontUIBold, "Collision Shapes (10 Types)", Vector2{ 12.0f, ly }, 13, 0.4f, Color{ 255, 215, 0, 255 });
                ly += 22.0f;

                // 10 Custom Collision Shape Buttons (2 columns)
                const char* shapeNames[] = {
                    "+ Square", "+ Rect Wall",
                    "+ Circle", "+ 3D Cube",
                    "+ Cylinder", "+ Capsule",
                    "+ Sphere", "+ Wedge Ramp",
                    "+ Fall Zone", "+ Kill Box"
                };
                EntityType shapeTypes[] = {
                    ENT_COL_SQUARE, ENT_COL_RECT,
                    ENT_COL_CIRCLE, ENT_COL_CUBE,
                    ENT_COL_CYLINDER, ENT_COL_CAPSULE,
                    ENT_COL_SPHERE, ENT_COL_WEDGE,
                    ENT_COL_FALL_ZONE, ENT_COL_KILL_ZONE
                };

                for (int s = 0; s < 10; s += 2) {
                    Rectangle b1 = { 10.0f, ly, 120.0f, 24.0f };
                    Rectangle b2 = { 136.0f, ly, 120.0f, 24.0f };
                    Color cBg = (s >= 8) ? Color{ 140, 50, 40, 255 } : Color{ 40, 55, 80, 255 };

                    auto SpawnShape = [&](EntityType st, int beh) {
                        LevelEntity ne{};
                        ne.id = (int)levelEntities.size();
                        ne.type = st;
                        Vector3 fwd = { -sinf(editorCamYaw) * cosf(editorCamPitch), sinf(editorCamPitch), -cosf(editorCamYaw) * cosf(editorCamPitch) };
                        ne.position = Vector3Add(editorCamPos, Vector3Scale(fwd, 6.0f));
                        ne.scale = 1.0f;
                        ne.sizeX = 2.0f; ne.sizeY = 2.0f; ne.sizeZ = 2.0f;
                        ne.radius = 2.0f;
                        ne.flipX = 1; ne.flipY = 1; ne.flipZ = 1;
                        ne.colBehavior = beh;
                        AssignEntityModelAndProps(ne);
                        levelEntities.push_back(ne);
                        selectedObs = (int)levelEntities.size() - 1;
                        editorMsg = TextFormat("Spawned %s!", ENTITY_TYPE_NAMES[st]);
                        editorMsgTimer = 2.0f;
                    };

                    if (DrawGButton(b1, shapeNames[s], cBg, WHITE)) {
                        SpawnShape(shapeTypes[s], (shapeTypes[s] == ENT_COL_FALL_ZONE) ? 1 : ((shapeTypes[s] == ENT_COL_KILL_ZONE) ? 2 : 0));
                    }
                    if (DrawGButton(b2, shapeNames[s+1], cBg, WHITE)) {
                        SpawnShape(shapeTypes[s+1], (shapeTypes[s+1] == ENT_COL_FALL_ZONE) ? 1 : ((shapeTypes[s+1] == ENT_COL_KILL_ZONE) ? 2 : 0));
                    }
                    ly += 27.0f;
                }

                ly += 8.0f;
                DrawLine(10, (int)ly, (int)leftDockW - 10, (int)ly, Color{ 52, 60, 78, 255 });
                ly += 10.0f;

                // Collider Behavior Section
                DrawTextEx(fontUIBold, "Collider Behavior:", Vector2{ 12.0f, ly }, 12, 0.4f, Color{ 142, 202, 230, 255 });
                ly += 20.0f;
                const char* behNames[] = { "Solid Wall", "Fall Abyss", "Kill Boat", "Bounce Pad" };
                Color behCols[] = { Color{ 30, 80, 140, 255 }, Color{ 160, 90, 20, 255 }, Color{ 160, 30, 30, 255 }, Color{ 130, 40, 130, 255 } };
                for (int b = 0; b < 4; b += 2) {
                    Rectangle b1 = { 10.0f, ly, 120.0f, 22.0f };
                    Rectangle b2 = { 136.0f, ly, 120.0f, 22.0f };
                    if (selectedObs >= 0 && selectedObs < (int)levelEntities.size()) {
                        auto& sel = levelEntities[selectedObs];
                        if (DrawGButton(b1, behNames[b], (sel.colBehavior == b) ? ColorBrightness(behCols[b], 0.3f) : behCols[b], WHITE, sel.colBehavior == b)) {
                            sel.colBehavior = b;
                            editorMsg = TextFormat("Set Behavior: %s", behNames[b]);
                            editorMsgTimer = 2.0f;
                        }
                        if (DrawGButton(b2, behNames[b+1], (sel.colBehavior == b+1) ? ColorBrightness(behCols[b+1], 0.3f) : behCols[b+1], WHITE, sel.colBehavior == b+1)) {
                            sel.colBehavior = b + 1;
                            editorMsg = TextFormat("Set Behavior: %s", behNames[b+1]);
                            editorMsgTimer = 2.0f;
                        }
                    } else {
                        DrawGButton(b1, behNames[b], Color{ 40, 45, 55, 180 }, Color{ 140, 140, 140, 255 });
                        DrawGButton(b2, behNames[b+1], Color{ 40, 45, 55, 180 }, Color{ 140, 140, 140, 255 });
                    }
                    ly += 25.0f;
                }

                ly += 8.0f;
                DrawLine(10, (int)ly, (int)leftDockW - 10, (int)ly, Color{ 52, 60, 78, 255 });
                ly += 10.0f;

                // Scene Hierarchy Section
                DrawTextEx(fontUIBold, TextFormat("Scene Hierarchy (%d Entities):", (int)levelEntities.size()), Vector2{ 12.0f, ly }, 12, 0.4f, Color{ 220, 225, 235, 255 });
                ly += 20.0f;

                // Hierarchy scroll handling
                if (mousePos.x > 0 && mousePos.x < leftDockW && mousePos.y > ly) {
                    hierarchyScroll -= (int)(GetMouseWheelMove() * 3.0f);
                    hierarchyScroll = Clamp(hierarchyScroll, 0, fmaxf(0, (int)levelEntities.size() - 8));
                }

                int maxVisible = 9;
                for (int i = hierarchyScroll; i < (int)levelEntities.size() && i < hierarchyScroll + maxVisible; i++) {
                    Rectangle hRec = { 10.0f, ly, leftDockW - 20.0f, 22.0f };
                    bool isSel = (selectedObs == i);
                    Color hCol = isSel ? Color{ 48, 120, 200, 255 } : Color{ 34, 40, 52, 255 };
                    std::string hLbl = TextFormat("#%d %s", i, ENTITY_TYPE_NAMES[levelEntities[i].type]);
                    if (hLbl.length() > 24) hLbl = hLbl.substr(0, 22) + "..";
                    if (DrawGButton(hRec, hLbl.c_str(), hCol, WHITE, isSel)) {
                        selectedObs = i;
                    }
                    ly += 24.0f;
                }

                if (selectedObs >= 0 && selectedObs < (int)levelEntities.size()) {
                    Rectangle btnFoc = { 10.0f, screenH - 36.0f, leftDockW - 20.0f, 26.0f };
                    if (DrawGButton(btnFoc, "Focus Camera (F)", Color{ 40, 100, 70, 255 }, WHITE, true)) {
                        editorCamPos = Vector3Add(levelEntities[selectedObs].position, Vector3{ 0.0f, 3.5f, 6.0f });
                        editorCamYaw = 3.14159f;
                        editorCamPitch = 0.35f;
                    }
                }
            }

            // -------------------------------------------------------------
            // C. VERTICAL VISUAL OBSTACLE CATALOG (Stacking vertically one under another)
            // -------------------------------------------------------------
            if (visualCatalogOpen) {
                float catW = 230.0f;
                float catX = leftDockCollapsed ? 12.0f : (leftDockW + 10.0f);
                float catY = 48.0f;
                float catH = screenH - 58.0f;
                Rectangle catRec = { catX, catY, catW, catH };

                DrawRectangleRounded(catRec, 0.04f, 4, Color{ 18, 22, 32, 252 });
                DrawRectangleRoundedLinesEx(catRec, 0.04f, 4, 1.2f, Color{ 71, 140, 191, 255 });

                DrawTextEx(fontUIBold, "Obstacle Catalog (Vertical)", Vector2{ catX + 12.0f, catY + 10.0f }, 12, 0.4f, Color{ 255, 215, 0, 255 });
                DrawLine((int)catX + 8, (int)catY + 30, (int)catX + (int)catW - 8, (int)catY + 30, Color{ 52, 60, 78, 255 });

                // Scroll handling for catalog
                if (CheckCollisionPointRec(mousePos, catRec)) {
                    catalogScroll -= (int)(GetMouseWheelMove() * 2.0f);
                    catalogScroll = Clamp(catalogScroll, 0, fmaxf(0, 19 - 9));
                }

                float cy = catY + 36.0f;
                const char* catObstacleNames[] = {
                    "OBS_A1 Bumper", "OBS_B2 Tipping", "OBS_B3 Spinner", "OBS_B5 Rotor",
                    "OBS_C1 Spiked Arc", "OBS_C2 Pendulum", "OBS_C4 Quad Mace", "OBS_C5 Wheel",
                    "OBS_F3 Swing Gate", "OBS_F10 Hammer Gate", "OBS_F11 Spiked Drum", "OBS_F13 Rotating Arm",
                    "OBS_F14 Slider", "OBS_F18 Post", "OBS_F23 Dual Gate",
                    "Rock Dark 1", "Rock Dark 2", "Wind Gust",
                    "CHECKPOINT Gate", "Jungle Tree"
                };

                for (int ci = catalogScroll; ci < 20 && ci < catalogScroll + 10; ci++) {
                    Rectangle cCard = { catX + 8.0f, cy, catW - 16.0f, 48.0f };
                    DrawRectangleRounded(cCard, 0.15f, 3, Color{ 28, 34, 48, 255 });
                    DrawRectangleRoundedLinesEx(cCard, 0.15f, 3, 1.0f, Color{ 48, 58, 80, 255 });

                    // Colored icon indicator
                    Color icColor = (ci == 18) ? Color{ 46, 204, 113, 255 } : ((ci == 9) ? Color{ 240, 90, 120, 255 } : Color{ 70, 130, 210, 255 });
                    DrawRectangle((int)cCard.x + 6, (int)cCard.y + 6, 36, 36, Color{ 14, 18, 26, 255 });
                    DrawRectangleLines((int)cCard.x + 6, (int)cCard.y + 6, 36, 36, icColor);
                    DrawCircle((int)cCard.x + 24, (int)cCard.y + 24, 10.0f, icColor);

                    DrawTextEx(fontUIBold, catObstacleNames[ci], Vector2{ cCard.x + 48.0f, cCard.y + 10.0f }, 11, 0.3f, WHITE);

                    Rectangle bSpawn = { cCard.x + 48.0f, cCard.y + 26.0f, 60.0f, 16.0f };
                    if (DrawGButton(bSpawn, "+ Spawn", Color{ 36, 90, 150, 255 }, WHITE)) {
                        LevelEntity ne{};
                        ne.id = (int)levelEntities.size();
                        ne.type = (EntityType)ci;
                        Vector3 fwd = { -sinf(editorCamYaw) * cosf(editorCamPitch), sinf(editorCamPitch), -cosf(editorCamYaw) * cosf(editorCamPitch) };
                        ne.position = Vector3Add(editorCamPos, Vector3Scale(fwd, 6.0f));
                        ne.scale = 1.0f;
                        ne.flipX = 1; ne.flipY = 1; ne.flipZ = 1;
                        AssignEntityModelAndProps(ne);
                        levelEntities.push_back(ne);
                        selectedObs = (int)levelEntities.size() - 1;
                        editorMsg = TextFormat("Spawned %s!", catObstacleNames[ci]);
                        editorMsgTimer = 2.0f;
                    }

                    cy += 54.0f;
                }
            }

            // -------------------------------------------------------------
            // D. RIGHT DOCK: GODOT INSPECTOR & PHYSICS (Width 320px)
            // -------------------------------------------------------------
            float rightDockW = 320.0f;
            Rectangle rightCollapseBtn = { inspectorCollapsed ? (screenW - 22.0f) : (screenW - rightDockW - 22.0f), 46.0f, 22.0f, 40.0f };
            if (DrawGButton(rightCollapseBtn, inspectorCollapsed ? "<" : ">", Color{ 32, 38, 52, 255 }, WHITE, true)) {
                inspectorCollapsed = !inspectorCollapsed;
            }

            if (!inspectorCollapsed) {
                Rectangle rightRec = { screenW - rightDockW, 42.0f, rightDockW, screenH - 42.0f };
                DrawRectangleRec(rightRec, Color{ 28, 33, 44, 252 });
                DrawLine((int)rightRec.x, 42, (int)rightRec.x, (int)screenH, Color{ 52, 60, 78, 255 });

                float ry = 48.0f;
                // Tab Header
                DrawTextEx(fontUIBold, "Inspector & Transform", Vector2{ rightRec.x + 14.0f, ry }, 13, 0.4f, Color{ 240, 243, 246, 255 });
                ry += 20.0f;

                // 3D Live Model Preview Viewport
                DrawRectangle((int)rightRec.x + 12, (int)ry, 296, 115, Color{ 14, 18, 24, 255 });
                DrawTextureRec(previewRT.texture, Rectangle{ 0, 0, (float)previewRT.texture.width, -(float)previewRT.texture.height }, Vector2{ rightRec.x + 12, ry }, WHITE);
                DrawRectangleLines((int)rightRec.x + 12, (int)ry, 296, 115, Color{ 60, 72, 95, 255 });
                ry += 122.0f;

                if (selectedObs >= 0 && selectedObs < (int)levelEntities.size()) {
                    auto& o = levelEntities[selectedObs];
                    DrawTextEx(fontUIBold, TextFormat("Selected: #%d (%s)", selectedObs, ENTITY_TYPE_NAMES[o.type]), Vector2{ rightRec.x + 14, ry }, 12, 0.4f, Color{ 142, 202, 230, 255 });
                    ry += 20.0f;

                    // Type switcher
                    Rectangle btnPrev = { rightRec.x + 12, ry, 140, 22 };
                    Rectangle btnNext = { rightRec.x + 164, ry, 140, 22 };
                    if (DrawGButton(btnPrev, "< Prev Type", Color{ 36, 43, 58, 255 }, WHITE, true)) {
                        o.type = (EntityType)(((int)o.type - 1 + ENT_COUNT) % ENT_COUNT);
                        AssignEntityModelAndProps(o);
                    }
                    if (DrawGButton(btnNext, "Next Type >", Color{ 36, 43, 58, 255 }, WHITE, true)) {
                        o.type = (EntityType)(((int)o.type + 1) % ENT_COUNT);
                        AssignEntityModelAndProps(o);
                    }
                    ry += 26.0f;

                    // Position Controls
                    DrawTextEx(fontUI, TextFormat("Position: (%.2f, %.2f, %.2f)", o.position.x, o.position.y, o.position.z), Vector2{ rightRec.x + 14, ry }, 11, 0.3f, Color{ 220, 225, 235, 255 });
                    ry += 15.0f;
                    Rectangle bXm = { rightRec.x + 12, ry, 68, 20 };
                    Rectangle bXp = { rightRec.x + 84, ry, 68, 20 };
                    Rectangle bZm = { rightRec.x + 158, ry, 68, 20 };
                    Rectangle bZp = { rightRec.x + 230, ry, 68, 20 };
                    if (DrawGButton(bXm, "X -", Color{ 36, 43, 58, 255 }, WHITE)) o.position.x -= 0.5f;
                    if (DrawGButton(bXp, "X +", Color{ 36, 43, 58, 255 }, WHITE)) o.position.x += 0.5f;
                    if (DrawGButton(bZm, "Z -", Color{ 36, 43, 58, 255 }, WHITE)) o.position.z -= 0.5f;
                    if (DrawGButton(bZp, "Z +", Color{ 36, 43, 58, 255 }, WHITE)) o.position.z += 0.5f;
                    ry += 24.0f;

                    Rectangle bYm = { rightRec.x + 12, ry, 68, 20 };
                    Rectangle bYp = { rightRec.x + 84, ry, 68, 20 };
                    Rectangle bRm = { rightRec.x + 158, ry, 68, 20 };
                    Rectangle bRp = { rightRec.x + 230, ry, 68, 20 };
                    if (DrawGButton(bYm, "Y -", Color{ 36, 43, 58, 255 }, WHITE)) o.position.y -= 0.25f;
                    if (DrawGButton(bYp, "Y +", Color{ 36, 43, 58, 255 }, WHITE)) o.position.y += 0.25f;
                    if (DrawGButton(bRm, "Rot -15", Color{ 36, 43, 58, 255 }, WHITE)) o.rotationY -= 15.0f;
                    if (DrawGButton(bRp, "Rot +15", Color{ 36, 43, 58, 255 }, WHITE)) o.rotationY += 15.0f;
                    ry += 26.0f;

                    // Scale & Size
                    DrawTextEx(fontUI, TextFormat("Scale: %.2fx | Size: (%.1f, %.1f, %.1f)", o.scale, o.sizeX, o.sizeY, o.sizeZ), Vector2{ rightRec.x + 14, ry }, 11, 0.3f, Color{ 220, 225, 235, 255 });
                    ry += 15.0f;
                    Rectangle bScM = { rightRec.x + 12, ry, 88, 20 };
                    Rectangle bScP = { rightRec.x + 104, ry, 88, 20 };
                    Rectangle bScR = { rightRec.x + 196, ry, 102, 20 };
                    if (DrawGButton(bScM, "Scale -0.1", Color{ 36, 43, 58, 255 }, WHITE)) { o.scale = Clamp(o.scale - 0.1f, 0.2f, 4.0f); AssignEntityModelAndProps(o); }
                    if (DrawGButton(bScP, "Scale +0.1", Color{ 36, 43, 58, 255 }, WHITE)) { o.scale = Clamp(o.scale + 0.1f, 0.2f, 4.0f); AssignEntityModelAndProps(o); }
                    if (DrawGButton(bScR, "Reset 1.0x", Color{ 36, 43, 58, 255 }, WHITE)) { o.scale = 1.0f; AssignEntityModelAndProps(o); }
                    ry += 24.0f;

                    // Instant Flip buttons in Inspector
                    Rectangle bIfX = { rightRec.x + 12, ry, 88, 20 };
                    Rectangle bIfY = { rightRec.x + 104, ry, 88, 20 };
                    Rectangle bIfZ = { rightRec.x + 196, ry, 102, 20 };
                    if (DrawGButton(bIfX, TextFormat("Flip X (%d)", o.flipX), Color{ 140, 45, 45, 255 }, WHITE)) { o.flipX *= -1; o.rotationY = fmodf(-o.rotationY + 360.0f, 360.0f); }
                    if (DrawGButton(bIfY, TextFormat("Flip Y (%d)", o.flipY), Color{ 40, 130, 50, 255 }, WHITE)) { o.flipY *= -1; o.rotationZ = fmodf(o.rotationZ + 180.0f, 360.0f); }
                    if (DrawGButton(bIfZ, TextFormat("Flip Z (%d)", o.flipZ), Color{ 40, 80, 160, 255 }, WHITE)) { o.flipZ *= -1; o.rotationY = fmodf(o.rotationY + 180.0f, 360.0f); }
                    ry += 26.0f;

                    // Duplicate and Delete
                    Rectangle bDup = { rightRec.x + 12, ry, 140, 22 };
                    Rectangle bDel = { rightRec.x + 164, ry, 140, 22 };
                    if (DrawGButton(bDup, "Duplicate (Ctrl+D)", Color{ 48, 85, 120, 255 }, WHITE, true) || (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_D))) {
                        LevelEntity dup = o;
                        dup.id = (int)levelEntities.size();
                        dup.position.x += 1.5f;
                        levelEntities.push_back(dup);
                        selectedObs = (int)levelEntities.size() - 1;
                        editorMsg = "Entity Duplicated!";
                        editorMsgTimer = 2.0f;
                    }
                    if (DrawGButton(bDel, "Delete (Del)", Color{ 145, 45, 45, 255 }, WHITE, true) || IsKeyPressed(KEY_DELETE)) {
                        levelEntities.erase(levelEntities.begin() + selectedObs);
                        selectedObs = -1;
                        editorMsg = "Entity Deleted!";
                        editorMsgTimer = 2.0f;
                    }
                    ry += 28.0f;
                } else {
                    DrawRectangleRec(Rectangle{ rightRec.x + 12, ry, rightDockW - 24, 60 }, Color{ 22, 26, 34, 255 });
                    DrawTextEx(fontUI, "No entity selected.", Vector2{ rightRec.x + 24, ry + 12 }, 12, 0.3f, Color{ 200, 205, 215, 255 });
                    DrawTextEx(fontUI, "Click in 3D or pick from Scene Hierarchy.", Vector2{ rightRec.x + 24, ry + 32 }, 11, 0.3f, Color{ 140, 150, 165, 255 });
                    ry += 68.0f;
                }

                DrawLine((int)rightRec.x + 10, (int)ry, (int)screenW - 10, (int)ry, Color{ 52, 60, 78, 255 });
                ry += 10.0f;

                // Gas Propulsion Physics Section (Saved Forever)
                DrawTextEx(fontUIBold, "Gas Propulsion Physics (JSON):", Vector2{ rightRec.x + 14, ry }, 12, 0.4f, Color{ 255, 215, 0, 255 });
                ry += 16.0f;
                DrawTextEx(fontUI, TextFormat("Tap: %.0f N | Hold: %.0f N", gPhysCfg.tapBurstForce, gPhysCfg.sustainedMaxForce), Vector2{ rightRec.x + 14, ry }, 11, 0.3f, Color{ 220, 225, 235, 255 });
                ry += 14.0f;
                Rectangle bTpM = { rightRec.x + 12, ry, 68, 20 };
                Rectangle bTpP = { rightRec.x + 84, ry, 68, 20 };
                Rectangle bHdM = { rightRec.x + 158, ry, 68, 20 };
                Rectangle bHdP = { rightRec.x + 230, ry, 68, 20 };
                if (DrawGButton(bTpM, "Tap -50", Color{ 36, 43, 58, 255 }, WHITE)) gPhysCfg.tapBurstForce = Clamp(gPhysCfg.tapBurstForce - 50.0f, 100.0f, 3000.0f);
                if (DrawGButton(bTpP, "Tap +50", Color{ 36, 43, 58, 255 }, WHITE)) gPhysCfg.tapBurstForce = Clamp(gPhysCfg.tapBurstForce + 50.0f, 100.0f, 3000.0f);
                if (DrawGButton(bHdM, "Hold -50", Color{ 36, 43, 58, 255 }, WHITE)) gPhysCfg.sustainedMaxForce = Clamp(gPhysCfg.sustainedMaxForce - 50.0f, 100.0f, 3000.0f);
                if (DrawGButton(bHdP, "Hold +50", Color{ 36, 43, 58, 255 }, WHITE)) gPhysCfg.sustainedMaxForce = Clamp(gPhysCfg.sustainedMaxForce + 50.0f, 100.0f, 3000.0f);
                ry += 24.0f;

                DrawTextEx(fontUI, TextFormat("Steer Torque: %.1f | Max Speed: %.1f", gPhysCfg.steerTorque, gPhysCfg.boatMaxSpeed), Vector2{ rightRec.x + 14, ry }, 11, 0.3f, Color{ 220, 225, 235, 255 });
                ry += 14.0f;
                Rectangle bTqM = { rightRec.x + 12, ry, 68, 20 };
                Rectangle bTqP = { rightRec.x + 84, ry, 68, 20 };
                Rectangle bSpM = { rightRec.x + 158, ry, 68, 20 };
                Rectangle bSpP = { rightRec.x + 230, ry, 68, 20 };
                if (DrawGButton(bTqM, "Tq -0.5", Color{ 36, 43, 58, 255 }, WHITE)) gPhysCfg.steerTorque = Clamp(gPhysCfg.steerTorque - 0.5f, 1.0f, 25.0f);
                if (DrawGButton(bTqP, "Tq +0.5", Color{ 36, 43, 58, 255 }, WHITE)) gPhysCfg.steerTorque = Clamp(gPhysCfg.steerTorque + 0.5f, 1.0f, 25.0f);
                if (DrawGButton(bSpM, "Spd -1", Color{ 36, 43, 58, 255 }, WHITE)) gPhysCfg.boatMaxSpeed = Clamp(gPhysCfg.boatMaxSpeed - 1.0f, 5.0f, 40.0f);
                if (DrawGButton(bSpP, "Spd +1", Color{ 36, 43, 58, 255 }, WHITE)) gPhysCfg.boatMaxSpeed = Clamp(gPhysCfg.boatMaxSpeed + 1.0f, 5.0f, 40.0f);
                ry += 28.0f;

                Rectangle btnSaveJSON = { rightRec.x + 12, ry, rightDockW - 24, 28 };
                if (DrawGButton(btnSaveJSON, "Save Level & Physics (Ctrl+S)", Color{ 34, 139, 34, 255 }, WHITE, true)) {
                    SaveLevelLayoutJSON(LEVEL_LAYOUT_PATH, levelEntities);
                    SavePhysicsConfigJSON("physics_config.json");
                    editorMsg = "Saved Layout & Physics to JSON!";
                    editorMsgTimer = 3.0f;
                }
            }

            // Toast Message
            if (editorMsgTimer > 0.0f) {
                editorMsgTimer -= dt;
                Vector2 msgSz = MeasureTextEx(fontUIBold, editorMsg.c_str(), 14, 0.4f);
                int toastW = (int)msgSz.x + 40;
                int toastH = 34;
                int toastX = GetScreenWidth() / 2 - toastW / 2;
                int toastY = 56;
                DrawRectangleRounded(Rectangle{ (float)toastX, (float)toastY, (float)toastW, (float)toastH }, 0.35f, 4, Color{ 25, 30, 42, 245 });
                DrawRectangleRoundedLinesEx(Rectangle{ (float)toastX, (float)toastY, (float)toastW, (float)toastH }, 0.35f, 4, 1.2f, Color{ 71, 140, 191, 255 });
                DrawTextEx(fontUIBold, editorMsg.c_str(), Vector2{ (float)toastX + 20, (float)toastY + 9 }, 14, 0.4f, WHITE);
            }
        }
        } else if (currentScreen == SCREEN_MAIN_MENU) {
            ClearBackground(Color{ 18, 22, 34, 255 });
            if (texMenuBg.id != 0) {
                DrawTexturePro(texMenuBg, Rectangle{ 0, 0, (float)texMenuBg.width, (float)texMenuBg.height },
                               Rectangle{ 0, 0, (float)GetScreenWidth(), (float)GetScreenHeight() }, Vector2{0, 0}, 0.0f, WHITE);
            }
            DrawRectangleGradientV(0, 0, GetScreenWidth(), GetScreenHeight(), Color{ 10, 15, 28, 140 }, Color{ 5, 8, 18, 210 });

            float logoBob = sinf(runTime * 2.4f) * 10.0f;
            float logoW = 620.0f;
            float logoH = 260.0f;
            if (texLogoSplash.id != 0) {
                float aspect = (float)texLogoSplash.width / (float)texLogoSplash.height;
                logoH = 250.0f;
                logoW = logoH * aspect;
                float logoX = ((float)GetScreenWidth() - logoW) * 0.5f;
                float logoY = 60.0f + logoBob;
                DrawTexturePro(texLogoSplash, Rectangle{ 0, 0, (float)texLogoSplash.width, (float)texLogoSplash.height },
                               Rectangle{ logoX, logoY, logoW, logoH }, Vector2{0, 0}, 0.0f, WHITE);
            } else {
                Vector2 lsz = MeasureTextEx(fontUIBold, "FART TOGETHER", 48, 1.0f);
                DrawTextEx(fontUIBold, "FART TOGETHER", Vector2{ ((float)GetScreenWidth() - lsz.x) * 0.5f, 100.0f + logoBob }, 48, 1.0f, Color{ 255, 215, 0, 255 });
            }

            float cardW = 460.0f;
            float cardH = 360.0f;
            float cardX = ((float)GetScreenWidth() - cardW) * 0.5f;
            float cardY = 380.0f;
            Rectangle menuCard = { cardX, cardY, cardW, cardH };
            DrawRectangleRounded(menuCard, 0.12f, 4, Color{ 15, 23, 42, 235 });
            DrawRectangleRoundedLinesEx(menuCard, 0.12f, 4, 1.8f, Color{ 59, 130, 246, 180 });

            float by = cardY + 28.0f;
            Rectangle btnSolo = { cardX + 24.0f, by, cardW - 48.0f, 54.0f };
            if (DrawMenuBtn(btnSolo, "PLAY SOLO FART RUN (1P)", Color{ 37, 99, 235, 255 }, WHITE, 18, true)) {
                singlePirateMode = true;
                currentScreen = SCREEN_GAMEPLAY;
                p1RimAngle = -PI;
            }
            by += 68.0f;

            Rectangle btnLan = { cardX + 24.0f, by, cardW - 48.0f, 54.0f };
            if (DrawMenuBtn(btnLan, "LAN MULTIPLAYER (CO-OP)", Color{ 16, 149, 193, 255 }, WHITE, 18, true)) {
                currentScreen = SCREEN_LAN_LOBBY;
            }
            by += 68.0f;

            Rectangle btnSett = { cardX + 24.0f, by, cardW - 48.0f, 48.0f };
            if (DrawMenuBtn(btnSett, "SETTINGS & AUDIO", Color{ 51, 65, 85, 255 }, WHITE, 16, true)) {
                currentScreen = SCREEN_SETTINGS;
            }
            by += 60.0f;

            Rectangle btnQuit = { cardX + 24.0f, by, cardW - 48.0f, 48.0f };
            if (DrawMenuBtn(btnQuit, "EXIT GAME", Color{ 127, 29, 29, 255 }, WHITE, 16, true)) {
                break;
            }

            const char* footerText = "Authentic 1:1 C++ Native Engine  |  Flat Water Buoyancy  |  360 deg Solo Steering";
            Vector2 fsz = MeasureTextEx(fontUI, footerText, 12, 0.3f);
            DrawTextEx(fontUI, footerText, Vector2{ ((float)GetScreenWidth() - fsz.x) * 0.5f, (float)GetScreenHeight() - 32.0f }, 12, 0.3f, Color{ 148, 163, 184, 200 });

        } else if (currentScreen == SCREEN_LAN_LOBBY) {
            ClearBackground(Color{ 18, 22, 34, 255 });
            if (texMenuBg.id != 0) {
                DrawTexturePro(texMenuBg, Rectangle{ 0, 0, (float)texMenuBg.width, (float)texMenuBg.height },
                               Rectangle{ 0, 0, (float)GetScreenWidth(), (float)GetScreenHeight() }, Vector2{0, 0}, 0.0f, WHITE);
            }
            DrawRectangleGradientV(0, 0, GetScreenWidth(), GetScreenHeight(), Color{ 10, 15, 28, 160 }, Color{ 5, 8, 18, 230 });

            float lobbyW = 860.0f;
            float lobbyH = 680.0f;
            float lobbyX = ((float)GetScreenWidth() - lobbyW) * 0.5f;
            float lobbyY = 80.0f;
            Rectangle lobbyCard = { lobbyX, lobbyY, lobbyW, lobbyH };
            DrawRectangleRounded(lobbyCard, 0.08f, 4, Color{ 15, 23, 42, 240 });
            DrawRectangleRoundedLinesEx(lobbyCard, 0.08f, 4, 1.8f, Color{ 16, 149, 193, 200 });

            DrawTextEx(fontUIBold, "LAN MULTIPLAYER CO-OP LOBBY", Vector2{ lobbyX + 32.0f, lobbyY + 24.0f }, 24, 0.5f, Color{ 255, 215, 0, 255 });
            DrawTextEx(fontUI, "Choose your Hippo Pirate and sync across Local Area Network", Vector2{ lobbyX + 34.0f, lobbyY + 56.0f }, 13, 0.3f, Color{ 148, 163, 184, 255 });

            float charW = 180.0f;
            float charH = 220.0f;
            float charGap = 16.0f;
            float charStartX = lobbyX + (lobbyW - (4 * charW + 3 * charGap)) * 0.5f;
            float charY = lobbyY + 95.0f;

            const char* charNames[4] = { "Chunky Green", "Fiery Orange", "Speedy Blue", "Smoky Grey" };
            for (int k = 0; k < 4; k++) {
                Rectangle cRec = { charStartX + k * (charW + charGap), charY, charW, charH };
                bool hov = CheckCollisionPointRec(mousePos, cRec);
                bool sel = (selectedLobbyChar == k);
                Color cFill = sel ? Color{ 30, 58, 95, 255 } : (hov ? Color{ 25, 35, 55, 255 } : Color{ 18, 26, 42, 240 });
                Color cBorder = sel ? Color{ 255, 215, 0, 255 } : (hov ? Color{ 59, 130, 246, 255 } : Color{ 51, 65, 85, 200 });

                DrawRectangleRounded(cRec, 0.12f, 4, cFill);
                DrawRectangleRoundedLinesEx(cRec, 0.12f, 4, sel ? 2.5f : 1.2f, cBorder);

                if (texLobbyChars[k].id != 0) {
                    float sprW = 140.0f;
                    float sprH = 140.0f;
                    DrawTexturePro(texLobbyChars[k], Rectangle{ 0, 0, (float)texLobbyChars[k].width, (float)texLobbyChars[k].height },
                                   Rectangle{ cRec.x + (charW - sprW) * 0.5f, cRec.y + 12.0f, sprW, sprH }, Vector2{0, 0}, 0.0f, WHITE);
                }
                Vector2 nsz = MeasureTextEx(fontUIBold, charNames[k], 14, 0.3f);
                DrawTextEx(fontUIBold, charNames[k], Vector2{ cRec.x + (charW - nsz.x) * 0.5f, cRec.y + 165.0f }, 14, 0.3f, sel ? Color{ 255, 215, 0, 255 } : WHITE);
                const char* stLabel = sel ? "V SELECTED" : "SELECT";
                Vector2 ssz = MeasureTextEx(fontUI, stLabel, 11, 0.2f);
                DrawTextEx(fontUI, stLabel, Vector2{ cRec.x + (charW - ssz.x) * 0.5f, cRec.y + 190.0f }, 11, 0.2f, sel ? Color{ 74, 222, 128, 255 } : Color{ 148, 163, 184, 200 });

                if (hov && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    selectedLobbyChar = k;
                }
            }

            float netY = charY + charH + 30.0f;
            DrawTextEx(fontUIBold, "LAN Network Configuration:", Vector2{ lobbyX + 34.0f, netY }, 15, 0.4f, WHITE);
            netY += 25.0f;

            Rectangle btnHost = { lobbyX + 34.0f, netY, 260.0f, 44.0f };
            Rectangle btnJoin = { lobbyX + 310.0f, netY, 260.0f, 44.0f };
            bool isHost = (LanGetRole() == LAN_HOST);
            bool isClient = (LanGetRole() == LAN_CLIENT);

            if (DrawMenuBtn(btnHost, isHost ? "V HOSTING ROOM (Port 7777)" : "HOST LAN ROOM (Port 7777)",
                            isHost ? Color{ 22, 101, 52, 255 } : Color{ 30, 41, 59, 255 }, WHITE, 13, true)) {
                if (isHost) LanShutdown(); else LanInitHost(7777);
            }

            if (DrawMenuBtn(btnJoin, isClient ? "V CONNECTED AS CLIENT" : "JOIN LOCAL HOST (7777)",
                            isClient ? Color{ 22, 101, 52, 255 } : Color{ 30, 41, 59, 255 }, WHITE, 13, true)) {
                if (isClient) LanShutdown(); else LanInitClient(7777);
            }

            Rectangle statusRec = { lobbyX + 590.0f, netY, lobbyW - 624.0f, 44.0f };
            DrawRectangleRounded(statusRec, 0.2f, 4, Color{ 18, 26, 42, 255 });
            DrawRectangleRoundedLinesEx(statusRec, 0.2f, 4, 1.0f, Color{ 51, 65, 85, 200 });
            const char* roleStr = isHost ? "ROLE: HOST" : (isClient ? "ROLE: CLIENT" : "ROLE: OFFLINE");
            DrawTextEx(fontUIBold, roleStr, Vector2{ statusRec.x + 14.0f, statusRec.y + 8.0f }, 12, 0.3f, isHost ? Color{ 74, 222, 128, 255 } : (isClient ? Color{ 56, 189, 248, 255 } : Color{ 148, 163, 184, 255 }));
            DrawTextEx(fontUI, "Status: Ready to Sync", Vector2{ statusRec.x + 14.0f, statusRec.y + 24.0f }, 11, 0.2f, Color{ 200, 210, 225, 255 });

            float actY = lobbyY + lobbyH - 75.0f;
            Rectangle btnBack = { lobbyX + 34.0f, actY, 220.0f, 48.0f };
            if (DrawMenuBtn(btnBack, "<- BACK TO MENU", Color{ 51, 65, 85, 255 }, WHITE, 14, true)) {
                currentScreen = SCREEN_MAIN_MENU;
            }

            Rectangle btnStartLan = { lobbyX + lobbyW - 320.0f, actY, 286.0f, 48.0f };
            if (DrawMenuBtn(btnStartLan, "START MULTIPLAYER MATCH", Color{ 37, 99, 235, 255 }, WHITE, 14, true)) {
                singlePirateMode = false;
                currentScreen = SCREEN_GAMEPLAY;
            }

        } else if (currentScreen == SCREEN_SETTINGS) {
            ClearBackground(Color{ 18, 22, 34, 255 });
            if (texMenuBg.id != 0) {
                DrawTexturePro(texMenuBg, Rectangle{ 0, 0, (float)texMenuBg.width, (float)texMenuBg.height },
                               Rectangle{ 0, 0, (float)GetScreenWidth(), (float)GetScreenHeight() }, Vector2{0, 0}, 0.0f, WHITE);
            }
            DrawRectangleGradientV(0, 0, GetScreenWidth(), GetScreenHeight(), Color{ 10, 15, 28, 160 }, Color{ 5, 8, 18, 230 });

            float settW = 640.0f;
            float settH = 580.0f;
            float settX = ((float)GetScreenWidth() - settW) * 0.5f;
            float settY = 120.0f;
            Rectangle settCard = { settX, settY, settW, settH };
            DrawRectangleRounded(settCard, 0.08f, 4, Color{ 15, 23, 42, 240 });
            DrawRectangleRoundedLinesEx(settCard, 0.08f, 4, 1.8f, Color{ 71, 140, 191, 200 });

            DrawTextEx(fontUIBold, "SETTINGS & AUDIO", Vector2{ settX + 32.0f, settY + 28.0f }, 22, 0.5f, Color{ 255, 215, 0, 255 });
            DrawTextEx(fontUI, "Configure audio levels, display and engine physics defaults", Vector2{ settX + 34.0f, settY + 58.0f }, 13, 0.3f, Color{ 148, 163, 184, 255 });

            float sy = settY + 105.0f;
            DrawTextEx(fontUIBold, "Audio Settings:", Vector2{ settX + 34.0f, sy }, 15, 0.4f, WHITE);
            sy += 28.0f;

            DrawTextEx(fontUI, TextFormat("Master Volume: %d%%", (int)(masterVolume * 100.0f)), Vector2{ settX + 34.0f, sy + 6.0f }, 14, 0.3f, Color{ 220, 225, 235, 255 });
            Rectangle btnVolM = { settX + 380.0f, sy, 80.0f, 32.0f };
            Rectangle btnVolP = { settX + 470.0f, sy, 80.0f, 32.0f };
            if (DrawMenuBtn(btnVolM, "-10%", Color{ 36, 43, 58, 255 }, WHITE, 13, true)) {
                masterVolume = Clamp(masterVolume - 0.10f, 0.0f, 1.0f);
                SetMasterVolume(masterVolume);
            }
            if (DrawMenuBtn(btnVolP, "+10%", Color{ 36, 43, 58, 255 }, WHITE, 13, true)) {
                masterVolume = Clamp(masterVolume + 0.10f, 0.0f, 1.0f);
                SetMasterVolume(masterVolume);
            }
            sy += 50.0f;

            DrawTextEx(fontUIBold, "Engine & Physics Features:", Vector2{ settX + 34.0f, sy }, 15, 0.4f, WHITE);
            sy += 28.0f;

            DrawTextEx(fontUI, "Water Buoyancy: 100% Flat (Roll=0, Pitch=0) [Active]", Vector2{ settX + 34.0f, sy }, 13, 0.3f, Color{ 74, 222, 128, 255 });
            sy += 26.0f;
            DrawTextEx(fontUI, "Obstacle Collision: Non-fatal Rocks & Rebound Pillar [Active]", Vector2{ settX + 34.0f, sy }, 13, 0.3f, Color{ 74, 222, 128, 255 });
            sy += 26.0f;
            DrawTextEx(fontUI, "Solo Mode Steering: 360 deg Free Rim Rotation [Active]", Vector2{ settX + 34.0f, sy }, 13, 0.3f, Color{ 74, 222, 128, 255 });
            sy += 26.0f;
            DrawTextEx(fontUI, "Fart Gas Propulsion: Persistent physics_config.json [Active]", Vector2{ settX + 34.0f, sy }, 13, 0.3f, Color{ 74, 222, 128, 255 });

            float actY = settY + settH - 70.0f;
            Rectangle btnBackMenu = { settX + 34.0f, actY, 200.0f, 44.0f };
            if (DrawMenuBtn(btnBackMenu, "<- BACK TO MENU", Color{ 51, 65, 85, 255 }, WHITE, 14, true)) {
                currentScreen = SCREEN_MAIN_MENU;
            }
        }

        EndDrawing();

        testFrameCount++;

        if (testScreenshotMode && testFrameCount >= targetTestFrame) {

            TakeScreenshot(testShotName.c_str());

            break;

        }

    }

    // Cleanup

    for (int s = 0; s < 6; s++) UnloadSound(fartSounds[s]);
    for (int s = 0; s < 3; s++) UnloadSound(sndTap[s]);
    for (int s = 0; s < 2; s++) UnloadSound(sndHold[s]);
    UnloadSound(sndExplode);
    UnloadSound(sndBounce);
    UnloadTexture(texLogoSplash);
    UnloadTexture(texMenuBg);
    UnloadTexture(texModeFartRun);
    for (int k = 0; k < 4; k++) UnloadTexture(texLobbyChars[k]);

    for (int f = 0; f < 4; f++) UnloadTexture(texFart[f]);

    UnloadTexture(texPalette);

    UnloadTexture(texGradientPalette);

    UnloadTexture(texCaustic);
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
    UnloadModel(wallColModel);
    UnloadTexture(texSkullGas);
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
    UnloadModel(gObsModels.mTree);

    if (fontUI.texture.id != GetFontDefault().texture.id) UnloadFont(fontUI);
    if (fontUIBold.texture.id != GetFontDefault().texture.id) UnloadFont(fontUIBold);

    UnloadShader(shaderWater);
    UnloadRenderTexture(previewRT);
    UnloadShader(shaderLighting);

    CloseAudioDevice();

    CloseWindow();

    return 0;

}