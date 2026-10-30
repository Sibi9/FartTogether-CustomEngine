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

static const float BUTT_PEAK_SCALE = 1.40f;

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
uniform float uButtSwell; // 0.0 to 1.0 (Gradual butt inflation)
uniform float uEyeBulge;  // 0.0 to 1.0 (Comedic bulging cartoon eyes)
out vec2 fragTexCoord;
out vec4 fragColor;
out vec3 fragNormal;
out vec3 fragPosition;

void main() {
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;
    fragNormal = vertexNormal;
    fragPosition = vertexPosition;

    vec3 pos = vertexPosition;

    // A. Gradual Butt Swell & Balloon Inflation (Local rear butt region)
    if (pos.z < 0.02 && pos.y > 0.20 && pos.y < 0.58 && uButtSwell > 0.005) {
        vec3 buttPivot = vec3(0.0, 0.38, -0.05);
        vec3 dButt = pos - buttPivot;
        float swellMask = clamp((0.02 - pos.z) / 0.18, 0.0, 1.0) * (1.0 - abs(pos.y - 0.38) / 0.22);
        swellMask = clamp(swellMask, 0.0, 1.0);
        pos.z -= uButtSwell * 0.32 * swellMask;
        pos.x += sign(pos.x) * abs(dButt.x) * uButtSwell * 0.45 * swellMask;
        pos.y += dButt.y * uButtSwell * 0.35 * swellMask;
    }

    // B. Comedic Bulging Eyes (Eyeballs have UV.x > 0.65 and sit at |X| <= 0.075, Y in [0.89, 0.97], Z > 0.14)
    if ((vertexTexCoord.x > 0.65 || (pos.y > 0.89 && pos.y < 0.97 && pos.z > 0.14 && abs(pos.x) < 0.08)) && uEyeBulge > 0.005) {
        vec3 eyeSocket = vec3(sign(pos.x) * 0.038, 0.930, 0.175);
        vec3 relEye = pos - eyeSocket;
        pos = eyeSocket + relEye * (1.0 + uEyeBulge * 1.45) + vec3(0.0, uEyeBulge * 0.03, uEyeBulge * 0.18);
    }

    gl_Position = mvp * vec4(pos, 1.0);
}
)";

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
uniform float uRedness;     // 0.0 = normal, 1.0 = deep scarlet vascular redness
uniform int uIsJungleTree;  // 1 = procedural two-tone brown trunk + lush green canopy
out vec4 finalColor;

void main() {
    vec4 texColor = texture(texture0, fragTexCoord);
    vec3 baseRgb = texColor.rgb;

    if (uIsJungleTree == 1) {
        if (fragPosition.y < 3.05) {
            // Rich warm bark brown trunk
            baseRgb = mix(vec3(0.46, 0.28, 0.14), vec3(0.60, 0.39, 0.21), clamp(fragPosition.y / 3.05, 0.0, 1.0));
        } else {
            // Vibrant layered tropical green foliage
            float hFactor = clamp((fragPosition.y - 3.05) / 4.2, 0.0, 1.0);
            baseRgb = mix(vec3(0.13, 0.50, 0.20), vec3(0.29, 0.76, 0.33), hFactor);
        }
    }

    vec3 N = normalize(fragNormal);
    vec3 L = normalize(lightDir);
    float diff = max(dot(N, L), 0.0);
    float celDiff = smoothstep(0.18, 0.58, diff) * 0.65 + diff * 0.35;
    // Normalized lighting prevents over-exposure washout!
    vec3 lighting = mix(ambientColor, lightColor, celDiff);

    // Localized Butt Redness Mask (Center: 0.0, 0.39, -0.12)
    vec3 buttCenter = vec3(0.0, 0.39, -0.12);
    vec3 buttScale  = vec3(0.35, 0.26, 0.24);
    float buttDist  = length((fragPosition - buttCenter) / buttScale);
    float buttMask  = clamp(1.0 - smoothstep(0.35, 1.15, buttDist), 0.0, 1.0);

    vec3 redSkin = mix(baseRgb, vec3(0.98, 0.08, 0.12), uRedness * buttMask * 0.92);
    vec3 litColor = redSkin * lighting * colDiffuse.rgb;
    finalColor = vec4(litColor, texColor.a * colDiffuse.a);
}
)";

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

        // Gradual Butt Bone Swell (1.0x to 1.45x starting immediately)
        boneScale = 1.0f + 0.45f * visualCharge;

        // Tremble vibration at 12Hz above 25% charge
        if (visualCharge > 0.25f) {
            float strength = (visualCharge - 0.25f) / 0.75f;
            trembleOffset = sinf(2.0f * PI * BUTT_TREMBLE_FREQ * runTime) * BUTT_TREMBLE_AMP * strength;
        } else {
            trembleOffset = 0.0f;
        }

        // Smooth Gradual Vascular Redness (starts at 5% charge and ramps smoothly to 100%!)
        redness = Clamp((visualCharge - 0.05f) / 0.80f, 0.0f, 1.0f);

    }

};

