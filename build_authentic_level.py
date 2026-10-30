import json, math, pathlib

ASSETS_DIR = pathlib.Path("CustomEngine/extracted_assets")
if not ASSETS_DIR.exists():
    ASSETS_DIR = pathlib.Path("extracted_assets")

print(f"Building authentic level using: {ASSETS_DIR}")

raw_entities = json.loads((ASSETS_DIR / "level3_entities.json").read_text(encoding="utf-8"))
extracted_trees = json.loads((ASSETS_DIR / "extracted_trees.json").read_text(encoding="utf-8"))

water_verts = []
with open(ASSETS_DIR / "meshes/GM_3D_Demo_Water.obj", encoding="utf-8") as f:
    for line in f:
        if line.startswith("v "):
            p = [float(x) for x in line.split()[1:4]]
            water_verts.append((p[0] + 38.44, p[1] + 0.57, p[2] + 156.14))

def get_water_info(px, pz):
    nearest = min(water_verts, key=lambda w: math.hypot(px - w[0], pz - w[2]))
    d_min = math.hypot(px - nearest[0], pz - nearest[2])
    # حساب مركز القناة واتجاه المجرى المحلي من الرؤوس المحيطة
    nearby = [w for w in water_verts if math.hypot(px - w[0], pz - w[2]) < 7.5]
    if len(nearby) >= 3:
        cx = sum(w[0] for w in nearby) / len(nearby)
        cz = sum(w[2] for w in nearby) / len(nearby)
    else:
        cx, cz = nearest[0], nearest[2]
    return d_min, nearest[1], cx, cz

def compute_a1_wall_yaw(px, pz, cx, cz):
    # GM_3D_ObstacleA1 يمتد طولياً على محور Z المحلي وأشواكه تتجه نحو +X المحلي
    # نحدد ما إذا كان المقطع يمتد على محور Z أو محور X
    dx = cx - px
    dz = cz - pz
    if abs(dx) > abs(dz):
        # الجدار يمتد شمال-جنوب (محور Z)، الأشواك تتجه نحو +X أو -X
        return 0.0 if dx > 0 else 180.0
    else:
        # الجدار يمتد شرق-غرب (محور X)، الأشواك تتجه نحو +Z أو -Z
        return 90.0 if dz > 0 else -90.0

def quat_to_yaw_deg(q):
    x, y, z, w = q
    siny_cosp = 2.0 * (w * y + z * x)
    cosy_cosp = 1.0 - 2.0 * (x * x + y * y)
    return math.degrees(math.atan2(siny_cosp, cosy_cosp))

parent_scales = []
for item in raw_entities:
    nm = item["name"]
    if nm.startswith("ft_animated_obstacle_") or nm.startswith("FT_"):
        parent_scales.append((item["pos"], item["scale"][0], item["rot"]))

def classify_entity(name):
    if "_Col" in name or ".Col" in name: return None
    if "CheckPoint" in name or "DirectionalArrow" in name: return None
    if name in ("GM_3D_Start", "GM_3D_Finish"): return None
    if name.startswith("ft_animated_obstacle_") or name.startswith("FT_") or name.startswith("Spike"): return None
    if name == "GM_3D_ObstacleF14": return None

    if "ObstacleA1" in name: return (0, 1.40, 1.0)
    if "ObstacleB2" in name: return (1, 2.80, 1.2)
    if "ObstacleB3" in name: return (2, 2.40, 1.6)
    if "ObstacleB5" in name: return (3, 3.60, 1.4)
    if "ObstacleC1" in name: return (4, 2.40, 1.4)
    if "ObstacleC2" in name: return (5, 2.00, 1.3)
    if "ObstacleC4" in name: return (6, 2.40, 1.6)
    if "ObstacleC5" in name: return (7, 3.60, 1.3)
    if "ObstacleF3" in name: return (8, 2.10, 1.5)
    if "ObstacleF10" in name: return (9, 3.20, 1.1)
    if "ObstacleF11" in name: return (10, 1.35, 1.8)
    if "ObstacleF13" in name: return (11, 2.60, 1.3)
    if "ObstacleF14" in name: return (12, 2.20, 1.4)
    if "ObstacleF18" in name: return (13, 0.95, 1.0)
    if "ObstacleF23" in name: return (14, 2.50, 1.4)
    if "Rock1" in name: return (15, 1.85, 0.0)
    if "Rock2" in name: return (16, 2.05, 0.0)
    if name.startswith("Gust"): return (17, 4.00, 1.0)
    return None

