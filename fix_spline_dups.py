import pathlib

p = pathlib.Path('CustomEngine/native/GameEngine.cpp')
t = p.read_text(encoding='utf-8')

target = '''        // 4. River Spline Physics & Flow

        float currentSplineT = FindClosestSplineT(raft.position);

        Vector3 riverCenter = GetSplinePoint(currentSplineT);

        Vector3 riverTangent = GetSplineTangent(currentSplineT);

        Vector3 riverNormal = Vector3{ riverTangent.z, 0.0f, -riverTangent.x };

'''

if target in t:
    t = t.replace(target, '')
    p.write_text(t, encoding='utf-8')
    print("SUCCESS: Removed duplicate spline declarations")
else:
    print("Target not found!")