// Unified Level Entity Architecture (Obstacles, Rocks, Trees, Checkpoints, Props)
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
    ENT_TREE_JUNGLE = 18, // GM_3D_JungleTree.obj (Authentic two-tone palm tree)
    ENT_CHECKPOINT = 19,  // GM_3D_CheckPointV1.obj
    ENT_COUNT = 20
};

static const char* ENTITY_TYPE_NAMES[] = {
    "OBS_A1_BUMPER", "OBS_B2_TIPPING", "OBS_B3_SPINNER", "OBS_B5_ROTOR",
    "OBS_C1_ARC", "OBS_C2_PENDULUM", "OBS_C4_QUAD", "OBS_C5_WHEEL",
    "OBS_F3_SWING", "OBS_F10_GATE", "OBS_F11_DRUM", "OBS_F13_ARM",
    "OBS_F14_SLIDER", "OBS_F18_SPIKE", "OBS_F23_DUAL",
    "ROCK_DARK_1", "ROCK_DARK_2", "WIND_GUST",
    "TREE_JUNGLE", "CHECKPOINT_GATE"
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

static bool CheckObstacleOBBCollision(Vector3 raftPos, const LevelEntity& ent, Vector3 animPos, float animYawDeg, float animRollDeg) {
    if (ent.type >= ENT_TREE_JUNGLE || ent.type == ENT_GUST) return false;
    // Check vertical clearance first
    float dy = fabsf((raftPos.y + 0.35f) - animPos.y);
    if (ent.type == ENT_OBS_C5) {
        if (dy > 4.6f * ent.scale) return false;
    } else if (dy > 2.2f * ent.scale) {
        return false;
    }

    // Transform boat position into obstacle's local horizontal space
    float rad = -animYawDeg * DEG2RAD;
    float dx = raftPos.x - animPos.x;
    float dz = raftPos.z - animPos.z;
    float localX = dx * cosf(rad) - dz * sinf(rad);
    float localZ = dx * sinf(rad) + dz * cosf(rad);

    const float boatR = 0.88f;
    float hx = 1.0f * ent.scale, hz = 1.0f * ent.scale;
    switch (ent.type) {
        case ENT_OBS_A1:  hx = 0.38f * ent.scale; hz = 2.35f * ent.scale; break; // Narrow wall pad!
        case ENT_OBS_B2:  hx = 4.80f * ent.scale; hz = 0.45f * ent.scale; break;
        case ENT_OBS_B3:  hx = 2.45f * ent.scale; hz = 0.48f * ent.scale; break;
        case ENT_OBS_B5:  hx = 4.90f * ent.scale; hz = 0.50f * ent.scale; break;
        case ENT_OBS_C1:  hx = 2.50f * ent.scale; hz = 0.50f * ent.scale; break;
        case ENT_OBS_C2:  hx = 1.15f * ent.scale; hz = 1.15f * ent.scale; break;
        case ENT_OBS_C4:  hx = 2.50f * ent.scale; hz = 1.25f * ent.scale; break;
        case ENT_OBS_C5:  hx = 4.30f * ent.scale; hz = 0.55f * ent.scale; break;
        case ENT_OBS_F3:  hx = 2.30f * ent.scale; hz = 0.38f * ent.scale; break;
        case ENT_OBS_F10: hx = 4.50f * ent.scale; hz = 0.55f * ent.scale; break;
        case ENT_OBS_F11: hx = 0.95f * ent.scale; hz = 0.95f * ent.scale; break;
        case ENT_OBS_F13: hx = 2.80f * ent.scale; hz = 0.50f * ent.scale; break;
        case ENT_OBS_F14: hx = 1.30f * ent.scale; hz = 2.20f * ent.scale; break;
        case ENT_OBS_F18: hx = 0.38f * ent.scale; hz = 0.38f * ent.scale; break;
        case ENT_OBS_F23: hx = 2.40f * ent.scale; hz = 0.40f * ent.scale; break;
        case ENT_ROCK_1:  hx = 1.45f * ent.scale; hz = 1.45f * ent.scale; break;
        case ENT_ROCK_2:  hx = 1.65f * ent.scale; hz = 1.65f * ent.scale; break;
        default: break;
    }
    return (fabsf(localX) < (hx + boatR)) && (fabsf(localZ) < (hz + boatR));
}

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
        case ENT_GUST:        e.model = nullptr;          e.radius = 4.50f * e.scale; break;
        case ENT_TREE_JUNGLE:  e.model = &gObsModels.mTree; e.radius = 1.35f * e.scale; break;
        case ENT_CHECKPOINT:   e.model = &gObsModels.mCP;   e.radius = 3.50f * e.scale; break;
        default:               e.type = ENT_OBS_A1; e.model = &gObsModels.mA1; e.radius = 1.85f * e.scale; break;
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
            "    \"rotY\": %.2f,\n"
            "    \"scale\": %.2f,\n"
            "    \"speed\": %.2f,\n"
            "    \"radius\": %.2f\n"
            "  }%s\n",
            (int)i, (int)e.type, e.position.x, e.position.y, e.position.z,
            e.rotationY, e.scale, e.animSpeed, e.radius,
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
            float x = 0.0f, y = 0.0f, z = 0.0f, rotY = 0.0f, scale = 1.0f, spd = 1.0f, rad = 2.0f;
            ParseJsonFloatField(objStart, objEnd, "\"x\"", x);
            ParseJsonFloatField(objStart, objEnd, "\"y\"", y);
            ParseJsonFloatField(objStart, objEnd, "\"z\"", z);
            ParseJsonFloatField(objStart, objEnd, "\"rotY\"", rotY);
            ParseJsonFloatField(objStart, objEnd, "\"scale\"", scale);
            ParseJsonFloatField(objStart, objEnd, "\"speed\"", spd);
            ParseJsonFloatField(objStart, objEnd, "\"radius\"", rad);

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
}

