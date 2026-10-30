import json, math, os, pathlib
from PIL import Image, ImageDraw

ASSETS_DIR = pathlib.Path("CustomEngine/extracted_assets")
if not ASSETS_DIR.exists():
    ASSETS_DIR = pathlib.Path("extracted_assets")

print(f"Using assets directory: {ASSETS_DIR}")

# 1. توليد سحابة الغاز الأخضر بالجمجمة الكرتونية (SkullGasCloud.png)
def create_skull_gas_texture():
    tex_dir = ASSETS_DIR / "textures"
    tex_dir.mkdir(parents=True, exist_ok=True)
    img = Image.new("RGBA", (256, 256), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    # سحابة غاز خضراء كرتونية متعددة الفصوص مع إطار داكن
    blobs = [(128, 130, 82), (75, 140, 56), (180, 140, 56), (95, 85, 52), (160, 85, 52), (128, 175, 54)]
    for bx, by, br in blobs:
        draw.ellipse((bx - br - 5, by - br - 5, bx + br + 5, by + br + 5), fill=(35, 110, 20, 235))
    for bx, by, br in blobs:
        draw.ellipse((bx - br, by - br, bx + br, by + br), fill=(118, 245, 50, 245))
    # إضاءة داخلية صفراء-خضراء
    draw.ellipse((70, 65, 186, 175), fill=(165, 255, 85, 250))
    # رسم جمجمة كرتونية بيضاء/خضراء فاتحة في قلب السحابة
    draw.ellipse((86, 62, 170, 142), fill=(245, 255, 235, 255), outline=(25, 65, 15, 255), width=4)
    draw.rounded_rectangle((104, 122, 152, 162), radius=8, fill=(245, 255, 235, 255), outline=(25, 65, 15, 255), width=4)
    # تجويف العينين والأنف والأسنان
    draw.ellipse((98, 88, 120, 112), fill=(25, 55, 15, 255))
    draw.ellipse((136, 88, 158, 112), fill=(25, 55, 15, 255))
    draw.polygon([(128, 112), (121, 126), (135, 126)], fill=(25, 55, 15, 255))
    for tx in (116, 128, 140):
        draw.line([(tx, 138), (tx, 158)], fill=(25, 55, 15, 255), width=3)
    out_path = tex_dir / "SkullGasCloud.png"
    img.save(out_path)
    print(f"Generated {out_path}")

# 2. تفتيح قوام Fart1..8.png حتى لا يظهر بمركز أسود قاتم
def brighten_fart_textures():
    for fn in ["Fart1.png", "Fart4.png", "Fart7.png", "Fart8.png"]:
        p = ASSETS_DIR / "textures" / fn
        if not p.exists(): continue
        im = Image.open(p).convert("RGBA")
        px = im.load()
        w, h = im.size
        for y in range(h):
            for x in range(w):
                r, g, b, a = px[x, y]
                if a > 5:
                    # رفع السطوع بحيث يصبح المركز (180..255) ويتلون بالأخضر الزاهي
                    lum = int(175 + (r / 255.0) * 80)
                    px[x, y] = (lum, lum, lum, a)
        im.save(p)
    print("Brightened Fart1..8.png textures for neon green tinting.")

# 3. بناء مجسمات الجلوس الطبيعية للقراصنة بدون تشوه الأذرع وبدون بقعة الظهر
def build_seated_pirate(src_obj, dst_obj, skin_u, skin_v):
    vs, vts, faces = [], [], []
    with open(src_obj, encoding="utf-8") as f:
        for line in f:
            if line.startswith("v "):
                vs.append([float(x) for x in line.split()[1:4]])
            elif line.startswith("vt "):
                u, v = [float(x) for x in line.split()[1:3]]
                if 0.42 < u < 0.50:
                    u, v = skin_u, skin_v
                vts.append((u, v))
            elif line.startswith("f "):
                faces.append(line.strip())

    new_vs = []
    for x, y, z in vs:
        nx, ny, nz = x, y, z
        # أ. دوران الذراعين حول مفصل الكتف (±0.20, 0.60, 0.0) بزاوية طبيعية
        if abs(x) > 0.20 and 0.36 < y < 0.78:
            sgn = 1.0 if x > 0 else -1.0
            sx = sgn * 0.20
            sy = 0.60
            dx = x - sx
            dy = y - sy
            dist = min(1.0, abs(dx) / 0.25)
            ang = -1.05 * dist * sgn # تدوير الذراع لأسفل
            ca, sa = math.cos(ang), math.sin(ang)
            rx = dx * ca - dy * sa
            ry = dx * sa + dy * ca
            nx = sx + rx * 0.88
            ny = sy + ry
            nz = z + abs(dx) * 0.38 # تقديم اليدين للأمام على حافة القارب
        # ب. ثني الساقين عند الورك (y < 0.36) للجلوس الطبيعي
        elif y < 0.36:
            hip_y = 0.36
            t = max(0.0, min(1.0, (hip_y - y) / 0.36))
            ny = 0.22 + (1.0 - t) * 0.14
            nz = z + t * 0.24
        new_vs.append([nx, ny, nz])

    # إعادة حساب المتجهات العمودية السلسة (Smooth Vertex Normals) لمنع السواد
    vns = [[0.0, 0.0, 0.0] for _ in new_vs]
    parsed_faces = []
    for fline in faces:
        toks = fline.split()[1:]
        f_idx = []
        for t in toks:
            parts = t.split("/")
            vi = int(parts[0]) - 1
            ti = int(parts[1]) - 1 if len(parts) > 1 and parts[1] else vi
            f_idx.append((vi, ti))
        parsed_faces.append(f_idx)
        if len(f_idx) >= 3:
            p0, p1, p2 = new_vs[f_idx[0][0]], new_vs[f_idx[1][0]], new_vs[f_idx[2][0]]
            ux, uy, uz = p1[0]-p0[0], p1[1]-p0[1], p1[2]-p0[2]
            vx, vy, vz = p2[0]-p0[0], p2[1]-p0[1], p2[2]-p0[2]
            fnx = uy*vz - uz*vy
            fny = uz*vx - ux*vz
            fnz = ux*vy - uy*vx
            for vi, _ in f_idx:
                vns[vi][0] += fnx
                vns[vi][1] += fny
                vns[vi][2] += fnz

    out_lines = ["# Authentic Seated Pirate Mesh with Smooth Normals"]
    for vx, vy, vz in new_vs:
        out_lines.append(f"v {vx:.5f} {vy:.5f} {vz:.5f}")
    for u, v in vts:
        out_lines.append(f"vt {u:.5f} {v:.5f}")
    for nx, ny, nz in vns:
        l = math.sqrt(nx*nx + ny*ny + nz*nz) or 1.0
        out_lines.append(f"vn {nx/l:.5f} {ny/l:.5f} {nz/l:.5f}")
    for f_idx in parsed_faces:
        ftoks = [f"{vi+1}/{ti+1}/{vi+1}" for vi, ti in f_idx]
        out_lines.append("f " + " ".join(ftoks))
    pathlib.Path(dst_obj).write_text("\n".join(out_lines) + "\n", encoding="utf-8")
    print(f"Rebuilt clean seated character mesh: {dst_obj}")

# 4. استخراج الأشجار الـ 37 من قطاعات البيئة وحذف مثلثاتها المدمجة لتصبح قابلة للتحريك في المحرر
def extract_and_strip_scene_trees():
    extracted_trees = []
    for s in range(2, 16):
        fname = ASSETS_DIR / f"meshes/Combined_Mesh_root__scene_{s}.obj"
        if not fname.exists(): continue
        # الاحتفاظ بنسخة احتياطية أصلية مرة واحدة
        bak = fname.with_suffix(".obj.orig")
        if not bak.exists():
            bak.write_bytes(fname.read_bytes())
        lines = bak.read_text(encoding="utf-8").splitlines()
        vs, vts = [], []
        for line in lines:
            if line.startswith("v "): vs.append([float(x) for x in line.split()[1:4]])
            elif line.startswith("vt "): vts.append([float(x) for x in line.split()[1:3]])

        kept_lines = []
        trunk_pts = []
        for line in lines:
            if line.startswith("f "):
                toks = line.split()[1:]
                is_tree = False
                for tok in toks:
                    parts = tok.split("/")
                    vi = int(parts[0]) - 1
                    ti = int(parts[1]) - 1 if len(parts) > 1 and parts[1] else vi
                    if ti < len(vts):
                        u, v = vts[ti]
                        if 0.52 <= u <= 0.56 and 0.62 <= v <= 0.66:
                            if vi < len(vs):
                                trunk_pts.append(vs[vi])
                            is_tree = True
                        elif 0.61 <= u <= 0.65 and 0.50 <= v <= 0.63:
                            is_tree = True
                if not is_tree:
                    kept_lines.append(line)
            else:
                kept_lines.append(line)
        fname.write_text("\n".join(kept_lines) + "\n", encoding="utf-8")

        # تجميع مواقع الأشجار المستخرجة
        clusters = []
        for pt in trunk_pts:
            placed = False
            for c in clusters:
                if math.hypot(pt[0] - c[0][0], pt[2] - c[0][2]) < 3.5:
                    c.append(pt)
                    placed = True
                    break
            if not placed:
                clusters.append([pt])
        for c in clusters:
            cx = sum(p[0] for p in c) / len(c)
            cy = min(p[1] for p in c)
            cz = sum(p[2] for p in c) / len(c)
            h = max(p[1] for p in c) - cy
            if not any(math.hypot(cx - tx, cz - tz) < 3.0 for tx, ty, tz, _ in extracted_trees):
                extracted_trees.append((round(cx, 3), round(cy, 3), round(cz, 3), round(max(0.75, min(1.45, h / 2.6)), 2)))

    out_trees = ASSETS_DIR / "extracted_trees.json"
    out_trees.write_text(json.dumps(extracted_trees, indent=2), encoding="utf-8")
    print(f"Extracted {len(extracted_trees)} movable trees and stripped static tree faces from scene_2..15 to {out_trees}.")

if __name__ == "__main__":
    create_skull_gas_texture()
    brighten_fart_textures()
    # P1 = Green Hippo (MainB), P2 = Orange/Tan Hippo (MainD)
    build_seated_pirate(ASSETS_DIR / "meshes/GM_3D_Character_MainB.obj", ASSETS_DIR / "meshes/GM_3D_Character_SeatedB.obj", 0.320, 0.265)
    build_seated_pirate(ASSETS_DIR / "meshes/GM_3D_Character_MainD.obj", ASSETS_DIR / "meshes/GM_3D_Character_SeatedA.obj", 0.198, 0.902)
    extract_and_strip_scene_trees()