clean_list = []
for item in raw_entities:
    info = classify_entity(item["name"])
    if not info: continue
    etype, base_rad, spd = info
    px, py, pz = item["pos"]
    d_water, wy, cx, cz = get_water_info(px, pz)

    if etype in (15, 16) and (d_water > 22.0 or abs(py - wy) > 8.0):
        continue

    rot_q = item.get("rot", [0.0, 0.0, 0.0, 1.0])
    sc = item.get("scale", [1.0, 1.0, 1.0])[0]

    for p_pos, p_sc, p_rot in parent_scales:
        if math.hypot(px - p_pos[0], pz - p_pos[2]) < 0.25:
            if abs(sc - 1.0) < 0.05 and p_sc > 0.1:
                sc = min(p_sc, 1.65)
            if abs(rot_q[0]) + abs(rot_q[1]) + abs(rot_q[2]) < 0.01:
                rot_q = p_rot
            break

    rot_y = quat_to_yaw_deg(rot_q)
    if etype == 0:
        # محاذاة تلقائية لمصد الحائط ObstacleA1 مع جدار القناة
        rot_y = compute_a1_wall_yaw(px, pz, cx, cz)
        py = wy + 0.12
    elif etype in (2, 6, 8, 10, 13, 17):
        py = wy + 0.10
    elif etype == 7: # العجلة المسننة العمودية C5 ترتكز فوق القناة
        py = wy + 3.80
    elif etype == 4: # القوس المسنن C1
        py = wy + 3.20

    if any(e["type"] == etype and math.hypot(e["x"] - px, e["z"] - pz) < 0.40 for e in clean_list):
        continue

    clean_list.append({
        "id": len(clean_list),
        "type": etype,
        "x": round(px, 3),
        "y": round(py, 3),
        "z": round(pz, 3),
        "rotY": round(rot_y, 2),
        "scale": round(max(0.5, min(sc, 2.2)), 2),
        "speed": spd,
        "radius": round(base_rad * max(0.7, min(sc, 1.4)), 2)
    })

# إضافة الأشجار الـ 37 المستخرجة ككائنات مستقلة قابلة للتحريك وتغيير الحجم (type = 18: ENT_TREE_JUNGLE)
for tx, ty, tz, tsc in extracted_trees:
    clean_list.append({
        "id": len(clean_list),
        "type": 18,
        "x": tx,
        "y": ty,
        "z": tz,
        "rotY": round((len(clean_list) * 47) % 360, 1),
        "scale": tsc,
        "speed": 0.0,
        "radius": round(1.35 * tsc, 2)
    })

# إضافة بوابات نقاط الحفظ ككائنات قابلة للتحريك وتغيير الحجم (type = 19: ENT_CHECKPOINT)
cp_list = [
    (41.88, 0.55, 19.79, -35.0),
    (30.38, 0.55, -10.64, 90.0),
    (-49.10, 9.85, -103.86, 90.0),
    (-122.48, 8.95, -91.45, 90.0),
    (-174.31, 8.95, -58.82, 0.0),
    (-198.09, 8.95, -103.12, 90.0),
    (-336.19, 8.95, -103.23, 90.0)
]
for cpx, cpy, cpz, cprot in cp_list:
    clean_list.append({
        "id": len(clean_list),
        "type": 19,
        "x": cpx,
        "y": cpy,
        "z": cpz,
        "rotY": cprot,
        "scale": 1.0,
        "speed": 0.0,
        "radius": 3.50
    })

json_str = json.dumps(clean_list, indent=2)
pathlib.Path("level_layout.json").write_text(json_str, encoding="utf-8")
pathlib.Path("CustomEngine/level_layout.json").write_text(json_str, encoding="utf-8")
print(f"Generated complete editable level_layout.json with {len(clean_list)} entities (obstacles, rocks, trees, checkpoints).")