int main(int argc, char** argv) {

    bool testScreenshotMode = false;
    bool testEditorMode = false;
    bool testDeathMode = false;
    bool testLanMode = false;
    int testSelectIdx = -1;
    bool useCustomTestCam = false;
    Vector3 customTestCamPos = { 0, 0, 0 };
    Vector3 customTestCamTarget = { 0, 0, 0 };

    std::string testShotName = "CustomEngine/scratch/active_gameplay.png";

    int testAngleMode = 0; // 0=behind, 1=side, 2=front, 3=top

    int targetTestFrame = 60;

    for (int i = 1; i < argc; i++) {

        if (strcmp(argv[i], "--test-screenshot") == 0) testScreenshotMode = true;
        if (strcmp(argv[i], "--test-editor") == 0) testEditorMode = true;
        if (strcmp(argv[i], "--test-death") == 0) testDeathMode = true;
        if (strcmp(argv[i], "--test-lan") == 0) testLanMode = true;
        if (strcmp(argv[i], "--test-select") == 0 && i + 1 < argc) testSelectIdx = atoi(argv[++i]);

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
    }

    SetConfigFlags(FLAG_WINDOW_UNDECORATED | FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);

    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Fart Together - Authentic 1:1 Engine");

    InitAudioDevice();

    SetTargetFPS(60);
    // 3D Model Live Preview and Editor Inspector state
    RenderTexture2D previewRT = LoadRenderTexture(280, 125);
    bool inspectorCollapsed = false;

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
    gObsModels.mF10 = LoadModel(ResolveAsset("extracted_assets/meshes/GM_3D_ObstacleF10.obj").c_str());
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

    // World Transforms

    Vector3 trackOffset  = Vector3{ 38.44f, 0.57f, 156.14f };

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

    // Checkpoint markers along the route

    struct RouteCheckpoint {

        Vector3 pos;

        float rotY;

        Model* model;

        bool passed;

    };

    std::vector<RouteCheckpoint> checkpoints = {
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
    bool editorMode = testEditorMode;
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
        PlaySound(fartSounds[5]);

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
            p1RimAngle += Clamp(rimMove1, -1.0f, 1.0f) * 2.44f * dt; // 140 deg/s angular slide speed
            p1RimAngle = Clamp(p1RimAngle, -3.10f, -0.15f);

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
                float tapPower = (GAS_TAP_BURST_FORCE / BOAT_MASS) * 0.35f;
                raft.velocity = Vector3Add(raft.velocity, Vector3Scale(p1ThrustWorld, tapPower));
                float steerTq = (p1OutwardLocal.x * p1ThrustLocal.z - p1OutwardLocal.z * p1ThrustLocal.x);
                raft.angularVelocityY += steerTq * (BOAT_STEER_TORQUE / BOAT_MASS) * 8.0f;
                EmitFartBurst(p1PosWorld, p1OutwardWorld, 0.4f);
                PlaySound(fartSounds[rand() % 6]);
            }
        }

        gasP1.Update(dt, p1InputDown, runTime);

        // Sustained Thrust Application for P1
        if (gasP1.isSustained && !raft.isDestroyed) {
            float curForce = GAS_MIN_FORCE + (GAS_NORMAL_FORCE - GAS_MIN_FORCE) * gasP1.sustainedCharge;
            float accel = (curForce / BOAT_MASS) * dt * 0.85f;
            raft.velocity = Vector3Add(raft.velocity, Vector3Scale(p1ThrustWorld, accel));
            float steerTq = (p1OutwardLocal.x * p1ThrustLocal.z - p1OutwardLocal.z * p1ThrustLocal.x);
            raft.angularVelocityY += steerTq * (BOAT_STEER_TORQUE / BOAT_MASS) * (curForce / GAS_NORMAL_FORCE) * dt * 22.0f;
            if (rand() % 4 == 0) {
                EmitFartBurst(p1PosWorld, p1OutwardWorld, gasP1.sustainedCharge);
            }
        }

        // Player 2 Independent Controls (Right Pirate): Arrow Left/Right + Arrow Up / Right Ctrl / Mouse Left (or Gamepad 1)
        bool p2InputDown = false;
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
                float tapPower = (GAS_TAP_BURST_FORCE / BOAT_MASS) * 0.35f;
                raft.velocity = Vector3Add(raft.velocity, Vector3Scale(p2ThrustWorld, tapPower));
                float steerTq = (p2OutwardLocal.x * p2ThrustLocal.z - p2OutwardLocal.z * p2ThrustLocal.x);
                raft.angularVelocityY += steerTq * (BOAT_STEER_TORQUE / BOAT_MASS) * 8.0f;
                EmitFartBurst(p2PosWorld, p2OutwardWorld, 0.4f);
                PlaySound(fartSounds[rand() % 6]);
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

        // Authentic Hydrodynamics & Spline Distance
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

        // Authentic Start Basin & Canal Wall Elastic Containment (Prevents clipping & swimming on land!)
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
        nearestWaterPt = riverCenter;
        Vector3 toNearest = Vector3Subtract(raft.position, nearestWaterPt);
        float crossWaterDist = Vector3DotProduct(toNearest, riverNormal);
        raft.pitch = Lerp(raft.pitch, -fwdAccel * 0.035f, 6.0f * dt);
        raft.roll  = Lerp(raft.roll, (crossWaterDist * 0.045f) + (raft.angularVelocityY * 0.12f), 6.0f * dt);
        raft.speed = horizSpeed;

        // 5. Update Kinetic Obstacles & Hazards with Authentic Animations & OBB
        for (auto& ent : levelEntities) {
            ent.animTimer += dt * ent.animSpeed;
            Vector3 animPos = ent.position;
            float animYaw = ent.rotationY;
            float animRoll = 0.0f;

            if (ent.type == ENT_OBS_B2) {
                animRoll = sinf(ent.animTimer * 2.0f) * 28.0f; // Seesaw tipping
            } else if (ent.type == ENT_OBS_B3) {
                animYaw += dt * 90.0f * ent.animSpeed;
            } else if (ent.type == ENT_OBS_B5) {
                animYaw += dt * 65.0f * ent.animSpeed;
            } else if (ent.type == ENT_OBS_C1) {
                animRoll = sinf(ent.animTimer * 2.2f) * 50.0f; // Pendulum swing
            } else if (ent.type == ENT_OBS_C2) {
                animRoll = cosf(ent.animTimer * 2.0f) * 55.0f;
            } else if (ent.type == ENT_OBS_C4) {
                animYaw += dt * 110.0f * ent.animSpeed;
            } else if (ent.type == ENT_OBS_C5) {
                animRoll = ent.animTimer * 85.0f; // Vertical spinning wheel
            } else if (ent.type == ENT_OBS_F3) {
                animYaw = ent.rotationY + sinf(ent.animTimer * 1.5f) * 75.0f;
            } else if (ent.type == ENT_OBS_F10) {
                animYaw = ent.rotationY + sinf(ent.animTimer * 1.2f) * 75.0f;
            } else if (ent.type == ENT_OBS_F11) {
                // Lateral reciprocating travel across water canal + spinning drum
                float rad = ent.rotationY * DEG2RAD;
                float latOffset = sinf(ent.animTimer * 1.8f + (float)ent.id) * 2.2f;
                animPos.x += cosf(rad) * latOffset;
                animPos.z -= sinf(rad) * latOffset;
                animRoll = ent.animTimer * 120.0f;
            } else if (ent.type == ENT_OBS_F13) {
                animYaw += dt * 75.0f * ent.animSpeed;
            } else if (ent.type == ENT_OBS_F14) {
                float rad = ent.rotationY * DEG2RAD;
                float slideOffset = sinf(ent.animTimer * 2.2f) * 2.0f;
                animPos.x += cosf(rad) * slideOffset;
                animPos.z -= sinf(rad) * slideOffset;
            } else if (ent.type == ENT_OBS_F23) {
                animYaw = ent.rotationY + sinf(ent.animTimer * 1.4f) * 65.0f;
            }

            if (ent.type == ENT_GUST) {
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
                if (raft.currentCP <= 0) {
                    raft.position = Vector3{ 15.55f, 0.58f, 90.80f };
                    raft.rotationY = 3.14159f;
                } else {
                    int cpIdx = Clamp(raft.currentCP - 1, 0, (int)checkpoints.size() - 1);
                    raft.position = checkpoints[cpIdx].pos + Vector3{ 0.0f, 0.4f, 0.0f };
                    float cpT = FindClosestSplineT(checkpoints[cpIdx].pos);
                    Vector3 cpTan = GetSplineTangent(cpT);
                    raft.rotationY = atan2f(cpTan.x, cpTan.z);
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

        // Checkpoints Check

        for (size_t i = 0; i < checkpoints.size(); i++) {

            float dist = Vector3Distance(raft.position, checkpoints[i].pos);

            if (dist < 14.0f && !checkpoints[i].passed && (int)i == raft.currentCP) {

                checkpoints[i].passed = true;

                raft.currentCP++;

                PlaySound(fartSounds[0]);

                if (i == checkpoints.size() - 1) raceFinished = true;

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

            // Raycast Click Selection in 3D
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && mousePos.x < (GetScreenWidth() - 340)) {
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

            // Keyboard Manipulation of Selected Entity (Continuous hold support!)
            if (selectedObs >= 0 && selectedObs < (int)levelEntities.size()) {
                auto& o = levelEntities[selectedObs];
                float editSpd = (IsKeyDown(KEY_LEFT_SHIFT) ? 10.0f : 4.0f) * dt;
                
                // J / L = Move X
                if (IsKeyDown(KEY_J)) o.position.x -= editSpd;
                if (IsKeyDown(KEY_L)) o.position.x += editSpd;
                // I / K = Move Z (along stream)
                if (IsKeyDown(KEY_I)) o.position.z += editSpd;
                if (IsKeyDown(KEY_K)) o.position.z -= editSpd;
                // U / O = Move Y (height)
                if (IsKeyDown(KEY_U)) o.position.y += editSpd;
                if (IsKeyDown(KEY_O)) o.position.y -= editSpd;
                // R / T = Rotate Y
                if (IsKeyDown(KEY_R)) o.rotationY -= 65.0f * dt;
                if (IsKeyDown(KEY_T)) o.rotationY += 65.0f * dt;

                // Focus on object (Unity 'F' key)
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

        BeginDrawing();

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

            // D. Checkpoints & Gates

            for (const auto& cp : checkpoints) {

                Color tint = cp.passed ? COLOR_ACCENT_GREEN : WHITE;

                rlPushMatrix();

                    rlTranslatef(cp.pos.x, cp.pos.y, cp.pos.z);

                    rlRotatef(cp.rotY, 0, 1, 0);

                    DrawModel(*cp.model, Vector3{ 0, 0, 0 }, 1.0f, tint);

                rlPopMatrix();

            }

            // A. Authentic Start Basin inside Canyon Walls & Water Mesh (Zero fake cylinders)

            // E. Draw All Level Entities (Obstacles, Rocks, Trees, Checkpoints)
        for (const auto& ent : levelEntities) {
            if (ent.model != nullptr) {
                Vector3 animPos = ent.position;
                float animYaw = ent.rotationY;
                float animRoll = 0.0f;

                if (ent.type == ENT_OBS_B2) {
                    animRoll = sinf(ent.animTimer * 2.0f) * 28.0f;
                } else if (ent.type == ENT_OBS_C1) {
                    animRoll = sinf(ent.animTimer * 2.2f) * 50.0f;
                } else if (ent.type == ENT_OBS_C2) {
                    animRoll = cosf(ent.animTimer * 2.0f) * 55.0f;
                } else if (ent.type == ENT_OBS_C5) {
                    animRoll = ent.animTimer * 85.0f;
                } else if (ent.type == ENT_OBS_F3) {
                    animYaw = ent.rotationY + sinf(ent.animTimer * 1.5f) * 75.0f;
                } else if (ent.type == ENT_OBS_F10) {
                    animYaw = ent.rotationY + sinf(ent.animTimer * 1.2f) * 75.0f;
                } else if (ent.type == ENT_OBS_F11) {
                    float rad = ent.rotationY * DEG2RAD;
                    float latOffset = sinf(ent.animTimer * 1.8f + (float)ent.id) * 2.2f;
                    animPos.x += cosf(rad) * latOffset;
                    animPos.z -= sinf(rad) * latOffset;
                    animRoll = ent.animTimer * 120.0f;
                } else if (ent.type == ENT_OBS_F14) {
                    float rad = ent.rotationY * DEG2RAD;
                    float slideOffset = sinf(ent.animTimer * 2.2f) * 2.0f;
                    animPos.x += cosf(rad) * slideOffset;
                    animPos.z -= sinf(rad) * slideOffset;
                } else if (ent.type == ENT_OBS_F23) {
                    animYaw = ent.rotationY + sinf(ent.animTimer * 1.4f) * 65.0f;
                }

                rlPushMatrix();
                rlTranslatef(animPos.x, animPos.y, animPos.z);
                rlRotatef(animYaw, 0.0f, 1.0f, 0.0f);
                if (animRoll != 0.0f) {
                    if (ent.type == ENT_OBS_C5) rlRotatef(animRoll, 0.0f, 0.0f, 1.0f);
                    else if (ent.type == ENT_OBS_F11) rlRotatef(animRoll, 1.0f, 0.0f, 0.0f);
                    else rlRotatef(animRoll, 0.0f, 0.0f, 1.0f);
                }
                float s = (ent.scale > 0.01f) ? ent.scale : 1.0f;
                rlScalef(s, s, s);
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
            float p1ButtSwell = (gasP1.boneScale - 1.0f) * 2.5f;
            float p1EyeBulge = fmaxf(gasP1.redness, (gasP1.releaseTimer > 0.0f) ? (gasP1.releaseTimer / GAS_RELEASE_SECONDS) : 0.0f);
            if (gasP1.isPressing && p1EyeBulge < 0.25f) p1EyeBulge = 0.25f + (gasP1.pressTimer / 1.0f) * 0.75f;
            p1EyeBulge = Clamp(p1EyeBulge, 0.0f, 1.0f);
            SetShaderValue(shaderLighting, uRednessLoc,   &p1Redness,   SHADER_UNIFORM_FLOAT);
            SetShaderValue(shaderLighting, uButtSwellLoc, &p1ButtSwell, SHADER_UNIFORM_FLOAT);
            SetShaderValue(shaderLighting, uEyeBulgeLoc,  &p1EyeBulge,  SHADER_UNIFORM_FLOAT);

            rlPushMatrix();
            float p1X = sinf(p1RimAngle) * 0.94f;
            float p1Z = cosf(p1RimAngle) * 0.94f;
            rlTranslatef(p1X, 0.28f + gasP1.trembleOffset, p1Z);
            float p1FaceYaw = (p1RimAngle * RAD2DEG) + 180.0f;
            rlRotatef(p1FaceYaw, 0, 1, 0);
            DrawModel(charB, Vector3{ 0, 0, 0 }, 1.0f, WHITE);
            rlPopMatrix();

            // Right Pirate (Player 2: Orange Hippo - SeatedA)
            float p2Redness = gasP2.redness;
            float p2ButtSwell = (gasP2.boneScale - 1.0f) * 2.5f;
            float p2EyeBulge = fmaxf(gasP2.redness, (gasP2.releaseTimer > 0.0f) ? (gasP2.releaseTimer / GAS_RELEASE_SECONDS) : 0.0f);
            if (gasP2.isPressing && p2EyeBulge < 0.25f) p2EyeBulge = 0.25f + (gasP2.pressTimer / 1.0f) * 0.75f;
            p2EyeBulge = Clamp(p2EyeBulge, 0.0f, 1.0f);
            SetShaderValue(shaderLighting, uRednessLoc,   &p2Redness,   SHADER_UNIFORM_FLOAT);
            SetShaderValue(shaderLighting, uButtSwellLoc, &p2ButtSwell, SHADER_UNIFORM_FLOAT);
            SetShaderValue(shaderLighting, uEyeBulgeLoc,  &p2EyeBulge,  SHADER_UNIFORM_FLOAT);

            rlPushMatrix();
            float p2X = sinf(p2RimAngle) * 0.94f;
            float p2Z = cosf(p2RimAngle) * 0.94f;
            rlTranslatef(p2X, 0.28f + gasP2.trembleOffset, p2Z);
            float p2FaceYaw = (p2RimAngle * RAD2DEG) + 180.0f;
            rlRotatef(p2FaceYaw, 0, 1, 0);
            DrawModel(charA, Vector3{ 0, 0, 0 }, 1.0f, WHITE);
            rlPopMatrix();

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

                        DrawLine3D(oPos, Vector3Add(oPos, Vector3{ 2.2f, 0, 0 }), RED);   // X Axis (Red)
                        DrawLine3D(oPos, Vector3Add(oPos, Vector3{ 0, 2.2f, 0 }), GREEN); // Y Axis (Green)
                        DrawLine3D(oPos, Vector3Add(oPos, Vector3{ 0, 0, 2.2f }), BLUE);  // Z Axis (Blue)
                        DrawSphere(Vector3Add(oPos, Vector3{ 2.2f, 0, 0 }), 0.15f, RED);
                        DrawSphere(Vector3Add(oPos, Vector3{ 0, 2.2f, 0 }), 0.15f, GREEN);
                        DrawSphere(Vector3Add(oPos, Vector3{ 0, 0, 2.2f }), 0.15f, BLUE);
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

            float gasFrac2 = Clamp(gasP2.gasEnergy / GAS_MAX, 0.0f, 1.0f);
            Color barFillCol2 = gasP2.gasDepleted ? Color{ 239, 68, 68, 255 } : Color{ 56, 189, 248, 255 };

            DrawRectangleRounded(Rectangle{ 65, 32, 184, 18 }, 0.5f, 6, Color{ 15, 23, 42, 210 });
            DrawRectangleRoundedLinesEx(Rectangle{ 65, 32, 184, 18 }, 0.5f, 6, 1.5f, Color{ 255, 255, 255, 160 });
            if (gasFrac2 > 0.02f) {
                DrawRectangleRounded(Rectangle{ 67, 34, 180.0f * gasFrac2, 14 }, 0.45f, 6, barFillCol2);
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

            for (size_t cp = 0; cp < checkpoints.size(); cp++) {
                float cpT = FindClosestSplineT(checkpoints[cp].pos);
                float frac = Clamp(cpT / (float)(SPLINE_POINT_COUNT - 1), 0.0f, 1.0f);
                int tickX = trackStartX + (int)(trackWidth * frac);
                DrawRectangle(tickX - 1, trackY - 3, 3, 11, checkpoints[cp].passed ? COLOR_ACCENT_GREEN : Color{ 30, 41, 59, 255 });
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
            if (hoverLeave && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                break;
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
        }

        // 6. Interactive 3D Level Editor Inspector Panel (Authentic Godot 4 Engine Style)
        if (editorMode) {
            float panelW = 340.0f;
            float panelH = 680.0f;
            Rectangle panelRec = { (float)GetScreenWidth() - panelW - 14, 52, panelW, panelH };

            auto DrawGodotBtn = [&](Rectangle rec, const char* label, Color bgCol, Color textCol, bool bold = false) -> bool {
                bool hov = CheckCollisionPointRec(mousePos, rec);
                Color fill = hov ? ColorBrightness(bgCol, 0.18f) : bgCol;
                DrawRectangleRounded(rec, 0.2f, 3, fill);
                DrawRectangleRoundedLinesEx(rec, 0.2f, 3, 1.0f, ColorBrightness(bgCol, 0.35f));
                Font f = bold ? fontUIBold : fontUI;
                Vector2 sz = MeasureTextEx(f, label, 13, 0.3f);
                DrawTextEx(f, label, Vector2{ rec.x + (rec.width - sz.x) * 0.5f, rec.y + (rec.height - sz.y) * 0.5f }, 13, 0.3f, textCol);
                return hov && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
            };

            // Inspector Collapse/Expand Arrow Button
            Rectangle collapseBtn = { inspectorCollapsed ? ((float)GetScreenWidth() - 32.0f) : (panelRec.x - 28.0f), 60.0f, 28.0f, 48.0f };
            if (DrawGodotBtn(collapseBtn, inspectorCollapsed ? "<" : ">", Color{ 32, 38, 52, 255 }, WHITE, true)) {
                inspectorCollapsed = !inspectorCollapsed;
            }

            if (!inspectorCollapsed) {
                // Drop Shadow & Panel Background
                DrawRectangleRounded(Rectangle{ panelRec.x + 3, panelRec.y + 4, panelW, panelH }, 0.02f, 4, Color{ 0, 0, 0, 90 });
                DrawRectangleRounded(panelRec, 0.02f, 4, Color{ 32, 37, 49, 252 });
                DrawRectangleRoundedLinesEx(panelRec, 0.02f, 4, 1.0f, Color{ 58, 66, 83, 255 });

                // Dock Tab Bar
                Rectangle tabRec = { panelRec.x, panelRec.y, 115, 30 };
                DrawRectangleRec(Rectangle{ panelRec.x, panelRec.y, panelW, 30 }, Color{ 25, 29, 38, 255 });
                DrawRectangleRec(tabRec, Color{ 32, 37, 49, 255 });
                DrawRectangle((int)tabRec.x, (int)tabRec.y + 28, (int)tabRec.width, 2, Color{ 71, 140, 191, 255 });
                DrawTextEx(fontUIBold, "Inspector", Vector2{ tabRec.x + 18, tabRec.y + 7 }, 14, 0.4f, Color{ 240, 243, 246, 255 });
                DrawTextEx(fontUI, "3D Level View", Vector2{ panelRec.x + panelW - 105, tabRec.y + 8 }, 12, 0.3f, Color{ 140, 148, 163, 255 });
                DrawLine((int)panelRec.x, (int)panelRec.y + 30, (int)panelRec.x + (int)panelW, (int)panelRec.y + 30, Color{ 58, 66, 83, 255 });

                float py = panelRec.y + 36;

                // 3D Live Model Preview Viewport
                DrawRectangle((int)panelRec.x + 12, (int)py, 280, 125, Color{ 14, 18, 24, 255 });
                DrawTextureRec(previewRT.texture, Rectangle{ 0, 0, (float)previewRT.texture.width, -(float)previewRT.texture.height }, Vector2{ panelRec.x + 12, py }, WHITE);
                DrawRectangleLines((int)panelRec.x + 12, (int)py, 280, 125, Color{ 60, 72, 95, 255 });
                py += 132;

                if (selectedObs >= 0 && selectedObs < (int)levelEntities.size()) {
                    auto& o = levelEntities[selectedObs];
                    DrawTextEx(fontUIBold, TextFormat("Selected: #%d (%s)", selectedObs, ENTITY_TYPE_NAMES[o.type]), Vector2{ panelRec.x + 14, py }, 13, 0.5f, Color{ 142, 202, 230, 255 });
                    py += 22;

                    // Entity Type Switcher
                    DrawTextEx(fontUI, "Entity Type:", Vector2{ panelRec.x + 14, py }, 11, 0.3f, Color{ 180, 190, 205, 255 });
                    py += 15;
                    Rectangle btnPrevType = { panelRec.x + 12, py, 120, 24 };
                    Rectangle btnNextType = { panelRec.x + 140, py, 120, 24 };
                    if (DrawGodotBtn(btnPrevType, "< Prev Type", Color{ 36, 43, 58, 255 }, WHITE, true)) {
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
                    }
                    py += 28;

                    // Scale Controls
                    DrawTextEx(fontUI, TextFormat("Scale: %.2fx (Radius: %.2fm)", o.scale, o.radius), Vector2{ panelRec.x + 14, py }, 11, 0.3f, Color{ 220, 225, 235, 255 });
                    py += 16;
                    Rectangle btnScM = { panelRec.x + 12, py, 75, 22 };
                    Rectangle btnScP = { panelRec.x + 92, py, 75, 22 };
                    Rectangle btnScR = { panelRec.x + 172, py, 88, 22 };
                    if (DrawGodotBtn(btnScM, "Scale -0.1", Color{ 36, 43, 58, 255 }, WHITE, true) || IsKeyPressed(KEY_LEFT_BRACKET)) {
                        o.scale = Clamp(o.scale - 0.1f, 0.2f, 3.5f);
                        AssignEntityModelAndProps(o);
                    }
                    if (DrawGodotBtn(btnScP, "Scale +0.1", Color{ 36, 43, 58, 255 }, WHITE, true) || IsKeyPressed(KEY_RIGHT_BRACKET)) {
                        o.scale = Clamp(o.scale + 0.1f, 0.2f, 3.5f);
                        AssignEntityModelAndProps(o);
                    }
                    if (DrawGodotBtn(btnScR, "Reset 1.0x", Color{ 36, 43, 58, 255 }, WHITE, true)) {
                        o.scale = 1.0f;
                        AssignEntityModelAndProps(o);
                    }
                    py += 26;

                    // Position X/Y/Z Controls
                    DrawTextEx(fontUI, TextFormat("Position: (%.2f, %.2f, %.2f)", o.position.x, o.position.y, o.position.z), Vector2{ panelRec.x + 14, py }, 11, 0.3f, Color{ 220, 225, 235, 255 });
                    py += 16;
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
                    py += 30;

                    // Duplicate & Delete Entity
                    Rectangle btnDup = { panelRec.x + 12, py, 120, 24 };
                    Rectangle btnDel = { panelRec.x + 140, py, 120, 24 };
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
                    py += 32;
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
                newE.type = ENT_OBS_A1;
                Vector3 fwd = { -sinf(editorCamYaw) * cosf(editorCamPitch), sinf(editorCamPitch), -cosf(editorCamYaw) * cosf(editorCamPitch) };
                newE.position = Vector3Add(editorCamPos, Vector3Scale(fwd, 6.5f));
                newE.position.y = 0.50f;
                newE.scale = 1.0f;
                AssignEntityModelAndProps(newE);
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
            }
            py += 44;

            // Godot Status & Shortcut Tray
            Rectangle trayRec = { panelRec.x + 8, panelRec.y + panelH - 62, panelW - 16, 54 };
            DrawRectangleRounded(trayRec, 0.15f, 3, Color{ 20, 23, 30, 255 });
            DrawRectangleRoundedLinesEx(trayRec, 0.15f, 3, 1.0f, Color{ 45, 52, 66, 255 });
            DrawTextEx(fontUI, "RMB+WASD: Fly Cam  |  F: Focus Selection", Vector2{ trayRec.x + 10, trayRec.y + 8 }, 11, 0.3f, Color{ 156, 163, 175, 255 });
            DrawTextEx(fontUI, "Hold J/L: X  |  I/K: Z  |  U/O: Y  |  R/T: Rot", Vector2{ trayRec.x + 10, trayRec.y + 24 }, 11, 0.3f, Color{ 156, 163, 175, 255 });
            DrawTextEx(fontUI, "F1: Exit Editor & Resume Game", Vector2{ trayRec.x + 10, trayRec.y + 38 }, 11, 0.3f, Color{ 147, 197, 253, 255 });

            // Godot Style Toast Notification
            if (editorMsgTimer > 0.0f) {
                editorMsgTimer -= dt;
                Vector2 msgSz = MeasureTextEx(fontUIBold, editorMsg.c_str(), 14, 0.4f);
                int toastW = (int)msgSz.x + 40;
                int toastH = 34;
                int toastX = GetScreenWidth() / 2 - toastW / 2;
                int toastY = 70;
                DrawRectangleRounded(Rectangle{ (float)toastX, (float)toastY, (float)toastW, (float)toastH }, 0.35f, 4, Color{ 25, 30, 42, 245 });
                DrawRectangleRoundedLinesEx(Rectangle{ (float)toastX, (float)toastY, (float)toastW, (float)toastH }, 0.35f, 4, 1.2f, Color{ 71, 140, 191, 255 });
                DrawTextEx(fontUIBold, editorMsg.c_str(), Vector2{ (float)toastX + 20, (float)toastY + 9 }, 14, 0.4f, WHITE);
            }
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