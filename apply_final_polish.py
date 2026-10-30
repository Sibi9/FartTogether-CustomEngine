with open(r"CustomEngine\native\GameEngine.cpp", "r", encoding="utf-8") as f:
    code = f.read()

# 1. Update particle size in EmitFartBurst
old_particle_spawn = """                particles[p].velocity = Vector3{ spread.x, 1.4f + spread.y, spread.z + 1.4f };
                particles[p].size = 1.4f + (chargePercent * 2.0f) + ((float)rand() / RAND_MAX) * 0.8f;
                if (isSuperSync) particles[p].size *= 1.35f;"""

new_particle_spawn = """                particles[p].velocity = Vector3{ spread.x * 0.7f, 1.1f + spread.y * 0.6f, spread.z * 0.7f + 1.2f };
                particles[p].size = 0.55f + (chargePercent * 0.45f) + ((float)rand() / RAND_MAX) * 0.25f;
                if (isSuperSync) particles[p].size *= 1.25f;"""

code = code.replace(old_particle_spawn, new_particle_spawn)

# 2. Update water normal smoothing in FS_WATER
old_water_n = """void main() {
    vec3 N = normalize(fragNormal);
    vec3 V = normalize(camPos - fragPos);"""

new_water_n = """void main() {
    // Perturb normal with smooth scrolling ripple derivative to eliminate polygonal triangulation seams
    vec2 ripUV = fragPos.xz * 0.12 + vec2(uTime * 0.02, uTime * 0.025);
    float hL = texture(causticTex, ripUV - vec2(0.01, 0.0)).r;
    float hR = texture(causticTex, ripUV + vec2(0.01, 0.0)).r;
    float hD = texture(causticTex, ripUV - vec2(0.0, 0.01)).r;
    float hU = texture(causticTex, ripUV + vec2(0.0, 0.01)).r;
    vec3 rippleN = normalize(vec3((hL - hR) * 0.45, 1.0, (hD - hU) * 0.45));

    vec3 N = normalize(fragNormal * 0.60 + rippleN * 0.40);
    vec3 V = normalize(camPos - fragPos);"""

code = code.replace(old_water_n, new_water_n)

# 3. Refine character Y position on boat tube
old_pos = "rlTranslatef(p1X, 0.44f + gasP1.trembleOffset, p1Z);"
new_pos = "rlTranslatef(p1X, 0.38f + gasP1.trembleOffset, p1Z);"
code = code.replace(old_pos, new_pos)

old_pos2 = "rlTranslatef(p2X, 0.44f + gasP2.trembleOffset, p2Z);"
new_pos2 = "rlTranslatef(p2X, 0.38f + gasP2.trembleOffset, p2Z);"
code = code.replace(old_pos2, new_pos2)

with open(r"CustomEngine\native\GameEngine.cpp", "w", encoding="utf-8") as f:
    f.write(code)

print("Applied final polish successfully")

