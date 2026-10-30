import math

def pose_obj(in_path, out_path, is_shirt=False):
    with open(in_path, 'r', encoding='utf-8') as f:
        lines = f.readlines()
    
    out_lines = []
    for line in lines:
        if line.startswith('v '):
            parts = line.split()
            x = float(parts[1])
            y = float(parts[2])
            z = float(parts[3])
            
            # Left arm transform (x < -0.18, y > 0.35)
            if x < -0.18 and y > 0.35:
                sx, sy, sz = -0.20, 0.52, 0.05
                dx = x - sx
                dy = y - sy
                dz = z - sz
                
                w = min(1.0, max(0.0, (-x - 0.18) / 0.15))
                
                # Rotate around Z (downward) by angle -72 deg (hands rest down on tube)
                angle_z = -72.0 * (math.pi / 180.0) * w
                # Rotate around Y (forward into +Z) by angle 28 deg
                angle_y = 28.0 * (math.pi / 180.0) * w
                
                x1 = dx * math.cos(angle_z) - dy * math.sin(angle_z)
                y1 = dx * math.sin(angle_z) + dy * math.cos(angle_z)
                z1 = dz
                
                x2 = x1 * math.cos(angle_y) + z1 * math.sin(angle_y)
                y2 = y1
                z2 = -x1 * math.sin(angle_y) + z1 * math.cos(angle_y)
                
                # Forearm and hand forward onto the tube
                if abs(dx) > 0.18:
                    fw = min(1.0, (abs(dx) - 0.18) / 0.22)
                    z2 += 0.14 * fw
                    y2 -= 0.05 * fw
                    x2 += 0.04 * fw # bring hands inward toward center
                
                x = sx + x2
                y = sy + y2
                z = sz + z2
                
            # Right arm transform (x > 0.18, y > 0.35)
            elif x > 0.18 and y > 0.35:
                sx, sy, sz = 0.20, 0.52, 0.05
                dx = x - sx
                dy = y - sy
                dz = z - sz
                
                w = min(1.0, max(0.0, (x - 0.18) / 0.15))
                
                angle_z = 72.0 * (math.pi / 180.0) * w
                angle_y = -28.0 * (math.pi / 180.0) * w
                
                x1 = dx * math.cos(angle_z) - dy * math.sin(angle_z)
                y1 = dx * math.sin(angle_z) + dy * math.cos(angle_z)
                z1 = dz
                
                x2 = x1 * math.cos(angle_y) + z1 * math.sin(angle_y)
                y2 = y1
                z2 = -x1 * math.sin(angle_y) + z1 * math.cos(angle_y)
                
                if abs(dx) > 0.18:
                    fw = min(1.0, (abs(dx) - 0.18) / 0.22)
                    z2 += 0.14 * fw
                    y2 -= 0.05 * fw
                    x2 -= 0.04 * fw
                    
                x = sx + x2
                y = sy + y2
                z = sz + z2
            
            # Natural forward lean for sitting in boat
            if y > 0.40:
                lean = (y - 0.40) * 0.10
                z += lean
            
            out_lines.append(f"v {x:.4f} {y:.4f} {z:.4f}\n")
        else:
            out_lines.append(line)
            
    with open(out_path, 'w', encoding='utf-8') as f:
        f.writelines(out_lines)
    print(f"Generated natural gripping seated pose: {out_path}")

pose_obj(r"CustomEngine\extracted_assets\meshes\GM_3D_Character_MainA.obj", r"CustomEngine\extracted_assets\meshes\GM_3D_Character_SeatedA.obj")
pose_obj(r"CustomEngine\extracted_assets\meshes\GM_3D_Character_MainB.obj", r"CustomEngine\extracted_assets\meshes\GM_3D_Character_SeatedB.obj")
pose_obj(r"CustomEngine\extracted_assets\meshes\GM_3D_Character_Shirt.obj", r"CustomEngine\extracted_assets\meshes\GM_3D_Character_SeatedShirt.obj", is_shirt=True)

