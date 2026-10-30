import re, os

print("Writing apply_full_overhaul_v6.py...")
with open("CustomEngine/native/GameEngine.cpp", "r", encoding="utf-8") as f:
    src = f.read()

# 1. Update Shaders
# Replace VS_LIGHTING to include vertex deformation uniforms: uButtSwell and uEyeBulge, and pass fragPosition
vs_lighting_old_pattern = r'static const char\* VS_LIGHTING = R"\([\s\S]*?\);'
vs_lighting_new = '''static const char* VS_LIGHTING = R"(
#version 330
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
in vec4 vertexColor;
uniform mat4 mvp;
uniform float uButtSwell; // 0.0 to 1.0 (Butt balloon inflation)
uniform float uEyeBulge;  // 0.0 to 1.0 (Straining bulging cartoon eyes)
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

    // A. Butt Swell & Balloon Inflation (Local rear butt: Z < -0.05, Y < 0.50)
    if (pos.z < -0.02 && pos.y < 0.55 && uButtSwell > 0.01) {
        float rearFactor = max(0.0, -pos.z);
        pos.z -= uButtSwell * 0.28 * (rearFactor / 0.18);
        pos.x *= (1.0 + uButtSwell * 0.22);
        pos.y += uButtSwell * 0.08 * (pos.y / 0.45);
    }

    // B. Straining Cartoon Bulging Eyes (Local head/eyes: Y in [0.82, 1.05], Z > 0.12, |X| in [0.08, 0.26])
    if (pos.y > 0.82 && pos.z > 0.12 && abs(pos.x) > 0.08 && uEyeBulge > 0.01) {
        pos.z += uEyeBulge * 0.24; // Bulge outwards forward!
        pos.y += uEyeBulge * 0.09; // Bulge upward
        pos.x += sign(pos.x) * uEyeBulge * 0.09; // Bulge outward
    }

    gl_Position = mvp * vec4(pos, 1.0);
}
)";'''

src = re.sub(vs_lighting_old_pattern, vs_lighting_new, src, count=1)

# Replace FS_LIGHTING to mask redness ONLY to the butt
fs_lighting_old_pattern = r'static const char\* FS_LIGHTING = R"\([\s\S]*?\);'
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
uniform float uRedness; // 0.0 = normal, 1.0 = deep scarlet vascular redness
out vec4 finalColor;

void main() {
    vec4 texColor = texture(texture0, fragTexCoord);
    vec3 N = normalize(fragNormal);
    vec3 L = normalize(lightDir);
    float diff = max(dot(N, L), 0.0);
    vec3 diffuse = diff * lightColor;
    vec3 ambient = ambientColor;
    vec3 lighting = ambient + diffuse;

    // Authentic Localized Butt Redness Mask (Center: 0.0, 0.40, -0.14)
    vec3 buttCenter = vec3(0.0, 0.40, -0.14);
    vec3 buttScale  = vec3(0.36, 0.28, 0.24);
    float buttDist  = length((fragPosition - buttCenter) / buttScale);
    float buttMask  = clamp(1.0 - smoothstep(0.45, 1.15, buttDist), 0.0, 1.0);

    // ONLY the butt turns fiery red, face and rest of body stay normal!
    vec3 redSkin = mix(texColor.rgb, vec3(0.96, 0.12, 0.15), uRedness * buttMask * 0.90);
    vec3 litColor = redSkin * lighting * colDiffuse.rgb;
    finalColor = vec4(litColor, texColor.a * colDiffuse.a);
}
)";'''

src = re.sub(fs_lighting_old_pattern, fs_lighting_new, src, count=1)

print("Shaders updated successfully!")

with open("CustomEngine/native/GameEngine.cpp", "w", encoding="utf-8") as f:
    f.write(src)

