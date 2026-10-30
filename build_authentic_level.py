import json, math, pathlib, os

# Determine base dir
if os.path.exists("CustomEngine/extracted_assets"):
    asset_base = pathlib.Path("CustomEngine/extracted_assets")
else:
    asset_base = pathlib.Path("extracted_assets")

raw_entities = json.loads((asset_base / "level3_entities.json").read_text(encoding="utf-8"))

# قراءة رؤوس مجسم الماء الحقيقي مع trackOffset=(38.44, 0.57, 156.14)
water_verts = []
with open(asset_base / "meshes/GM_3D_Demo_Water.obj", encoding="utf-8") as f:
    for line in f:
        if line.startswith("v "):
            parts = [float(x) for x in line.split()[1:4]]
            water_verts.append((parts[0] + 38.44, parts[1] + 0.57, parts[2] + 156.14))

def closest_water(px, pz):
    best_d = 1e9
    best_y = 0.55
    for wx, wy, wz in water_verts:
        d = math.hypot(px - wx, pz - wz)
        if d < best_d:
            best_d = d
            best_y = wy
    return best_d, best_y

def quat_to_yaw_deg(q):
    x, y, z, w = q
    siny_cosp = 2.0 * (w * y + z * x)
    cosy_cosp = 1.0 - 2.0 * (x * x + y * y)
    return math.degrees(math.atan2(siny_cosp, cosy_cosp))

# جمع مقاييس الآباء (مثل ft_animated_obstacle_f11_*) لتوريثها للأبناء إن لزم
parent_scales = []
for item in raw_entities:
    nm = item["name"]
    if nm.startswith("ft_animated_obstacle_") or nm.startswith("FT_"):
        parent_scales.append((item["pos"], item["scale"][0], item["rot"]))

def classify_entity(name):
    # تجاهل مجسمات التصادم، نقاط الحفظ، وعقد الآباء الفارغة
    if "_Col" in name or ".Col" in name: return None
    if "CheckPoint" in name or "DirectionalArrow" in name: return None
    if name in ("GM_3D_Start", "GM_3D_Finish"): return None
    if name.startswith("ft_animated_obstacle_") or name.startswith("FT_") or name.startswith("Spike"): return None
    if name == "GM_3D_ObstacleF14": return None # الأب لـ F14_P1 و F14_P2

    if "ObstacleA1" in name: return (0, 1.85, 1.0)
    if "ObstacleB2" in name: return (1, 3.20, 1.2)
    if "ObstacleB3" in name: return (2, 2.50, 1.5)
    if "ObstacleB5" in name: return (3, 3.80, 1.4)
    if "ObstacleC1" in name: return (4, 2.50, 1.3)
    if "ObstacleC2" in name: return (5, 2.20, 1.2)
    if "ObstacleC4" in name: return (6, 2.50, 1.5)
    if "ObstacleC5" in name: return (7, 3.80, 1.2)
    if "ObstacleF3" in name: return (8, 2.20, 1.5)
    if "ObstacleF10" in name: return (9, 3.50, 1.0)
    if "ObstacleF11" in name: return (10, 1.65, 1.8)
    if "ObstacleF13" in name: return (11, 2.80, 1.2)
    if "ObstacleF14" in name: return (12, 2.40, 1.2)
    if "ObstacleF18" in name: return (13, 1.20, 1.0)
    if "ObstacleF23" in name: return (14, 2.80, 1.4)
    if "Rock1" in name: return (15, 2.10, 0.0)
    if "Rock2" in name: return (16, 2.30, 0.0)
    if name.startswith("Gust"): return (17, 4.50, 1.0)
    return None

clean_list = []
for item in raw_entities:
    info = classify_entity(item["name"])
    if not info:
        continue
    etype, base_rad, spd = info
    px, py, pz = item["pos"]
    d_water, wy = closest_water(px, pz)

    # استبعاد الصخور البعيدة عن المجرى المائي أو الغائرة تحت الوادي
    if etype in (15, 16) and (d_water > 14.0 or abs(py - wy) > 5.0):
        continue

    rot_q = item.get("rot", [0.0, 0.0, 0.0, 1.0])
    sc = item.get("scale", [1.0, 1.0, 1.0])[0]

    # مطابقة مع الأب إن وجد في نفس الموقع
    for p_pos, p_sc, p_rot in parent_scales:
        if math.hypot(px - p_pos[0], pz - p_pos[2]) < 0.25:
            if abs(sc - 1.0) < 0.05 and p_sc > 0.1:
                sc = min(p_sc, 1.85) # سقف منطقي لحجم الأسطوانة المسننة
            if abs(rot_q[0]) + abs(rot_q[1]) + abs(rot_q[2]) < 0.01:
                rot_q = p_rot
            break

    rot_y = quat_to_yaw_deg(rot_q)

    # منع التكرار التام في نفس النقطة (مثل الـ 18 نسخة المزدوجة من F11)
    if any(e["type"] == etype and math.hypot(e["x"] - px, e["z"] - pz) < 0.35 for e in clean_list):
        continue

    # ضبط ارتفاع العوائق العائمة لتكون متناسقة مع سطح الماء الفعلي
    if etype in (0, 2, 8, 10, 13, 17):
        py = wy + 0.15

    clean_list.append({
        "id": len(clean_list),
        "type": etype,
        "x": round(px, 3),
        "y": round(py, 3),
        "z": round(pz, 3),
        "rotY": round(rot_y, 2),
        "scale": round(max(0.5, min(sc, 2.2)), 2),
        "speed": spd,
        "radius": round(base_rad * max(0.7, min(sc, 1.5)), 2)
    })

out_json = json.dumps(clean_list, indent=2)
pathlib.Path("level_layout.json").write_text(out_json, encoding="utf-8")
if os.path.exists("CustomEngine"):
    pathlib.Path("CustomEngine/level_layout.json").write_text(out_json, encoding="utf-8")
print(f"Successfully generated authentic level_layout.json with {len(clean_list)} verified entities.")

