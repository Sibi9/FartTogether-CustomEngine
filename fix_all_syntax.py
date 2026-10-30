with open("CustomEngine/native/GameEngine.cpp", "r", encoding="utf-8") as f:
    c = f.read()

# 1. Clean up VS_LIGHTING and FS_LIGHTING
idx_vs = c.find("static const char* VS_LIGHTING = R\"(")
idx_water = c.find("static const char* VS_WATER = R\"(")
assert idx_vs != -1 and idx_water != -1, "Shader markers not found"

clean_shaders = '''static const char* VS_LIGHTING = R"(
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

    // A. Butt Swell & Balloon Inflation (Local rear butt: Z < -0.02, Y < 0.55)
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
)";

'''

c = c[:idx_vs] + clean_shaders + c[idx_water:]

# 2. Add uButtSwellLoc & uEyeBulgeLoc declarations
target_red = 'int uRednessLoc   = GetShaderLocation(shaderLighting, "uRedness");'
idx_red = c.find(target_red)
assert idx_red != -1, "target_red not found"
new_red_block = '''int uRednessLoc   = GetShaderLocation(shaderLighting, "uRedness");
    int uButtSwellLoc = GetShaderLocation(shaderLighting, "uButtSwell");
    int uEyeBulgeLoc  = GetShaderLocation(shaderLighting, "uEyeBulge");
    float zeroVal = 0.0f;
    SetShaderValue(shaderLighting, uButtSwellLoc, &zeroVal, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shaderLighting, uEyeBulgeLoc,  &zeroVal, SHADER_UNIFORM_FLOAT);'''
c = c[:idx_red] + new_red_block + c[idx_red + len(target_red):]

# 3. Add toCenter and crossDist before inFlumeCanal
target_flume = 'bool inFlumeCanal = (raft.position.z > 36.0f);'
idx_flume = c.find(target_flume)
assert idx_flume != -1, "target_flume not found"
new_flume_block = '''Vector3 toCenter = Vector3Subtract(raft.position, riverCenter);
        float crossDist = Vector3DotProduct(toCenter, riverNormal);

        bool inFlumeCanal = (raft.position.z > 36.0f);'''
c = c[:idx_flume] + new_flume_block + c[idx_flume + len(target_flume):]

# 4. Fix GetMouseRay -> GetScreenToWorldRay
c = c.replace("Ray mRay = GetMouseRay(mousePos, camera);", "Ray mRay = GetScreenToWorldRay(mousePos, camera);")

with open("CustomEngine/native/GameEngine.cpp", "w", encoding="utf-8") as f:
    f.write(c)

print("Fixed syntax cleanly in GameEngine.cpp!")

