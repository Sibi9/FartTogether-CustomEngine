with open('CustomEngine/native/GameEngine.cpp', 'r', encoding='utf-8') as f:
    c = f.read()

# At the first occurrence (around line 978 / 1950), declare it with Matrix:
target_first = """        float rimRadius = 1.25f;

        Vector3 p1OutwardLocal = Vector3{ sinf(p1RimAngle), 0.0f, cosf(p1RimAngle) };

        Vector3 p1ThrustLocal  = Vector3Scale(p1OutwardLocal, -1.0f); // Inward push!



        boatRotM = MatrixRotateY(raft.rotationY);"""

replacement_first = """        float rimRadius = 1.25f;

        Vector3 p1OutwardLocal = Vector3{ sinf(p1RimAngle), 0.0f, cosf(p1RimAngle) };

        Vector3 p1ThrustLocal  = Vector3Scale(p1OutwardLocal, -1.0f); // Inward push!



        Matrix boatRotM = MatrixRotateY(raft.rotationY);"""

if target_first in c:
    c = c.replace(target_first, replacement_first)
    print("Found and fixed first declaration")
else:
    # Try normalized whitespace
    print("Direct target not found, trying search")
    idx = c.find("float rimRadius = 1.25f;")
    print("idx:", idx)
    part = c[idx:idx+300]
    print("part:", repr(part))

with open('CustomEngine/native/GameEngine.cpp', 'w', encoding='utf-8') as f:
    f.write(c)
