with open('CustomEngine/native/GameEngine.cpp', 'r', encoding='utf-8') as f:
    code = f.read()

# 1. p1PosWorld and p2PosWorld height offset
old_p1_pos = """        Vector3 p1PosWorld = Vector3Add(raft.position, Vector3Transform(Vector3Scale(p1OutwardLocal, rimRadius), boatRotM));"""
new_p1_pos = """        Vector3 p1PosWorld = Vector3Add(raft.position, Vector3Transform(Vector3Scale(p1OutwardLocal, rimRadius), boatRotM));
        p1PosWorld.y += 0.55f;"""

assert old_p1_pos in code, 'old_p1_pos not found'
code = code.replace(old_p1_pos, new_p1_pos)
print('Updated p1PosWorld height')

old_p2_pos = """        Vector3 p2PosWorld = Vector3Add(raft.position, Vector3Transform(Vector3Scale(p2OutwardLocal, rimRadius), boatRotM));"""
new_p2_pos = """        Vector3 p2PosWorld = Vector3Add(raft.position, Vector3Transform(Vector3Scale(p2OutwardLocal, rimRadius), boatRotM));
        p2PosWorld.y += 0.55f;"""

assert old_p2_pos in code, 'old_p2_pos not found'
code = code.replace(old_p2_pos, new_p2_pos)
print('Updated p2PosWorld height')

# 2. Disable Depth Test for Billboard Fart Particles
old_particles = """            // I. Cartoon Billboard Fart Particles (Proper OpenGL Alpha Blending & Depth Mask Disable)
            for (int p = 0; p < MAX_PARTICLES; p++) {
                if (particles[p].active) {
                    Texture2D tex = texFart[particles[p].spriteIndex];
                    Rectangle srcRec = Rectangle{ 0, 0, (float)tex.width, (float)tex.height };
                    Color pTint = Color{ 255, 255, 255, (unsigned char)(particles[p].alpha * 220) };
                    DrawBillboardPro(camera, tex, srcRec, particles[p].position, Vector3{ 0, 1, 0 },
                                     Vector2{ particles[p].size, particles[p].size * 0.65f },
                                     Vector2{ particles[p].size * 0.5f, particles[p].size * 0.32f },
                                     particles[p].rotation, pTint);
                }
            }
            EndBlendMode();
            rlEnableDepthMask();"""

new_particles = """            // I. Cartoon Billboard Fart Particles (Buttery Smooth Billows, Zero Water Clipping)
            rlDisableDepthTest();
            for (int p = 0; p < MAX_PARTICLES; p++) {
                if (particles[p].active) {
                    Texture2D tex = texFart[particles[p].spriteIndex];
                    Rectangle srcRec = Rectangle{ 0, 0, (float)tex.width, (float)tex.height };
                    Color pTint = Color{ 255, 255, 255, (unsigned char)(particles[p].alpha * 220) };
                    DrawBillboardPro(camera, tex, srcRec, particles[p].position, Vector3{ 0, 1, 0 },
                                     Vector2{ particles[p].size, particles[p].size * 0.65f },
                                     Vector2{ particles[p].size * 0.5f, particles[p].size * 0.32f },
                                     particles[p].rotation, pTint);
                }
            }
            EndBlendMode();
            rlEnableDepthTest();
            rlEnableDepthMask();"""

assert old_particles in code, 'old_particles not found'
code = code.replace(old_particles, new_particles)
print('Updated particle depth test disable')

with open('CustomEngine/native/GameEngine.cpp', 'w', encoding='utf-8') as f:
    f.write(code)
print('Successfully wrote CustomEngine/native/GameEngine.cpp')

