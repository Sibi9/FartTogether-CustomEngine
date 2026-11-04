import sys, os, math
sys.path.insert(0, os.path.abspath('.'))
import numpy as np
from check_pirate_placement import load_obj_vertices

verts = np.array(load_obj_vertices('Models_Export/Pirates/Pirate_SeatedA_Grip.obj'))

R = 0.98
H = 0.51
scale = 1.0

def place_pirate(verts, angle_deg, lean_deg=0.0):
    angle_rad = math.radians(angle_deg)
    yaw_rad = angle_rad + math.pi
    cos_yaw = math.cos(yaw_rad); sin_yaw = math.sin(yaw_rad)
    cos_lean = math.cos(math.radians(lean_deg)); sin_lean = math.sin(math.radians(lean_deg))
    seat_x = R * math.sin(angle_rad)
    seat_z = R * math.cos(angle_rad)
    
    placed = []
    for v in verts:
        dy = v[1] - 0.34
        dz = v[2]
        ly = dy * cos_lean - dz * sin_lean
        lz = dy * sin_lean + dz * cos_lean
        vy_leaned = ly + 0.34
        vz_leaned = lz
        
        wx = v[0] * cos_yaw + vz_leaned * sin_yaw
        wz = -v[0] * sin_yaw + vz_leaned * cos_yaw
        wy = vy_leaned
        
        px = seat_x + wx
        py = H + wy
        pz = seat_z + wz
        placed.append([px, py, pz])
    return np.array(placed)

p0 = place_pirate(verts, 0.0, 0.0)

# 4. Floor test: lowest vertex with r < 0.85
rs = np.sqrt(p0[:, 0]**2 + p0[:, 2]**2)
over_floor_m = (rs < 0.85)
min_y_over_floor = float(np.min(p0[over_floor_m, 1]))
pass_d4 = 0.515 <= min_y_over_floor < 0.545
print(f"Test d.4: Lowest vertex with r < 0.85: {min_y_over_floor:.4f} m (required: >= 0.515 and < 0.545) -> {'PASS' if pass_d4 else 'FAIL'}")

# 6. Head test: lowest vertex of head over floor with clearance >= 0.15
orig_verts = np.array(load_obj_vertices('Models_Export/Pirates/Pirate_SeatedA.obj'))
head_idx = [i for i, v in enumerate(orig_verts) if v[1] > 0.80]
head_pts = p0[head_idx]
min_head_y = float(np.min(head_pts[:, 1]))
floor_y = 0.5176
head_clearance = min_head_y - floor_y
pass_d6 = head_clearance >= 0.15
print(f"Test d.6: Head min Y: {min_head_y:.4f} m, floor Y: {floor_y:.4f} m, clearance: {head_clearance:.4f} m (required: >= 0.15) -> {'PASS' if pass_d6 else 'FAIL'}")

# 5. Four pirates together: closest distance between heads of two different pirates
angles = [0.0, 90.0, 180.0, 270.0]
pirates_4 = [place_pirate(verts, a, 0.0) for a in angles]
heads_4 = [p[head_idx] for p in pirates_4]

min_head_dist = 1e9
pair_info = None
for i in range(4):
    for j in range(i + 1, 4):
        h_i = heads_4[i]
        h_j = heads_4[j]
        diff = h_i[:, None, :] - h_j[None, :, :]
        dist_matrix = np.sqrt(np.sum(diff**2, axis=-1))
        d = float(np.min(dist_matrix))
        if d < min_head_dist:
            min_head_dist = d
            pair_info = (angles[i], angles[j])

pass_d5 = min_head_dist >= 0.15
print(f"Test d.5: Closest distance between heads of 4 pirates (angles {pair_info[0]} and {pair_info[1]}): {min_head_dist:.4f} m (required: >= 0.15, ref 0.291) -> {'PASS' if pass_d5 else 'FAIL'}")

# w.3 Lean 6 deg check: feet slide
feet_idx = [i for i, v in enumerate(verts) if v[1] < 0.025 and v[2] < 0.10]
p0_lean0 = place_pirate(verts, 0.0, 0.0)
p0_lean6 = place_pirate(verts, 0.0, 6.0)
feet_disp = np.linalg.norm(p0_lean6[feet_idx] - p0_lean0[feet_idx], axis=-1)
max_feet_slide = float(np.max(feet_disp))
pass_w3 = max_feet_slide <= 0.03
print(f"Check w.3: Max feet slide at lean=6 deg: {max_feet_slide:.4f} m (required: <= 0.03 m) -> {'PASS' if pass_w3 else 'FAIL'}")
