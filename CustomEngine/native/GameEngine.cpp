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
#ifndef _WIN32
#ifndef strncpy_s
#define strncpy_s(dst, src, count) do { strncpy((dst), (src), (count)); (dst)[(count)-1] = '\0'; } while(0)
#endif
#endif
#include "LanNetwork.h"
#include "Localization.h"
#include "Codepoints.h"

// Rule: Game version must be incremented on every update!
static const char* GAME_VERSION_TAG = "v8.4.0";
static int s_camTouchId = -1;
GameLanguage gCurrentLanguage = LANG_AR; // Default Arabic as requested
static bool gShowInviteModal = false;


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

#if defined(PLATFORM_ANDROID) || defined(__ANDROID__)
static const char* VS_LIGHTING = R"(
#version 100
precision highp float;
attribute vec3 vertexPosition;
attribute vec2 vertexTexCoord;
attribute vec3 vertexNormal;
attribute vec4 vertexColor;
uniform mat4 mvp;
uniform float uButtSwell;
uniform float uEyeBulge;
varying vec2 fragTexCoord;
varying vec4 fragColor;
varying vec3 fragNormal;
varying vec3 fragPosition;

void main() {
    fragTexCoord = vertexTexCoord;
    fragColor    = vertexColor;
    fragNormal   = vertexNormal;
    fragPosition = vertexPosition;
    vec3 pos     = vertexPosition;

    if (pos.z < -0.01 && pos.y >= 0.18 && pos.y <= 0.52 && uButtSwell > 0.001) {
        float wy = sin(clamp((pos.y - 0.18) / 0.34, 0.0, 1.0) * 3.14159265);
        float wz = clamp((-pos.z - 0.01) / 0.12, 0.0, 1.0);
        float wButt = wy * wz;
        vec3 buttPivot = vec3(0.0, 0.34, -0.02);
        pos = mix(pos, buttPivot + (pos - buttPivot) * (1.0 + uButtSwell), wButt);
        pos.z -= uButtSwell * 0.14 * wButt;
    }

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
}
)";

static const char* FS_LIGHTING = R"(
#version 100
precision highp float;
varying vec2 fragTexCoord;
varying vec4 fragColor;
varying vec3 fragNormal;
varying vec3 fragPosition;
uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform vec3 lightDir;
uniform vec3 lightColor;
uniform vec3 ambientColor;
uniform float uRedness;

void main() {
    vec4 texColor = texture2D(texture0, fragTexCoord);
    vec3 N = normalize(fragNormal);
    vec3 L = normalize(lightDir);
    float ndotl = dot(N, L) * 0.5 + 0.5;
    float toonDiff = smoothstep(0.32, 0.78, ndotl);
    vec3 lighting = ambientColor + toonDiff * lightColor;

    vec3 buttCenter = vec3(0.0, 0.34, -0.12);
    vec3 buttScale  = vec3(0.26, 0.20, 0.16);
    float buttDist  = length((fragPosition - buttCenter) / buttScale);
    float buttMask  = (fragPosition.z < -0.01) ? clamp(1.0 - smoothstep(0.35, 1.05, buttDist), 0.0, 1.0) : 0.0;

    vec3 redSkin = mix(texColor.rgb, vec3(0.96, 0.12, 0.12), uRedness * buttMask * 0.92);
    gl_FragColor = vec4(redSkin * lighting * colDiffuse.rgb, texColor.a * colDiffuse.a);
}
)";

static const char* VS_WATER = R"(
#version 100
precision highp float;
attribute vec3 vertexPosition;
attribute vec2 vertexTexCoord;
attribute vec3 vertexNormal;
uniform mat4 mvp;
uniform vec3 trackOffset;
uniform float uTime;
varying vec2 fragTexCoord;
varying vec3 fragNormal;
varying vec3 fragPos;

void main() {
    fragTexCoord = vertexTexCoord;
    vec3 pos = vertexPosition;
    float w1 = sin(pos.z * 0.32 - uTime * 1.8 + pos.x * 0.12) * 0.05;
    float w2 = cos(pos.x * 0.40 + pos.z * 0.22 - uTime * 2.2) * 0.04;
    pos.y += w1 + w2;
    fragPos = pos + trackOffset;
    float dHdx = cos(pos.x * 0.40 + pos.z * 0.22 - uTime * 2.2) * 0.016;
    float dHdz = cos(pos.z * 0.32 - uTime * 1.8 + pos.x * 0.12) * 0.016;
    fragNormal = normalize(vec3(-dHdx, 1.0, -dHdz));
    gl_Position = mvp * vec4(pos, 1.0);
}
)";

static const char* FS_WATER = R"(
#version 100
precision highp float;
varying vec2 fragTexCoord;
varying vec3 fragNormal;
varying vec3 fragPos;
uniform sampler2D texture0;
uniform float uTime;

void main() {
    vec3 deepCol    = vec3(0.04, 0.45, 0.78);
    vec3 shallowCol = vec3(0.10, 0.68, 0.94);
    vec3 foamCol    = vec3(0.78, 0.96, 1.00);

    vec2 p1 = fragPos.xz * 0.55 + vec2(uTime * 0.45, -uTime * 0.60);
    vec2 p2 = fragPos.xz * 0.72 + vec2(-uTime * 0.50, uTime * 0.40);
    float c1 = sin(p1.x) * cos(p1.y) + sin(p1.x * 1.7 + p1.y * 1.3) * 0.5;
    float c2 = cos(p2.x) * sin(p2.y) + cos(p2.x * 1.4 - p2.y * 1.6) * 0.5;
    float causticPattern = smoothstep(0.35, 0.95, (c1 + c2) * 0.5 + 0.5);
    float foamSparkle    = smoothstep(0.82, 0.98, (c1 * c2) + 0.4);

    vec3 waterCol = mix(deepCol, shallowCol, 0.55 + 0.45 * causticPattern);
    waterCol = mix(waterCol, foamCol, causticPattern * 0.32 + foamSparkle * 0.45);
    gl_FragColor = vec4(waterCol, 0.95);
}
)";
#else
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

#endif

// =============================================================================
// AAA Engine Optimization: 6-Plane Camera Frustum Culling
// =============================================================================
struct FrustumPlanes {
    Vector4 planes[6]; // Left, Right, Bottom, Top, Near, Far
};

static FrustumPlanes ExtractCameraFrustum(const Camera3D& cam, float aspect) {
    Matrix matView = MatrixLookAt(cam.position, cam.target, cam.up);
    Matrix matProj = MatrixPerspective(cam.fovy * DEG2RAD, aspect, 0.2f, 220.0f);
    Matrix vp      = MatrixMultiply(matView, matProj);

    FrustumPlanes fp;
    fp.planes[0] = Vector4{ vp.m3 + vp.m0, vp.m7 + vp.m4, vp.m11 + vp.m8,  vp.m15 + vp.m12 }; // Left
    fp.planes[1] = Vector4{ vp.m3 - vp.m0, vp.m7 - vp.m4, vp.m11 - vp.m8,  vp.m15 - vp.m12 }; // Right
    fp.planes[2] = Vector4{ vp.m3 + vp.m1, vp.m7 + vp.m5, vp.m11 + vp.m9,  vp.m15 + vp.m13 }; // Bottom
    fp.planes[3] = Vector4{ vp.m3 - vp.m1, vp.m7 - vp.m5, vp.m11 - vp.m9,  vp.m15 - vp.m13 }; // Top
    fp.planes[4] = Vector4{ vp.m3 + vp.m2, vp.m7 + vp.m6, vp.m11 + vp.m10, vp.m15 + vp.m14 }; // Near
    fp.planes[5] = Vector4{ vp.m3 - vp.m2, vp.m7 - vp.m6, vp.m11 - vp.m10, vp.m15 - vp.m14 }; // Far
    for (int i = 0; i < 6; i++) {
        float len = sqrtf(fp.planes[i].x * fp.planes[i].x + fp.planes[i].y * fp.planes[i].y + fp.planes[i].z * fp.planes[i].z);
        if (len > 1e-5f) {
            fp.planes[i].x /= len; fp.planes[i].y /= len; fp.planes[i].z /= len; fp.planes[i].w /= len;
        }
    }
    return fp;
}

static bool IsAABBInFrustum(const FrustumPlanes& fp, Vector3 bMin, Vector3 bMax) {
    for (int i = 0; i < 6; i++) {
        const Vector4& p = fp.planes[i];
        Vector3 pv = {
            (p.x >= 0.0f) ? bMax.x : bMin.x,
            (p.y >= 0.0f) ? bMax.y : bMin.y,
            (p.z >= 0.0f) ? bMax.z : bMin.z
        };
        if (p.x * pv.x + p.y * pv.y + p.z * pv.z + p.w < -3.0f) {
            return false;
        }
    }
    return true;
}

static bool IsSphereInFrustum(const FrustumPlanes& fp, Vector3 center, float radius) {
    for (int i = 0; i < 6; i++) {
        const Vector4& p = fp.planes[i];
        if (p.x * center.x + p.y * center.y + p.z * center.z + p.w < -radius) {
            return false;
        }
    }
    return true;
}

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
static bool ParseJsonIntField(const char* objStart, const char* objEnd, const char* key, int& outVal);

struct GameSettingsConfig {
    // Audio (0 .. 100)
    float masterVolume = 80.0f;
    float musicVolume  = 80.0f;
    float sfxVolume    = 80.0f;
    float fartVolume   = 80.0f;

    // Graphics
    int   qualityLevel = 2;     // 0=Low, 1=Medium, 2=High, 3=Ultra
    bool  cameraShake  = true;
    float maxCamDist   = 30.0f; // 10m .. 50m
    bool  showFPS      = false; // F5 toggle or Settings toggle

    // Device profile
    int   language     = 0;     // 0=Arabic (Default), 1=English
    int   deviceMode   = 0;     // 0=Auto-Detect, 1=PC (Keyboard/Mouse), 2=Mobile (Touchscreen)
    float mouseSens    = 1.0f;
    bool  invertMouseY = false;

    // Mobile touch HUD configuration
    float touchBtnScale   = 1.00f; // 0.50f .. 1.80f
    float touchBtnOpacity = 0.85f; // 0.30f .. 1.00f
    float steerLeftPosX   = 0.08f; // Screen width fraction
    float steerLeftPosY   = 0.78f; // Screen height fraction
    float steerRightPosX  = 0.22f;
    float steerRightPosY  = 0.78f;
    float gasBtnPosX      = 0.86f;
    float gasBtnPosY      = 0.76f;
};
static GameSettingsConfig gSettings;

static void SaveGameSettingsJSON(const std::string& path = "game_settings.json") {
    std::string resolved = ResolveAsset(path);
    FILE* f = fopen(resolved.c_str(), "w");
    if (!f) f = fopen(path.c_str(), "w");
    if (!f) return;
    fprintf(f,
        "{\n"
        "  \"masterVolume\": %.1f,\n"
        "  \"musicVolume\": %.1f,\n"
        "  \"sfxVolume\": %.1f,\n"
        "  \"fartVolume\": %.1f,\n"
        "  \"qualityLevel\": %d,\n"
        "  \"cameraShake\": %s,\n"
        "  \"maxCamDist\": %.1f,\n"
        "  \"showFPS\": %s,\n"
        "  \"deviceMode\": %d,\n"
        "  \"mouseSens\": %.2f,\n"
        "  \"invertMouseY\": %s,\n"
        "  \"touchBtnScale\": %.2f,\n"
        "  \"touchBtnOpacity\": %.2f,\n"
        "  \"steerLeftPosX\": %.3f,\n"
        "  \"steerLeftPosY\": %.3f,\n"
        "  \"steerRightPosX\": %.3f,\n"
        "  \"steerRightPosY\": %.3f,\n"
        "  \"gasBtnPosX\": %.3f,\n"
        "  \"gasBtnPosY\": %.3f\n"
        "}\n",
        gSettings.masterVolume, gSettings.musicVolume, gSettings.sfxVolume, gSettings.fartVolume,
        gSettings.qualityLevel, gSettings.cameraShake ? "true" : "false", gSettings.maxCamDist,
        gSettings.showFPS ? "true" : "false", gSettings.deviceMode, gSettings.mouseSens,
        gSettings.invertMouseY ? "true" : "false", gSettings.touchBtnScale, gSettings.touchBtnOpacity,
        gSettings.steerLeftPosX, gSettings.steerLeftPosY, gSettings.steerRightPosX, gSettings.steerRightPosY,
        gSettings.gasBtnPosX, gSettings.gasBtnPosY);
    fclose(f);
}

static void LoadGameSettingsJSON(const std::string& path = "game_settings.json") {
    std::string resolved = ResolveAsset(path);
    char* txt = LoadFileText(resolved.c_str());
    if (!txt) { SaveGameSettingsJSON(path); return; }
    const char* end = txt + strlen(txt);
    ParseJsonFloatField(txt, end, "\"masterVolume\"", gSettings.masterVolume);
    ParseJsonFloatField(txt, end, "\"musicVolume\"", gSettings.musicVolume);
    ParseJsonFloatField(txt, end, "\"sfxVolume\"", gSettings.sfxVolume);
    ParseJsonFloatField(txt, end, "\"fartVolume\"", gSettings.fartVolume);
    int q = gSettings.qualityLevel;
    if (ParseJsonIntField(txt, end, "\"qualityLevel\"", q)) gSettings.qualityLevel = q;
    ParseJsonFloatField(txt, end, "\"maxCamDist\"", gSettings.maxCamDist);
    int dm = gSettings.deviceMode;
    if (ParseJsonIntField(txt, end, "\"deviceMode\"", dm)) gSettings.deviceMode = dm;
    ParseJsonFloatField(txt, end, "\"mouseSens\"", gSettings.mouseSens);
    ParseJsonFloatField(txt, end, "\"touchBtnScale\"", gSettings.touchBtnScale);
    ParseJsonFloatField(txt, end, "\"touchBtnOpacity\"", gSettings.touchBtnOpacity);
    ParseJsonFloatField(txt, end, "\"steerLeftPosX\"", gSettings.steerLeftPosX);
    ParseJsonFloatField(txt, end, "\"steerLeftPosY\"", gSettings.steerLeftPosY);
    ParseJsonFloatField(txt, end, "\"steerRightPosX\"", gSettings.steerRightPosX);
    ParseJsonFloatField(txt, end, "\"steerRightPosY\"", gSettings.steerRightPosY);
    ParseJsonFloatField(txt, end, "\"gasBtnPosX\"", gSettings.gasBtnPosX);
    ParseJsonFloatField(txt, end, "\"gasBtnPosY\"", gSettings.gasBtnPosY);
    if (strstr(txt, "\"cameraShake\": false") || strstr(txt, "\"cameraShake\":false")) gSettings.cameraShake = false;
    if (strstr(txt, "\"showFPS\": true") || strstr(txt, "\"showFPS\":true")) gSettings.showFPS = true;
    UnloadFileText(txt);
}

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
    SCREEN_MAIN_MENU  = 0,
    SCREEN_LAN_LOBBY  = 1,
    SCREEN_SETTINGS   = 2,
    SCREEN_GAMEPLAY   = 3,
    SCREEN_FIND_LOBBY = 4
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
    // 12 Custom Collision Shapes (Plan v7.0 Task 3)
    ENT_COL_FLAT_WALL = 20, // Flat vertical wall plane
    ENT_COL_FLAT_FLOOR = 21,// Flat horizontal water/floor supporting plane
    ENT_COL_SQUARE = 22,   // Square base collision box
    ENT_COL_RECT = 23,     // Rectangle / Wall collision box
    ENT_COL_CIRCLE = 24,   // Circle horizontal collision
    ENT_COL_CUBE = 25,     // 3D Cube
    ENT_COL_CYLINDER = 26, // 3D Cylinder
    ENT_COL_CAPSULE = 27,  // 3D Capsule
    ENT_COL_SPHERE = 28,   // 3D Sphere
    ENT_COL_WEDGE = 29,    // Wedge / Ramp
    ENT_COL_FALL_ZONE = 30,// Fall into abyss zone
    ENT_COL_KILL_ZONE = 31,// Explode / Kill boat zone
    ENT_COUNT = 32
};

