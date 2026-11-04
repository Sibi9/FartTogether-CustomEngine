#!/usr/bin/env python3
"""
check_pirate_placement.py
Validates pirate placement on the boat ring according to physical mesh measurements.
Uses PIL to render 2D projection diagrams (Top-down X-Z and Radial-Height R-Y).
"""

import os
import sys
import math
import argparse
from PIL import Image, ImageDraw, ImageFont

def load_obj_vertices(path):
    if not os.path.exists(path):
        raise FileNotFoundError(f"OBJ file not found: {path}")
    verts = []
    with open(path, 'r', encoding='utf-8', errors='ignore') as f:
        for line in f:
            if line.startswith('v '):
                parts = line.split()
                verts.append([float(parts[1]), float(parts[2]), float(parts[3])])
    return verts

def get_boat_surface_y(r):
    # Cushion surface: r in [0.0..1.06] is y ≈ 0.5176, then slopes down to y ≈ 0.34 at r = 1.209
    if r < 1.06:
        return 0.5176
    elif r <= 1.21:
        t = (r - 1.06) / (1.209 - 1.06)
        return 0.5176 - t * (0.5176 - 0.3400)
    else:
        return 0.3400

def check_placement(obj_dir, R, H, angle_deg, boat_yaw_deg=0.0, lean_deg=0.0, scale=1.0, out_png=None, pirate_model=None):
    # Search paths for boat and pirate models
    boat_candidates = [
        os.path.join(obj_dir, 'Boat', 'BoatMain_ThreeP.obj'),
        os.path.join(obj_dir, 'BoatMain_ThreeP.obj'),
        os.path.join('CustomEngine', 'extracted_assets', 'meshes', 'GM_3D_BoatMain_ThreeP.obj'),
        'Models_Export/Boat/BoatMain_ThreeP.obj'
    ]
    boat_path = next((p for p in boat_candidates if os.path.exists(p)), None)
    if not boat_path:
        raise FileNotFoundError("BoatMain_ThreeP.obj not found in search paths")

    pirate_candidates = []
    if pirate_model:
        pirate_candidates.extend([
            pirate_model,
            os.path.join(obj_dir, 'Pirates', pirate_model),
            os.path.join(obj_dir, pirate_model),
            os.path.join('Models_Export', 'Pirates', pirate_model),
            os.path.join('CustomEngine', 'extracted_assets', 'meshes', pirate_model)
        ])
    pirate_candidates.extend([
        os.path.join(obj_dir, 'Pirates', 'Pirate_SeatedA.obj'),
        os.path.join(obj_dir, 'Pirate_SeatedA.obj'),
        os.path.join('CustomEngine', 'extracted_assets', 'meshes', 'GM_3D_Character_SeatedA.obj'),
        'Models_Export/Pirates/Pirate_SeatedA.obj'
    ])
    pirate_path = next((p for p in pirate_candidates if os.path.exists(p)), None)
    if not pirate_path:
        raise FileNotFoundError("Pirate_SeatedA.obj not found in search paths")

    boat_verts = load_obj_vertices(boat_path)
    pirate_verts = load_obj_vertices(pirate_path)

    # Boat geometry dimensions
    boat_rs = [math.sqrt(v[0]**2 + v[2]**2) for v in boat_verts]
    hull_outer_r = max(boat_rs) # ~1.2085
    boat_diam = hull_outer_r * 2.0

    pirate_ys = [v[1] for v in pirate_verts]
    pirate_height = (max(pirate_ys) - min(pirate_ys)) * scale
    height_to_diam_ratio = pirate_height / boat_diam

    # Extract boat handle/post vertices (high Y posts)
    handle_verts = []
    cos_by = math.cos(math.radians(boat_yaw_deg))
    sin_by = math.sin(math.radians(boat_yaw_deg))
    for v in boat_verts:
        if v[1] >= 0.55:
            r = math.sqrt(v[0]**2 + v[2]**2)
            if 0.88 <= r <= 1.20:
                rx = v[0] * cos_by + v[2] * sin_by
                rz = -v[0] * sin_by + v[2] * cos_by
                handle_verts.append([rx, v[1], rz])

    # Place pirate
    angle_rad = math.radians(angle_deg)
    yaw_rad = angle_rad + math.pi
    cos_yaw = math.cos(yaw_rad)
    sin_yaw = math.sin(yaw_rad)

    lean_rad = math.radians(lean_deg)
    cos_lean = math.cos(lean_rad)
    sin_lean = math.sin(lean_rad)

    placed_pirate = []
    feet_indices = []
    butt_indices = []

    seat_x = R * math.sin(angle_rad)
    seat_z = R * math.cos(angle_rad)

    for i, v in enumerate(pirate_verts):
        vx = v[0] * scale
        vy = v[1] * scale
        vz = v[2] * scale

        if v[1] < -0.02 + 0.045 and v[2] < 0.10:
            feet_indices.append(i)
        if v[2] < -0.20 and 0.15 < v[1] < 0.45:
            butt_indices.append(i)

        # Lean around hip (0, 0.34, 0)
        dy = vy - 0.34 * scale
        dz = vz
        ly = dy * cos_lean - dz * sin_lean
        lz = dy * sin_lean + dz * cos_lean
        vy_leaned = ly + 0.34 * scale
        vz_leaned = lz

        # Rotate Yaw (angle + 180) around Y
        wx = vx * cos_yaw + vz_leaned * sin_yaw
        wz = -vx * sin_yaw + vz_leaned * cos_yaw
        wy = vy_leaned

        # Translate to seat
        px = seat_x + wx
        py = H + wy
        pz = seat_z + wz
        placed_pirate.append([px, py, pz])

    # 1. feet_vs_rim metric
    feet_rs = [math.sqrt(placed_pirate[fi][0]**2 + placed_pirate[fi][2]**2) for fi in feet_indices]
    feet_ys = [placed_pirate[fi][1] for fi in feet_indices]
    avg_feet_r = sum(feet_rs) / len(feet_rs)
    max_feet_r = max(feet_rs)
    cushion_target_r = 1.10
    feet_vs_rim = avg_feet_r - cushion_target_r
    feet_outside_hull = max_feet_r > hull_outer_r

    # 2. butt_overhang_beyond_hull metric
    butt_rs = [math.sqrt(placed_pirate[bi][0]**2 + placed_pirate[bi][2]**2) for bi in butt_indices]
    max_butt_r = max(butt_rs)
    butt_overhang = max_butt_r - hull_outer_r

    # 3. handle_hit_vertices metric
    handle_hits = 0
    for pv in placed_pirate:
        pr = math.sqrt(pv[0]**2 + pv[2]**2)
        if 0.88 <= pr <= 1.20 and pv[1] >= 0.50:
            for hv in handle_verts:
                dist = math.sqrt((pv[0]-hv[0])**2 + (pv[1]-hv[1])**2 + (pv[2]-hv[2])**2)
                if dist < 0.075:
                    handle_hits += 1
                    break

    # 4. buried_vertices_pct metric
    buried_count = 0
    for pv in placed_pirate:
        pr = math.sqrt(pv[0]**2 + pv[2]**2)
        surf_y = get_boat_surface_y(pr)
        if pv[1] < surf_y - 0.02:
            buried_count += 1
    buried_pct = (buried_count / len(placed_pirate)) * 100.0

    # 5. Hand clearance above floor
    hand_pts = [placed_pirate[i] for i, v in enumerate(pirate_verts) if v[1] < 0.25 and v[2] > 0.15]
    min_hand_y = min(p[1] for p in hand_pts) if hand_pts else H
    floor_y = 0.5176
    hand_clearance = min_hand_y - floor_y

    feet_ok = abs(feet_vs_rim) <= 0.03 and not feet_outside_hull
    butt_ok = 0.04 <= butt_overhang <= 0.20
    handle_ok = (handle_hits == 0)
    buried_ok = (buried_pct < 1.0)
    ratio_ok = (0.40 <= height_to_diam_ratio <= 0.47) if (not pirate_model or "Grip" not in pirate_model) else (0.35 <= height_to_diam_ratio <= 0.47)

    passed = feet_ok and butt_ok and handle_ok and buried_ok and ratio_ok

    print(f"=== Placement Check (Angle={angle_deg}°, R={R:.2f}, H={H:.2f}, BoatYaw={boat_yaw_deg}°) ===")
    print(f"  Result: {'PASS' if passed else 'FAIL'}")
    print(f"  feet_vs_rim:                 {feet_vs_rim:+.4f} m (avg_r={avg_feet_r:.3f}, max_r={max_feet_r:.3f}, hull_r={hull_outer_r:.3f}) [{'OK' if feet_ok else 'FAIL'}]")
    print(f"  butt_overhang_beyond_hull:   {butt_overhang:+.4f} m (max_r={max_butt_r:.3f}) [{'OK' if butt_ok else 'FAIL'}]")
    print(f"  handle_hit_vertices:         {handle_hits} [{'OK' if handle_ok else 'FAIL'}]")
    print(f"  buried_vertices_pct:         {buried_pct:.2f}% ({buried_count}/{len(placed_pirate)}) [{'OK' if buried_ok else 'FAIL'}]")
    print(f"  pirate_height/boat_diam:     {height_to_diam_ratio:.4f} (h={pirate_height:.3f}, d={boat_diam:.3f}) [{'OK' if ratio_ok else 'FAIL'}]")
    print(f"  min_hand_clearance:          {hand_clearance:+.4f} m (min_y={min_hand_y:.3f}, floor_y={floor_y:.3f})")

    if out_png:
        try:
            w, h = 1200, 600
            img = Image.new('RGB', (w, h), color=(24, 28, 38))
            draw = ImageDraw.Draw(img)

            # Left Panel: Top-down (X-Z)
            cx, cy = 300, 300
            scale_px = 180.0

            def world_to_top(wx, wz):
                return (int(cx + wx * scale_px), int(cy - wz * scale_px))

            r_hull_px = hull_outer_r * scale_px
            draw.ellipse([cx - r_hull_px, cy - r_hull_px, cx + r_hull_px, cy + r_hull_px], outline=(70, 130, 240), width=2)
            r_inn_px = 0.87 * scale_px
            draw.ellipse([cx - r_inn_px, cy - r_inn_px, cx + r_inn_px, cy + r_inn_px], outline=(60, 180, 200), width=1)
            r_tgt_px = 1.10 * scale_px
            draw.ellipse([cx - r_tgt_px, cy - r_tgt_px, cx + r_tgt_px, cy + r_tgt_px], outline=(220, 200, 70), width=1)

            for hv in handle_verts[::2]:
                hpt = world_to_top(hv[0], hv[2])
                draw.rectangle([hpt[0]-2, hpt[1]-2, hpt[0]+2, hpt[1]+2], fill=(240, 70, 70))

            for pv in placed_pirate:
                ppt = world_to_top(pv[0], pv[2])
                draw.point(ppt, fill=(100, 220, 120))

            for fi in feet_indices:
                fpt = world_to_top(placed_pirate[fi][0], placed_pirate[fi][2])
                draw.rectangle([fpt[0]-2, fpt[1]-2, fpt[0]+2, fpt[1]+2], fill=(255, 60, 220))

            for bi in butt_indices:
                bpt = world_to_top(placed_pirate[bi][0], placed_pirate[bi][2])
                draw.rectangle([bpt[0]-2, bpt[1]-2, bpt[0]+2, bpt[1]+2], fill=(255, 165, 30))

            draw.text((20, 20), f"Top View (Angle={angle_deg} deg, R={R:.2f}, H={H:.2f})", fill=(255, 255, 255))
            draw.text((20, 40), f"Blue=Hull Outer ({hull_outer_r:.2f}m), Yellow=Cushion ({cushion_target_r}m)", fill=(180, 200, 220))
            draw.text((20, 60), f"Red=Handles, Green=Pirate, Magenta=Feet, Orange=Butt", fill=(180, 200, 220))

            # Right Panel: Radial-Height Profile
            rx0, ry0 = 660, 520
            r_sc, y_sc = 360.0, 360.0

            def rad_to_screen(r, y):
                return (int(rx0 + (r - 0.70) * r_sc), int(ry0 - y * y_sc))

            draw.line([rx0, ry0, rx0 + 500, ry0], fill=(120, 130, 150), width=1)
            draw.line([rx0, ry0, rx0, ry0 - 450], fill=(120, 130, 150), width=1)

            prof_pts = []
            for k in range(55):
                cr = 0.75 + k * 0.01
                cy_val = get_boat_surface_y(cr)
                prof_pts.append(rad_to_screen(cr, cy_val))
            draw.line(prof_pts, fill=(0, 230, 255), width=2)

            hull_scr_x = int(rx0 + (hull_outer_r - 0.70) * r_sc)
            draw.line([hull_scr_x, ry0, hull_scr_x, ry0 - 450], fill=(70, 130, 240), width=2)

            for pv in placed_pirate:
                pr = math.sqrt(pv[0]**2 + pv[2]**2)
                spt = rad_to_screen(pr, pv[1])
                draw.point(spt, fill=(100, 220, 120))

            for fi in feet_indices:
                pr = math.sqrt(placed_pirate[fi][0]**2 + placed_pirate[fi][2]**2)
                fpt = rad_to_screen(pr, placed_pirate[fi][1])
                draw.rectangle([fpt[0]-2, fpt[1]-2, fpt[0]+2, fpt[1]+2], fill=(255, 60, 220))

            for bi in butt_indices:
                pr = math.sqrt(placed_pirate[bi][0]**2 + placed_pirate[bi][2]**2)
                bpt = rad_to_screen(pr, placed_pirate[bi][1])
                draw.rectangle([bpt[0]-2, bpt[1]-2, bpt[0]+2, bpt[1]+2], fill=(255, 165, 30))

            # Status Banner
            status_color = (60, 220, 100) if passed else (240, 60, 60)
            draw.rectangle([660, 20, 1170, 110], outline=status_color, width=2)
            draw.text((675, 26), f"STATUS: {'PASS' if passed else 'FAIL'}", fill=status_color)
            draw.text((675, 46), f"feet_vs_rim: {feet_vs_rim:+.4f}m (OK: +-0.03m)", fill=(220, 220, 220))
            draw.text((675, 64), f"butt_overhang: {butt_overhang:+.4f}m (OK: 0.04..0.20m, target ~0.10m)", fill=(220, 220, 220))
            draw.text((675, 82), f"handle_hits: {handle_hits} | buried_pct: {buried_pct:.2f}% | h/d: {height_to_diam_ratio:.3f}", fill=(220, 220, 220))

            os.makedirs(os.path.dirname(os.path.abspath(out_png)), exist_ok=True)
            img.save(out_png)
            print(f"  Saved visualization to: {out_png}")
        except Exception as e:
            print(f"  Warning: could not save PNG ({e})")

    return passed

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="Check pirate placement on boat ring.")
    parser.add_argument('--dir', default='Models_Export', help='Directory with OBJ models')
    parser.add_argument('--R', type=float, default=0.98, help='Seat radius')
    parser.add_argument('--H', type=float, default=0.51, help='Seat height')
    parser.add_argument('--angle', type=float, default=0.0, help='Seat angle in degrees')
    parser.add_argument('--boat-yaw', type=float, default=0.0, help='Boat mesh yaw in degrees')
    parser.add_argument('--lean', type=float, default=0.0, help='Pirate lean in degrees')
    parser.add_argument('--scale', type=float, default=1.0, help='Pirate scale')
    parser.add_argument('--model', default=None, help='Custom pirate OBJ filename/path')
    parser.add_argument('--out', default=None, help='Output PNG file path')

    args = parser.parse_args()
    ok = check_placement(args.dir, args.R, args.H, args.angle, args.boat_yaw, args.lean, args.scale, args.out, args.model)
    sys.exit(0 if ok else 1)
