import json, math, pathlib, os

# Determine base dir
if os.path.exists("CustomEngine/extracted_assets"):
    asset_base = pathlib.Path("CustomEngine/extracted_assets")
else:
    asset_base = pathlib.Path("extracted_assets")

sp = json.loads((asset_base / "river_spline.json").read_text(encoding="utf-8"))

water_verts = []
with open(asset_base / "meshes/GM_3D_Demo_Water.obj", encoding="utf-8") as f:
    for line in f:
        if line.startswith("v "):
            v = [float(x) for x in line.split()[1:4]]
            water_verts.append((v[0] + 38.44, v[1] + 0.57, v[2] + 156.14))

# 1. تصحيح نقاط الـ Spline لتتبع مركز مجسم الماء الحقيقي
corrected_sp = []
for x, y, z in sp:
    # البحث عن أقرب رأس ماء، ثم أخذ متوسط الرؤوس القريبة منه (نصف قطر 6م) للحصول على مركز القناة
    nearest = min(water_verts, key=lambda w: math.hypot(x - w[0], z - w[2]))
    cluster = [w for w in water_verts if math.hypot(nearest[0] - w[0], nearest[2] - w[2]) < 5.5]
    if cluster:
        cx = sum(w[0] for w in cluster) / len(cluster)
        cy = sum(w[1] for w in cluster) / len(cluster)
        cz = sum(w[2] for w in cluster) / len(cluster)
        # إذا كانت النقطة الأصلية منحرفة بأكثر من 2.5م عن الماء، نعتمد مركز الماء الفعلي
        if math.hypot(x - nearest[0], z - nearest[2]) > 2.5:
            corrected_sp.append((cx, cy, cz))
        else:
            corrected_sp.append((x, cy, z))
    else:
        corrected_sp.append((x, y, z))

# 2. بناء عينة مكانية منتظمة لرؤوس مجسم الماء (Grid 1.5m) لفحص الطفو الفوري O(1)/O(N_small)
grid = {}
for wx, wy, wz in water_verts:
    gx, gz = int(round(wx / 1.5)), int(round(wz / 1.5))
    key = (gx, gz)
    if key not in grid or wy > grid[key][1]:
        grid[key] = (wx, wy, wz)

water_samples = sorted(grid.values(), key=lambda w: (w[0], w[2]))

lines = [
    "// Auto-generated Authentic River Spline & Water Surface Samples",
    "#pragma once",
    f"static const int SPLINE_POINT_COUNT = {len(corrected_sp)};",
    "static const Vector3 AUTHENTIC_RIVER_SPLINE[SPLINE_POINT_COUNT] = {"
]
for i, (x, y, z) in enumerate(corrected_sp):
    comma = "," if i + 1 < len(corrected_sp) else ""
    lines.append(f"    {{ {x:.4f}f, {y:.4f}f, {z:.4f}f }}{comma}")
lines.append("};\n")

lines.append(f"static const int WATER_SAMPLE_COUNT = {len(water_samples)};")
lines.append("static const Vector3 AUTHENTIC_WATER_SAMPLES[WATER_SAMPLE_COUNT] = {")
for i, (wx, wy, wz) in enumerate(water_samples):
    comma = "," if i + 1 < len(water_samples) else ""
    lines.append(f"    {{ {wx:.3f}f, {wy:.3f}f, {wz:.3f}f }}{comma}")
lines.append("};\n")

content = "\n".join(lines)
os.makedirs("CustomEngine/native", exist_ok=True)
pathlib.Path("CustomEngine/native/spline_code.inl").write_text(content, encoding="utf-8")
if os.path.exists("native"):
    pathlib.Path("native/spline_code.inl").write_text(content, encoding="utf-8")

print(f"Generated native/spline_code.inl with {len(corrected_sp)} spline points and {len(water_samples)} water surface samples.")