static const char* ENTITY_TYPE_NAMES[] = {
    "OBS_A1_BUMPER", "OBS_B2_TIPPING", "OBS_B3_SPINNER", "OBS_B5_ROTOR",
    "OBS_C1_ARC", "OBS_C2_PENDULUM", "OBS_C4_QUAD", "OBS_C5_WHEEL",
    "OBS_F3_SWING", "OBS_F10_GATE", "OBS_F11_DRUM", "OBS_F13_ARM",
    "OBS_F14_SLIDER", "OBS_F18_SPIKE", "OBS_F23_DUAL",
    "ROCK_DARK_1", "ROCK_DARK_2", "WIND_GUST",
    "CHECKPOINT_GATE", "TREE_JUNGLE",
    "COL_FLAT_WALL", "COL_FLAT_FLOOR",
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
    int colBehavior; // 0=Solid Wall, 1=Fall/Abyss, 2=Kill/Explode, 3=Bounce Pad, 4=Water/Floor Support
    bool cpPassed;
    Model* model;
    // Per-obstacle custom controls (Plan v7.0 Task 2)
    float arcAngleDeg;    // Degree of rotation arc (0 to 360)
    int   animMode;       // 0=Ping-Pong, 1=Continuous CW (+360), -1=Continuous CCW (-360)
    float phaseOffsetDeg; // Start phase in degrees (0..360)
    float slideDist;      // Slide distance in meters
    float bounceMult;     // Rebound push multiplier
    int   isLethal;       // 1=Explodes boat on contact, 0=Non-lethal bounce/push
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

// Real Water Mesh Triangle Grid for accurate surface elevation and cliff-edge falling (Plan v7.0 Task 1)
struct WaterTri2D {
    float x0, z0, x1, z1, x2, z2;
    float minX, maxX, minZ, maxZ, avgY;
};
static std::vector<WaterTri2D> gWaterTris;

static void AddWaterTriangle(float x0, float y0, float z0, float x1, float y1, float z1, float x2, float y2, float z2) {
    float area2 = fabsf((x1 - x0) * (z2 - z0) - (x2 - x0) * (z1 - z0));
    if (area2 < 0.001f) return;
    WaterTri2D wt;
    wt.x0 = x0; wt.z0 = z0;
    wt.x1 = x1; wt.z1 = z1;
    wt.x2 = x2; wt.z2 = z2;
    wt.minX = fminf(x0, fminf(x1, x2)) - 0.65f;
    wt.maxX = fmaxf(x0, fmaxf(x1, x2)) + 0.65f;
    wt.minZ = fminf(z0, fminf(z1, z2)) - 0.65f;
    wt.maxZ = fmaxf(z0, fmaxf(z1, z2)) + 0.65f;
    wt.avgY = (y0 + y1 + y2) / 3.0f;
    gWaterTris.push_back(wt);
}

static const float WATER_GRID_CELL_SIZE = 16.0f;
static const int WATER_GRID_DIM = 64;
static const float WATER_GRID_OFFSET_X = 512.0f;
static const float WATER_GRID_OFFSET_Z = 512.0f;
static std::vector<int> gWaterSpatialGrid[WATER_GRID_DIM][WATER_GRID_DIM];

static inline void SpatialGridCoord(float x, float z, int& outGX, int& outGZ) {
    outGX = (int)((x + WATER_GRID_OFFSET_X) / WATER_GRID_CELL_SIZE);
    outGZ = (int)((z + WATER_GRID_OFFSET_Z) / WATER_GRID_CELL_SIZE);
    if (outGX < 0) outGX = 0; if (outGX >= WATER_GRID_DIM) outGX = WATER_GRID_DIM - 1;
    if (outGZ < 0) outGZ = 0; if (outGZ >= WATER_GRID_DIM) outGZ = WATER_GRID_DIM - 1;
}

static void BuildWaterTriangleGrid(const Model& waterModel, Vector3 offset) {
    gWaterTris.clear();
    for (int gx = 0; gx < WATER_GRID_DIM; gx++) {
        for (int gz = 0; gz < WATER_GRID_DIM; gz++) {
            gWaterSpatialGrid[gx][gz].clear();
        }
    }

    for (int mIdx = 0; mIdx < waterModel.meshCount; mIdx++) {
        const Mesh& m = waterModel.meshes[mIdx];
        const float* v = m.vertices;
        if (!v || m.triangleCount <= 0) continue;

        if (m.indices != nullptr) {
            for (int t = 0; t < m.triangleCount; t++) {
                int i0 = m.indices[t * 3 + 0];
                int i1 = m.indices[t * 3 + 1];
                int i2 = m.indices[t * 3 + 2];
                AddWaterTriangle(
                    v[i0*3+0] + offset.x, v[i0*3+1] + offset.y, v[i0*3+2] + offset.z,
                    v[i1*3+0] + offset.x, v[i1*3+1] + offset.y, v[i1*3+2] + offset.z,
                    v[i2*3+0] + offset.x, v[i2*3+1] + offset.y, v[i2*3+2] + offset.z
                );
            }
        } else {
            for (int t = 0; t < m.triangleCount; t++) {
                const float* p = &v[t * 9];
                AddWaterTriangle(
                    p[0] + offset.x, p[1] + offset.y, p[2] + offset.z,
                    p[3] + offset.x, p[4] + offset.y, p[5] + offset.z,
                    p[6] + offset.x, p[7] + offset.y, p[8] + offset.z
                );
            }
        }
    }

    for (size_t i = 0; i < gWaterTris.size(); i++) {
        int minGX, minGZ, maxGX, maxGZ;
        SpatialGridCoord(gWaterTris[i].minX, gWaterTris[i].minZ, minGX, minGZ);
        SpatialGridCoord(gWaterTris[i].maxX, gWaterTris[i].maxZ, maxGX, maxGZ);
        for (int gx = minGX; gx <= maxGX; gx++) {
            for (int gz = minGZ; gz <= maxGZ; gz++) {
                gWaterSpatialGrid[gx][gz].push_back((int)i);
            }
        }
    }
    printf("[WaterGrid] Loaded %zu water triangles into 16x16m Spatial Hash Grid.\n", gWaterTris.size());
}

static bool IsSinglePointInWaterTri(float px, float pz, float& outWaterY) {
    float dxPond = px - 15.43f, dzPond = pz - 84.0f;
    if (pz >= 74.0f && (dxPond * dxPond + dzPond * dzPond) <= (16.2f * 16.2f)) {
        outWaterY = 0.55f;
        return true;
    }

    int gx, gz;
    SpatialGridCoord(px, pz, gx, gz);
    const auto& cellTris = gWaterSpatialGrid[gx][gz];
    for (int idx : cellTris) {
        const auto& t = gWaterTris[idx];
        if (px < t.minX || px > t.maxX || pz < t.minZ || pz > t.maxZ) continue;
        float dX = px - t.x2, dZ = pz - t.z2;
        float dX21 = t.x2 - t.x1, dZ12 = t.z1 - t.z2;
        float D = dZ12 * (t.x0 - t.x2) + dX21 * (t.z0 - t.z2);
        if (fabsf(D) < 1e-5f) continue;
        float s = (dZ12 * dX + dX21 * dZ) / D;
        float tVal = ((t.z2 - t.z0) * dX + (t.x0 - t.x2) * dZ) / D;
        if (s >= -0.18f && tVal >= -0.18f && (s + tVal) <= 1.18f) {
            outWaterY = t.avgY;
            return true;
        }
    }
    return false;
}

static bool IsPointOverWaterMesh(float px, float pz, float& outWaterY) {
    if (IsSinglePointInWaterTri(px, pz, outWaterY)) return true;
    const float rimCheck = 0.65f;
    if (IsSinglePointInWaterTri(px + rimCheck, pz, outWaterY)) return true;
    if (IsSinglePointInWaterTri(px - rimCheck, pz, outWaterY)) return true;
    if (IsSinglePointInWaterTri(px, pz + rimCheck, outWaterY)) return true;
    if (IsSinglePointInWaterTri(px, pz - rimCheck, outWaterY)) return true;
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

// Per-obstacle accurate animated transforms (Plan v7.0 Task 2)
static void GetEntityAnimatedTransform(const LevelEntity& ent, Vector3& outPos, float& outYaw, float& outPitch, float& outRoll) {
    outPos   = ent.position;
    outYaw   = ent.rotationY;
    outPitch = ent.rotationX;
    outRoll  = ent.rotationZ;

    float phaseRad = ent.phaseOffsetDeg * DEG2RAD;
    float t = ent.animTimer * 1.85f + phaseRad;
    float halfArc = ent.arcAngleDeg * 0.5f;
    bool continuous360 = (ent.animMode != 0 && ent.arcAngleDeg >= 359.0f);
    float dirSign = (ent.animMode < 0) ? -1.0f : 1.0f;

    switch (ent.type) {
        case ENT_OBS_B2:
        case ENT_OBS_C1:
            if (continuous360) outPitch = ent.rotationX + fmodf(ent.animTimer * 110.0f * dirSign + ent.phaseOffsetDeg, 360.0f);
            else outPitch = ent.rotationX - halfArc + sinf(t) * halfArc;
            break;

        case ENT_OBS_F10: {
            float maxSymArc = (ent.arcAngleDeg / 180.0f) * 73.07f;
            if (continuous360) outRoll = ent.rotationZ + fmodf(ent.animTimer * 100.0f * dirSign + ent.phaseOffsetDeg, 360.0f);
            else outRoll = ent.rotationZ - maxSymArc + sinf(t) * maxSymArc;
            break;
        }

        case ENT_OBS_F13:
        case ENT_OBS_B3:
        case ENT_OBS_B5:
        case ENT_OBS_C4:
            if (continuous360) outYaw = fmodf(ent.rotationY + ent.animTimer * 120.0f * dirSign + ent.phaseOffsetDeg, 360.0f);
            else outYaw = ent.rotationY + sinf(t) * halfArc;
            break;

        case ENT_OBS_C5:
            if (continuous360) outRoll = ent.rotationZ + fmodf(ent.animTimer * 95.0f * dirSign + ent.phaseOffsetDeg, 360.0f);
            else outRoll = ent.rotationZ + sinf(t) * halfArc;
            break;

        case ENT_OBS_C2:
            if (continuous360) outRoll = ent.rotationZ + fmodf(ent.animTimer * 100.0f * dirSign + ent.phaseOffsetDeg, 360.0f);
            else outRoll = ent.rotationZ + sinf(t) * halfArc;
            break;

        case ENT_OBS_F3:
        case ENT_OBS_F23:
            if (continuous360) outYaw = fmodf(ent.rotationY + ent.animTimer * 110.0f * dirSign + ent.phaseOffsetDeg, 360.0f);
            else outYaw = ent.rotationY + sinf(t) * halfArc;
            break;

        case ENT_OBS_F14: {
            float slide = sinf(t) * ent.slideDist;
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

// Plan v7.3: Exact 3D Triangle Mesh Collision Architecture ("تقنية المثلثات")
struct Tri3D {
    Vector3 v0, v1, v2;
};

struct ObstacleTriMesh {
    std::vector<Tri3D> triangles;
    BoundingBox localBBox;
    float localRadius = 0.0f;
};

static ObstacleTriMesh gObsTriMeshes[ENT_COUNT];
static ObstacleTriMesh gObsTriMeshF10_Arms;
static ObstacleTriMesh gObsTriMeshF10_Hammer;

// Mathematically exact closest point on a 3D triangle to point p (Christer Ericson RTCD)
static inline Vector3 ClosestPointOnTriangle(Vector3 p, Vector3 a, Vector3 b, Vector3 c) {
    Vector3 ab = Vector3Subtract(b, a);
    Vector3 ac = Vector3Subtract(c, a);
    Vector3 ap = Vector3Subtract(p, a);

    float d1 = Vector3DotProduct(ab, ap);
    float d2 = Vector3DotProduct(ac, ap);
    if (d1 <= 0.0f && d2 <= 0.0f) return a; // Vertex A

    Vector3 bp = Vector3Subtract(p, b);
    float d3 = Vector3DotProduct(ab, bp);
    float d4 = Vector3DotProduct(ac, bp);
    if (d3 >= 0.0f && d4 <= d3) return b; // Vertex B

    float vc = d1 * d4 - d3 * d2;
    if (vc <= 0.0f && d1 >= 0.0f && d3 <= 0.0f) {
        float v = d1 / (d1 - d3);
        return Vector3Add(a, Vector3Scale(ab, v)); // Edge AB
    }

    Vector3 cp = Vector3Subtract(p, c);
    float d5 = Vector3DotProduct(ab, cp);
    float d6 = Vector3DotProduct(ac, cp);
    if (d6 >= 0.0f && d5 <= d6) return c; // Vertex C

    float vb = d5 * d2 - d1 * d6;
    if (vb <= 0.0f && d2 >= 0.0f && d6 <= 0.0f) {
        float w = d2 / (d2 - d6);
        return Vector3Add(a, Vector3Scale(ac, w)); // Edge AC
    }

    float va = d3 * d6 - d5 * d4;
    if (va <= 0.0f && (d4 - d3) >= 0.0f && (d5 - d6) >= 0.0f) {
        float w = (d4 - d3) / ((d4 - d3) + (d5 - d6));
        return Vector3Add(b, Vector3Scale(Vector3Subtract(c, b), w)); // Edge BC
    }

    float denom = 1.0f / (va + vb + vc);
    float v = vb * denom;
    float w = vc * denom;
    return Vector3Add(a, Vector3Add(Vector3Scale(ab, v), Vector3Scale(ac, w))); // Face interior
}

static void ExtractTriMeshFromModel(const Model& model, ObstacleTriMesh& outMesh) {
    outMesh.triangles.clear();
    Vector3 minPt = { 1e9f, 1e9f, 1e9f };
    Vector3 maxPt = { -1e9f, -1e9f, -1e9f };

    for (int mIdx = 0; mIdx < model.meshCount; mIdx++) {
        const Mesh& m = model.meshes[mIdx];
        const float* v = m.vertices;
        if (!v || m.triangleCount <= 0) continue;

        if (m.indices != nullptr) {
            for (int t = 0; t < m.triangleCount; t++) {
                int i0 = m.indices[t * 3 + 0];
                int i1 = m.indices[t * 3 + 1];
                int i2 = m.indices[t * 3 + 2];
                Tri3D tri = {
                    Vector3{ v[i0*3+0], v[i0*3+1], v[i0*3+2] },
                    Vector3{ v[i1*3+0], v[i1*3+1], v[i1*3+2] },
                    Vector3{ v[i2*3+0], v[i2*3+1], v[i2*3+2] }
                };
                outMesh.triangles.push_back(tri);
                minPt = Vector3Min(minPt, tri.v0); minPt = Vector3Min(minPt, tri.v1); minPt = Vector3Min(minPt, tri.v2);
                maxPt = Vector3Max(maxPt, tri.v0); maxPt = Vector3Max(maxPt, tri.v1); maxPt = Vector3Max(maxPt, tri.v2);
            }
        } else {
            for (int t = 0; t < m.triangleCount; t++) {
                const float* p = &v[t * 9];
                Tri3D tri = {
                    Vector3{ p[0], p[1], p[2] },
                    Vector3{ p[3], p[4], p[5] },
                    Vector3{ p[6], p[7], p[8] }
                };
                outMesh.triangles.push_back(tri);
                minPt = Vector3Min(minPt, tri.v0); minPt = Vector3Min(minPt, tri.v1); minPt = Vector3Min(minPt, tri.v2);
                maxPt = Vector3Max(maxPt, tri.v0); maxPt = Vector3Max(maxPt, tri.v1); maxPt = Vector3Max(maxPt, tri.v2);
            }
        }
    }
    outMesh.localBBox = BoundingBox{ minPt, maxPt };
    float r0 = Vector3Length(minPt);
    float r1 = Vector3Length(maxPt);
    outMesh.localRadius = fmaxf(r0, r1);
}

// Tests boat multi-sphere collision against an obstacle's triangles with zero excess margins
static bool TestBoatAgainstTriMesh(const ObstacleTriMesh& triMesh, const Matrix& mWorld, const Matrix& mInv,
                                  const Vector3* boatPts, const float* boatRadii, int numPts,
                                  Vector3* outContactNormal = nullptr) {
    if (triMesh.triangles.empty()) return false;

    for (int p = 0; p < numPts; p++) {
        Vector3 pWorld = boatPts[p];
        float rWorld = boatRadii[p];
        Vector3 pLoc = Vector3Transform(pWorld, mInv);

        float expR = rWorld * 1.5f;
        if (pLoc.x < triMesh.localBBox.min.x - expR || pLoc.x > triMesh.localBBox.max.x + expR ||
            pLoc.y < triMesh.localBBox.min.y - expR || pLoc.y > triMesh.localBBox.max.y + expR ||
            pLoc.z < triMesh.localBBox.min.z - expR || pLoc.z > triMesh.localBBox.max.z + expR) {
            continue;
        }

        float rWorldSq = rWorld * rWorld;

        for (const auto& tri : triMesh.triangles) {
            float triMinY = fminf(tri.v0.y, fminf(tri.v1.y, tri.v2.y));
            float triMaxY = fmaxf(tri.v0.y, fmaxf(tri.v1.y, tri.v2.y));
            if (pLoc.y < triMinY - expR || pLoc.y > triMaxY + expR) continue;

            float triMinX = fminf(tri.v0.x, fminf(tri.v1.x, tri.v2.x));
            float triMaxX = fmaxf(tri.v0.x, fmaxf(tri.v1.x, tri.v2.x));
            if (pLoc.x < triMinX - expR || pLoc.x > triMaxX + expR) continue;

            float triMinZ = fminf(tri.v0.z, fminf(tri.v1.z, tri.v2.z));
            float triMaxZ = fmaxf(tri.v0.z, fmaxf(tri.v1.z, tri.v2.z));
            if (pLoc.z < triMinZ - expR || pLoc.z > triMaxZ + expR) continue;

            Vector3 qLoc = ClosestPointOnTriangle(pLoc, tri.v0, tri.v1, tri.v2);
            Vector3 qWorld = Vector3Transform(qLoc, mWorld);
            float dSq = Vector3DistanceSqr(pWorld, qWorld);
            if (dSq < rWorldSq) {
                if (outContactNormal) {
                    Vector3 diff = Vector3Subtract(pWorld, qWorld);
                    float len = Vector3Length(diff);
                    *outContactNormal = (len > 0.0001f) ? Vector3Scale(diff, 1.0f / len) : Vector3{ 0.0f, 1.0f, 0.0f };
                }
                return true;
            }
        }
    }
    return false;
}

// Plan v7.3: Exact Obstacle Triangle Collision Check
static bool CheckObstacleTriangleMeshCollision(Vector3 raftPos, float raftRotY, const LevelEntity& ent,
                                               Vector3 animPos, float animYawDeg, float animPitchDeg, float animRollDeg,
                                               Vector3* outContactNormal = nullptr) {
    if (ent.type == ENT_TREE_JUNGLE || ent.type == ENT_GUST || ent.type == ENT_CHECKPOINT) return false;
    if (ent.type >= ENT_COL_FLAT_WALL) return false; // Handled by 12 custom shape colliders

    float s = (ent.scale > 0.01f) ? ent.scale : 1.0f;
    float meshR = gObsTriMeshes[ent.type].localRadius;
    if (meshR <= 0.1f) meshR = 3.5f;
    float maxReach = s * meshR + 1.8f;

    // Fast spherical broad-phase check
    float dXZ = Vector2Distance(Vector2{ raftPos.x, raftPos.z }, Vector2{ animPos.x, animPos.z });
    if (dXZ > maxReach) return false;
    if (fabsf(raftPos.y - animPos.y) > (maxReach + 2.0f)) return false;

    // Define 6 boat test points representing the raft hull and pirates
    Vector3 boatPts[6];
    float boatRadii[6];

    float radY = raftRotY * DEG2RAD;
    Vector3 fwd = { sinf(radY), 0.0f, cosf(radY) };
    Vector3 right = { cosf(radY), 0.0f, -sinf(radY) };

    // Waterline center
    boatPts[0] = Vector3{ raftPos.x, raftPos.y + 0.25f, raftPos.z };
    boatRadii[0] = 0.50f;

    // Front rim
    boatPts[1] = Vector3Add(boatPts[0], Vector3Scale(fwd, 0.38f));
    boatRadii[1] = 0.32f;

    // Back rim
    boatPts[2] = Vector3Subtract(boatPts[0], Vector3Scale(fwd, 0.38f));
    boatRadii[2] = 0.32f;

    // Left rim
    boatPts[3] = Vector3Subtract(boatPts[0], Vector3Scale(right, 0.38f));
    boatRadii[3] = 0.32f;

    // Right rim
    boatPts[4] = Vector3Add(boatPts[0], Vector3Scale(right, 0.38f));
    boatRadii[4] = 0.32f;

    // Pirates torso/head
    boatPts[5] = Vector3{ raftPos.x, raftPos.y + 0.70f, raftPos.z };
    boatRadii[5] = 0.38f;

    // Handle ENT_OBS_F10 (Split arms and hammer)
    if (ent.type == ENT_OBS_F10) {
        float fx = s * (float)ent.flipX;
        float fy = s * (float)ent.flipY;
        float fz = s * (float)ent.flipZ;

        // 1. Arms (pivot at animPos, rotated by animYaw, then animRoll around Z)
        Matrix matScaleArms = MatrixScale(fx, fy, fz);
        Matrix matRotZArms  = MatrixRotateZ(animRollDeg * DEG2RAD);
        Matrix matRotYArms  = MatrixRotateY(animYawDeg * DEG2RAD);
        Matrix matTransArms = MatrixTranslate(animPos.x, animPos.y, animPos.z);
        Matrix mWorldArms = MatrixMultiply(matScaleArms, MatrixMultiply(matRotZArms, MatrixMultiply(matRotYArms, matTransArms)));
        Matrix mInvArms   = MatrixInvert(mWorldArms);

        if (TestBoatAgainstTriMesh(gObsTriMeshF10_Arms, mWorldArms, mInvArms, boatPts, boatRadii, 6, outContactNormal)) {
            return true;
        }

        // 2. Hammer (wrist at -7.23, 2.20 in arms local space, hanging down with zero roll)
        float radRoll = animRollDeg * DEG2RAD;
        const float wristX = -7.23f, wristY = 2.20f;
        float newWristX = wristX * cosf(radRoll) - wristY * sinf(radRoll);
        float newWristY = wristX * sinf(radRoll) + wristY * cosf(radRoll);

        Matrix matScaleHam = MatrixScale(fx, fy, fz);
        Matrix matTransHamRel = MatrixTranslate((newWristX - wristX) * fx, (newWristY - wristY) * fy, 0.0f);
        Matrix matRotYHam = MatrixRotateY(animYawDeg * DEG2RAD);
        Matrix matTransHamBase = MatrixTranslate(animPos.x, animPos.y, animPos.z);
        Matrix mWorldHam = MatrixMultiply(matScaleHam, MatrixMultiply(matTransHamRel, MatrixMultiply(matRotYHam, matTransHamBase)));
        Matrix mInvHam   = MatrixInvert(mWorldHam);

        if (TestBoatAgainstTriMesh(gObsTriMeshF10_Hammer, mWorldHam, mInvHam, boatPts, boatRadii, 6, outContactNormal)) {
            return true;
        }
        return false;
    }

    // Standard Obstacles
    float fx = s * (float)ent.flipX;
    float fy = s * (float)ent.flipY;
    float fz = s * (float)ent.flipZ;

    Matrix matScale = MatrixScale(fx, fy, fz);
    Matrix matRotZ  = MatrixRotateZ(animRollDeg * DEG2RAD);
    Matrix matRotX  = MatrixRotateX(animPitchDeg * DEG2RAD);
    Matrix matRotY  = MatrixRotateY(animYawDeg * DEG2RAD);
    Matrix matTrans = MatrixTranslate(animPos.x, animPos.y, animPos.z);

    Matrix mWorld = MatrixMultiply(matScale, MatrixMultiply(matRotZ, MatrixMultiply(matRotX, MatrixMultiply(matRotY, matTrans))));
    Matrix mInv   = MatrixInvert(mWorld);

    return TestBoatAgainstTriMesh(gObsTriMeshes[ent.type], mWorld, mInv, boatPts, boatRadii, 6, outContactNormal);
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
    if (e.arcAngleDeg <= 0.0f) e.arcAngleDeg = 180.0f;
    if (e.slideDist <= 0.0f) e.slideDist = 1.80f;
    if (e.bounceMult <= 0.0f) e.bounceMult = 1.65f;
    if (e.isLethal != 0 && e.isLethal != 1) {
        bool isHazardType = (e.type >= ENT_OBS_A1 && e.type <= ENT_OBS_F23 && e.type != ENT_OBS_F18) || (e.type == ENT_COL_KILL_ZONE);
        e.isLethal = isHazardType ? 1 : 0;
    }
    if (e.type == ENT_OBS_F18 || e.type == ENT_ROCK_1 || e.type == ENT_ROCK_2) {
        e.isLethal = 0;
    }

    switch (e.type) {
        case ENT_OBS_A1:  e.model = &gObsModels.mA1;  e.radius = 1.85f * e.scale; break;
        case ENT_OBS_B2:  e.model = &gObsModels.mB2;  e.radius = 3.20f * e.scale; e.arcAngleDeg = 180.0f; e.animMode = 0; break;
        case ENT_OBS_B3:  e.model = &gObsModels.mB3;  e.radius = 2.50f * e.scale; e.arcAngleDeg = 360.0f; e.animMode = -1; break;
        case ENT_OBS_B5:  e.model = &gObsModels.mB5;  e.radius = 3.80f * e.scale; e.arcAngleDeg = 360.0f; e.animMode = -1; break;
        case ENT_OBS_C1:  e.model = &gObsModels.mC1;  e.radius = 2.50f * e.scale; e.arcAngleDeg = 180.0f; e.animMode = 0; break;
        case ENT_OBS_C2:  e.model = &gObsModels.mC2;  e.radius = 2.20f * e.scale; e.arcAngleDeg = 130.0f; e.animMode = 0; break;
        case ENT_OBS_C4:  e.model = &gObsModels.mC4;  e.radius = 2.50f * e.scale; e.arcAngleDeg = 360.0f; e.animMode = -1; break;
        case ENT_OBS_C5:  e.model = &gObsModels.mC5;  e.radius = 3.80f * e.scale; e.arcAngleDeg = 360.0f; e.animMode = -1; break;
        case ENT_OBS_F3:  e.model = &gObsModels.mF3;  e.radius = 2.20f * e.scale; e.arcAngleDeg = 180.0f; e.animMode = 0; break;
        case ENT_OBS_F10: e.model = &gObsModels.mF10; e.radius = 3.50f * e.scale; e.arcAngleDeg = 180.0f; e.animMode = 0; break;
        case ENT_OBS_F11: e.model = &gObsModels.mF11; e.radius = 1.65f * e.scale; break;
        case ENT_OBS_F13: e.model = &gObsModels.mF13; e.radius = 2.80f * e.scale; e.arcAngleDeg = 360.0f; e.animMode = -1; break;
        case ENT_OBS_F14: e.model = &gObsModels.mF14; e.radius = 2.40f * e.scale; e.slideDist = 1.80f; break;
        case ENT_OBS_F18: e.model = &gObsModels.mF18; e.radius = 1.20f * e.scale; e.isLethal = 0; e.bounceMult = 1.65f; break;
        case ENT_OBS_F23: e.model = &gObsModels.mF23; e.radius = 2.80f * e.scale; e.arcAngleDeg = 180.0f; e.animMode = 0; break;
        case ENT_ROCK_1:  e.model = &gObsModels.mR1;  e.radius = 2.10f * e.scale; e.isLethal = 0; e.bounceMult = 1.65f; break;
        case ENT_ROCK_2:  e.model = &gObsModels.mR2;  e.radius = 2.30f * e.scale; e.isLethal = 0; e.bounceMult = 1.65f; break;
        case ENT_GUST:         e.model = nullptr;         e.radius = 4.50f * e.scale; break;
        case ENT_CHECKPOINT:   e.model = &gObsModels.mCP;  e.radius = 3.50f * e.scale; break;
        case ENT_TREE_JUNGLE:  e.model = &gObsModels.mTree;e.radius = 1.35f * e.scale; break;
        // 12 Custom Collision Shapes
        case ENT_COL_FLAT_WALL:
            e.model = nullptr;
            e.sizeX = 0.25f; e.sizeY = 3.50f; e.sizeZ = 6.00f;
            e.colBehavior = 0; e.radius = 3.0f;
            break;
        case ENT_COL_FLAT_FLOOR:
            e.model = nullptr;
            e.sizeX = 8.00f; e.sizeY = 0.25f; e.sizeZ = 8.00f;
            e.colBehavior = 4; e.radius = 4.0f;
            break;
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
    auto SaveToFile = [&](const std::string& targetPath) {
        FILE* f = fopen(targetPath.c_str(), "w");
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
                "    \"colBehavior\": %d,\n"
                "    \"arcAngleDeg\": %.2f,\n"
                "    \"animMode\": %d,\n"
                "    \"phaseOffsetDeg\": %.2f,\n"
                "    \"slideDist\": %.2f,\n"
                "    \"bounceMult\": %.2f,\n"
                "    \"isLethal\": %d\n"
                "  }%s\n",
                (int)i, (int)e.type, e.position.x, e.position.y, e.position.z,
                e.rotationX, e.rotationY, e.rotationZ,
                e.flipX, e.flipY, e.flipZ,
                e.scale, e.animSpeed, e.radius,
                e.sizeX, e.sizeY, e.sizeZ, e.colBehavior,
                e.arcAngleDeg, e.animMode, e.phaseOffsetDeg,
                e.slideDist, e.bounceMult, e.isLethal,
                (i + 1 < list.size()) ? "," : "");
        }
        fprintf(f, "]\n");
        fclose(f);
    };

    SaveToFile(ResolveAsset(path));
    SaveToFile(path);
    SaveToFile("CustomEngine/level_layout.json");
    SaveToFile("../level_layout.json");
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
            float arcDeg = 180.0f;
            int aMode = 0;
            float phDeg = 0.0f;
            float slDist = 1.80f;
            float bMult = 1.65f;
            int lethal = ((t >= ENT_OBS_A1 && t <= ENT_OBS_F23 && t != ENT_OBS_F18) || t == ENT_COL_KILL_ZONE) ? 1 : 0;

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

            bool hasArc = ParseJsonFloatField(objStart, objEnd, "\"arcAngleDeg\"", arcDeg);
            ParseJsonIntField(objStart, objEnd, "\"animMode\"", aMode);
            ParseJsonFloatField(objStart, objEnd, "\"phaseOffsetDeg\"", phDeg);
            ParseJsonFloatField(objStart, objEnd, "\"slideDist\"", slDist);
            ParseJsonFloatField(objStart, objEnd, "\"bounceMult\"", bMult);
            bool hasLethal = ParseJsonIntField(objStart, objEnd, "\"isLethal\"", lethal);

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
            if (hasArc) {
                e.arcAngleDeg = arcDeg;
                e.animMode = aMode;
                e.phaseOffsetDeg = phDeg;
                e.slideDist = slDist;
                e.bounceMult = bMult;
            }
            if (hasLethal) {
                e.isLethal = lethal;
            }
            list.push_back(e);
        }
        cursor = objEnd + 1;
    }
    UnloadFileText(rawText);
    return !list.empty();
}

// Plan v7.0 Slanted & Trapezoid UI Drawing Primitives (Authentic Fart Together UI)
static void DrawSlantedRect(float x, float y, float w, float h, float slant, Color col) {
    Vector2 v1 = { x + slant, y };
    Vector2 v2 = { x,         y + h };
    Vector2 v3 = { x + w,     y + h };
    Vector2 v4 = { x + w + slant, y };
    DrawTriangle(v1, v2, v3, col);
    DrawTriangle(v1, v3, v4, col);
}

static void DrawSlantedRectLines(float x, float y, float w, float h, float slant, float thick, Color col) {
    Vector2 v1 = { x + slant, y };
    Vector2 v2 = { x,         y + h };
    Vector2 v3 = { x + w,     y + h };
    Vector2 v4 = { x + w + slant, y };
    DrawLineEx(v1, v2, thick, col);
    DrawLineEx(v2, v3, thick, col);
    DrawLineEx(v3, v4, thick, col);
    DrawLineEx(v4, v1, thick, col);
}

static void DrawTrapezoid(float x, float y, float w, float h, float inset, Color col) {
    Vector2 v1 = { x + inset, y };
    Vector2 v2 = { x,         y + h };
    Vector2 v3 = { x + w,     y + h };
    Vector2 v4 = { x + w - inset, y };
    DrawTriangle(v1, v2, v3, col);
    DrawTriangle(v1, v3, v4, col);
}

static void DrawTrapezoidLines(float x, float y, float w, float h, float inset, float thick, Color col) {
    Vector2 v1 = { x + inset, y };
    Vector2 v2 = { x,         y + h };
    Vector2 v3 = { x + w,     y + h };
    Vector2 v4 = { x + w - inset, y };
    DrawLineEx(v1, v2, thick, col);
    DrawLineEx(v2, v3, thick, col);
    DrawLineEx(v3, v4, thick, col);
    DrawLineEx(v4, v1, thick, col);
}

// 3D Isometric Shape Miniature Renderer for Left Dock Shape Cards (Plan v7.0 Task 3)
static void DrawColliderShapeIcon2D(int shapeIdx, Rectangle r, Color iconCol) {
    float cx = r.x + 20.0f;
    float cy = r.y + r.height * 0.5f;

    switch (shapeIdx) {
        case 0: { // Flat Wall (Cyan thin vertical wall plane)
            Vector2 p1 = { cx - 10, cy - 14 };
            Vector2 p2 = { cx + 8,  cy - 8 };
            Vector2 p3 = { cx + 8,  cy + 14 };
            Vector2 p4 = { cx - 10, cy + 8 };
            DrawTriangle(p1, p4, p3, ColorAlpha(iconCol, 0.45f));
            DrawTriangle(p1, p3, p2, ColorAlpha(iconCol, 0.45f));
            DrawLineEx(p1, p2, 1.8f, iconCol);
            DrawLineEx(p2, p3, 1.8f, iconCol);
            DrawLineEx(p3, p4, 1.8f, iconCol);
            DrawLineEx(p4, p1, 1.8f, iconCol);
            break;
        }
        case 1: { // Flat Floor (Green/Cyan horizontal ground grid)
            Vector2 p1 = { cx,      cy - 12 };
            Vector2 p2 = { cx + 15, cy };
            Vector2 p3 = { cx,      cy + 12 };
            Vector2 p4 = { cx - 15, cy };
            DrawTriangle(p1, p4, p3, ColorAlpha(iconCol, 0.40f));
            DrawTriangle(p1, p3, p2, ColorAlpha(iconCol, 0.40f));
            DrawLineEx(p1, p2, 1.5f, iconCol);
            DrawLineEx(p2, p3, 1.5f, iconCol);
            DrawLineEx(p3, p4, 1.5f, iconCol);
            DrawLineEx(p4, p1, 1.5f, iconCol);
            DrawLineEx(Vector2{ cx, cy - 6 }, Vector2{ cx, cy + 6 }, 1.2f, iconCol);
            DrawLineEx(Vector2{ cx - 7, cy }, Vector2{ cx + 7, cy }, 1.2f, iconCol);
            break;
        }
        case 2: { // Square
            DrawRectangleLinesEx(Rectangle{ cx - 11, cy - 11, 22, 22 }, 1.8f, iconCol);
            DrawLineEx(Vector2{ cx - 11, cy - 11 }, Vector2{ cx + 11, cy + 11 }, 1.0f, ColorAlpha(iconCol, 0.5f));
            break;
        }
        case 3: { // Rect Wall
            DrawRectangleLinesEx(Rectangle{ cx - 14, cy - 9, 28, 18 }, 1.8f, iconCol);
            DrawRectangle(cx - 12, cy - 7, 24, 14, ColorAlpha(iconCol, 0.35f));
            break;
        }
        case 4: { // Circle
            DrawCircleLines(cx, cy, 12.0f, iconCol);
            DrawCircle(cx, cy, 10.0f, ColorAlpha(iconCol, 0.35f));
            break;
        }
        case 5: { // 3D Cube
            Vector2 top[4] = { { cx, cy - 14 }, { cx + 11, cy - 7 }, { cx, cy }, { cx - 11, cy - 7 } };
            DrawTriangle(top[0], top[3], top[2], ColorBrightness(iconCol, 0.2f));
            DrawTriangle(top[0], top[2], top[1], ColorBrightness(iconCol, 0.2f));
            Vector2 left[4] = { { cx - 11, cy - 7 }, { cx, cy }, { cx, cy + 12 }, { cx - 11, cy + 5 } };
            DrawTriangle(left[0], left[3], left[2], ColorAlpha(iconCol, 0.85f));
            DrawTriangle(left[0], left[2], left[1], ColorAlpha(iconCol, 0.85f));
            Vector2 right[4] = { { cx, cy }, { cx + 11, cy - 7 }, { cx + 11, cy + 5 }, { cx, cy + 12 } };
            DrawTriangle(right[0], right[2], right[1], ColorBrightness(iconCol, -0.2f));
            DrawTriangle(right[0], right[3], right[2], ColorBrightness(iconCol, -0.2f));
            break;
        }
        case 6: { // Cylinder
            DrawEllipseLines(cx, cy - 7, 10, 4, iconCol);
            DrawEllipseLines(cx, cy + 7, 10, 4, iconCol);
            DrawLineEx(Vector2{ cx - 10, cy - 7 }, Vector2{ cx - 10, cy + 7 }, 1.5f, iconCol);
            DrawLineEx(Vector2{ cx + 10, cy - 7 }, Vector2{ cx + 10, cy + 7 }, 1.5f, iconCol);
            break;
        }
        case 7: { // Capsule
            DrawCircleLines(cx, cy - 6, 8.0f, iconCol);
            DrawCircleLines(cx, cy + 6, 8.0f, iconCol);
            DrawLineEx(Vector2{ cx - 8, cy - 6 }, Vector2{ cx - 8, cy + 6 }, 1.5f, iconCol);
            DrawLineEx(Vector2{ cx + 8, cy - 6 }, Vector2{ cx + 8, cy + 6 }, 1.5f, iconCol);
            break;
        }
        case 8: { // Sphere
            DrawCircleLines(cx, cy, 11.0f, iconCol);
            DrawEllipseLines(cx, cy, 11, 4, iconCol);
            DrawEllipseLines(cx, cy, 4, 11, iconCol);
            break;
        }
        case 9: { // Wedge Ramp
            Vector2 w1 = { cx - 12, cy + 10 };
            Vector2 w2 = { cx + 12, cy + 10 };
            Vector2 w3 = { cx + 12, cy - 10 };
            DrawTriangle(w1, w2, w3, ColorAlpha(iconCol, 0.45f));
            DrawLineEx(w1, w2, 1.8f, iconCol);
            DrawLineEx(w2, w3, 1.8f, iconCol);
            DrawLineEx(w3, w1, 1.8f, iconCol);
            break;
        }
        case 10: { // Fall Zone (Orange box with down arrows)
            DrawRectangleLinesEx(Rectangle{ cx - 12, cy - 10, 24, 20 }, 1.5f, iconCol);
            DrawLineEx(Vector2{ cx - 4, cy - 6 }, Vector2{ cx - 4, cy + 4 }, 2.0f, iconCol);
            DrawLineEx(Vector2{ cx - 7, cy + 1 }, Vector2{ cx - 4, cy + 5 }, 2.0f, iconCol);
            DrawLineEx(Vector2{ cx - 1, cy + 1 }, Vector2{ cx - 4, cy + 5 }, 2.0f, iconCol);
            DrawLineEx(Vector2{ cx + 4, cy - 6 }, Vector2{ cx + 4, cy + 4 }, 2.0f, iconCol);
            DrawLineEx(Vector2{ cx + 1, cy + 1 }, Vector2{ cx + 4, cy + 5 }, 2.0f, iconCol);
            DrawLineEx(Vector2{ cx + 7, cy + 1 }, Vector2{ cx + 4, cy + 5 }, 2.0f, iconCol);
            break;
        }
        case 11: { // Kill Box (Red danger box with cross)
            DrawRectangleLinesEx(Rectangle{ cx - 12, cy - 10, 24, 20 }, 1.8f, iconCol);
            DrawLineEx(Vector2{ cx - 6, cy - 5 }, Vector2{ cx + 6, cy + 5 }, 2.2f, iconCol);
            DrawLineEx(Vector2{ cx + 6, cy - 5 }, Vector2{ cx - 6, cy + 5 }, 2.2f, iconCol);
            break;
        }
        default: break;
    }
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
    static int gTestSettingsTab = 3;

    std::string testScreenArg = "";
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--test-screen") == 0 && i + 1 < argc) testScreenArg = argv[++i];
        if (strcmp(argv[i], "--test-screenshot") == 0) testScreenshotMode = true;
        if (strcmp(argv[i], "--test-settings-tab") == 0 && i + 1 < argc) gTestSettingsTab = atoi(argv[++i]);
        if (strcmp(argv[i], "--test-editor") == 0) testEditorMode = true;
        if (strcmp(argv[i], "--test-catalog") == 0) { testCatalogMode = true; testEditorMode = true; }
        if (strcmp(argv[i], "--test-death") == 0) testDeathMode = true;
        if (strcmp(argv[i], "--test-lan") == 0) testLanMode = true;
        if (strcmp(argv[i], "--test-select") == 0 && i + 1 < argc) testSelectIdx = atoi(argv[++i]);

        if (strcmp(argv[i], "--shot-name") == 0 && i + 1 < argc) {
            testShotName = argv[++i];
            if (testShotName.find(".png") == std::string::npos) testShotName += ".png";
            if (testShotName.find("/") == std::string::npos && testShotName.find("\\") == std::string::npos) {
                testShotName = "screenshots/" + testShotName;
            }
            testScreenshotMode = true;
        }

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
    int gActiveGizmoAxis = -1; int gActiveGizmoMode = 0; // -1=none, 0=X, 1=Y, 2=Z
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

    Font fontUI     = LoadFontEx(ResolveAsset("extracted_assets/fonts/ui_font.ttf").c_str(), 36, (int*)gArFontCodepoints, gArFontCodepointCount);
    Font fontUIBold = LoadFontEx(ResolveAsset("extracted_assets/fonts/ui_font_bold.ttf").c_str(), 42, (int*)gArFontCodepoints, gArFontCodepointCount);
    SetTextureFilter(fontUI.texture, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(fontUIBold.texture, TEXTURE_FILTER_BILINEAR);
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

    // Plan v7.3: Extract Exact 3D Triangle Collision Meshes for All Obstacles
    ExtractTriMeshFromModel(gObsModels.mA1,          gObsTriMeshes[ENT_OBS_A1]);
    ExtractTriMeshFromModel(gObsModels.mB2,          gObsTriMeshes[ENT_OBS_B2]);
    ExtractTriMeshFromModel(gObsModels.mB3,          gObsTriMeshes[ENT_OBS_B3]);
    ExtractTriMeshFromModel(gObsModels.mB5,          gObsTriMeshes[ENT_OBS_B5]);
    ExtractTriMeshFromModel(gObsModels.mC1,          gObsTriMeshes[ENT_OBS_C1]);
    ExtractTriMeshFromModel(gObsModels.mC2,          gObsTriMeshes[ENT_OBS_C2]);
    ExtractTriMeshFromModel(gObsModels.mC4,          gObsTriMeshes[ENT_OBS_C4]);
    ExtractTriMeshFromModel(gObsModels.mC5,          gObsTriMeshes[ENT_OBS_C5]);
    ExtractTriMeshFromModel(gObsModels.mF3,          gObsTriMeshes[ENT_OBS_F3]);
    ExtractTriMeshFromModel(gObsModels.mF10,         gObsTriMeshes[ENT_OBS_F10]);
    ExtractTriMeshFromModel(gObsModels.mF10_Arms,    gObsTriMeshF10_Arms);
    ExtractTriMeshFromModel(gObsModels.mF10_Hammer,  gObsTriMeshF10_Hammer);
    ExtractTriMeshFromModel(gObsModels.mF11,         gObsTriMeshes[ENT_OBS_F11]);
    ExtractTriMeshFromModel(gObsModels.mF13,         gObsTriMeshes[ENT_OBS_F13]);
    ExtractTriMeshFromModel(gObsModels.mF14,         gObsTriMeshes[ENT_OBS_F14]);
    ExtractTriMeshFromModel(gObsModels.mF18,         gObsTriMeshes[ENT_OBS_F18]);
    ExtractTriMeshFromModel(gObsModels.mF23,         gObsTriMeshes[ENT_OBS_F23]);
    ExtractTriMeshFromModel(gObsModels.mR1,          gObsTriMeshes[ENT_ROCK_1]);
    ExtractTriMeshFromModel(gObsModels.mR2,          gObsTriMeshes[ENT_ROCK_2]);

    Model crocModel = LoadModel(ResolveAsset("extracted_assets/meshes/GM_3D_Crocodile.obj").c_str());
    Model sharkModel= LoadModel(ResolveAsset("extracted_assets/meshes/Shark.obj").c_str());

    // Canyon Environment Clustered Sectors (scene_2..15, 14 sectors fully loaded on PC & Android!)
    static const int ENV_SECTOR_COUNT = 14;
    Model envSectors[ENV_SECTOR_COUNT];
    BoundingBox envSectorBounds[ENV_SECTOR_COUNT];
    for (int s = 0; s < ENV_SECTOR_COUNT; s++) {
        std::string secName = "extracted_assets/meshes/Combined_Mesh_root__scene_" + std::to_string(s + 2) + ".obj";
        envSectors[s] = LoadModel(ResolveAsset(secName).c_str());
        if (envSectors[s].meshCount > 0) {
            envSectorBounds[s] = GetMeshBoundingBox(envSectors[s].meshes[0]);
            for (int m = 1; m < envSectors[s].meshCount; m++) {
                BoundingBox mb = GetMeshBoundingBox(envSectors[s].meshes[m]);
                envSectorBounds[s].min.x = fminf(envSectorBounds[s].min.x, mb.min.x);
                envSectorBounds[s].min.y = fminf(envSectorBounds[s].min.y, mb.min.y);
                envSectorBounds[s].min.z = fminf(envSectorBounds[s].min.z, mb.min.z);
                envSectorBounds[s].max.x = fmaxf(envSectorBounds[s].max.x, mb.max.x);
                envSectorBounds[s].max.y = fmaxf(envSectorBounds[s].max.y, mb.max.y);
                envSectorBounds[s].max.z = fmaxf(envSectorBounds[s].max.z, mb.max.z);
            }
        }
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
        if (envSectors[s].meshCount > 0) ApplyBgrMat(envSectors[s]);
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

    // 7 New Audio Effects & Music (Plan v8.0 Task 1)
    Sound sndMenuBeachOst = LoadSound(ResolveAudioFile("menu_beach_ost.wav").c_str());
    Sound sndSeagull      = LoadSound(ResolveAudioFile("seagull_call.wav").c_str());
    Sound sndShipHorn     = LoadSound(ResolveAudioFile("ship_horn.wav").c_str());
    Sound sndSnore        = LoadSound(ResolveAudioFile("snore_loop.wav").c_str());
    Sound sndEatMunch     = LoadSound(ResolveAudioFile("eat_munch.wav").c_str());
    Sound sndBurp         = LoadSound(ResolveAudioFile("burp.wav").c_str());
    Sound sndCheckpoint   = LoadSound(ResolveAudioFile("checkpoint_chime.wav").c_str());

    // Vibrant Turquoise Fallback Base Texture (Prevents any black water on Adreno/Mali GPUs!)
    Image imgCyan = GenImageColor(64, 64, Color{ 28, 168, 235, 255 });
    Texture2D texWaterCyan = LoadTextureFromImage(imgCyan);
    UnloadImage(imgCyan);
    waterModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texWaterCyan;

    // Preload Main Menu Animated Video Frames (Plan v8.3.1) - High Quality PNG (36 frames)
    std::vector<Texture2D> menuVideoFrames;
#if defined(PLATFORM_ANDROID) || defined(__ANDROID__)
    const int frameStep = 2; // 18 frames on mobile for instant loading
#else
    const int frameStep = 1; // 36 full frames on PC
#endif
    for (int f = 1; f <= 36; f += frameStep) {
        char fName[128];
        snprintf(fName, sizeof(fName), "extracted_assets/textures/menu_frames/frame_%03d.png", f);
        std::string rPath = ResolveAsset(fName);
        if (FileExists(rPath.c_str())) {
            Texture2D ft = LoadTexture(rPath.c_str());
            if (ft.id != 0) {
                SetTextureFilter(ft, TEXTURE_FILTER_BILINEAR);
                menuVideoFrames.push_back(ft);
            }
        }
    }

    // 3 Oasis Animated Scene Textures (Plan v8.0 Task 1 & 2)
    Texture2D texMenuOasisFull  = LoadTexture(ResolveAsset("extracted_assets/textures/Menu_Oasis_Full.png").c_str());
    Texture2D texMenuOasisClean = LoadTexture(ResolveAsset("extracted_assets/textures/Menu_Oasis_CleanPond.png").c_str());
    Texture2D texMenuRaftSleep  = LoadTexture(ResolveAsset("extracted_assets/textures/Menu_Pirate_SleepingRaft.png").c_str());
    SetTextureFilter(texMenuOasisFull, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(texMenuOasisClean, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(texMenuRaftSleep, TEXTURE_FILTER_BILINEAR);

    // Load authentic UI textures for Main Menu, Lobby, and Settings (Plan v7.0 Task 4)
    Texture2D texMainLogo        = LoadTexture(ResolveAsset("extracted_assets/textures/Icon_Logo.png").c_str());
    Texture2D texLogoSplash      = LoadTexture(ResolveAsset("extracted_assets/textures/Logo_Splash.png").c_str());
    Texture2D texMenuBg          = LoadTexture(ResolveAsset("extracted_assets/textures/Background.png").c_str());
    Texture2D texMarkerYou       = LoadTexture(ResolveAsset("extracted_assets/textures/Marker_BlueYou.png").c_str());
    Texture2D texIconTikGreen   = LoadTexture(ResolveAsset("extracted_assets/textures/Icon_TikGreen.png").c_str());
    Texture2D texMarkerPlayer   = LoadTexture(ResolveAsset("extracted_assets/textures/Marker_BluePlayer.png").c_str());
    Texture2D texKeySpace       = LoadTexture(ResolveAsset("extracted_assets/textures/keyboard_space.png").c_str());
    Texture2D texIconKEnter     = LoadTexture(ResolveAsset("extracted_assets/textures/Icon_KEnter.png").c_str());
    SetTextureFilter(texIconTikGreen, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(texMarkerPlayer, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(texKeySpace, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(texIconKEnter, TEXTURE_FILTER_BILINEAR);
    Texture2D texModeFartRun     = LoadTexture(ResolveAsset("extracted_assets/textures/Mode_FartRun.png").c_str());
    Texture2D texModeTeamRace    = LoadTexture(ResolveAsset("extracted_assets/textures/Mode_TeamRace.png").c_str());
    Texture2D texModeRaftRumble  = LoadTexture(ResolveAsset("extracted_assets/textures/Mode_RaftRumble.png").c_str());
    Texture2D texIconGeneral     = LoadTexture(ResolveAsset("extracted_assets/textures/Icon_General.png").c_str());
    Texture2D texIconGraphic     = LoadTexture(ResolveAsset("extracted_assets/textures/Icon_Graphic.png").c_str());
    Texture2D texIconSound       = LoadTexture(ResolveAsset("extracted_assets/textures/Icon_Sound.png").c_str());
    Texture2D texIconControl     = LoadTexture(ResolveAsset("extracted_assets/textures/Icon_Control.png").c_str());
    Texture2D texIconDiscord     = LoadTexture(ResolveAsset("extracted_assets/textures/Icon_Discord.png").c_str());
    Texture2D texIconCopy        = LoadTexture(ResolveAsset("extracted_assets/textures/Icon_Copy.png").c_str());
    Texture2D texIconArrow       = LoadTexture(ResolveAsset("extracted_assets/textures/Icon_ArrowM.png").c_str());
    SetTextureFilter(texMainLogo, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(texMarkerYou, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(texModeFartRun, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(texModeTeamRace, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(texModeRaftRumble, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(texIconGeneral, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(texIconGraphic, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(texIconSound, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(texIconControl, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(texIconDiscord, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(texIconCopy, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(texIconArrow, TEXTURE_FILTER_BILINEAR);

    Texture2D texLobbyCards[4] = {
        LoadTexture(ResolveAsset("extracted_assets/textures/Character_LobbyGrey.png").c_str()),
        LoadTexture(ResolveAsset("extracted_assets/textures/Character_LobbyGreen.png").c_str()),
        LoadTexture(ResolveAsset("extracted_assets/textures/Character_LobbyBlue.png").c_str()),
        LoadTexture(ResolveAsset("extracted_assets/textures/Character_LobbyOrange.png").c_str())
    };
    for (int k = 0; k < 4; k++) SetTextureFilter(texLobbyCards[k], TEXTURE_FILTER_BILINEAR);

    Texture2D texLobbyChars[4] = {
        LoadTexture(ResolveAsset("extracted_assets/textures/Character_LobbyGreen.png").c_str()),
        LoadTexture(ResolveAsset("extracted_assets/textures/Character_LobbyOrange.png").c_str()),
        LoadTexture(ResolveAsset("extracted_assets/textures/Character_LobbyBlue.png").c_str()),
        LoadTexture(ResolveAsset("extracted_assets/textures/Character_LobbyGrey.png").c_str())
    };
    int selectedLobbyChar = 0;
    LoadGameSettingsJSON("game_settings.json");

    auto ApplySettingsLive = [&]() {
        float mVol = Clamp(gSettings.masterVolume / 100.0f, 0.0f, 1.0f);
        SetMasterVolume(mVol);
        SetSoundVolume(sndMenuBeachOst, Clamp(gSettings.musicVolume / 100.0f, 0.0f, 1.0f));
        float sfxV = Clamp(gSettings.sfxVolume / 100.0f, 0.0f, 1.0f);
        SetSoundVolume(sndSeagull, sfxV * 0.55f);
        SetSoundVolume(sndShipHorn, sfxV * 0.50f);
        SetSoundVolume(sndSnore, sfxV * 0.65f);
        SetSoundVolume(sndEatMunch, sfxV * 0.70f);
        SetSoundVolume(sndBurp, sfxV * 0.80f);
        SetSoundVolume(sndExplode, sfxV);
        SetSoundVolume(sndBounce, sfxV);
        SetSoundVolume(sndCheckpoint, sfxV * 0.90f);
        float fartV = Clamp(gSettings.fartVolume / 100.0f, 0.0f, 1.0f);
        for (int i = 0; i < 3; i++) SetSoundVolume(sndTap[i], fartV);
        for (int i = 0; i < 2; i++) SetSoundVolume(sndHold[i], fartV);
        for (int i = 0; i < 6; i++) SetSoundVolume(fartSounds[i], fartV);

        int qLevel = gSettings.qualityLevel;
        int qWaterLoc = GetShaderLocation(shaderWater, "uQualityLevel");
        if (qWaterLoc != -1) SetShaderValue(shaderWater, qWaterLoc, &qLevel, SHADER_UNIFORM_INT);
        int qLightLoc = GetShaderLocation(shaderLighting, "uQualityLevel");
        if (qLightLoc != -1) SetShaderValue(shaderLighting, qLightLoc, &qLevel, SHADER_UNIFORM_INT);

        TextureFilter filter = (qLevel == 0) ? TEXTURE_FILTER_POINT :
                               (qLevel == 1) ? TEXTURE_FILTER_BILINEAR :
                               (qLevel == 2) ? TEXTURE_FILTER_TRILINEAR : TEXTURE_FILTER_ANISOTROPIC_4X;
        SetTextureFilter(texPalette, filter);
        SetTextureFilter(texRockDark, filter);
        SetTextureFilter(texCaustic, filter);
        SetTextureFilter(texFoam1, filter);
        SetTextureFilter(texFoam2, filter);

        if (qLevel == 0) SetTargetFPS(30);
        else if (qLevel == 1) SetTargetFPS(60);
        else if (qLevel == 2) SetTargetFPS(60);
        else if (qLevel == 3) SetTargetFPS(120);
    };
    ApplySettingsLive();

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
        LanInitHost(7777, nullptr);
        currentScreen = SCREEN_LAN_LOBBY;
        testScreenshotMode = true;
    } else if (testScreenArg == "settings") {
        currentScreen = SCREEN_SETTINGS;
        testScreenshotMode = true;
    } else if (testScreenArg == "find_lobby") {
        LanStartDiscoveryListener(7778);
        currentScreen = SCREEN_FIND_LOBBY;
        testScreenshotMode = true;
    } else if (testScreenArg == "mobile") {
        currentScreen = SCREEN_GAMEPLAY;
        gSettings.deviceMode = 2;
        gSettings.showFPS = true;
        testScreenshotMode = true;
    } else if (testScreenArg == "checkpoint") {
        currentScreen = SCREEN_GAMEPLAY;
        gSettings.showFPS = true;
        raft.position = Vector3{ 16.5f, 0.45f, 64.0f };
        raft.velocity = Vector3{ 0.0f, 0.0f, -4.5f };
        raft.currentCP = 1;
        testScreenshotMode = true;
    }

    bool editorMode = testEditorMode;
    bool editorAnimateObstacles = true; // Natural obstacle motion enabled by default!
    int selectedObs = (testSelectIdx >= 0) ? testSelectIdx : (testEditorMode ? 0 : -1);
    Vector3 editorCamPos = testEditorMode ? Vector3{ 17.2f, 2.8f, 70.8f } : Vector3{ 15.55f, 6.0f, 75.0f };
    float editorCamYaw = 3.14159f;
    float editorCamPitch = 0.35f;
    if (testEditorMode && selectedObs >= 0 && selectedObs < (int)levelEntities.size()) {
        editorCamPos = Vector3Add(levelEntities[selectedObs].position, Vector3{ 0.0f, 3.5f, 6.0f });
        editorCamYaw = 3.14159f;
        editorCamPitch = 0.35f;
    }
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

        bool isMobileTouchHUD = (gSettings.deviceMode == 2) || (gSettings.deviceMode == 0 && (GetTouchPointCount() > 0));
        bool touchSteerLeft = false;
        bool touchSteerRight = false;
        bool touchGas = false;
        bool touchSettings = false;
        float sW = (float)GetScreenWidth();
        float sH = (float)GetScreenHeight();
        Vector2 posLeftBtn  = Vector2{ gSettings.steerLeftPosX * sW, gSettings.steerLeftPosY * sH };
        Vector2 posRightBtn = Vector2{ gSettings.steerRightPosX * sW, gSettings.steerRightPosY * sH };
        Vector2 posGasBtn   = Vector2{ gSettings.gasBtnPosX * sW, gSettings.gasBtnPosY * sH };
        Vector2 posGearBtn  = Vector2{ sW - 55.0f, 45.0f };
        float rSteer = 48.0f * gSettings.touchBtnScale;
        float rGas   = 62.0f * gSettings.touchBtnScale;
        float rGear  = 24.0f;

        // Ambient coastal soundscape & OST management (Plan v8.0 Task 2)
        static float seagullTimer = 4.0f;
        static float shipHornTimer = 10.0f;
        if (currentScreen != SCREEN_GAMEPLAY) {
            if (!IsSoundPlaying(sndMenuBeachOst)) PlaySound(sndMenuBeachOst);
            seagullTimer -= dt;
            if (seagullTimer <= 0.0f) {
                PlaySound(sndSeagull);
                seagullTimer = 11.0f + (float)(rand() % 6);
            }
            shipHornTimer -= dt;
            if (shipHornTimer <= 0.0f) {
                PlaySound(sndShipHorn);
                shipHornTimer = 21.0f + (float)(rand() % 8);
            }
        } else {
            if (IsSoundPlaying(sndMenuBeachOst)) StopSound(sndMenuBeachOst);
        }

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
        bool p2InputDown = false;
        bool p2InputReleased = false;
        float p2NetRimMove = 0.0f;

        // Update Mobile Touch HUD Inputs
        isMobileTouchHUD = (gSettings.deviceMode == 2) || (gSettings.deviceMode == 0 && (GetTouchPointCount() > 0));
        sW = (float)GetScreenWidth();
        sH = (float)GetScreenHeight();
        posLeftBtn  = Vector2{ gSettings.steerLeftPosX * sW, gSettings.steerLeftPosY * sH };
        posRightBtn = Vector2{ gSettings.steerRightPosX * sW, gSettings.steerRightPosY * sH };
        posGasBtn   = Vector2{ gSettings.gasBtnPosX * sW, gSettings.gasBtnPosY * sH };
        posGearBtn  = Vector2{ sW - 55.0f, 45.0f };
        rSteer = 48.0f * gSettings.touchBtnScale;
        rGas   = 62.0f * gSettings.touchBtnScale;
        rGear  = 24.0f;

        if (isMobileTouchHUD && !editorMode) {
            int touchCount = GetTouchPointCount();
            for (int t = 0; t < touchCount; t++) {
                Vector2 tp = GetTouchPosition(t);
                if (CheckCollisionPointCircle(tp, posLeftBtn, rSteer))   touchSteerLeft = true;
                if (CheckCollisionPointCircle(tp, posRightBtn, rSteer))  touchSteerRight = true;
                if (CheckCollisionPointCircle(tp, posGasBtn, rGas))      touchGas = true;
                if (CheckCollisionPointCircle(tp, posGearBtn, rGear))    touchSettings = true;
            }
            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
                if (CheckCollisionPointCircle(mousePos, posLeftBtn, rSteer))  touchSteerLeft = true;
                if (CheckCollisionPointCircle(mousePos, posRightBtn, rSteer)) touchSteerRight = true;
                if (CheckCollisionPointCircle(mousePos, posGasBtn, rGas))     touchGas = true;
            }
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                if (CheckCollisionPointCircle(mousePos, posGearBtn, rGear))   touchSettings = true;
            }
            if (touchSettings) {
                currentScreen = SCREEN_SETTINGS;
            }

            // Mobile Camera Rotation via Screen Drag / Swipe (outside virtual buttons)
            static Vector2 s_lastTouchPos = { -1.0f, -1.0f };
            bool camDragActive = false;
            Vector2 camDragDelta = { 0.0f, 0.0f };

            int tCount = GetTouchPointCount();
            for (int t = 0; t < tCount; t++) {
                Vector2 tp = GetTouchPosition(t);
                bool onUI = CheckCollisionPointCircle(tp, posLeftBtn, rSteer * 1.35f) ||
                            CheckCollisionPointCircle(tp, posRightBtn, rSteer * 1.35f) ||
                            CheckCollisionPointCircle(tp, posGasBtn, rGas * 1.35f) ||
                            CheckCollisionPointCircle(tp, posGearBtn, rGear * 2.2f);
                if (!onUI) {
                    if (s_camTouchId == -1 || s_camTouchId == t) {
                        s_camTouchId = t;
                        if (s_lastTouchPos.x >= 0.0f) {
                            camDragDelta = Vector2{ tp.x - s_lastTouchPos.x, tp.y - s_lastTouchPos.y };
                        }
                        s_lastTouchPos = tp;
                        camDragActive = true;
                        break;
                    }
                }
            }
            if (!camDragActive) {
                s_camTouchId = -1;
                s_lastTouchPos = Vector2{ -1.0f, -1.0f };
            }

            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && !camDragActive) {
                bool onUI = CheckCollisionPointCircle(mousePos, posLeftBtn, rSteer * 1.35f) ||
                            CheckCollisionPointCircle(mousePos, posRightBtn, rSteer * 1.35f) ||
                            CheckCollisionPointCircle(mousePos, posGasBtn, rGas * 1.35f) ||
                            CheckCollisionPointCircle(mousePos, posGearBtn, rGear * 2.2f);
                if (!onUI) {
                    camDragDelta = GetMouseDelta();
                    camDragActive = true;
                }
            }

            if (camDragActive && (fabsf(camDragDelta.x) > 0.05f || fabsf(camDragDelta.y) > 0.05f)) {
                float sens = gSettings.mouseSens * 0.0040f;
                camCtrl.yaw   -= camDragDelta.x * sens;
                camCtrl.pitch += (gSettings.invertMouseY ? -1.0f : 1.0f) * camDragDelta.y * sens;
                camCtrl.pitch = Clamp(camCtrl.pitch, 0.05f, 0.95f);
                camCtrl.autoRecenterTimer = 0.0f;
            }
        }

        if (IsKeyPressed(KEY_F5)) {
            gSettings.showFPS = !gSettings.showFPS;
            SaveGameSettingsJSON("game_settings.json");
        }

                // =====================================================================
        // AAA LAN Co-op Synchronization & Dual-Pirate Input Routing (v8.4.0)
        // User Rim Rule: 1 Player = 360°, 2 Players = 1/2 each, 3 = 1/3, 4 = 1/4
        // =====================================================================
        static uint32_t netSeqHost             = 0;
        static uint32_t netSeqClient           = 0;
        static uint16_t localClientTapSeq      = 0;
        static uint16_t localClientSuicideSeq  = 0;
        static uint16_t hostLastExecClientTap  = 0;
        static uint16_t hostLastExecSuicide    = 0;
        static uint16_t hostP1TapSeq           = 0;
        static uint16_t clientLastSeenP1Tap    = 0;
        static uint16_t clientLastSeenP2Tap    = 0;
        static LanInputPacket gLatchedClientIn = { 0 };
        static float    gClientInputHoldTimer  = 0.0f;
        static bool     s_prevLocalGasDown     = false;

        auto ApplyPirateRimMovement = [](float& angle, float moveInput, float dtStep, int playerIndex, int totalPlayers) {
            if (totalPlayers <= 1) {
                // 1 Player: 360 degrees full rotation around rim
                angle += moveInput * 2.60f * dtStep;
                while (angle >  PI) angle -= 2.0f * PI;
                while (angle < -PI) angle += 2.0f * PI;
                return;
            }
            angle += moveInput * 2.50f * dtStep;
            if (totalPlayers == 2) {
                // 2 Players: Half boat each (P1 left [-3.10f, -0.05f], P2 right [0.05f, 3.10f])
                if (playerIndex == 0) {
                    angle = Clamp(angle, -3.10f, -0.05f);
                } else {
                    angle = Clamp(angle,  0.05f,  3.10f);
                }
            } else if (totalPlayers == 3) {
                // 3 Players: Third of boat each
                if (playerIndex == 0) {
                    angle = Clamp(angle, -3.10f, -1.08f);
                } else if (playerIndex == 1) {
                    angle = Clamp(angle, -1.02f,  1.02f);
                } else {
                    angle = Clamp(angle,  1.08f,  3.10f);
                }
            } else if (totalPlayers >= 4) {
                // 4 Players: Quarter of boat each
                if (playerIndex == 0) {
                    angle = Clamp(angle, -3.10f, -1.60f);
                } else if (playerIndex == 1) {
                    angle = Clamp(angle, -1.54f, -0.05f);
                } else if (playerIndex == 2) {
                    angle = Clamp(angle,  0.05f,  1.54f);
                } else {
                    angle = Clamp(angle,  1.60f,  3.10f);
                }
            }
        };

        int activeTotalPlayers = (LanGetRole() == LAN_OFFLINE) ? 1 : gConnectedPlayers;
        if (activeTotalPlayers < 1) activeTotalPlayers = 1;

        // 1. Read Local Player Unified Input (Keyboard + Touch HUD + Gamepad)
        float localRimMove = 0.0f;
        if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)  || touchSteerLeft)  localRimMove -= 1.0f;
        if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT) || touchSteerRight) localRimMove += 1.0f;
        if (IsGamepadAvailable(0)) {
            float gpAxis = GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X);
            if (fabsf(gpAxis) > 0.15f) localRimMove += gpAxis;
        }
        localRimMove = Clamp(localRimMove, -1.0f, 1.0f);

        bool gp0Down = IsGamepadAvailable(0) && (IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN) || GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_TRIGGER) > 0.3f);
        bool localGasDown = (!raft.isDestroyed) && (IsKeyDown(KEY_W) || IsKeyDown(KEY_SPACE) || IsKeyDown(KEY_UP) || gp0Down || touchGas);

        // Automated sequence for test screenshot
        if (testScreenshotMode) {
            if (testFrameCount >= 20 && testFrameCount < 52) localGasDown = true;
            else if (testFrameCount == 52) localGasDown = false;
        }

        // True edge detection for Gas Release (Works identically on PC Keyboard & Mobile Touch!)
        bool localGasReleased = (s_prevLocalGasDown && !localGasDown);
        s_prevLocalGasDown = localGasDown;

        float rimRadius = 0.94f;
        Matrix boatRotM = MatrixRotateY(raft.rotationY);

        if (LanGetRole() == LAN_CLIENT) {
            // -----------------------------------------------------------------
            // CLIENT MODE: Local Player controls PIRATE 2 (p2RimAngle, gasP2)
            // -----------------------------------------------------------------
            singlePirateMode = false;

            // Client-Side Prediction for Pirate 2 rim rotation with sector bounds
            if (!raft.isDestroyed) {
                ApplyPirateRimMovement(p2RimAngle, localRimMove, dt, 1, activeTotalPlayers);
            }

            Vector3 p2OutwardLocal = Vector3{ sinf(p2RimAngle), 0.0f, cosf(p2RimAngle) };
            Vector3 p2ThrustLocal  = Vector3Scale(p2OutwardLocal, -1.0f);
            Vector3 p2ThrustWorld  = Vector3Transform(p2ThrustLocal, boatRotM);
            Vector3 p2OutwardWorld = Vector3Transform(p2OutwardLocal, boatRotM);
            Vector3 p2PosWorld     = Vector3Add(raft.position, Vector3Transform(Vector3Scale(p2OutwardLocal, rimRadius), boatRotM));
            p2PosWorld.y += 0.55f;

            // Detect Client Tap Burst BEFORE gasP2.Update resets pressTimer!
            if (localGasReleased && gasP2.pressTimer <= GAS_TAP_THRESHOLD_SECONDS && gasP2.pressTimer > 0.01f && !gasP2.gasDepleted && !raft.isDestroyed) {
                if (gasP2.gasEnergy >= GAS_TAP_COST) {
                    gasP2.gasEnergy -= GAS_TAP_COST;
                    localClientTapSeq++;
                    // Client-Side Immediate Impulse Prediction
                    float tapDt    = Clamp(gasP2.pressTimer, 0.06f, 0.10f);
                    float tapAccel = (gPhysCfg.tapBurstForce / BOAT_MASS) * tapDt;
                    raft.velocity  = Vector3Add(raft.velocity, Vector3Scale(p2ThrustWorld, tapAccel));
                    float steerTq  = (p2OutwardLocal.x * p2ThrustLocal.z - p2OutwardLocal.z * p2ThrustLocal.x);
                    raft.angularVelocityY += steerTq * (gPhysCfg.steerTorque / BOAT_MASS) * 22.0f * tapDt;
                    EmitFartBurst(p2PosWorld, p2OutwardWorld, 0.4f);
                    PlaySound(sndTap[rand() % 3]);
                }
            }

            gasP2.Update(dt, localGasDown, runTime);

            // Client-Side Immediate Sustained Thrust Prediction for Pirate 2
            if (gasP2.isSustained && !raft.isDestroyed) {
                float curForce = gPhysCfg.sustainedMinForce + (gPhysCfg.sustainedMaxForce - gPhysCfg.sustainedMinForce) * gasP2.sustainedCharge;
                float accel    = (curForce / BOAT_MASS) * dt;
                raft.velocity  = Vector3Add(raft.velocity, Vector3Scale(p2ThrustWorld, accel));
                float steerTq  = (p2OutwardLocal.x * p2ThrustLocal.z - p2OutwardLocal.z * p2ThrustLocal.x);
                raft.angularVelocityY += steerTq * (gPhysCfg.steerTorque / BOAT_MASS) * 22.0f * dt;
                if (rand() % 4 == 0) {
                    EmitFartBurst(p2PosWorld, p2OutwardWorld, gasP2.sustainedCharge);
                }
            }

            if (IsKeyPressed(KEY_R)) localClientSuicideSeq++;

            // Send Authoritative Client Input Packet (with Monotonic Event Counters!) to Host
            LanInputPacket inPkt{};
            inPkt.magic          = 0x46415254;
            inPkt.type           = LAN_PKT_INPUT;
            inPkt.seq            = ++netSeqClient;
            inPkt.rimMove        = localRimMove;
            inPkt.clientRimAngle = p2RimAngle;
            inPkt.gasDown        = localGasDown ? 1 : 0;
            inPkt.gasReleased    = localGasReleased ? 1 : 0;
            inPkt.tapBurstSeq    = localClientTapSeq;
            inPkt.suicideSeq     = localClientSuicideSeq;
            LanClientSendInput(inPkt);

            // Poll Authoritative State from Host & Reconcile Smoothly
            LanStatePacket sp{};
            if (LanClientPollState(sp)) {
                Vector3 authPos = Vector3{ sp.raftX, sp.raftY, sp.raftZ };
                float posErr = Vector3Distance(raft.position, authPos);
                if (posErr > 4.5f) {
                    raft.position = authPos;
                } else {
                    raft.position = Vector3Lerp(raft.position, authPos, 0.38f);
                }
                raft.velocity = Vector3Lerp(raft.velocity, Vector3{ sp.velX, sp.velY, sp.velZ }, 0.55f);

                // Shortest-arc angular reconciliation
                float rotDiff = sp.rotY - raft.rotationY;
                while (rotDiff >  PI) rotDiff -= 2.0f * PI;
                while (rotDiff < -PI) rotDiff += 2.0f * PI;
                raft.rotationY += rotDiff * 0.45f;
                raft.angularVelocityY = sp.angVelY;
                raft.pitch = sp.pitch;
                raft.roll  = sp.roll;

                // Replicate Host's Pirate 1 State & VFX
                p1RimAngle             = sp.p1RimAngle;
                gasP1.gasEnergy        = sp.p1Energy;
                gasP1.sustainedCharge  = sp.p1Charge;
                gasP1.isSustained      = (sp.p1Sustained != 0);
                gasP1.isPressing       = (sp.p1Sustained != 0 || sp.p1Eye > 0.05f);
                gasP1.gasDepleted      = (sp.p1Depleted != 0);
                gasP1.redness          = sp.p1Redness;
                gasP1.boneScale        = 1.0f + sp.p1Swell;

                // Reconcile Pirate 2 energy from Host
                gasP2.gasEnergy        = sp.p2Energy;
                gasP2.gasDepleted      = (sp.p2Depleted != 0);

                // Trigger Host P1 Tap Burst VFX/Sound on Client when hostP1TapSeq increments
                if (sp.p1TapSeq != clientLastSeenP1Tap) {
                    clientLastSeenP1Tap = sp.p1TapSeq;
                    Vector3 p1OutL = Vector3{ sinf(p1RimAngle), 0.0f, cosf(p1RimAngle) };
                    Vector3 p1OutW = Vector3Transform(p1OutL, boatRotM);
                    Vector3 p1PosW = Vector3Add(raft.position, Vector3Transform(Vector3Scale(p1OutL, rimRadius), boatRotM));
                    p1PosW.y += 0.55f;
                    EmitFartBurst(p1PosW, p1OutW, 0.4f);
                    PlaySound(sndTap[rand() % 3]);
                }

                if (gasP1.isSustained && !raft.isDestroyed && (rand() % 4 == 0)) {
                    Vector3 p1OutL = Vector3{ sinf(p1RimAngle), 0.0f, cosf(p1RimAngle) };
                    Vector3 p1OutW = Vector3Transform(p1OutL, boatRotM);
                    Vector3 p1PosW = Vector3Add(raft.position, Vector3Transform(Vector3Scale(p1OutL, rimRadius), boatRotM));
                    p1PosW.y += 0.55f;
                    EmitFartBurst(p1PosW, p1OutW, gasP1.sustainedCharge);
                }

                raceTimer      = sp.raceTimer;
                totalDeaths    = sp.totalDeaths;
                raft.deaths    = sp.totalDeaths;
                raft.currentCP = sp.currentCP;
                if (sp.isDestroyed && !raft.isDestroyed) {
                    TriggerBoatDeath(DEATH_OBSTACLE);
                } else if (!sp.isDestroyed && raft.isDestroyed) {
                    raft.isDestroyed = false;
                    ragdollA.active  = false;
                    ragdollB.active  = false;
                }
            }

            // Play sustained hold sound if EITHER Pirate 1 or Pirate 2 is holding gas!
            if ((gasP1.isSustained || gasP2.isSustained) && !raft.isDestroyed) {
                if (!IsSoundPlaying(sndHold[0]) && !IsSoundPlaying(sndHold[1])) {
                    PlaySound(sndHold[rand() % 2]);
                }
            } else {
                if (IsSoundPlaying(sndHold[0])) StopSound(sndHold[0]);
                if (IsSoundPlaying(sndHold[1])) StopSound(sndHold[1]);
            }

        } else {
            // -----------------------------------------------------------------
            // HOST OR SOLO MODE:
            // Local Player controls PIRATE 1 (p1RimAngle, gasP1).
            // In LAN_HOST mode, Remote Client controls PIRATE 2 (p2RimAngle, gasP2)!
            // -----------------------------------------------------------------
            singlePirateMode = (LanGetRole() == LAN_OFFLINE);

            if (!raft.isDestroyed) {
                ApplyPirateRimMovement(p1RimAngle, localRimMove, dt, 0, activeTotalPlayers);
            }

            Vector3 p1OutwardLocal = Vector3{ sinf(p1RimAngle), 0.0f, cosf(p1RimAngle) };
            Vector3 p1ThrustLocal  = Vector3Scale(p1OutwardLocal, -1.0f);
            Vector3 p1ThrustWorld  = Vector3Transform(p1ThrustLocal, boatRotM);
            Vector3 p1OutwardWorld = Vector3Transform(p1OutwardLocal, boatRotM);
            Vector3 p1PosWorld     = Vector3Add(raft.position, Vector3Transform(Vector3Scale(p1OutwardLocal, rimRadius), boatRotM));
            p1PosWorld.y += 0.55f;

            if (localGasReleased && gasP1.pressTimer <= GAS_TAP_THRESHOLD_SECONDS && gasP1.pressTimer > 0.01f && !gasP1.gasDepleted && !raft.isDestroyed) {
                if (gasP1.gasEnergy >= GAS_TAP_COST) {
                    gasP1.gasEnergy -= GAS_TAP_COST;
                    hostP1TapSeq++;
                    float tapDt    = Clamp(gasP1.pressTimer, 0.06f, 0.10f);
                    float tapAccel = (gPhysCfg.tapBurstForce / BOAT_MASS) * tapDt;
                    raft.velocity  = Vector3Add(raft.velocity, Vector3Scale(p1ThrustWorld, tapAccel));
                    float steerTq  = (p1OutwardLocal.x * p1ThrustLocal.z - p1OutwardLocal.z * p1ThrustLocal.x);
                    raft.angularVelocityY += steerTq * (gPhysCfg.steerTorque / BOAT_MASS) * 22.0f * tapDt;
                    EmitFartBurst(p1PosWorld, p1OutwardWorld, 0.4f);
                    PlaySound(sndTap[rand() % 3]);
                }
            }

            gasP1.Update(dt, localGasDown, runTime);

            if (gasP1.isSustained && !raft.isDestroyed) {
                float curForce = gPhysCfg.sustainedMinForce + (gPhysCfg.sustainedMaxForce - gPhysCfg.sustainedMinForce) * gasP1.sustainedCharge;
                float accel    = (curForce / BOAT_MASS) * dt;
                raft.velocity  = Vector3Add(raft.velocity, Vector3Scale(p1ThrustWorld, accel));
                float steerTq  = (p1OutwardLocal.x * p1ThrustLocal.z - p1OutwardLocal.z * p1ThrustLocal.x);
                raft.angularVelocityY += steerTq * (gPhysCfg.steerTorque / BOAT_MASS) * 22.0f * dt;
                if (rand() % 4 == 0) {
                    EmitFartBurst(p1PosWorld, p1OutwardWorld, gasP1.sustainedCharge);
                }
            }

            // Process Remote Client (Pirate 2) in LAN_HOST Mode!
            if (LanGetRole() == LAN_HOST) {
                LanInputPacket newClientIn{};
                if (LanHostPollInput(newClientIn)) {
                    gLatchedClientIn      = newClientIn;
                    gClientInputHoldTimer = 0.18f; // Hold latched input for 180ms across Wi-Fi frame gaps!

                    // Sync Client's exact predicted rim angle smoothly
                    float angDiff = newClientIn.clientRimAngle - p2RimAngle;
                    while (angDiff >  PI) angDiff -= 2.0f * PI;
                    while (angDiff < -PI) angDiff += 2.0f * PI;
                    p2RimAngle += angDiff * 0.65f;
                    ApplyPirateRimMovement(p2RimAngle, 0.0f, dt, 1, activeTotalPlayers);
                } else {
                    if (gClientInputHoldTimer > 0.0f) {
                        gClientInputHoldTimer -= dt;
                    } else {
                        gLatchedClientIn.rimMove = 0.0f;
                        gLatchedClientIn.gasDown = 0;
                    }
                }

                if (!raft.isDestroyed) {
                    ApplyPirateRimMovement(p2RimAngle, Clamp(gLatchedClientIn.rimMove, -1.0f, 1.0f), dt, 1, activeTotalPlayers);
                }

                Vector3 p2OutwardLocal = Vector3{ sinf(p2RimAngle), 0.0f, cosf(p2RimAngle) };
                Vector3 p2ThrustLocal  = Vector3Scale(p2OutwardLocal, -1.0f);
                Vector3 p2ThrustWorld  = Vector3Transform(p2ThrustLocal, boatRotM);
                Vector3 p2OutwardWorld = Vector3Transform(p2OutwardLocal, boatRotM);
                Vector3 p2PosWorld     = Vector3Add(raft.position, Vector3Transform(Vector3Scale(p2OutwardLocal, rimRadius), boatRotM));
                p2PosWorld.y += 0.55f;

                // Execute Reliable Client Tap Burst when tapBurstSeq increments!
                if (gLatchedClientIn.tapBurstSeq != hostLastExecClientTap && !raft.isDestroyed) {
                    hostLastExecClientTap = gLatchedClientIn.tapBurstSeq;
                    if (gasP2.gasEnergy >= GAS_TAP_COST) {
                        gasP2.gasEnergy -= GAS_TAP_COST;
                    }
                    float tapDt    = 0.085f;
                    float tapAccel = (gPhysCfg.tapBurstForce / BOAT_MASS) * tapDt;
                    raft.velocity  = Vector3Add(raft.velocity, Vector3Scale(p2ThrustWorld, tapAccel));
                    float steerTq  = (p2OutwardLocal.x * p2ThrustLocal.z - p2OutwardLocal.z * p2ThrustLocal.x);
                    raft.angularVelocityY += steerTq * (gPhysCfg.steerTorque / BOAT_MASS) * 22.0f * tapDt;
                    EmitFartBurst(p2PosWorld, p2OutwardWorld, 0.4f);
                    PlaySound(sndTap[rand() % 3]);
                }

                // Execute Client Suicide Request if suicideSeq incremented
                if (gLatchedClientIn.suicideSeq != hostLastExecSuicide) {
                    hostLastExecSuicide = gLatchedClientIn.suicideSeq;
                    if (!raft.isDestroyed) TriggerBoatDeath(DEATH_SUICIDE);
                }

                bool clientGasDown = (gLatchedClientIn.gasDown != 0) && !raft.isDestroyed;
                gasP2.Update(dt, clientGasDown, runTime);

                // Apply Client Sustained Thrust to Boat Physics on Host!
                if (gasP2.isSustained && !raft.isDestroyed) {
                    float curForce = gPhysCfg.sustainedMinForce + (gPhysCfg.sustainedMaxForce - gPhysCfg.sustainedMinForce) * gasP2.sustainedCharge;
                    float accel    = (curForce / BOAT_MASS) * dt;
                    raft.velocity  = Vector3Add(raft.velocity, Vector3Scale(p2ThrustWorld, accel));
                    float steerTq  = (p2OutwardLocal.x * p2ThrustLocal.z - p2OutwardLocal.z * p2ThrustLocal.x);
                    raft.angularVelocityY += steerTq * (gPhysCfg.steerTorque / BOAT_MASS) * 22.0f * dt;
                    if (rand() % 4 == 0) {
                        EmitFartBurst(p2PosWorld, p2OutwardWorld, gasP2.sustainedCharge);
                    }
                }
            }

            // Play sustained hold sound if EITHER Pirate 1 or Pirate 2 is holding gas!
            bool anySustained = gasP1.isSustained || (!singlePirateMode && gasP2.isSustained);
            if (anySustained && !raft.isDestroyed) {
                if (!IsSoundPlaying(sndHold[0]) && !IsSoundPlaying(sndHold[1])) {
                    PlaySound(sndHold[rand() % 2]);
                }
            } else {
                if (IsSoundPlaying(sndHold[0])) StopSound(sndHold[0]);
                if (IsSoundPlaying(sndHold[1])) StopSound(sndHold[1]);
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
            for (const auto& ent : levelEntities) {
                if (ent.type == ENT_COL_FLAT_FLOOR || ent.colBehavior == 4) {
                    float dx = fabsf(raft.position.x - ent.position.x);
                    float dz = fabsf(raft.position.z - ent.position.z);
                    float hx = (ent.sizeX > 0.05f) ? ent.sizeX : 4.0f;
                    float hz = (ent.sizeZ > 0.05f) ? ent.sizeZ : 4.0f;
                    if (dx <= (hx + 0.88f) && dz <= (hz + 0.88f)) {
                        overWater = true;
                        dummyY = fmaxf(dummyY, ent.position.y);
                    }
                }
            }
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

        // Broadcast authoritative state from Host (Plan v8.4.0)
        if (LanGetRole() == LAN_HOST) {
            LanStatePacket sp{};
            sp.magic       = 0x46415254;
            sp.type        = LAN_PKT_STATE;
            sp.seq         = ++netSeqHost;
            sp.raftX       = raft.position.x;
            sp.raftY       = raft.position.y;
            sp.raftZ       = raft.position.z;
            sp.velX        = raft.velocity.x;
            sp.velY        = raft.velocity.y;
            sp.velZ        = raft.velocity.z;
            sp.rotY        = raft.rotationY;
            sp.angVelY     = raft.angularVelocityY;
            sp.pitch       = raft.pitch;
            sp.roll        = raft.roll;
            sp.p1RimAngle  = p1RimAngle;
            sp.p2RimAngle  = p2RimAngle;
            sp.p1Energy    = gasP1.gasEnergy;
            sp.p2Energy    = gasP2.gasEnergy;
            sp.p1Charge    = gasP1.sustainedCharge;
            sp.p2Charge    = gasP2.sustainedCharge;
            sp.p1Redness   = gasP1.redness;
            sp.p2Redness   = gasP2.redness;
            sp.p1Swell     = gasP1.boneScale - 1.0f;
            sp.p2Swell     = gasP2.boneScale - 1.0f;
            sp.p1Eye       = gasP1.isPressing ? Clamp(gasP1.pressTimer / 0.45f, 0.0f, 1.0f) : 0.0f;
            sp.p2Eye       = gasP2.isPressing ? Clamp(gasP2.pressTimer / 0.45f, 0.0f, 1.0f) : 0.0f;
            sp.p1Sustained = gasP1.isSustained ? 1 : 0;
            sp.p2Sustained = gasP2.isSustained ? 1 : 0;
            sp.p1Depleted  = gasP1.gasDepleted ? 1 : 0;
            sp.p2Depleted  = gasP2.gasDepleted ? 1 : 0;
            sp.p1TapSeq    = hostP1TapSeq;
            sp.p2TapSeq    = hostLastExecClientTap;
            sp.raceTimer   = raceTimer;
            sp.totalDeaths = raft.deaths;
            sp.currentCP   = raft.currentCP;
            sp.isDestroyed = raft.isDestroyed ? 1 : 0;
            LanHostBroadcastState(sp);
        }


        // 5. Check Custom Collision Shapes (12 types) & Obstacles
        const float COLLISION_ACTIVE_RADIUS_SQ = 15.0f * 15.0f; // Proximity-Gated Collision (15m radius)
        for (const auto& ent : levelEntities) {
            float cdx = raft.position.x - ent.position.x;
            float cdz = raft.position.z - ent.position.z;
            if (cdx * cdx + cdz * cdz > COLLISION_ACTIVE_RADIUS_SQ) {
                continue; // Entity further than 15m is dormant - zero collision matrix overhead!
            }
            if (ent.type >= ENT_COL_FLAT_WALL && ent.type < ENT_COUNT) {
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
                    if (ent.colBehavior == 4 || ent.type == ENT_COL_FLAT_FLOOR) {
                        overWater = true;
                        raft.position.y = fmaxf(raft.position.y, ent.position.y + 0.35f);
                    } else if (ent.colBehavior == 1 || ent.type == ENT_COL_FALL_ZONE) {
                        overWater = false;
                    } else if (ent.colBehavior == 2 || ent.type == ENT_COL_KILL_ZONE) {
                        if (!raft.isDestroyed && raft.invulnerableTimer <= 0.0f) {
                            TriggerBoatDeath(DEATH_OBSTACLE);
                        }
                    } else if (ent.colBehavior == 3) {
                        Vector3 push = Vector3Normalize(Vector3Subtract(raft.position, ent.position));
                        raft.velocity = Vector3Scale(push, 18.0f);
                        PlaySound(fartSounds[3]);
                    } else { // 0 = Solid Wall (Flat Wall, Square, Cube, etc.)
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

        // 5. Check Obstacle & Hazard Collisions with Exact 3D Triangle Mesh Precision ("تقنية المثلثات")
        for (auto& ent : levelEntities) {
            Vector3 animPos;
            float animYaw, animPitch, animRoll;
            GetEntityAnimatedTransform(ent, animPos, animYaw, animPitch, animRoll);

            if (ent.type == ENT_GUST) {
                float d = Vector3Distance(raft.position, ent.position);
                if (d < ent.radius && !raft.isDestroyed) {
                    float rad = ent.rotationY * DEG2RAD;
                    Vector3 gustDir = { sinf(rad), 0.0f, cosf(rad) };
                    raft.velocity.x += gustDir.x * 24.0f * dt;
                    raft.velocity.z += gustDir.z * 24.0f * dt;
                    camCtrl.shakeTrauma = fmaxf(camCtrl.shakeTrauma, 0.20f);
                }
                continue;
            }

            Vector3 contactNormal = { 0, 1, 0 };
            bool hit = CheckObstacleTriangleMeshCollision(raft.position, raft.rotationY, ent, animPos, animYaw, animPitch, animRoll, &contactNormal);

            if (hit) {
                if (ent.type == ENT_ROCK_1 || ent.type == ENT_ROCK_2) {
                    // Solid sliding collision against rock geometry (smooth redirection, no explosion)
                    Vector3 n = contactNormal;
                    n.y = 0.0f;
                    if (Vector3Length(n) > 0.001f) {
                        n = Vector3Normalize(n);
                        raft.position = Vector3Add(raft.position, Vector3Scale(n, 0.06f));
                        float vn = raft.velocity.x * n.x + raft.velocity.z * n.z;
                        if (vn < 0.0f) {
                            raft.velocity.x -= vn * n.x * 1.25f;
                            raft.velocity.z -= vn * n.z * 1.25f;
                        }
                    }
                } else if (ent.isLethal != 0) {
                    // Lethal Hazard: Explodes boat ONLY on true contact with obstacle mesh triangles
                    if (!raft.isDestroyed && raft.invulnerableTimer <= 0.0f) {
                        TriggerBoatDeath(DEATH_OBSTACLE);
                    }
                } else {
                    // Non-Lethal Hazard (e.g. #77 OBS_F18_SPIKE rebound pillar & custom bounce pads):
                    // Bounces and pushes the boat away safely along contact normal, NEVER exploding!
                    Vector3 push = contactNormal;
                    push.y = 0.0f;
                    if (Vector3Length(push) < 0.001f) {
                        push = Vector3Normalize(Vector3Subtract(raft.position, animPos));
                        push.y = 0.0f;
                    }
                    if (Vector3Length(push) < 0.001f) push = Vector3{ 0.0f, 0.0f, 1.0f };
                    else push = Vector3Normalize(push);

                    float impactSpd = Vector3Length(Vector3{ raft.velocity.x, 0.0f, raft.velocity.z });
                    float pushSpeed = Clamp(impactSpd * ent.bounceMult, 8.0f, 15.0f);
                    raft.velocity.x = push.x * pushSpeed;
                    raft.velocity.z = push.z * pushSpeed;
                    raft.position = Vector3Add(raft.position, Vector3Scale(push, 0.25f));
                    raft.invulnerableTimer = 0.50f; // Temporary hazard-grace timer on bounce!
                    camCtrl.shakeTrauma = fmaxf(camCtrl.shakeTrauma, Clamp(impactSpd / 12.0f, 0.25f, 0.55f));
                    PlaySound(sndBounce);
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
                        PlaySound(sndCheckpoint); // Authentic checkpoint chime! (Plan v8.0 Task 5)
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
                Vector2 sOrig   = GetWorldToScreen(oPos, camera);
                Vector2 sMoveX  = GetWorldToScreen(Vector3Add(oPos, Vector3{ 2.8f, 0.0f, 0.0f }), camera);
                Vector2 sMoveY  = GetWorldToScreen(Vector3Add(oPos, Vector3{ 0.0f, 2.8f, 0.0f }), camera);
                Vector2 sMoveZ  = GetWorldToScreen(Vector3Add(oPos, Vector3{ 0.0f, 0.0f, 2.8f }), camera);

                Vector2 sScaleX = GetWorldToScreen(Vector3Add(oPos, Vector3{ 3.8f, 0.0f, 0.0f }), camera);
                Vector2 sScaleY = GetWorldToScreen(Vector3Add(oPos, Vector3{ 0.0f, 3.8f, 0.0f }), camera);
                Vector2 sScaleZ = GetWorldToScreen(Vector3Add(oPos, Vector3{ 0.0f, 0.0f, 3.8f }), camera);

                Vector2 sRotX   = GetWorldToScreen(Vector3Add(oPos, Vector3{ 0.0f, 1.55f, 1.55f }), camera);
                Vector2 sRotY   = GetWorldToScreen(Vector3Add(oPos, Vector3{ 1.55f, 0.0f, 1.55f }), camera);
                Vector2 sRotZ   = GetWorldToScreen(Vector3Add(oPos, Vector3{ 1.55f, 1.55f, 0.0f }), camera);

                Vector2 sFlipX  = GetWorldToScreen(Vector3Add(oPos, Vector3{ -1.8f, 0.0f, 0.0f }), camera);
                Vector2 sFlipY  = GetWorldToScreen(Vector3Add(oPos, Vector3{ 0.0f, -1.8f, 0.0f }), camera);
                Vector2 sFlipZ  = GetWorldToScreen(Vector3Add(oPos, Vector3{ 0.0f, 0.0f, -1.8f }), camera);

                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !mouseOverUI) {
                    if (Vector2Distance(mousePos, sScaleX) < 18.0f) { gActiveGizmoMode = 2; gActiveGizmoAxis = 0; }
                    else if (Vector2Distance(mousePos, sScaleY) < 18.0f) { gActiveGizmoMode = 2; gActiveGizmoAxis = 1; }
                    else if (Vector2Distance(mousePos, sScaleZ) < 18.0f) { gActiveGizmoMode = 2; gActiveGizmoAxis = 2; }
                    else if (Vector2Distance(mousePos, sOrig) < 16.0f && editorGizmoMode == 3) { gActiveGizmoMode = 3; gActiveGizmoAxis = 0; }
                    else if (Vector2Distance(mousePos, sRotX) < 18.0f) { gActiveGizmoMode = 1; gActiveGizmoAxis = 0; }
                    else if (Vector2Distance(mousePos, sRotY) < 18.0f) { gActiveGizmoMode = 1; gActiveGizmoAxis = 1; }
                    else if (Vector2Distance(mousePos, sRotZ) < 18.0f) { gActiveGizmoMode = 1; gActiveGizmoAxis = 2; }
                    else if (Vector2Distance(mousePos, sFlipX) < 16.0f) { sel.flipX *= -1; sel.rotationY = fmodf(-sel.rotationY + 360.0f, 360.0f); editorMsg = "Flipped Object along X Axis!"; editorMsgTimer = 2.0f; }
                    else if (Vector2Distance(mousePos, sFlipY) < 16.0f) { sel.flipY *= -1; sel.rotationZ = fmodf(sel.rotationZ + 180.0f, 360.0f); editorMsg = "Flipped Object along Y Axis!"; editorMsgTimer = 2.0f; }
                    else if (Vector2Distance(mousePos, sFlipZ) < 16.0f) { sel.flipZ *= -1; sel.rotationY = fmodf(sel.rotationY + 180.0f, 360.0f); editorMsg = "Flipped Object along Z Axis!"; editorMsgTimer = 2.0f; }
                    else {
                        float dX = DistPointToSegment2D(mousePos, sOrig, sMoveX);
                        float dY = DistPointToSegment2D(mousePos, sOrig, sMoveY);
                        float dZ = DistPointToSegment2D(mousePos, sOrig, sMoveZ);
                        float minD = fminf(dX, fminf(dY, dZ));
                        if (minD < 20.0f) {
                            gActiveGizmoAxis = (minD == dX) ? 0 : ((minD == dY) ? 1 : 2);
                            if (editorGizmoMode == 2) gActiveGizmoMode = 1; // Rotate
                            else if (editorGizmoMode == 3) gActiveGizmoMode = 2; // Scale
                            else gActiveGizmoMode = 0; // Move
                        }
                    }
                }

                if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
                    gActiveGizmoAxis = -1;
                    gActiveGizmoMode = 0;
                }

                if (gActiveGizmoAxis >= 0 && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
                    Vector2 mDelta = GetMouseDelta();
                    Vector2 axisEnd = (gActiveGizmoAxis == 0) ? sMoveX : ((gActiveGizmoAxis == 1) ? sMoveY : sMoveZ);
                    Vector2 screenDir = { axisEnd.x - sOrig.x, axisEnd.y - sOrig.y };
                    float screenLen = fmaxf(Vector2Length(screenDir), 1.0f);
                    screenDir.x /= screenLen; screenDir.y /= screenLen;
                    float projPixels = mDelta.x * screenDir.x + mDelta.y * screenDir.y;

                    if (gActiveGizmoMode == 0) { // Move
                        float deltaWorld = projPixels * (2.8f / screenLen);
                        if (gActiveGizmoAxis == 0) sel.position.x += deltaWorld;
                        if (gActiveGizmoAxis == 1) sel.position.y += deltaWorld;
                        if (gActiveGizmoAxis == 2) sel.position.z += deltaWorld;
                    } else if (gActiveGizmoMode == 2) { // Axis Scale
                        float deltaScale = projPixels * 0.025f;
                        if (gActiveGizmoAxis == 0) sel.sizeX = fmaxf(0.10f, sel.sizeX + deltaScale);
                        if (gActiveGizmoAxis == 1) sel.sizeY = fmaxf(0.10f, sel.sizeY + deltaScale);
                        if (gActiveGizmoAxis == 2) sel.sizeZ = fmaxf(0.10f, sel.sizeZ + deltaScale);
                        sel.scale = fmaxf(0.15f, (sel.sizeX + sel.sizeY + sel.sizeZ) / 3.0f);
                    } else if (gActiveGizmoMode == 3) { // Uniform Scale
                        float deltaScale = (mDelta.x - mDelta.y) * 0.015f;
                        sel.scale = fmaxf(0.15f, sel.scale + deltaScale);
                        sel.sizeX = fmaxf(0.10f, sel.sizeX + deltaScale);
                        sel.sizeY = fmaxf(0.10f, sel.sizeY + deltaScale);
                        sel.sizeZ = fmaxf(0.10f, sel.sizeZ + deltaScale);
                    } else if (gActiveGizmoMode == 1) { // Rotate
                        float deltaDeg = (mDelta.x - mDelta.y) * 0.65f;
                        if (gActiveGizmoAxis == 0) sel.rotationX = fmodf(sel.rotationX + deltaDeg + 360.0f, 360.0f);
                        if (gActiveGizmoAxis == 1) sel.rotationY = fmodf(sel.rotationY + deltaDeg + 360.0f, 360.0f);
                        if (gActiveGizmoAxis == 2) sel.rotationZ = fmodf(sel.rotationZ + deltaDeg + 360.0f, 360.0f);
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

        // Auto Recenter (Fixed v8.3.1 - Aligned forward, disabled on mobile/touch)
#if !defined(PLATFORM_ANDROID) && !defined(__ANDROID__)
        if (!IsMouseButtonDown(MOUSE_BUTTON_RIGHT) && !camCtrl.freeLookMode && s_camTouchId == -1) {
            camCtrl.autoRecenterTimer += dt;
            if (camCtrl.autoRecenterTimer > 4.0f && raft.speed > 2.5f) {
                float targetYaw = raft.rotationY;
                float diff = targetYaw - camCtrl.yaw;
                while (diff >  PI) diff -= 2.0f * PI;
                while (diff < -PI) diff += 2.0f * PI;
                camCtrl.yaw += diff * 1.0f * dt;
            }
        }
#endif
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

        // Player Names Floating in White above Pirates' Heads (Plan v8.3.1 User Request)
        if (!editorMode && !raft.isDestroyed) {
            float rimSeatRadius = 0.95f;
            float rimSeatHeight = 0.40f;

            // Pirate 1 (Local Player on Host, Remote Player on Client)
            float p1X = sinf(p1RimAngle) * rimSeatRadius;
            float p1Z = cosf(p1RimAngle) * rimSeatRadius;
            Vector3 localP1 = Vector3{ p1X, rimSeatHeight + 1.25f, p1Z };
            Vector3 rotP1 = Vector3RotateByAxisAngle(localP1, Vector3{ 0, 1, 0 }, raft.rotationY);
            Vector3 worldP1Head = Vector3Add(raft.position, rotP1);

            Vector2 sP1 = GetWorldToScreen(worldP1Head, camera);
            if (sP1.x >= -50.0f && sP1.x <= (float)GetScreenWidth() + 50.0f && sP1.y >= -50.0f && sP1.y <= (float)GetScreenHeight() + 50.0f) {
                const char* nameP1 = (LanGetRole() == LAN_CLIENT) ? LanGetRemotePlayerName() : LanGetLocalPlayerName();
                if (!nameP1 || strlen(nameP1) == 0) nameP1 = "Player 1";
                Vector2 nmSz = MeasureTextEx(fontUIBold, nameP1, 18, 0.4f);
                float pillW = nmSz.x + 20.0f;
                float pillH = 26.0f;
                DrawRectangleRounded(Rectangle{ sP1.x - pillW * 0.5f, sP1.y - pillH * 0.5f, pillW, pillH }, 0.45f, 4, Color{ 15, 23, 42, 215 });
                DrawRectangleRoundedLinesEx(Rectangle{ sP1.x - pillW * 0.5f, sP1.y - pillH * 0.5f, pillW, pillH }, 0.45f, 4, 1.2f, Color{ 255, 255, 255, 180 });
                DrawTextEx(fontUIBold, nameP1, Vector2{ sP1.x - nmSz.x * 0.5f, sP1.y - nmSz.y * 0.5f }, 18, 0.4f, WHITE);
            }

            if (!singlePirateMode) {
                // Pirate 2 (Remote Player on Host, Local Player on Client)
                float p2X = sinf(p2RimAngle) * rimSeatRadius;
                float p2Z = cosf(p2RimAngle) * rimSeatRadius;
                Vector3 localP2 = Vector3{ p2X, rimSeatHeight + 1.25f, p2Z };
                Vector3 rotP2 = Vector3RotateByAxisAngle(localP2, Vector3{ 0, 1, 0 }, raft.rotationY);
                Vector3 worldP2Head = Vector3Add(raft.position, rotP2);

                Vector2 sP2 = GetWorldToScreen(worldP2Head, camera);
                if (sP2.x >= -50.0f && sP2.x <= (float)GetScreenWidth() + 50.0f && sP2.y >= -50.0f && sP2.y <= (float)GetScreenHeight() + 50.0f) {
                    const char* nameP2 = (LanGetRole() == LAN_CLIENT) ? LanGetLocalPlayerName() : LanGetRemotePlayerName();
                    if (!nameP2 || strlen(nameP2) == 0) nameP2 = "Player 2";
                    Vector2 nmSz = MeasureTextEx(fontUIBold, nameP2, 18, 0.4f);
                    float pillW = nmSz.x + 20.0f;
                    float pillH = 26.0f;
                    DrawRectangleRounded(Rectangle{ sP2.x - pillW * 0.5f, sP2.y - pillH * 0.5f, pillW, pillH }, 0.45f, 4, Color{ 15, 23, 42, 215 });
                    DrawRectangleRoundedLinesEx(Rectangle{ sP2.x - pillW * 0.5f, sP2.y - pillH * 0.5f, pillW, pillH }, 0.45f, 4, 1.2f, Color{ 56, 189, 248, 180 });
                    DrawTextEx(fontUIBold, nameP2, Vector2{ sP2.x - nmSz.x * 0.5f, sP2.y - nmSz.y * 0.5f }, 18, 0.4f, WHITE);
                }
            }
        }
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

            rlEnableBackfaceCulling();
            FrustumPlanes camFrustum = ExtractCameraFrustum(camera, (float)GetScreenWidth() / (float)GetScreenHeight());
            for (int s = 0; s < ENV_SECTOR_COUNT; s++) {
                if (envSectors[s].meshCount > 0) {
                    if (IsAABBInFrustum(camFrustum, envSectorBounds[s].min, envSectorBounds[s].max)) {
                        DrawModel(envSectors[s], Vector3{ 0, 0, 0 }, 1.0f, WHITE);
                    }
                }
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
        const float OBSTACLE_CULL_DIST_SQ = 90.0f * 90.0f;
        for (const auto& ent : levelEntities) {
            if (!editorMode) {
                float dx = ent.position.x - raft.position.x;
                float dz = ent.position.z - raft.position.z;
                if (dx * dx + dz * dz > OBSTACLE_CULL_DIST_SQ) {
                    continue; // Distance culling (>90m)
                }
            }
            float sRad = 8.5f * ((ent.scale > 0.01f) ? ent.scale : 1.0f);
            if (!editorMode && !IsSphereInFrustum(camFrustum, ent.position, sRad)) {
                continue; // Camera frustum culling & animation sleep
            }
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

            // Task 3: 12 Custom Collision Shapes rendering in editor mode
            if (ent.type >= ENT_COL_FLAT_WALL && ent.type < ENT_COUNT) {
                if (editorMode) {
                    Color colTint = Color{ 0, 200, 255, 140 }; // Solid Wall
                    if (ent.colBehavior == 1 || ent.type == ENT_COL_FALL_ZONE) colTint = Color{ 255, 160, 0, 140 }; // Fall Zone
                    else if (ent.colBehavior == 2 || ent.type == ENT_COL_KILL_ZONE) colTint = Color{ 240, 40, 40, 160 }; // Kill Zone
                    else if (ent.colBehavior == 3) colTint = Color{ 220, 50, 220, 150 }; // Bounce Pad
                    else if (ent.colBehavior == 4 || ent.type == ENT_COL_FLAT_FLOOR) colTint = Color{ 50, 230, 130, 150 }; // Water/Floor Support

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
                    } else if (ent.type == ENT_COL_FLAT_FLOOR) {
                        DrawCubeWires(Vector3{ 0, 0, 0 }, hx * 2.0f, 0.15f, hz * 2.0f, colTint);
                        DrawCube(Vector3{ 0, 0, 0 }, hx * 2.0f, 0.05f, hz * 2.0f, ColorAlpha(colTint, 0.25f));
                    } else if (ent.type == ENT_COL_FLAT_WALL) {
                        DrawCubeWires(Vector3{ 0, 0, 0 }, hx * 2.0f, hy * 2.0f, hz * 2.0f, colTint);
                        DrawCube(Vector3{ 0, 0, 0 }, 0.05f, hy * 2.0f, hz * 2.0f, ColorAlpha(colTint, 0.30f));
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
                    if (isSel) {
                        // Plan v7.3: Exact Triangle Mesh Collision Wireframe (No oversized boxes or circles!)
                        if (levelEntities[i].model != nullptr) {
                            Vector3 animPos = levelEntities[i].position;
                            float animYaw = levelEntities[i].rotationY, animPitch = levelEntities[i].rotationX, animRoll = levelEntities[i].rotationZ;
                            GetEntityAnimatedTransform(levelEntities[i], animPos, animYaw, animPitch, animRoll);

                            rlPushMatrix();
                            rlTranslatef(animPos.x, animPos.y, animPos.z);
                            rlRotatef(animYaw, 0.0f, 1.0f, 0.0f);
                            if (fabsf(animPitch) > 0.01f) rlRotatef(animPitch, 1.0f, 0.0f, 0.0f);
                            if (fabsf(animRoll)  > 0.01f) rlRotatef(animRoll,  0.0f, 0.0f, 1.0f);
                            float s = (levelEntities[i].scale > 0.01f) ? levelEntities[i].scale : 1.0f;
                            rlScalef(s * (float)levelEntities[i].flipX, s * (float)levelEntities[i].flipY, s * (float)levelEntities[i].flipZ);
                            Color wireCol = (levelEntities[i].isLethal != 0) ? Color{ 255, 55, 55, 240 } : Color{ 40, 235, 110, 240 };
                            DrawModelWires(*levelEntities[i].model, Vector3{ 0, 0, 0 }, 1.001f, wireCol);
                            rlPopMatrix();
                        }
                    } else {
                        float r = (levelEntities[i].radius > 0.5f) ? levelEntities[i].radius : 1.5f;
                        DrawCircle3D(Vector3{ oPos.x, 0.05f, oPos.z }, r * 0.75f, Vector3{ 1, 0, 0 }, 90.0f, Color{ 80, 180, 255, 70 });
                    }

                    if (isSel) {

                        // Plan v7.0 Task 3: Universal Move + Scale + Rotate + Flip 3D Gizmo
                        // 1. Move Arrows (0 -> 2.8m)
                        DrawLine3D(oPos, Vector3Add(oPos, Vector3{ 2.8f, 0, 0 }), RED);
                        DrawLine3D(oPos, Vector3Add(oPos, Vector3{ 0, 2.8f, 0 }), GREEN);
                        DrawLine3D(oPos, Vector3Add(oPos, Vector3{ 0, 0, 2.8f }), BLUE);
                        DrawSphere(Vector3Add(oPos, Vector3{ 2.8f, 0, 0 }), 0.18f, RED);
                        DrawSphere(Vector3Add(oPos, Vector3{ 0, 2.8f, 0 }), 0.18f, GREEN);
                        DrawSphere(Vector3Add(oPos, Vector3{ 0, 0, 2.8f }), 0.18f, BLUE);

                        // 2. Scale Cubes (at 3.8m on axes + white cube at center)
                        DrawLine3D(Vector3Add(oPos, Vector3{ 2.8f, 0, 0 }), Vector3Add(oPos, Vector3{ 3.8f, 0, 0 }), Color{ 255, 120, 120, 255 });
                        DrawLine3D(Vector3Add(oPos, Vector3{ 0, 2.8f, 0 }), Vector3Add(oPos, Vector3{ 0, 3.8f, 0 }), Color{ 120, 255, 120, 255 });
                        DrawLine3D(Vector3Add(oPos, Vector3{ 0, 0, 2.8f }), Vector3Add(oPos, Vector3{ 0, 0, 3.8f }), Color{ 120, 150, 255, 255 });
                        DrawCube(Vector3Add(oPos, Vector3{ 3.8f, 0, 0 }), 0.32f, 0.32f, 0.32f, RED);
                        DrawCube(Vector3Add(oPos, Vector3{ 0, 3.8f, 0 }), 0.32f, 0.32f, 0.32f, GREEN);
                        DrawCube(Vector3Add(oPos, Vector3{ 0, 0, 3.8f }), 0.32f, 0.32f, 0.32f, BLUE);
                        DrawCube(oPos, 0.28f, 0.28f, 0.28f, WHITE);

                        // 3. 3D Rotation Rings (radius 2.2m)
                        DrawCircle3D(oPos, 2.2f, Vector3{ 1, 0, 0 }, 90.0f, Color{ 0, 255, 120, 170 }); // Yaw ring
                        DrawCircle3D(oPos, 2.2f, Vector3{ 0, 1, 0 }, 90.0f, Color{ 255, 60, 60, 170 }); // Pitch ring
                        DrawCircle3D(oPos, 2.2f, Vector3{ 0, 0, 1 }, 90.0f, Color{ 60, 120, 255, 170 }); // Roll ring
                        DrawSphere(Vector3Add(oPos, Vector3{ 1.55f, 0.0f, 1.55f }), 0.16f, Color{ 0, 255, 120, 255 });
                        DrawSphere(Vector3Add(oPos, Vector3{ 0.0f, 1.55f, 1.55f }), 0.16f, Color{ 255, 60, 60, 255 });
                        DrawSphere(Vector3Add(oPos, Vector3{ 1.55f, 1.55f, 0.0f }), 0.16f, Color{ 60, 120, 255, 255 });

                        // 4. Negative Flip handles (-X, -Y, -Z)
                        DrawCube(Vector3Add(oPos, Vector3{ -1.8f, 0, 0 }), 0.32f, 0.32f, 0.32f, Color{ 220, 50, 50, 220 });
                        DrawCube(Vector3Add(oPos, Vector3{ 0, -1.8f, 0 }), 0.32f, 0.32f, 0.32f, Color{ 50, 200, 50, 220 });
                        DrawCube(Vector3Add(oPos, Vector3{ 0, 0, -1.8f }), 0.32f, 0.32f, 0.32f, Color{ 50, 100, 220, 220 });
                    }
                }
            }

        EndMode3D();

        // Player Names Floating in White above Pirates' Heads (Plan v8.3.1 User Request)
        if (!editorMode && !raft.isDestroyed) {
            float rimSeatRadius = 0.95f;
            float rimSeatHeight = 0.40f;

            // Pirate 1 (Local Player on Host, Remote Player on Client)
            float p1X = sinf(p1RimAngle) * rimSeatRadius;
            float p1Z = cosf(p1RimAngle) * rimSeatRadius;
            Vector3 localP1 = Vector3{ p1X, rimSeatHeight + 1.25f, p1Z };
            Vector3 rotP1 = Vector3RotateByAxisAngle(localP1, Vector3{ 0, 1, 0 }, raft.rotationY);
            Vector3 worldP1Head = Vector3Add(raft.position, rotP1);

            Vector2 sP1 = GetWorldToScreen(worldP1Head, camera);
            if (sP1.x >= -50.0f && sP1.x <= (float)GetScreenWidth() + 50.0f && sP1.y >= -50.0f && sP1.y <= (float)GetScreenHeight() + 50.0f) {
                const char* nameP1 = (LanGetRole() == LAN_CLIENT) ? LanGetRemotePlayerName() : LanGetLocalPlayerName();
                if (!nameP1 || strlen(nameP1) == 0) nameP1 = "Player 1";
                Vector2 nmSz = MeasureTextEx(fontUIBold, nameP1, 18, 0.4f);
                float pillW = nmSz.x + 20.0f;
                float pillH = 26.0f;
                DrawRectangleRounded(Rectangle{ sP1.x - pillW * 0.5f, sP1.y - pillH * 0.5f, pillW, pillH }, 0.45f, 4, Color{ 15, 23, 42, 215 });
                DrawRectangleRoundedLinesEx(Rectangle{ sP1.x - pillW * 0.5f, sP1.y - pillH * 0.5f, pillW, pillH }, 0.45f, 4, 1.2f, Color{ 255, 255, 255, 180 });
                DrawTextEx(fontUIBold, nameP1, Vector2{ sP1.x - nmSz.x * 0.5f, sP1.y - nmSz.y * 0.5f }, 18, 0.4f, WHITE);
            }

            if (!singlePirateMode) {
                // Pirate 2 (Remote Player on Host, Local Player on Client)
                float p2X = sinf(p2RimAngle) * rimSeatRadius;
                float p2Z = cosf(p2RimAngle) * rimSeatRadius;
                Vector3 localP2 = Vector3{ p2X, rimSeatHeight + 1.25f, p2Z };
                Vector3 rotP2 = Vector3RotateByAxisAngle(localP2, Vector3{ 0, 1, 0 }, raft.rotationY);
                Vector3 worldP2Head = Vector3Add(raft.position, rotP2);

                Vector2 sP2 = GetWorldToScreen(worldP2Head, camera);
                if (sP2.x >= -50.0f && sP2.x <= (float)GetScreenWidth() + 50.0f && sP2.y >= -50.0f && sP2.y <= (float)GetScreenHeight() + 50.0f) {
                    const char* nameP2 = (LanGetRole() == LAN_CLIENT) ? LanGetLocalPlayerName() : LanGetRemotePlayerName();
                    if (!nameP2 || strlen(nameP2) == 0) nameP2 = "Player 2";
                    Vector2 nmSz = MeasureTextEx(fontUIBold, nameP2, 18, 0.4f);
                    float pillW = nmSz.x + 20.0f;
                    float pillH = 26.0f;
                    DrawRectangleRounded(Rectangle{ sP2.x - pillW * 0.5f, sP2.y - pillH * 0.5f, pillW, pillH }, 0.45f, 4, Color{ 15, 23, 42, 215 });
                    DrawRectangleRoundedLinesEx(Rectangle{ sP2.x - pillW * 0.5f, sP2.y - pillH * 0.5f, pillW, pillH }, 0.45f, 4, 1.2f, Color{ 56, 189, 248, 180 });
                    DrawTextEx(fontUIBold, nameP2, Vector2{ sP2.x - nmSz.x * 0.5f, sP2.y - nmSz.y * 0.5f }, 18, 0.4f, WHITE);
                }
            }
        }

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
            DrawTextEx(fontUIBold, LocText(STR_SUICIDE_HUD), Vector2{ 155.0f, (float)GetScreenHeight() - 41.0f }, 16, 0.3f, Color{ 248, 113, 113, 255 });

            if ((hoverSuicide && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) || IsKeyPressed(KEY_R)) {
                if (!editorMode && !raft.isDestroyed) {
                    TriggerBoatDeath(DEATH_SUICIDE);
                }
            }
        }

                // 5. [F1] LEVEL EDITOR Toggle Handling (HUD button removed in play mode per user request!)
        {
            Rectangle btnEditorRec = { (float)GetScreenWidth() - 175, (float)GetScreenHeight() - 44, 160, 32 };
            bool hoverEditor = CheckCollisionPointRec(mousePos, btnEditorRec);
            if ((hoverEditor && editorMode && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) || IsKeyPressed(KEY_F1)) {
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

            // Only render button while INSIDE editorMode to allow returning to Play Mode!
            // When !editorMode, button is hidden completely from in-game HUD!
            if (editorMode) {
                Color btnBg = Color{ 71, 140, 191, 245 };
                Color btnBorder = Color{ 110, 180, 230, 255 };
                DrawRectangleRounded(btnEditorRec, 0.25f, 4, btnBg);
                DrawRectangleRoundedLinesEx(btnEditorRec, 0.25f, 4, 1.0f, btnBorder);
                const char* toggleLbl = "Play Mode (F1)";
                Vector2 tSz = MeasureTextEx(fontUIBold, toggleLbl, 14, 0.4f);
                DrawTextEx(fontUIBold, toggleLbl, Vector2{ btnEditorRec.x + (btnEditorRec.width - tSz.x) * 0.5f, btnEditorRec.y + 8 }, 14, 0.4f, WHITE);
            }

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

                // 12 Visual Isometric Collision Shape Cards (Plan v7.0 Task 3)
                const char* shapeLabels[12] = {
                    "Flat Wall", "Flat Floor",
                    "Square", "Rect Wall",
                    "Circle", "3D Cube",
                    "Cylinder", "Capsule",
                    "Sphere", "Wedge Ramp",
                    "Fall Zone", "Kill Box"
                };
                EntityType shapeTypes[12] = {
                    ENT_COL_FLAT_WALL, ENT_COL_FLAT_FLOOR,
                    ENT_COL_SQUARE, ENT_COL_RECT,
                    ENT_COL_CIRCLE, ENT_COL_CUBE,
                    ENT_COL_CYLINDER, ENT_COL_CAPSULE,
                    ENT_COL_SPHERE, ENT_COL_WEDGE,
                    ENT_COL_FALL_ZONE, ENT_COL_KILL_ZONE
                };
                Color shapeColors[12] = {
                    Color{ 0, 220, 255, 255 }, Color{ 50, 230, 140, 255 },
                    Color{ 255, 215, 0, 255 }, Color{ 100, 180, 255, 255 },
                    Color{ 255, 140, 50, 255 }, Color{ 120, 150, 255, 255 },
                    Color{ 180, 120, 255, 255 }, Color{ 255, 120, 200, 255 },
                    Color{ 100, 230, 255, 255 }, Color{ 230, 200, 80, 255 },
                    Color{ 255, 160, 40, 255 }, Color{ 240, 50, 50, 255 }
                };

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

                for (int s = 0; s < 12; s += 2) {
                    Rectangle b1 = { 10.0f, ly, 120.0f, 48.0f };
                    Rectangle b2 = { 136.0f, ly, 120.0f, 48.0f };

                    auto DrawShapeCard = [&](Rectangle cRec, int idx) {
                        bool hov = CheckCollisionPointRec(mousePos, cRec);
                        Color cFill = hov ? Color{ 36, 45, 62, 255 } : Color{ 24, 30, 42, 255 };
                        Color cBrd  = hov ? shapeColors[idx] : Color{ 48, 58, 80, 255 };
                        DrawRectangleRounded(cRec, 0.15f, 3, cFill);
                        DrawRectangleRoundedLinesEx(cRec, 0.15f, 3, 1.2f, cBrd);
                        DrawColliderShapeIcon2D(idx, cRec, shapeColors[idx]);
                        DrawTextEx(fontUIBold, shapeLabels[idx], Vector2{ cRec.x + 36.0f, cRec.y + 16.0f }, 11, 0.2f, WHITE);
                        if (hov && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                            int beh = (shapeTypes[idx] == ENT_COL_FLAT_FLOOR) ? 4 : ((shapeTypes[idx] == ENT_COL_FALL_ZONE) ? 1 : ((shapeTypes[idx] == ENT_COL_KILL_ZONE) ? 2 : 0));
                            SpawnShape(shapeTypes[idx], beh);
                        }
                    };

                    DrawShapeCard(b1, s);
                    DrawShapeCard(b2, s + 1);
                    ly += 52.0f;
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

                    // Obstacle Specific Controls (Plan v7.0 Task 2)
                    DrawLine((int)rightRec.x + 10, (int)ry, (int)screenW - 10, (int)ry, Color{ 52, 60, 78, 255 }); ry += 8.0f;
                    DrawTextEx(fontUIBold, "Obstacle Specific Controls:", Vector2{ rightRec.x + 14, ry }, 12, 0.4f, Color{ 255, 215, 0, 255 }); ry += 16.0f;

                    // Arc Angle
                    DrawTextEx(fontUI, TextFormat("Arc Angle: %.0f deg", o.arcAngleDeg), Vector2{ rightRec.x + 14, ry }, 11, 0.3f, Color{ 220, 225, 235, 255 }); ry += 14.0f;
                    Rectangle bA1 = { rightRec.x + 12, ry, 68, 20 };
                    Rectangle bA2 = { rightRec.x + 84, ry, 68, 20 };
                    Rectangle bA3 = { rightRec.x + 158, ry, 68, 20 };
                    Rectangle bA4 = { rightRec.x + 230, ry, 68, 20 };
                    if (DrawGButton(bA1, "60 deg", Color{ 36, 43, 58, 255 }, WHITE, o.arcAngleDeg == 60.0f)) o.arcAngleDeg = 60.0f;
                    if (DrawGButton(bA2, "90 deg", Color{ 36, 43, 58, 255 }, WHITE, o.arcAngleDeg == 90.0f)) o.arcAngleDeg = 90.0f;
                    if (DrawGButton(bA3, "180 deg", Color{ 36, 43, 58, 255 }, WHITE, o.arcAngleDeg == 180.0f)) o.arcAngleDeg = 180.0f;
                    if (DrawGButton(bA4, "360 deg", Color{ 36, 43, 58, 255 }, WHITE, o.arcAngleDeg >= 359.0f)) o.arcAngleDeg = 360.0f;
                    ry += 24.0f;

                    // Motion Mode
                    DrawTextEx(fontUI, "Motion Mode & Direction:", Vector2{ rightRec.x + 14, ry }, 11, 0.3f, Color{ 220, 225, 235, 255 }); ry += 14.0f;
                    Rectangle bM1 = { rightRec.x + 12, ry, 94, 20 };
                    Rectangle bM2 = { rightRec.x + 110, ry, 94, 20 };
                    Rectangle bM3 = { rightRec.x + 208, ry, 94, 20 };
                    if (DrawGButton(bM1, "Ping-Pong", Color{ 36, 43, 58, 255 }, WHITE, o.animMode == 0)) o.animMode = 0;
                    if (DrawGButton(bM2, "360 CW", Color{ 36, 43, 58, 255 }, WHITE, o.animMode == 1)) { o.animMode = 1; if (o.arcAngleDeg < 359.0f) o.arcAngleDeg = 360.0f; }
                    if (DrawGButton(bM3, "360 CCW", Color{ 36, 43, 58, 255 }, WHITE, o.animMode == -1)) { o.animMode = -1; if (o.arcAngleDeg < 359.0f) o.arcAngleDeg = 360.0f; }
                    ry += 24.0f;

                    // Speed & Phase
                    DrawTextEx(fontUI, TextFormat("Speed: %.1fx | Phase: %.0f deg", o.animSpeed, o.phaseOffsetDeg), Vector2{ rightRec.x + 14, ry }, 11, 0.3f, Color{ 220, 225, 235, 255 }); ry += 14.0f;
                    Rectangle bSpM = { rightRec.x + 12, ry, 68, 20 };
                    Rectangle bSpP = { rightRec.x + 84, ry, 68, 20 };
                    Rectangle bPhM = { rightRec.x + 158, ry, 68, 20 };
                    Rectangle bPhP = { rightRec.x + 230, ry, 68, 20 };
                    if (DrawGButton(bSpM, "Spd -0.2", Color{ 36, 43, 58, 255 }, WHITE)) o.animSpeed = Clamp(o.animSpeed - 0.2f, 0.0f, 5.0f);
                    if (DrawGButton(bSpP, "Spd +0.2", Color{ 36, 43, 58, 255 }, WHITE)) o.animSpeed = Clamp(o.animSpeed + 0.2f, 0.0f, 5.0f);
                    if (DrawGButton(bPhM, "Phase -45", Color{ 36, 43, 58, 255 }, WHITE)) o.phaseOffsetDeg = fmodf(o.phaseOffsetDeg - 45.0f + 360.0f, 360.0f);
                    if (DrawGButton(bPhP, "Phase +45", Color{ 36, 43, 58, 255 }, WHITE)) o.phaseOffsetDeg = fmodf(o.phaseOffsetDeg + 45.0f, 360.0f);
                    ry += 24.0f;

                    // Slide Dist & Bounce Mult
                    DrawTextEx(fontUI, TextFormat("Slide: %.2fm | Bounce: %.2fx", o.slideDist, o.bounceMult), Vector2{ rightRec.x + 14, ry }, 11, 0.3f, Color{ 220, 225, 235, 255 }); ry += 14.0f;
                    Rectangle bSlM = { rightRec.x + 12, ry, 68, 20 };
                    Rectangle bSlP = { rightRec.x + 84, ry, 68, 20 };
                    Rectangle bBncM = { rightRec.x + 158, ry, 68, 20 };
                    Rectangle bBncP = { rightRec.x + 230, ry, 68, 20 };
                    if (DrawGButton(bSlM, "Sl -0.25", Color{ 36, 43, 58, 255 }, WHITE)) o.slideDist = Clamp(o.slideDist - 0.25f, 0.2f, 8.0f);
                    if (DrawGButton(bSlP, "Sl +0.25", Color{ 36, 43, 58, 255 }, WHITE)) o.slideDist = Clamp(o.slideDist + 0.25f, 0.2f, 8.0f);
                    if (DrawGButton(bBncM, "Bnc -0.25", Color{ 36, 43, 58, 255 }, WHITE)) o.bounceMult = Clamp(o.bounceMult - 0.25f, 0.5f, 5.0f);
                    if (DrawGButton(bBncP, "Bnc +0.25", Color{ 36, 43, 58, 255 }, WHITE)) o.bounceMult = Clamp(o.bounceMult + 0.25f, 0.5f, 5.0f);
                    ry += 24.0f;

                    // Collision Lethality
                    Rectangle bLethal = { rightRec.x + 12, ry, rightDockW - 24, 22 };
                    const char* letText = o.isLethal ? "Effect: LETHAL (Explode Boat)" : "Effect: NON-LETHAL (Push / Bounce)";
                    Color letCol = o.isLethal ? Color{ 150, 45, 45, 255 } : Color{ 45, 120, 75, 255 };
                    if (DrawGButton(bLethal, letText, letCol, WHITE, true)) {
                        o.isLethal = !o.isLethal;
                    }
                    ry += 26.0f;
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

        // =====================================================================
        // Smart Mobile Touch HUD Controls Rendering (Plan v8.1.0 Task 5 - Zero Text Cartoon)
        // =====================================================================
        if (isMobileTouchHUD && !editorMode) {
            float alphaMul = Clamp(gSettings.touchBtnOpacity, 0.30f, 1.0f);
            unsigned char aOuter = (unsigned char)(240 * alphaMul);
            unsigned char aInner = (unsigned char)(220 * alphaMul);

            // 1. Cartoon Steer Left & Steer Right Buttons (Original Game Arrow Icon, ZERO text)
            auto DrawCartoonSteerBtn = [&](Vector2 center, float rad, bool isRightArrow, bool isPressed) {
                Color rimCol  = isPressed ? Color{ 255, 230, 60, aOuter } : Color{ 245, 158, 18, aOuter };
                Color bodyCol = isPressed ? Color{ 255, 38, 118, aInner } : Color{ 30, 45, 72, aInner };
                DrawCircleV(Vector2{ center.x, center.y + 4.0f }, rad + 4.0f, Color{ 10, 15, 25, (unsigned char)(140 * alphaMul) });
                DrawCircleV(center, rad + 4.0f, rimCol);
                DrawCircleV(center, rad - 3.0f, bodyCol);
                // Cartoon gloss highlight
                DrawCircleV(Vector2{ center.x - rad * 0.25f, center.y - rad * 0.28f }, rad * 0.35f, Color{ 255, 255, 255, (unsigned char)(45 * alphaMul) });

                if (texIconArrow.id != 0) {
                    float aw = rad * 0.95f, ah = rad * 1.15f;
                    // Icon_ArrowM.png points RIGHT natively -> Right is +width, Left is -width!
                    float srcW = isRightArrow ? (float)texIconArrow.width : -(float)texIconArrow.width;
                    Rectangle src = { 0.0f, 0.0f, srcW, (float)texIconArrow.height };
                    Rectangle dst = { center.x, center.y, aw, ah };
                    DrawTexturePro(texIconArrow, src, dst, Vector2{ aw * 0.5f, ah * 0.5f }, 0.0f, WHITE);
                } else {
                    DrawTextEx(fontUIBold, isRightArrow ? ">" : "<", Vector2{ center.x - 8.0f, center.y - 12.0f }, 24, 0.3f, WHITE);
                }
            };

            DrawCartoonSteerBtn(posLeftBtn,  rSteer, false, touchSteerLeft);
            DrawCartoonSteerBtn(posRightBtn, rSteer, true,  touchSteerRight);

            // 2. Cartoon Fart Gas Button (Rubber Float Ring Style + 3 Animated Green Fart Clouds — ZERO TEXT!)
            {
                float pulse = touchGas ? (1.08f + 0.06f * sinf(runTime * 18.0f)) : 1.0f;
                float gRad  = rGas * pulse;
                Color rimCol  = touchGas ? Color{ 255, 90, 30, aOuter } : Color{ 250, 185, 25, aOuter };
                Color bodyCol = touchGas ? Color{ 220, 48, 48, aInner } : Color{ 42, 168, 52, aInner };

                DrawCircleV(Vector2{ posGasBtn.x, posGasBtn.y + 5.0f }, gRad + 5.0f, Color{ 10, 15, 25, (unsigned char)(150 * alphaMul) });
                DrawCircleV(posGasBtn, gRad + 5.0f, rimCol);
                DrawCircleV(posGasBtn, gRad - 3.0f, bodyCol);

                // Animated swirling green fart clouds inside button
                Texture2D fartIconTex = texFart[0];
                if (fartIconTex.id != 0) {
                    float fSz1 = gRad * 1.35f;
                    float fSz2 = gRad * 1.05f;
                    Rectangle srcF = { 0, 0, (float)fartIconTex.width, (float)fartIconTex.height };
                    DrawTexturePro(fartIconTex, srcF, Rectangle{ posGasBtn.x + gRad * 0.12f, posGasBtn.y + gRad * 0.08f, fSz1, fSz1 * 0.78f },
                                   Vector2{ fSz1 * 0.5f, fSz1 * 0.39f }, sinf(runTime * 3.0f) * 12.0f, Color{ 135, 255, 55, aOuter });
                    DrawTexturePro(fartIconTex, srcF, Rectangle{ posGasBtn.x - gRad * 0.18f, posGasBtn.y - gRad * 0.10f, fSz2, fSz2 * 0.78f },
                                   Vector2{ fSz2 * 0.5f, fSz2 * 0.39f }, -15.0f + cosf(runTime * 4.0f) * 10.0f, Color{ 195, 255, 95, aOuter });
                } else {
                    DrawCircleV(Vector2{ posGasBtn.x + gRad * 0.18f, posGasBtn.y + gRad * 0.10f }, gRad * 0.42f, Color{ 130, 245, 50, aOuter });
                    DrawCircleV(Vector2{ posGasBtn.x - gRad * 0.15f, posGasBtn.y + gRad * 0.18f }, gRad * 0.35f, Color{ 155, 255, 70, aOuter });
                    DrawCircleV(Vector2{ posGasBtn.x + gRad * 0.05f, posGasBtn.y - gRad * 0.15f }, gRad * 0.38f, Color{ 175, 255, 95, aOuter });
                }

                // Cartoon speed thrust streaks
                DrawLineEx(Vector2{ posGasBtn.x - gRad * 0.55f, posGasBtn.y + gRad * 0.25f },
                           Vector2{ posGasBtn.x - gRad * 0.15f, posGasBtn.y + gRad * 0.05f }, 3.5f, Color{ 255, 255, 255, (unsigned char)(190 * alphaMul) });
                DrawLineEx(Vector2{ posGasBtn.x - gRad * 0.48f, posGasBtn.y + gRad * 0.42f },
                           Vector2{ posGasBtn.x - gRad * 0.10f, posGasBtn.y + gRad * 0.22f }, 3.0f, Color{ 255, 255, 255, (unsigned char)(170 * alphaMul) });
            }

            // 3. Settings Gear Button
            DrawCircleV(posGearBtn, rGear, Color{ 51, 65, 85, 220 });
            DrawCircleLines((int)posGearBtn.x, (int)posGearBtn.y, rGear, Color{ 203, 213, 225, 200 });
            DrawTextEx(fontUIBold, "*", Vector2{ posGearBtn.x - 6.0f, posGearBtn.y - 10.0f }, 20, 0.3f, WHITE);
        }

        // Live FPS Counter Badge — Relocated to (x = 258, y = 14) to NEVER overlap race timer
        if (gSettings.showFPS) {
            float fpsX = 258.0f, fpsY = 14.0f;
            DrawRectangleRounded(Rectangle{ fpsX, fpsY, 110.0f, 28.0f }, 0.4f, 4, Color{ 15, 23, 42, 230 });
            DrawRectangleRoundedLinesEx(Rectangle{ fpsX, fpsY, 110.0f, 28.0f }, 0.4f, 4, 1.5f, Color{ 34, 197, 94, 255 });
            DrawTextEx(fontUIBold, TextFormat("FPS: %d", GetFPS()), Vector2{ fpsX + 16.0f, fpsY + 6.0f }, 15, 0.4f, Color{ 34, 197, 94, 255 });
        }
        } else if (currentScreen == SCREEN_MAIN_MENU) {
            ClearBackground(Color{ 18, 22, 34, 255 });

            float scrW = (float)GetScreenWidth();
            float scrH = (float)GetScreenHeight();

            // Mute / Stop background beach and snore sounds in main menu as commanded
            if (IsSoundPlaying(sndMenuBeachOst)) StopSound(sndMenuBeachOst);
            if (IsSoundPlaying(sndSnore)) StopSound(sndSnore);
            if (IsSoundPlaying(sndEatMunch)) StopSound(sndEatMunch);

            // Animated Video Background (Plan v8.3.0 Task 1) - Silenced, 12 FPS smooth loop
            if (!menuVideoFrames.empty()) {
                static float menuVideoTimer = 0.0f;
                menuVideoTimer += dt;
                int fIdx = (int)(menuVideoTimer * 12.0f) % (int)menuVideoFrames.size();
                Texture2D curFrame = menuVideoFrames[fIdx];
                if (curFrame.id != 0) {
                    DrawTexturePro(curFrame, Rectangle{ 0, 0, (float)curFrame.width, (float)curFrame.height },
                                   Rectangle{ 0, 0, scrW, scrH }, Vector2{0, 0}, 0.0f, WHITE);
                }
            } else if (texMenuOasisFull.id != 0) {
                DrawTexturePro(texMenuOasisFull, Rectangle{ 0, 0, (float)texMenuOasisFull.width, (float)texMenuOasisFull.height },
                               Rectangle{ 0, 0, scrW, scrH }, Vector2{0, 0}, 0.0f, WHITE);
            } else if (texMenuOasisClean.id != 0) {
                DrawTexturePro(texMenuOasisClean, Rectangle{ 0, 0, (float)texMenuOasisClean.width, (float)texMenuOasisClean.height },
                               Rectangle{ 0, 0, scrW, scrH }, Vector2{0, 0}, 0.0f, WHITE);
            }

            // Top-Left Cartoon Logo
            if (texMainLogo.id != 0) {
                float logoW = 345.0f, logoH = 158.0f;
                DrawTexturePro(texMainLogo, Rectangle{ 0, 0, (float)texMainLogo.width, (float)texMainLogo.height },
                               Rectangle{ 38.0f, 32.0f, logoW, logoH }, Vector2{0, 0}, 0.0f, WHITE);
            }

            // 5 Main Menu Items: 0: PLAY SOLO, 1: CREATE LOBBY, 2: FIND LOBBY, 3: SETTINGS, 4: EXIT GAME
            static int menuSel = 0;
            const LocalizedString menuItems[5] = {
                STR_PLAY,
                STR_CREATE_LOBBY,
                STR_FIND_LOBBY,
                STR_SETTINGS,
                STR_EXIT_GAME
            };

            if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W))   menuSel = (menuSel + 4) % 5;
            if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) menuSel = (menuSel + 1) % 5;

            float menuStartX = 0.0f;
            float menuStartY = 210.0f;
            float menuGapY   = 60.0f;

            for (int m = 0; m < 5; m++) {
                float my = menuStartY + m * menuGapY;
                float mw = (menuSel == m) ? 390.0f : 290.0f;
                float mh = 50.0f;

                Rectangle itemRec = { menuStartX, my, mw, mh };
                bool isHov = CheckCollisionPointRec(mousePos, itemRec);
                if (isHov) menuSel = m;

                if (menuSel == m) {
                    DrawSlantedRect(0.0f, my, mw, mh, 16.0f, Color{ 34, 197, 94, 250 });
                    DrawSlantedRectLines(0.0f, my, mw, mh, 16.0f, 2.0f, Color{ 255, 255, 255, 230 });

                    if (texIconArrow.id != 0) {
                        float aw = 20.0f, ah = 28.0f;
                        DrawTexturePro(texIconArrow, Rectangle{ 0, 0, (float)texIconArrow.width, (float)texIconArrow.height },
                                       Rectangle{ 38.0f, my + 11.0f, aw, ah }, Vector2{0, 0}, 0.0f, WHITE);
                    }
                    DrawTextEx(fontUIBold, LocText(menuItems[m]), Vector2{ 68.0f, my + 10.0f }, 24, 0.4f, WHITE);
                } else {
                    DrawTextEx(fontUIBold, LocText(menuItems[m]), Vector2{ 50.0f, my + 12.0f }, 21, 0.4f, Color{ 255, 255, 255, 235 });
                }

                if ((isHov && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) || (menuSel == m && (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)))) {
                    if (m == 0) { // PLAY SOLO (Single Pirate Mode, 360 degree rotation)
                        LanShutdown();
                        singlePirateMode = true;
                        p1RimAngle = 3.14159f;
                        raft.position = Vector3{ 15.55f, 0.58f, 90.80f };
                        raft.velocity = Vector3{ 0.0f, 0.0f, 0.0f };
                        raft.rotationY = 3.14159f;
                        raft.isDestroyed = false;
                        raft.currentCP = 0;
                        gasP1.gasEnergy = GAS_MAX;
                        gasP1.gasDepleted = false;
                        currentScreen = SCREEN_GAMEPLAY;
                    } else if (m == 1) { // CREATE LOBBY
                        LanInitHost(7777, nullptr);
                        gHostReady = false;
                        gClientReady = false;
                        gSoloPracticeMode = false;
                        currentScreen = SCREEN_LAN_LOBBY;
                    } else if (m == 2) { // FIND LOBBY
                        LanStartDiscoveryListener(7778);
                        currentScreen = SCREEN_FIND_LOBBY;
                    } else if (m == 3) { // SETTINGS
                        currentScreen = SCREEN_SETTINGS;
                    } else if (m == 4) { // EXIT GAME
                        CloseWindow();
                        return 0;
                    }
                }
            }

            // Static room code box deleted as requested (v8.3.1)

            // Language Toggle in Top-Right
            Rectangle langBtnRec = { scrW - 190.0f, 20.0f, 165.0f, 42.0f };
            bool hovLang = CheckCollisionPointRec(mousePos, langBtnRec);
            DrawSlantedRect(langBtnRec.x, langBtnRec.y, langBtnRec.width, langBtnRec.height, 10.0f, hovLang ? Color{ 45, 65, 95, 240 } : Color{ 20, 32, 48, 220 });
            DrawSlantedRectLines(langBtnRec.x, langBtnRec.y, langBtnRec.width, langBtnRec.height, 10.0f, 1.2f, Color{ 80, 115, 160, 200 });
            const char* langToggleTxt = (gCurrentLanguage == LANG_AR) ? "English [EN]" : LocText(STR_LANG_VALUE);
            Vector2 ltSz = MeasureTextEx(fontUIBold, langToggleTxt, 16, 0.3f);
            DrawTextEx(fontUIBold, langToggleTxt, Vector2{ langBtnRec.x + (langBtnRec.width - ltSz.x) * 0.5f, langBtnRec.y + 11.0f }, 16, 0.3f, Color{ 255, 225, 75, 255 });
            if (hovLang && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                gCurrentLanguage = (gCurrentLanguage == LANG_AR) ? LANG_EN : LANG_AR;
            }

            // Bottom-Left key hints
            DrawTextEx(fontUIBold, LocText(STR_ESC_BACK), Vector2{ 38.0f, scrH - 42.0f }, 15, 0.3f, Color{ 220, 230, 245, 220 });
            DrawTextEx(fontUIBold, LocText(STR_ENTER_SELECT), Vector2{ 175.0f, scrH - 42.0f }, 15, 0.3f, Color{ 220, 230, 245, 220 });

            // Bottom-Right Version Badge (v8.3.0)
            DrawSlantedRect(scrW - 165.0f, scrH - 52.0f, 145.0f, 38.0f, 10.0f, Color{ 18, 28, 42, 230 });
            DrawSlantedRectLines(scrW - 165.0f, scrH - 52.0f, 145.0f, 38.0f, 10.0f, 1.2f, Color{ 48, 70, 98, 220 });
            Vector2 verSz = MeasureTextEx(fontUIBold, GAME_VERSION_TAG, 18, 0.4f);
            DrawTextEx(fontUIBold, GAME_VERSION_TAG, Vector2{ scrW - 165.0f + (145.0f - verSz.x) * 0.5f, scrH - 44.0f }, 18, 0.4f, Color{ 172, 255, 45, 255 });
        } else if (currentScreen == SCREEN_LAN_LOBBY) {
            ClearBackground(Color{ 18, 22, 34, 255 });
            LanUpdateDiscoveryAndLobby(GetFrameTime());

            float scrW = (float)GetScreenWidth();
            float scrH = (float)GetScreenHeight();

            // Authentic Stage Backdrop (Image 5 & 98a07c Parity):
            // Upper half: Warm terracotta canyon wall (y = 0..320)
            DrawRectangle(0, 0, (int)scrW, 320, Color{ 186, 108, 92, 255 });
            for (int s = 0; s < 7; s++) {
                float strataY = 45.0f + s * 42.0f;
                DrawLineEx(Vector2{ 0.0f, strataY }, Vector2{ scrW, strataY + 14.0f }, 2.5f, Color{ 198, 118, 100, 180 });
                DrawLineEx(Vector2{ 0.0f, strataY + 8.0f }, Vector2{ scrW, strataY + 22.0f }, 1.5f, Color{ 174, 98, 84, 180 });
            }

            // Lower half: Warm sandy / wooden flat floor (y = 320..H)
            DrawRectangle(0, 320, (int)scrW, (int)scrH - 320, Color{ 198, 150, 116, 255 });
            DrawLineEx(Vector2{ 0.0f, 320.0f }, Vector2{ scrW, 320.0f }, 3.0f, Color{ 145, 85, 72, 255 });
            for (int j = 0; j < 6; j++) {
                float floorY = 360.0f + j * 58.0f;
                DrawLineEx(Vector2{ 0.0f, floorY }, Vector2{ scrW, floorY }, 1.5f, Color{ 182, 134, 100, 190 });
            }

            // Side desert pebbles & shrub decoration
            DrawCircle(95, 332, 16.0f, Color{ 140, 92, 80, 220 });
            DrawCircle(135, 338, 11.0f, Color{ 160, 110, 95, 220 });
            DrawCircle(175, 334, 14.0f, Color{ 135, 88, 76, 220 });
            DrawCircle((int)scrW - 110, 332, 18.0f, Color{ 140, 92, 80, 220 });
            DrawCircle((int)scrW - 150, 338, 12.0f, Color{ 160, 110, 95, 220 });

            // Top-Left Back / Leave Button (v8.3.1 Mobile & PC Support)
            Rectangle leaveRec = { 25.0f, 16.0f, 150.0f, 48.0f };
            bool hovLeave = CheckCollisionPointRec(mousePos, leaveRec);
            DrawSlantedRect(leaveRec.x, leaveRec.y, leaveRec.width, leaveRec.height, 10.0f, hovLeave ? Color{ 238, 52, 132, 255 } : Color{ 180, 40, 95, 230 });
            const char* leaveTxt = (gCurrentLanguage == LANG_AR) ? "< خروج" : "< LEAVE";
            Vector2 lvSz = MeasureTextEx(fontUIBold, leaveTxt, 17, 0.3f);
            DrawTextEx(fontUIBold, leaveTxt, Vector2{ leaveRec.x + (leaveRec.width - lvSz.x) * 0.5f, leaveRec.y + 13.0f }, 17, 0.3f, WHITE);
            if ((hovLeave && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) || IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_BACK)) {
                LanShutdown();
                currentScreen = SCREEN_MAIN_MENU;
            }

            // 3D Standing Pirates on the stage floor
            Camera3D lobbyCam = { 0 };
            lobbyCam.position = Vector3{ 0.0f, 1.05f, 4.20f };
            lobbyCam.target   = Vector3{ 0.0f, 0.55f, 0.0f };
            lobbyCam.up       = Vector3{ 0.0f, 1.0f, 0.0f };
            lobbyCam.fovy     = 36.0f;
            lobbyCam.projection = CAMERA_PERSPECTIVE;

            BeginMode3D(lobbyCam);
                float pBob = sinf(runTime * 2.5f) * 0.02f;
                uint8_t p1Col = (LanGetRole() == LAN_CLIENT) ? LanGetHostColor() : (uint8_t)selectedLobbyChar;
                uint8_t p2Col = (LanGetRole() == LAN_CLIENT) ? (uint8_t)selectedLobbyChar : LanGetClientColor();
                if (gConnectedPlayers <= 1) {
                    rlPushMatrix();
                        rlTranslatef(0.0f, 0.08f + pBob, 0.0f);
                        rlScalef(1.28f, 1.28f, 1.28f);
                        Model* pCurChar = (p1Col % 2 == 0) ? &charB : &charA;
                        DrawModel(*pCurChar, Vector3{0,0,0}, 1.0f, WHITE);
                    rlPopMatrix();
                } else {
                    rlPushMatrix();
                        rlTranslatef(-0.95f, 0.08f + pBob, 0.0f);
                        rlScalef(1.28f, 1.28f, 1.28f);
                        Model* pHostModel = (p1Col % 2 == 0) ? &charB : &charA;
                        DrawModel(*pHostModel, Vector3{0,0,0}, 1.0f, WHITE);
                    rlPopMatrix();

                    rlPushMatrix();
                        rlTranslatef(0.95f, 0.08f + pBob, 0.0f);
                        rlScalef(1.28f, 1.28f, 1.28f);
                        Model* pClientModel = (p2Col % 2 == 0) ? &charB : &charA;
                        DrawModel(*pClientModel, Vector3{0,0,0}, 1.0f, WHITE);
                    rlPopMatrix();
                }
            EndMode3D();

            // Name Markers & White Player Names above Pirates (Plan v8.3.1 User Request)
            Vector2 p1Head = GetWorldToScreen(Vector3{ (gConnectedPlayers <= 1) ? 0.0f : -0.95f, 1.55f, 0.0f }, lobbyCam);
            const char* hostPName = (LanGetRole() == LAN_CLIENT) ? LanGetRemotePlayerName() : LanGetLocalPlayerName();
            if (!hostPName || strlen(hostPName) == 0) hostPName = "Captain 1";
            Vector2 hNameSz = MeasureTextEx(fontUIBold, hostPName, 18, 0.4f);
            float hPillW = hNameSz.x + 24.0f;
            DrawRectangleRounded(Rectangle{ p1Head.x - hPillW * 0.5f, p1Head.y - 32.0f, hPillW, 28.0f }, 0.45f, 4, Color{ 15, 23, 42, 220 });
            DrawRectangleRoundedLinesEx(Rectangle{ p1Head.x - hPillW * 0.5f, p1Head.y - 32.0f, hPillW, 28.0f }, 0.45f, 4, 1.2f, Color{ 255, 255, 255, 200 });
            DrawTextEx(fontUIBold, hostPName, Vector2{ p1Head.x - hNameSz.x * 0.5f, p1Head.y - 28.0f }, 18, 0.4f, WHITE);

            // Green Tik above You if ready
            if (gHostReady) {
                if (texIconTikGreen.id != 0) {
                    DrawTexturePro(texIconTikGreen, Rectangle{ 0, 0, (float)texIconTikGreen.width, (float)texIconTikGreen.height },
                                   Rectangle{ p1Head.x - 18.0f, p1Head.y - 70.0f, 36.0f, 36.0f }, Vector2{0,0}, 0.0f, WHITE);
                } else {
                    DrawCircle((int)p1Head.x, (int)p1Head.y - 52, 16.0f, Color{ 34, 197, 94, 255 });
                }
            }

            if (gConnectedPlayers >= 2) {
                Vector2 p2Head = GetWorldToScreen(Vector3{ 0.95f, 1.55f, 0.0f }, lobbyCam);
                const char* clientPName = (LanGetRole() == LAN_CLIENT) ? LanGetLocalPlayerName() : LanGetRemotePlayerName();
                if (!clientPName || strlen(clientPName) == 0) clientPName = "Captain 2";
                Vector2 cNameSz = MeasureTextEx(fontUIBold, clientPName, 18, 0.4f);
                float cPillW = cNameSz.x + 24.0f;
                DrawRectangleRounded(Rectangle{ p2Head.x - cPillW * 0.5f, p2Head.y - 32.0f, cPillW, 28.0f }, 0.45f, 4, Color{ 15, 23, 42, 220 });
                DrawRectangleRoundedLinesEx(Rectangle{ p2Head.x - cPillW * 0.5f, p2Head.y - 32.0f, cPillW, 28.0f }, 0.45f, 4, 1.2f, Color{ 56, 189, 248, 200 });
                DrawTextEx(fontUIBold, clientPName, Vector2{ p2Head.x - cNameSz.x * 0.5f, p2Head.y - 28.0f }, 18, 0.4f, WHITE);

                if (gClientReady) {
                    if (texIconTikGreen.id != 0) {
                        DrawTexturePro(texIconTikGreen, Rectangle{ 0, 0, (float)texIconTikGreen.width, (float)texIconTikGreen.height },
                                       Rectangle{ p2Head.x - 18.0f, p2Head.y - 70.0f, 36.0f, 36.0f }, Vector2{0,0}, 0.0f, WHITE);
                    } else {
                        DrawCircle((int)p2Head.x, (int)p2Head.y - 52, 16.0f, Color{ 34, 197, 94, 255 });
                    }
                }
            }

            // Top Center Banner: LOBBY
            float topBannerW = 320.0f, topBannerH = 48.0f;
            float topBannerX = (scrW - topBannerW) * 0.5f;
            DrawTrapezoid(topBannerX, 0, topBannerW, topBannerH, 18.0f, Color{ 28, 33, 53, 255 });
            DrawTrapezoidLines(topBannerX, 0, topBannerW, topBannerH, 18.0f, 1.5f, Color{ 70, 80, 110, 255 });
            Vector2 lobSz = MeasureTextEx(fontUIBold, LocText(STR_LOBBY_TITLE), 22, 0.5f);
            DrawTextEx(fontUIBold, LocText(STR_LOBBY_TITLE), Vector2{ (scrW - lobSz.x) * 0.5f, 12.0f }, 22, 0.5f, WHITE);

            // Top Right: Dynamic Room Code (v8.3.1 Clean Parity)
            float codeBoxW = 220.0f, codeBoxH = 54.0f;
            float codeBoxX = scrW - codeBoxW - 35.0f;
            float codeBoxY = 16.0f;
            DrawSlantedRect(codeBoxX, codeBoxY, codeBoxW, codeBoxH, 12.0f, Color{ 22, 30, 48, 220 });
            DrawSlantedRectLines(codeBoxX, codeBoxY, codeBoxW, codeBoxH, 12.0f, 1.2f, Color{ 56, 75, 110, 200 });
            DrawTextEx(fontUI, LocText(STR_YOUR_CODE), Vector2{ codeBoxX + 22.0f, codeBoxY + 8.0f }, 11, 0.3f, Color{ 160, 175, 200, 255 });
            DrawTextEx(fontUIBold, LanGetRoomCode(), Vector2{ codeBoxX + 22.0f, codeBoxY + 24.0f }, 20, 0.4f, Color{ 52, 235, 120, 255 });

            // 4 Character Color Cards (y = 358, h = 76)
            float cardW = 76.0f, cardH = 76.0f, cardGap = 16.0f;
            float totalCardsW = 4 * cardW + 3 * cardGap;
            float startCardsX = (scrW - totalCardsW) * 0.5f;
            float cardsY = 358.0f;

            for (int c = 0; c < 4; c++) {
                Rectangle cRec = { startCardsX + c * (cardW + cardGap), cardsY, cardW, cardH };
                bool hovC = CheckCollisionPointRec(mousePos, cRec);
                bool isSel = (selectedLobbyChar == c);

                DrawRectangleRounded(cRec, 0.18f, 4, WHITE);
                if (isSel) {
                    DrawRectangleRoundedLinesEx(cRec, 0.18f, 4, 3.5f, Color{ 238, 52, 132, 255 });
                } else if (hovC) {
                    DrawRectangleRoundedLinesEx(cRec, 0.18f, 4, 2.0f, Color{ 245, 158, 11, 240 });
                }

                if (texLobbyCards[c].id != 0) {
                    DrawTexturePro(texLobbyCards[c], Rectangle{ 0, 0, (float)texLobbyCards[c].width, (float)texLobbyCards[c].height },
                                   Rectangle{ cRec.x + 6, cRec.y + 6, cardW - 12, cardH - 12 }, Vector2{0,0}, 0.0f, WHITE);
                }

                if (hovC && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    selectedLobbyChar = c;
                    if (LanGetRole() == LAN_CLIENT) {
                        LanClientSendLobbyUpdate(gClientReady, (uint8_t)selectedLobbyChar);
                    } else if (LanGetRole() == LAN_HOST) {
                        LanSetHostColor((uint8_t)selectedLobbyChar);
                    }
                }
            }

            // Section: SELECT MODE (y = 452)
            float smBarW = 285.0f, smBarH = 42.0f;
            float smBarX = (scrW - smBarW) * 0.5f;
            DrawSlantedRect(smBarX, 450.0f, smBarW, smBarH, 12.0f, Color{ 126, 114, 228, 255 });
            Vector2 smSz = MeasureTextEx(fontUIBold, LocText(STR_SELECT_MODE), 18, 0.4f);
            DrawTextEx(fontUIBold, LocText(STR_SELECT_MODE), Vector2{ (scrW - smSz.x) * 0.5f, 460.0f }, 18, 0.4f, WHITE);

            // Mode thumbnails box
            float modeBoxW = 540.0f, modeBoxH = 110.0f;
            float modeBoxX = (scrW - modeBoxW) * 0.5f;
            DrawRectangleRounded(Rectangle{ modeBoxX, 498.0f, modeBoxW, modeBoxH }, 0.12f, 4, WHITE);

            // 3 Mode Thumbnails
            float mThumbW = 160.0f, mThumbH = 94.0f;
            Texture2D mThumbs[3] = { texModeFartRun, texModeTeamRace, texModeRaftRumble };
            const LocalizedString mTitles[3] = { STR_MODE_FARTRUN, STR_MODE_TEAMRACE, STR_MODE_RAFTRUMBLE };

            for (int m = 0; m < 3; m++) {
                Rectangle mRec = { modeBoxX + 10.0f + m * (mThumbW + 15.0f), 506.0f, mThumbW, mThumbH };
                bool isM0 = (m == 0);

                if (mThumbs[m].id != 0) {
                    DrawTexturePro(mThumbs[m], Rectangle{ 0, 0, (float)mThumbs[m].width, (float)mThumbs[m].height },
                                   mRec, Vector2{0,0}, 0.0f, isM0 ? WHITE : Color{ 160, 160, 160, 200 });
                } else {
                    DrawRectangleRec(mRec, isM0 ? Color{ 45, 125, 220, 255 } : Color{ 80, 85, 100, 255 });
                }

                if (isM0) {
                    DrawRectangleLinesEx(mRec, 3.0f, Color{ 238, 52, 132, 255 });
                }

                DrawRectangle(mRec.x, mRec.y + mRec.height - 24.0f, mRec.width, 24.0f, Color{ 15, 23, 42, 210 });
                Vector2 mtSz = MeasureTextEx(fontUIBold, LocText(mTitles[m]), 12, 0.2f);
                DrawTextEx(fontUIBold, LocText(mTitles[m]), Vector2{ mRec.x + (mRec.width - mtSz.x) * 0.5f, mRec.y + mRec.height - 18.0f }, 12, 0.2f, WHITE);
            }

            // Bottom-Right Action Button: [SPACE] READY (Image 5 Parity)
            float rdyW = 320.0f, rdyH = 56.0f;
            float rdyX = scrW - rdyW - 35.0f;
            float rdyY = scrH - rdyH - 25.0f;

            Rectangle rdyRec = { rdyX, rdyY, rdyW, rdyH };
            bool hovRdy = CheckCollisionPointRec(mousePos, rdyRec);

            bool localIsReady = (LanGetRole() == LAN_CLIENT) ? gClientReady : gHostReady;
            Color rdyBg = localIsReady ? Color{ 34, 197, 94, 255 } : (hovRdy ? Color{ 255, 60, 135, 255 } : Color{ 238, 52, 132, 255 });
            DrawSlantedRect(rdyRec.x, rdyRec.y, rdyRec.width, rdyRec.height, 14.0f, rdyBg);
            DrawSlantedRectLines(rdyRec.x, rdyRec.y, rdyRec.width, rdyRec.height, 14.0f, 2.0f, WHITE);

            if (texKeySpace.id != 0) {
                DrawTexturePro(texKeySpace, Rectangle{ 0, 0, (float)texKeySpace.width, (float)texKeySpace.height },
                               Rectangle{ rdyRec.x + 22.0f, rdyRec.y + 12.0f, 48.0f, 32.0f }, Vector2{0,0}, 0.0f, WHITE);
            }

            const char* rdyLabel = localIsReady ? LocText(STR_NOT_READY_BTN) : LocText(STR_READY_BTN);
            DrawTextEx(fontUIBold, rdyLabel, Vector2{ rdyRec.x + 85.0f, rdyRec.y + 15.0f }, 22, 0.5f, WHITE);

            if ((hovRdy && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) || IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ENTER)) {
                if (LanGetRole() == LAN_HOST) {
                    gHostReady = !gHostReady;
                } else if (LanGetRole() == LAN_CLIENT) {
                    gClientReady = !gClientReady;
                    LanClientSendLobbyUpdate(gClientReady, (uint8_t)selectedLobbyChar);
                }
            }

            // Start Race Condition: In 2-Player mode, both players must be ready!
            if (LanGetRole() == LAN_HOST && gConnectedPlayers >= 2 && gHostReady && gClientReady) {
                LanSetMatchStarted(true);
                LanSendStartSignal();
                singlePirateMode = false;
                p1RimAngle = 3.14159f;
                p2RimAngle = 0.0f;
                raft.position = Vector3{ 15.55f, 0.58f, 90.80f };
                raft.velocity = Vector3Zero();
                raft.rotationY = 3.14159f;
                raft.isDestroyed = false;
                raft.currentCP = 0;
                gasP1.gasEnergy = GAS_MAX;
                gasP2.gasEnergy = GAS_MAX;
                currentScreen = SCREEN_GAMEPLAY;
            } else if (LanGetRole() == LAN_CLIENT && LanIsMatchStarted()) {
                singlePirateMode = false;
                p1RimAngle = 3.14159f;
                p2RimAngle = 0.0f;
                raft.position = Vector3{ 15.55f, 0.58f, 90.80f };
                raft.velocity = Vector3Zero();
                raft.rotationY = 3.14159f;
                raft.isDestroyed = false;
                raft.currentCP = 0;
                gasP1.gasEnergy = GAS_MAX;
                gasP2.gasEnergy = GAS_MAX;
                currentScreen = SCREEN_GAMEPLAY;
            }

            // Bottom-Left Status
            DrawTextEx(fontUIBold, LanGetStatusText(), Vector2{ 25.0f, scrH - 55.0f }, 15, 0.3f, Color{ 220, 230, 245, 230 });

        } else if (currentScreen == SCREEN_FIND_LOBBY) {
            ClearBackground(Color{ 18, 22, 34, 255 });
            if (texMenuOasisClean.id != 0) {
                DrawTexturePro(texMenuOasisClean, Rectangle{ 0, 0, (float)texMenuOasisClean.width, (float)texMenuOasisClean.height },
                               Rectangle{ 0, 0, (float)GetScreenWidth(), (float)GetScreenHeight() }, Vector2{0,0}, 0.0f, WHITE);
            }
            DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Color{ 28, 24, 52, 140 });

            float scrW = (float)GetScreenWidth();
            float scrH = (float)GetScreenHeight();

            // Top Header: Back Button (v8.3.1 Mobile & PC Support)
            Rectangle backRec = { 25.0f, 20.0f, 130.0f, 50.0f };
            bool hovBack = CheckCollisionPointRec(mousePos, backRec);
            DrawSlantedRect(backRec.x, backRec.y, backRec.width, backRec.height, 12.0f, hovBack ? Color{ 255, 60, 135, 255 } : Color{ 255, 38, 118, 255 });
            const char* backTxt = (gCurrentLanguage == LANG_AR) ? "< رجوع" : "< BACK";
            Vector2 bkSz = MeasureTextEx(fontUIBold, backTxt, 18, 0.3f);
            DrawTextEx(fontUIBold, backTxt, Vector2{ backRec.x + (backRec.width - bkSz.x) * 0.5f, backRec.y + 14.0f }, 18, 0.3f, WHITE);
            if ((hovBack && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) || IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_BACK)) {
                currentScreen = SCREEN_MAIN_MENU;
            }

            // Purple slanted banner title
            DrawSlantedRect(175.0f, 20.0f, 720.0f, 50.0f, 12.0f, Color{ 138, 122, 245, 255 });
            DrawTextEx(fontUIBold, LocText(STR_FIND_TITLE), Vector2{ 200.0f, 32.0f }, 24, 0.4f, WHITE);

            LanStartDiscoveryListener(7778);
            LanUpdateDiscoveryAndLobby(GetFrameTime());

            float barW = 920.0f;
            float barX = (scrW - barW) * 0.5f;

            // Player Name Input Field (REQUIRED CONDITION - Plan v8.3.1)
            float nameBarY = 82.0f;
            DrawRectangleRounded(Rectangle{ barX, nameBarY, barW, 48.0f }, 0.15f, 4, Color{ 245, 247, 255, 255 });
            DrawRectangleRoundedLinesEx(Rectangle{ barX, nameBarY, barW, 48.0f }, 0.15f, 4, 1.5f, Color{ 138, 122, 245, 200 });

            const char* namePrompt = (gCurrentLanguage == LANG_AR) ? "اسمك (إلزامي):" : "Your Name (Required):";
            DrawTextEx(fontUIBold, namePrompt, Vector2{ barX + 20.0f, nameBarY + 14.0f }, 17, 0.4f, Color{ 40, 50, 75, 255 });

            Rectangle nameInRec = { barX + 240.0f, nameBarY + 6.0f, 320.0f, 36.0f };
            static bool s_editNameActive = false;
            if (CheckCollisionPointRec(mousePos, nameInRec) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                s_editNameActive = true;
            }
            if (s_editNameActive) {
                int ch = GetCharPressed();
                while (ch > 0) {
                    size_t len = strlen(gLocalPlayerName);
                    if (len < 22 && ((ch >= 32 && ch <= 126) || (ch >= 0x0600 && ch <= 0x06FF))) {
                        gLocalPlayerName[len] = (char)ch;
                        gLocalPlayerName[len + 1] = '\0';
                    }
                    ch = GetCharPressed();
                }
                if (IsKeyPressed(KEY_BACKSPACE)) {
                    size_t len = strlen(gLocalPlayerName);
                    if (len > 0) gLocalPlayerName[len - 1] = '\0';
                }
                if (IsKeyPressed(KEY_ENTER) || (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !CheckCollisionPointRec(mousePos, nameInRec))) {
                    s_editNameActive = false;
                }
            }
            DrawRectangleRounded(nameInRec, 0.12f, 4, s_editNameActive ? WHITE : Color{ 230, 238, 250, 255 });
            DrawRectangleRoundedLinesEx(nameInRec, 0.12f, 4, 1.5f, s_editNameActive ? Color{ 34, 197, 94, 255 } : Color{ 160, 175, 200, 200 });
            const char* dispName = (strlen(gLocalPlayerName) > 0) ? gLocalPlayerName : "Enter name...";
            DrawTextEx(fontUIBold, dispName, Vector2{ nameInRec.x + 12.0f, nameInRec.y + 8.0f }, 18, 0.4f,
                       (strlen(gLocalPlayerName) > 0) ? Color{ 25, 35, 60, 255 } : Color{ 140, 155, 175, 255 });

            // Name status indicator
            bool nameValid = (strlen(gLocalPlayerName) > 0);
            if (!nameValid) {
                const char* warnTxt = (gCurrentLanguage == LANG_AR) ? "! يرجى كتابة اسمك أولاً !" : "! Please enter your name first !";
                DrawTextEx(fontUIBold, warnTxt, Vector2{ barX + 580.0f, nameBarY + 14.0f }, 16, 0.4f, Color{ 239, 68, 68, 255 });
            } else {
                DrawCircle((int)(barX + 585.0f), (int)(nameBarY + 24.0f), 8.0f, Color{ 34, 197, 94, 255 });
                DrawTextEx(fontUI, "Ready", Vector2{ barX + 600.0f, nameBarY + 14.0f }, 15, 0.3f, Color{ 34, 197, 94, 255 });
            }

            // Quick Join Code Bar
            float barY = 140.0f;
            float barH = 54.0f;
            DrawRectangleRounded(Rectangle{ barX, barY, barW, barH }, 0.12f, 4, Color{ 252, 252, 255, 255 });
            DrawTextEx(fontUIBold, LocText(STR_ROOM_CODE_LBL), Vector2{ barX + 24.0f, barY + 17.0f }, 17, 0.4f, Color{ 40, 50, 75, 255 });

            static char s_manualRoomCode[16] = "";
            static bool s_editCodeActive = false;
            Rectangle codeInRec = { barX + 160.0f, barY + 7.0f, 200.0f, 40.0f };
            if (CheckCollisionPointRec(mousePos, codeInRec) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                s_editCodeActive = true;
            }
            if (s_editCodeActive) {
                int ch = GetCharPressed();
                while (ch > 0) {
                    size_t len = strlen(s_manualRoomCode);
                    if (len < 6 && ((ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z'))) {
                        s_manualRoomCode[len] = (char)toupper(ch);
                        s_manualRoomCode[len + 1] = '\0';
                    }
                    ch = GetCharPressed();
                }
                if (IsKeyPressed(KEY_BACKSPACE)) {
                    size_t len = strlen(s_manualRoomCode);
                    if (len > 0) s_manualRoomCode[len - 1] = '\0';
                }
                if (IsKeyPressed(KEY_ENTER) || (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !CheckCollisionPointRec(mousePos, codeInRec))) {
                    s_editCodeActive = false;
                }
            }
            DrawRectangleRounded(codeInRec, 0.1f, 4, s_editCodeActive ? WHITE : Color{ 235, 240, 250, 255 });
            DrawRectangleRoundedLinesEx(codeInRec, 0.1f, 4, 1.5f, s_editCodeActive ? Color{ 34, 197, 94, 255 } : Color{ 138, 122, 245, 180 });
            const char* codeDisp = (strlen(s_manualRoomCode) > 0) ? s_manualRoomCode : "Code (e.g. A80119)";
            DrawTextEx(fontUIBold, codeDisp, Vector2{ codeInRec.x + 12.0f, codeInRec.y + 10.0f }, 18, 0.4f,
                       (strlen(s_manualRoomCode) > 0) ? Color{ 238, 52, 132, 255 } : Color{ 140, 155, 175, 255 });

            Rectangle btnJoinCode = { barX + barW - 220.0f, barY + 7.0f, 200.0f, 40.0f };
            bool hovJCode = CheckCollisionPointRec(mousePos, btnJoinCode);
            DrawSlantedRect(btnJoinCode.x, btnJoinCode.y, btnJoinCode.width, btnJoinCode.height, 10.0f,
                           (!nameValid) ? Color{ 140, 150, 165, 220 } : (hovJCode ? Color{ 255, 60, 135, 255 } : Color{ 255, 38, 118, 255 }));
            Vector2 jcSz = MeasureTextEx(fontUIBold, LocText(STR_JOIN_BY_CODE), 16, 0.4f);
            DrawTextEx(fontUIBold, LocText(STR_JOIN_BY_CODE), Vector2{ btnJoinCode.x + (btnJoinCode.width - jcSz.x) * 0.5f, btnJoinCode.y + 11.0f }, 16, 0.4f, WHITE);
            if (hovJCode && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && nameValid) {
                if (strlen(s_manualRoomCode) == 6) {
                    LanJoinRoom(s_manualRoomCode, 7777, s_manualRoomCode);
                } else {
                    LanJoinRoom("127.0.0.1", 7777, nullptr);
                }
                currentScreen = SCREEN_LAN_LOBBY;
            }

            // Discovered Rooms List (Wide Horizontal Cards)
            float listX = barX;
            float listY = 205.0f;
            float cardW = barW;
            float cardH = 76.0f;
            float cardGap = 12.0f;

            auto& rooms = LanGetDiscoveredRooms();

            if (rooms.empty()) {
                Rectangle emptyCard = { listX, listY, cardW, 190.0f };
                DrawRectangleRounded(emptyCard, 0.08f, 4, Color{ 252, 252, 255, 245 });
                DrawRectangleRoundedLinesEx(emptyCard, 0.08f, 4, 2.0f, Color{ 138, 122, 245, 180 });

                if (texHipo.id != 0) {
                    float hSz = 68.0f;
                    DrawTexturePro(texHipo, Rectangle{ 0, 0, (float)texHipo.width, (float)texHipo.height },
                                   Rectangle{ listX + 45.0f, listY + 30.0f, hSz, hSz }, Vector2{0,0}, sinf(runTime * 3.0f) * 6.0f, WHITE);
                }

                DrawTextEx(fontUIBold, LocText(STR_WAITING_HOST_MSG), Vector2{ listX + 135.0f, listY + 35.0f }, 18, 0.4f, Color{ 40, 50, 75, 255 });
                const char* tipTxt = (gCurrentLanguage == LANG_AR) ? "افتح غرفة على جهاز آخر على نفس الواي فاي، وسيتم اكتشافها تلقائياً هنا." : "Open a room on another PC/phone on this Wi-Fi, and it will appear here automatically.";
                DrawTextEx(fontUI, tipTxt, Vector2{ listX + 135.0f, listY + 68.0f }, 14, 0.3f, Color{ 100, 115, 140, 255 });

                Rectangle btnCreateNow = { listX + 135.0f, listY + 110.0f, 250.0f, 48.0f };
                bool hovCNow = CheckCollisionPointRec(mousePos, btnCreateNow);
                DrawSlantedRect(btnCreateNow.x, btnCreateNow.y, btnCreateNow.width, btnCreateNow.height, 10.0f,
                               (!nameValid) ? Color{ 140, 150, 165, 220 } : (hovCNow ? Color{ 255, 60, 135, 255 } : Color{ 255, 38, 118, 255 }));
                Vector2 cnSz = MeasureTextEx(fontUIBold, LocText(STR_CREATE_NOW_BTN), 16, 0.4f);
                DrawTextEx(fontUIBold, LocText(STR_CREATE_NOW_BTN), Vector2{ btnCreateNow.x + (btnCreateNow.width - cnSz.x) * 0.5f, btnCreateNow.y + 14.0f }, 16, 0.4f, WHITE);
                if (hovCNow && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && nameValid) {
                    LanInitHost(7777, nullptr);
                    currentScreen = SCREEN_LAN_LOBBY;
                }

                Rectangle joinLocalBtn = { listX + 415.0f, listY + 110.0f, 250.0f, 48.0f };
                bool hovJLocal = CheckCollisionPointRec(mousePos, joinLocalBtn);
                DrawSlantedRect(joinLocalBtn.x, joinLocalBtn.y, joinLocalBtn.width, joinLocalBtn.height, 10.0f,
                               (!nameValid) ? Color{ 140, 150, 165, 220 } : (hovJLocal ? Color{ 34, 197, 94, 255 } : Color{ 22, 163, 74, 255 }));
                const char* locJoinTxt = (gCurrentLanguage == LANG_AR) ? "انضمام لمضيف محلي" : "JOIN LOCAL HOST";
                Vector2 ljSz = MeasureTextEx(fontUIBold, locJoinTxt, 16, 0.3f);
                DrawTextEx(fontUIBold, locJoinTxt, Vector2{ joinLocalBtn.x + (joinLocalBtn.width - ljSz.x) * 0.5f, joinLocalBtn.y + 14.0f }, 16, 0.3f, WHITE);
                if (hovJLocal && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && nameValid) {
                    LanJoinRoom("127.0.0.1", 7777, nullptr);
                    currentScreen = SCREEN_LAN_LOBBY;
                }
            } else {
                for (size_t r = 0; r < rooms.size() && r < 4; r++) {
                    float ry = listY + r * (cardH + cardGap);
                    Rectangle rCard = { listX, ry, cardW, cardH };
                    bool hovCard = CheckCollisionPointRec(mousePos, rCard);

                    DrawRectangleRounded(rCard, 0.12f, 4, hovCard ? Color{ 250, 232, 82, 255 } : Color{ 252, 252, 255, 255 });
                    DrawRectangleRoundedLinesEx(rCard, 0.12f, 4, 1.8f, Color{ 138, 122, 245, 200 });

                    // Pirate Face Icon
                    Texture2D faceTex = texLobbyCards[rooms[r].hostColor % 4];
                    if (faceTex.id != 0) {
                        DrawTexturePro(faceTex, Rectangle{ 0, 0, (float)faceTex.width, (float)faceTex.height },
                                       Rectangle{ listX + 16.0f, ry + 10.0f, 56.0f, 56.0f }, Vector2{0,0}, 0.0f, WHITE);
                    }

                    std::string rTitle = rooms[r].hostName + " (" + rooms[r].roomCode + ")";
                    DrawTextEx(fontUIBold, rTitle.c_str(), Vector2{ listX + 88.0f, ry + 16.0f }, 19, 0.4f, Color{ 30, 40, 65, 255 });
                    std::string subInfo = "IP: " + rooms[r].hostIp + ":" + std::to_string(rooms[r].hostPort) + "  |  Code: " + rooms[r].roomCode;
                    DrawTextEx(fontUI, subInfo.c_str(), Vector2{ listX + 88.0f, ry + 44.0f }, 13, 0.3f, Color{ 90, 105, 130, 255 });

                    // Player Count Badge
                    std::string pCountStr = std::to_string(rooms[r].playerCount) + "/" + std::to_string(rooms[r].maxPlayers) + " " + LocText(STR_PLAYERS_COUNT);
                    DrawTextEx(fontUIBold, pCountStr.c_str(), Vector2{ listX + cardW - 320.0f, ry + 26.0f }, 15, 0.4f, Color{ 70, 85, 115, 255 });

                    Rectangle btnJoin = { listX + cardW - 170.0f, ry + 12.0f, 150.0f, 52.0f };
                    bool hovJoin = CheckCollisionPointRec(mousePos, btnJoin);
                    DrawSlantedRect(btnJoin.x, btnJoin.y, btnJoin.width, btnJoin.height, 10.0f,
                                   (!nameValid) ? Color{ 140, 150, 165, 220 } : (hovJoin ? Color{ 40, 215, 80, 255 } : Color{ 34, 197, 94, 255 }));
                    Vector2 jSz = MeasureTextEx(fontUIBold, LocText(STR_JOIN_ROOM_BTN), 18, 0.4f);
                    DrawTextEx(fontUIBold, LocText(STR_JOIN_ROOM_BTN), Vector2{ btnJoin.x + (btnJoin.width - jSz.x) * 0.5f, btnJoin.y + 16.0f }, 18, 0.4f, WHITE);

                    if ((hovJoin || hovCard) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && nameValid) {
                        LanJoinRoom(rooms[r].hostIp.c_str(), rooms[r].hostPort, rooms[r].roomCode.c_str());
                        currentScreen = SCREEN_LAN_LOBBY;
                    }
                }
            }

            // Bottom Create Button
            float fBotY = (float)GetScreenHeight() - 52.0f;
            Rectangle btnCreateRec = { listX, fBotY, 240.0f, 40.0f };
            bool hovCreate = CheckCollisionPointRec(mousePos, btnCreateRec);
            DrawSlantedRect(btnCreateRec.x, btnCreateRec.y, btnCreateRec.width, btnCreateRec.height, 10.0f,
                           (!nameValid) ? Color{ 140, 150, 165, 220 } : (hovCreate ? Color{ 245, 158, 11, 255 } : Color{ 217, 119, 6, 255 }));
            DrawTextEx(fontUIBold, LocText(STR_CREATE_NOW_BTN), Vector2{ btnCreateRec.x + 22.0f, btnCreateRec.y + 11.0f }, 14, 0.4f, WHITE);
            if (hovCreate && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && nameValid) {
                LanInitHost(7777, nullptr);
                currentScreen = SCREEN_LAN_LOBBY;
            }
        } else if (currentScreen == SCREEN_SETTINGS) {
            ClearBackground(Color{ 18, 22, 34, 255 });

            if (texMenuOasisClean.id != 0) {
                DrawTexturePro(texMenuOasisClean, Rectangle{ 0, 0, (float)texMenuOasisClean.width, (float)texMenuOasisClean.height },
                               Rectangle{ 0, 0, (float)GetScreenWidth(), (float)GetScreenHeight() }, Vector2{0,0}, 0.0f, WHITE);
            }
            DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Color{ 12, 16, 26, 215 });

            // Top-Left Pink Slanted Back Button
            Rectangle backRec = { 0.0f, 32.0f, 120.0f, 54.0f };
            bool hovBack = CheckCollisionPointRec(mousePos, backRec);
            DrawSlantedRect(backRec.x, backRec.y, backRec.width, backRec.height, 14.0f, hovBack ? Color{ 255, 60, 135, 255 } : Color{ 255, 38, 118, 255 });
            if (texIconArrow.id != 0) {
                DrawTexturePro(texIconArrow, Rectangle{ 0, 0, -(float)texIconArrow.width, (float)texIconArrow.height },
                               Rectangle{ 45.0f, backRec.y + 14.0f, 26.0f, 26.0f }, Vector2{0,0}, 0.0f, WHITE);
            }
            if ((hovBack && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) || IsKeyPressed(KEY_ESCAPE)) {
                SaveGameSettingsJSON("game_settings.json");
                currentScreen = SCREEN_MAIN_MENU;
            }

            // Title: SETTINGS
            DrawTextEx(fontUIBold, LocText(STR_SETTINGS), Vector2{ 220.0f, 38.0f }, 38, 0.6f, WHITE);

            // 4 Slanted Tabs
            static int activeSettTab = gTestSettingsTab; // 0=GENERAL, 1=GRAPHICS, 2=AUDIO, 3=CONTROLS
            const char* tabNames[4] = { LocText(STR_TAB_GENERAL), LocText(STR_TAB_GRAPHICS), LocText(STR_TAB_AUDIO), LocText(STR_TAB_CONTROLS) };
            Texture2D tabIcons[4] = { texIconGeneral, texIconGraphic, texIconSound, texIconControl };

            float tabW = 205.0f, tabH = 50.0f, tabX = 200.0f, tabY = 125.0f;
            for (int tb = 0; tb < 4; tb++) {
                Rectangle tRec = { tabX + tb * (tabW + 14.0f), tabY, tabW, tabH };
                bool hovTab = CheckCollisionPointRec(mousePos, tRec);
                bool isAct = (activeSettTab == tb);

                Color tBg = isAct ? Color{ 138, 122, 245, 255 } : (hovTab ? Color{ 45, 52, 75, 240 } : Color{ 28, 33, 48, 235 });
                DrawSlantedRect(tRec.x, tRec.y, tRec.width, tRec.height, 12.0f, tBg);

                if (tabIcons[tb].id != 0) {
                    DrawTexturePro(tabIcons[tb], Rectangle{ 0, 0, (float)tabIcons[tb].width, (float)tabIcons[tb].height },
                                   Rectangle{ tRec.x + 28.0f, tRec.y + 12.0f, 24.0f, 24.0f }, Vector2{0,0}, 0.0f, WHITE);
                }
                DrawTextEx(fontUIBold, tabNames[tb], Vector2{ tRec.x + 60.0f, tRec.y + 15.0f }, 15, 0.4f, WHITE);

                if (hovTab && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) activeSettTab = tb;
            }

            // Settings Rows
            float rowX = 200.0f, rowW = 875.0f, rowH = 64.0f, rowY = 195.0f, rowGap = 12.0f;

            auto DrawSettingRow = [&](int idx, const char* label, auto drawControls) {
                float ry = rowY + idx * (rowH + rowGap);
                Rectangle rRec = { rowX, ry, rowW, rowH };
                bool isHov = CheckCollisionPointRec(mousePos, rRec);

                Color rowBg = isHov ? Color{ 250, 232, 82, 255 } : Color{ 252, 252, 255, 255 };
                Color txtCol = isHov ? Color{ 24, 24, 34, 255 } : Color{ 105, 95, 205, 255 };

                DrawSlantedRect(rRec.x, rRec.y, rRec.width, rRec.height, 12.0f, rowBg);
                DrawTextEx(fontUIBold, label, Vector2{ rRec.x + 40.0f, ry + 22.0f }, 17, 0.4f, txtCol);

                drawControls(rRec, ry, isHov);
            };

            if (activeSettTab == 1) { // GRAPHICS
                // Row 0: Quality
                DrawSettingRow(0, "Quality Level", [&](Rectangle rRec, float ry, bool isHov) {
                    const char* qNames[4] = { "<  Low (30 FPS)  >", "<  Medium (60 FPS)  >", "<  High (60 FPS)  >", "<  Ultra (120 FPS)  >" };
                    Rectangle bQ = { rRec.x + rRec.width - 270.0f, ry + 16.0f, 240.0f, 34.0f };
                    bool hovQ = CheckCollisionPointRec(mousePos, bQ);
                    DrawSlantedRect(bQ.x, bQ.y, bQ.width, bQ.height, 8.0f, hovQ ? Color{ 126, 114, 228, 255 } : Color{ 230, 235, 248, 255 });
                    Vector2 qSz = MeasureTextEx(fontUIBold, qNames[gSettings.qualityLevel], 14, 0.3f);
                    DrawTextEx(fontUIBold, qNames[gSettings.qualityLevel], Vector2{ bQ.x + (bQ.width - qSz.x) * 0.5f, bQ.y + 8.0f }, 14, 0.3f, hovQ ? WHITE : Color{ 30, 35, 55, 255 });
                    if (hovQ && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                        gSettings.qualityLevel = (gSettings.qualityLevel + 1) % 4;
                        ApplySettingsLive();
                        SaveGameSettingsJSON("game_settings.json");
                    }
                });

                // Row 1: Camera Shake
                DrawSettingRow(1, LocText(STR_CAM_SHAKE_ROW), [&](Rectangle rRec, float ry, bool isHov) {
                    Rectangle bSw = { rRec.x + rRec.width - 180.0f, ry + 16.0f, 140.0f, 34.0f };
                    bool hovSw = CheckCollisionPointRec(mousePos, bSw);
                    DrawSlantedRect(bSw.x, bSw.y, bSw.width, bSw.height, 8.0f, Color{ 220, 225, 235, 255 });
                    if (gSettings.cameraShake) {
                        DrawSlantedRect(bSw.x + 65.0f, bSw.y + 2.0f, 70.0f, 30.0f, 8.0f, Color{ 52, 185, 58, 255 });
                    } else {
                        DrawSlantedRect(bSw.x + 4.0f, bSw.y + 2.0f, 65.0f, 30.0f, 8.0f, Color{ 120, 125, 140, 255 });
                    }
                    DrawTextEx(fontUIBold, "OFF", Vector2{ bSw.x + 22.0f, bSw.y + 8.0f }, 13, 0.3f, gSettings.cameraShake ? Color{ 120, 125, 140, 255 } : WHITE);
                    DrawTextEx(fontUIBold, "ON", Vector2{ bSw.x + 88.0f, bSw.y + 8.0f }, 13, 0.3f, gSettings.cameraShake ? WHITE : Color{ 120, 125, 140, 255 });
                    if (hovSw && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                        gSettings.cameraShake = !gSettings.cameraShake;
                        SaveGameSettingsJSON("game_settings.json");
                    }
                });

                // Row 2: Max Camera Distance
                DrawSettingRow(2, LocText(STR_MAX_CAM_DIST_ROW), [&](Rectangle rRec, float ry, bool isHov) {
                    float sldX = rRec.x + rRec.width - 340.0f;
                    DrawTextEx(fontUIBold, TextFormat("%.0fm", gSettings.maxCamDist), Vector2{ sldX - 55.0f, ry + 22.0f }, 15, 0.3f, Color{ 30, 35, 55, 255 });
                    Rectangle sldBar = { sldX, ry + 27.0f, 280.0f, 10.0f };
                    DrawRectangleRounded(sldBar, 0.5f, 4, Color{ 215, 220, 235, 255 });
                    float frac = (gSettings.maxCamDist - 10.0f) / 40.0f;
                    DrawRectangleRounded(Rectangle{ sldBar.x, sldBar.y, sldBar.width * frac, 10.0f }, 0.5f, 4, Color{ 126, 114, 228, 255 });
                    DrawCircle((int)(sldBar.x + sldBar.width * frac), (int)(sldBar.y + 5.0f), 10.0f, Color{ 238, 52, 132, 255 });
                    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mousePos, Rectangle{ sldBar.x - 10, sldBar.y - 12, sldBar.width + 20, 34 })) {
                        gSettings.maxCamDist = Clamp(10.0f + ((mousePos.x - sldBar.x) / sldBar.width) * 40.0f, 10.0f, 50.0f);
                        SaveGameSettingsJSON("game_settings.json");
                    }
                });

                // Row 3: Show FPS Counter (F5)
                DrawSettingRow(2, LocText(STR_SHOW_FPS_ROW), [&](Rectangle rRec, float ry, bool isHov) {
                    Rectangle bSw = { rRec.x + rRec.width - 180.0f, ry + 16.0f, 140.0f, 34.0f };
                    bool hovSw = CheckCollisionPointRec(mousePos, bSw);
                    DrawSlantedRect(bSw.x, bSw.y, bSw.width, bSw.height, 8.0f, Color{ 220, 225, 235, 255 });
                    if (gSettings.showFPS) {
                        DrawSlantedRect(bSw.x + 65.0f, bSw.y + 2.0f, 70.0f, 30.0f, 8.0f, Color{ 52, 185, 58, 255 });
                    } else {
                        DrawSlantedRect(bSw.x + 4.0f, bSw.y + 2.0f, 65.0f, 30.0f, 8.0f, Color{ 120, 125, 140, 255 });
                    }
                    DrawTextEx(fontUIBold, "OFF", Vector2{ bSw.x + 22.0f, bSw.y + 8.0f }, 13, 0.3f, gSettings.showFPS ? Color{ 120, 125, 140, 255 } : WHITE);
                    DrawTextEx(fontUIBold, "ON", Vector2{ bSw.x + 88.0f, bSw.y + 8.0f }, 13, 0.3f, gSettings.showFPS ? WHITE : Color{ 120, 125, 140, 255 });
                    if (hovSw && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                        gSettings.showFPS = !gSettings.showFPS;
                        SaveGameSettingsJSON("game_settings.json");
                    }
                });

            } else if (activeSettTab == 2) { // AUDIO
                auto DrawAudioSlider = [&](int idx, const char* lbl, float& val) {
                    DrawSettingRow(idx, lbl, [&](Rectangle rRec, float ry, bool isHov) {
                        float sldX = rRec.x + rRec.width - 340.0f;
                        DrawTextEx(fontUIBold, TextFormat("%.0f", val), Vector2{ sldX - 45.0f, ry + 22.0f }, 16, 0.3f, Color{ 30, 35, 55, 255 });
                        Rectangle sldBar = { sldX, ry + 27.0f, 280.0f, 10.0f };
                        DrawRectangleRounded(sldBar, 0.5f, 4, Color{ 215, 220, 235, 255 });
                        float frac = val / 100.0f;
                        DrawRectangleRounded(Rectangle{ sldBar.x, sldBar.y, sldBar.width * frac, 10.0f }, 0.5f, 4, Color{ 126, 114, 228, 255 });
                        DrawCircle((int)(sldBar.x + sldBar.width * frac), (int)(sldBar.y + 5.0f), 10.0f, Color{ 238, 52, 132, 255 });
                        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mousePos, Rectangle{ sldBar.x - 10, sldBar.y - 12, sldBar.width + 20, 34 })) {
                            val = Clamp(((mousePos.x - sldBar.x) / sldBar.width) * 100.0f, 0.0f, 100.0f);
                            ApplySettingsLive();
                            SaveGameSettingsJSON("game_settings.json");
                        }
                    });
                };
                DrawAudioSlider(0, "Master Volume", gSettings.masterVolume);
                DrawAudioSlider(1, "Music (OST & Waves)", gSettings.musicVolume);
                DrawAudioSlider(2, "Sound Effects (SFX)", gSettings.sfxVolume);
                DrawAudioSlider(3, "Fart & Gas Volume", gSettings.fartVolume);

            } else if (activeSettTab == 0) { // GENERAL
                DrawSettingRow(1, LocText(STR_DEVICE_PROFILE), [&](Rectangle rRec, float ry, bool isHov) {
                    const char* dNames[3] = { "<  Auto-Detect  >", "<  PC (Keyboard/Mouse)  >", "<  Mobile (Touchscreen)  >" };
                    Rectangle bD = { rRec.x + rRec.width - 260.0f, ry + 16.0f, 230.0f, 34.0f };
                    bool hovD = CheckCollisionPointRec(mousePos, bD);
                    DrawSlantedRect(bD.x, bD.y, bD.width, bD.height, 8.0f, hovD ? Color{ 126, 114, 228, 255 } : Color{ 230, 235, 248, 255 });
                    Vector2 dSz = MeasureTextEx(fontUIBold, dNames[gSettings.deviceMode], 13, 0.3f);
                    DrawTextEx(fontUIBold, dNames[gSettings.deviceMode], Vector2{ bD.x + (bD.width - dSz.x) * 0.5f, bD.y + 9.0f }, 13, 0.3f, hovD ? WHITE : Color{ 30, 35, 55, 255 });
                    if (hovD && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                        gSettings.deviceMode = (gSettings.deviceMode + 1) % 3;
                        SaveGameSettingsJSON("game_settings.json");
                    }
                });

                DrawSettingRow(2, LocText(STR_INVERT_Y_ROW), [&](Rectangle rRec, float ry, bool isHov) {
                    Rectangle bSw = { rRec.x + rRec.width - 180.0f, ry + 16.0f, 140.0f, 34.0f };
                    bool hovSw = CheckCollisionPointRec(mousePos, bSw);
                    DrawSlantedRect(bSw.x, bSw.y, bSw.width, bSw.height, 8.0f, Color{ 220, 225, 235, 255 });
                    if (gSettings.invertMouseY) {
                        DrawSlantedRect(bSw.x + 65.0f, bSw.y + 2.0f, 70.0f, 30.0f, 8.0f, Color{ 52, 185, 58, 255 });
                    } else {
                        DrawSlantedRect(bSw.x + 4.0f, bSw.y + 2.0f, 65.0f, 30.0f, 8.0f, Color{ 120, 125, 140, 255 });
                    }
                    DrawTextEx(fontUIBold, "OFF", Vector2{ bSw.x + 22.0f, bSw.y + 8.0f }, 13, 0.3f, gSettings.invertMouseY ? Color{ 120, 125, 140, 255 } : WHITE);
                    DrawTextEx(fontUIBold, "ON", Vector2{ bSw.x + 88.0f, bSw.y + 8.0f }, 13, 0.3f, gSettings.invertMouseY ? WHITE : Color{ 120, 125, 140, 255 });
                    if (hovSw && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                        gSettings.invertMouseY = !gSettings.invertMouseY;
                        SaveGameSettingsJSON("game_settings.json");
                    }
                });

                DrawSettingRow(3, LocText(STR_LEVEL_EDITOR_ROW), [&](Rectangle rRec, float ry, bool isHov) {
                    Rectangle bEd = { rRec.x + rRec.width - 250.0f, ry + 14.0f, 220.0f, 38.0f };
                    bool hovEd = CheckCollisionPointRec(mousePos, bEd);
                    DrawSlantedRect(bEd.x, bEd.y, bEd.width, bEd.height, 8.0f, hovEd ? Color{ 34, 197, 94, 255 } : Color{ 22, 163, 74, 255 });
                    DrawTextEx(fontUIBold, "[ OPEN LEVEL EDITOR ]", Vector2{ bEd.x + 18.0f, bEd.y + 11.0f }, 14, 0.4f, WHITE);
                    if (hovEd && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                        editorMode = true;
                        currentScreen = SCREEN_GAMEPLAY;
                    }
                });

                DrawSettingRow(3, LocText(STR_LANGUAGE_LBL), [&](Rectangle rRec, float ry, bool isHov) {
                    const char* lNames[2] = { (const char*)u8"<  العربية (افتراضي)  >", "<  English  >" };
                    Rectangle bL = { rRec.x + rRec.width - 260.0f, ry + 16.0f, 230.0f, 34.0f };
                    bool hovL = CheckCollisionPointRec(mousePos, bL);
                    DrawSlantedRect(bL.x, bL.y, bL.width, bL.height, 8.0f, hovL ? Color{ 126, 114, 228, 255 } : Color{ 230, 235, 248, 255 });
                    Vector2 lSz = MeasureTextEx(fontUIBold, lNames[gCurrentLanguage], 13, 0.3f);
                    DrawTextEx(fontUIBold, lNames[gCurrentLanguage], Vector2{ bL.x + (bL.width - lSz.x) * 0.5f, bL.y + 9.0f }, 13, 0.3f, hovL ? WHITE : Color{ 30, 35, 55, 255 });
                    if (hovL && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                        gCurrentLanguage = (gCurrentLanguage == LANG_AR) ? LANG_EN : LANG_AR;
                        gSettings.language = (int)gCurrentLanguage;
                        SaveGameSettingsJSON("game_settings.json");
                    }
                });

            } else { // CONTROLS
                static int controlSubTab = (gSettings.deviceMode == 2) ? 1 : 0; // 0=PC, 1=Mobile Touch

                // Sub tab switcher: PC vs Mobile
                Rectangle bSubPC  = { rowX, rowY - 42.0f, 150.0f, 34.0f };
                Rectangle bSubMob = { rowX + 160.0f, rowY - 42.0f, 180.0f, 34.0f };
                bool hovSubPC  = CheckCollisionPointRec(mousePos, bSubPC);
                bool hovSubMob = CheckCollisionPointRec(mousePos, bSubMob);
                DrawSlantedRect(bSubPC.x, bSubPC.y, bSubPC.width, bSubPC.height, 8.0f, (controlSubTab == 0) ? Color{ 126, 114, 228, 255 } : Color{ 45, 55, 75, 220 });
                DrawTextEx(fontUIBold, "PC Keyboard/Mouse", Vector2{ bSubPC.x + 12.0f, bSubPC.y + 9.0f }, 13, 0.3f, WHITE);
                DrawSlantedRect(bSubMob.x, bSubMob.y, bSubMob.width, bSubMob.height, 8.0f, (controlSubTab == 1) ? Color{ 126, 114, 228, 255 } : Color{ 45, 55, 75, 220 });
                DrawTextEx(fontUIBold, "Mobile Touch Controls", Vector2{ bSubMob.x + 14.0f, bSubMob.y + 9.0f }, 13, 0.3f, WHITE);
                if (hovSubPC && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) controlSubTab = 0;
                if (hovSubMob && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) controlSubTab = 1;

                if (controlSubTab == 0) { // PC MODE
                    DrawSettingRow(0, "P1 Fart Gas Controls", [&](Rectangle rRec, float ry, bool isHov) {
                        DrawTextEx(fontUIBold, "A (Left) / D (Right) & Space/W (Gas)", Vector2{ rRec.x + rRec.width - 320.0f, ry + 22.0f }, 15, 0.4f, Color{ 30, 35, 55, 255 });
                    });
                    DrawSettingRow(1, "Level Editor Mode", [&](Rectangle rRec, float ry, bool isHov) {
                        Rectangle bEd = { rRec.x + rRec.width - 250.0f, ry + 14.0f, 220.0f, 38.0f };
                        bool hovEd = CheckCollisionPointRec(mousePos, bEd);
                        DrawSlantedRect(bEd.x, bEd.y, bEd.width, bEd.height, 8.0f, hovEd ? Color{ 34, 197, 94, 255 } : Color{ 22, 163, 74, 255 });
                        DrawTextEx(fontUIBold, "[ OPEN EDITOR (F1) ]", Vector2{ bEd.x + 16.0f, bEd.y + 11.0f }, 14, 0.4f, WHITE);
                        if (hovEd && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                            editorMode = true;
                            currentScreen = SCREEN_GAMEPLAY;
                        }
                    });
                    DrawSettingRow(2, "Show FPS Toggle Key", [&](Rectangle rRec, float ry, bool isHov) {
                        DrawTextEx(fontUIBold, "[ F5 Key ]", Vector2{ rRec.x + rRec.width - 150.0f, ry + 22.0f }, 15, 0.4f, Color{ 30, 35, 55, 255 });
                    });
                    DrawSettingRow(3, "Suicide Respawn Key", [&](Rectangle rRec, float ry, bool isHov) {
                        DrawTextEx(fontUIBold, "[ R Key ]", Vector2{ rRec.x + rRec.width - 150.0f, ry + 22.0f }, 15, 0.4f, Color{ 30, 35, 55, 255 });
                    });
                } else { // MOBILE TOUCH MODE
                    DrawSettingRow(0, "Touch Button Scale", [&](Rectangle rRec, float ry, bool isHov) {
                        float sldX = rRec.x + rRec.width - 340.0f;
                        DrawTextEx(fontUIBold, TextFormat("%.0f%%", gSettings.touchBtnScale * 100.0f), Vector2{ sldX - 60.0f, ry + 22.0f }, 15, 0.3f, Color{ 30, 35, 55, 255 });
                        Rectangle sldBar = { sldX, ry + 27.0f, 280.0f, 10.0f };
                        DrawRectangleRounded(sldBar, 0.5f, 4, Color{ 215, 220, 235, 255 });
                        float frac = (gSettings.touchBtnScale - 0.50f) / 1.30f;
                        DrawRectangleRounded(Rectangle{ sldBar.x, sldBar.y, sldBar.width * frac, 10.0f }, 0.5f, 4, Color{ 126, 114, 228, 255 });
                        DrawCircle((int)(sldBar.x + sldBar.width * frac), (int)(sldBar.y + 5.0f), 10.0f, Color{ 238, 52, 132, 255 });
                        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mousePos, Rectangle{ sldBar.x - 10, sldBar.y - 12, sldBar.width + 20, 34 })) {
                            gSettings.touchBtnScale = Clamp(0.50f + ((mousePos.x - sldBar.x) / sldBar.width) * 1.30f, 0.50f, 1.80f);
                            SaveGameSettingsJSON("game_settings.json");
                        }
                    });

                    DrawSettingRow(1, "Steer Buttons Pos X / Y", [&](Rectangle rRec, float ry, bool isHov) {
                        Rectangle bReset = { rRec.x + rRec.width - 240.0f, ry + 16.0f, 200.0f, 34.0f };
                        bool hovR = CheckCollisionPointRec(mousePos, bReset);
                        DrawSlantedRect(bReset.x, bReset.y, bReset.width, bReset.height, 8.0f, hovR ? Color{ 126, 114, 228, 255 } : Color{ 230, 235, 248, 255 });
                        DrawTextEx(fontUIBold, "Reset Left/Right Pos", Vector2{ bReset.x + 20.0f, bReset.y + 8.0f }, 13, 0.3f, hovR ? WHITE : Color{ 30, 35, 55, 255 });
                        if (hovR && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                            gSettings.steerLeftPosX = 0.08f; gSettings.steerLeftPosY = 0.78f;
                            gSettings.steerRightPosX = 0.22f; gSettings.steerRightPosY = 0.78f;
                            SaveGameSettingsJSON("game_settings.json");
                        }
                    });

                    DrawSettingRow(2, "Gas Button Pos X / Y", [&](Rectangle rRec, float ry, bool isHov) {
                        Rectangle bReset = { rRec.x + rRec.width - 240.0f, ry + 16.0f, 200.0f, 34.0f };
                        bool hovR = CheckCollisionPointRec(mousePos, bReset);
                        DrawSlantedRect(bReset.x, bReset.y, bReset.width, bReset.height, 8.0f, hovR ? Color{ 126, 114, 228, 255 } : Color{ 230, 235, 248, 255 });
                        DrawTextEx(fontUIBold, "Reset Gas Button Pos", Vector2{ bReset.x + 22.0f, bReset.y + 8.0f }, 13, 0.3f, hovR ? WHITE : Color{ 30, 35, 55, 255 });
                        if (hovR && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                            gSettings.gasBtnPosX = 0.86f; gSettings.gasBtnPosY = 0.76f;
                            SaveGameSettingsJSON("game_settings.json");
                        }
                    });

                    DrawSettingRow(3, "USB Keyboard/Mouse OTG", [&](Rectangle rRec, float ry, bool isHov) {
                        DrawTextEx(fontUIBold, "[ Auto-Supported via USB Host ]", Vector2{ rRec.x + rRec.width - 280.0f, ry + 22.0f }, 14, 0.3f, Color{ 34, 197, 94, 255 });
                    });
                }
            }

            // Scrollbar on right
            DrawRectangleRounded(Rectangle{ 1100.0f, rowY, 8.0f, 4 * rowH + 3 * rowGap }, 0.5f, 4, Color{ 220, 225, 240, 255 });
            DrawRectangleRounded(Rectangle{ 1100.0f, rowY + 30.0f, 8.0f, 90.0f }, 0.5f, 4, Color{ 126, 114, 228, 255 });

            // Bottom Links
            float botLinkY = (float)GetScreenHeight() - 48.0f;
            Rectangle bPriv = { 200.0f, botLinkY, 150.0f, 32.0f };
            Rectangle bTerms = { 370.0f, botLinkY, 150.0f, 32.0f };
            DrawSlantedRect(bPriv.x, bPriv.y, bPriv.width, bPriv.height, 8.0f, Color{ 252, 252, 255, 230 });
            DrawTextEx(fontUIBold, LocText(STR_PRIVACY_POLICY), Vector2{ bPriv.x + 22.0f, bPriv.y + 8.0f }, 13, 0.3f, Color{ 80, 70, 160, 255 });
            DrawSlantedRect(bTerms.x, bTerms.y, bTerms.width, bTerms.height, 8.0f, Color{ 252, 252, 255, 230 });
            DrawTextEx(fontUIBold, LocText(STR_TERMS_OF_SERVICE), Vector2{ bTerms.x + 18.0f, bTerms.y + 8.0f }, 13, 0.3f, Color{ 80, 70, 160, 255 });
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
    UnloadSound(sndMenuBeachOst);
    UnloadSound(sndSeagull);
    UnloadSound(sndShipHorn);
    UnloadSound(sndSnore);
    UnloadSound(sndEatMunch);
    UnloadSound(sndBurp);
    UnloadSound(sndCheckpoint);
    UnloadTexture(texMenuOasisFull);
    UnloadTexture(texMenuOasisClean);
    UnloadTexture(texMenuRaftSleep);
    UnloadTexture(texMainLogo);
    UnloadTexture(texLogoSplash);
    UnloadTexture(texMenuBg);
    UnloadTexture(texMarkerYou);
    UnloadTexture(texIconTikGreen);
    UnloadTexture(texMarkerPlayer);
    UnloadTexture(texKeySpace);
    UnloadTexture(texIconKEnter);
    UnloadTexture(texModeFartRun);
    UnloadTexture(texModeTeamRace);
    UnloadTexture(texModeRaftRumble);
    UnloadTexture(texIconGeneral);
    UnloadTexture(texIconGraphic);
    UnloadTexture(texIconSound);
    UnloadTexture(texIconControl);
    UnloadTexture(texIconDiscord);
    UnloadTexture(texIconCopy);
    UnloadTexture(texIconArrow);
    for (int k = 0; k < 4; k++) UnloadTexture(texLobbyCards[k]);
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
        if (envSectors[s].meshCount > 0) UnloadModel(envSectors[s]);
    }
    for (auto& vf : menuVideoFrames) {
        if (vf.id != 0) UnloadTexture(vf);
    }
    UnloadTexture(texWaterCyan);
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