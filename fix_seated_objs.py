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
                
                # Rotate around Z (downward) by angle -55 deg
                angle_z = -55.0 * (math.pi / 180.0) * w
                # Rotate around Y (forward) by angle 22 deg
                angle_y = 22.0 * (math.pi / 180.0) * w
                
                x1 = dx * math.cos(angle_z) - dy * math.sin(angle_z)
                y1 = dx * math.sin(angle_z) + dy * math.cos(angle_z)
                z1 = dz
                
                x2 = x1 * math.cos(angle_y) + z1 * math.sin(angle_y)
                y2 = y1
                z2 = -x1 * math.sin(angle_y) + z1 * math.cos(angle_y)
                
                if abs(dx) > 0.22:
                    fw = min(1.0, (abs(dx) - 0.22) / 0.18)
                    z2 += 0.06 * fw
                    y2 += 0.02 * fw
                
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
                
                angle_z = 55.0 * (math.pi / 180.0) * w
                angle_y = -22.0 * (math.pi / 180.0) * w
                
                x1 = dx * math.cos(angle_z) - dy * math.sin(angle_z)
                y1 = dx * math.sin(angle_z) + dy * math.cos(angle_z)
                z1 = dz
                
                x2 = x1 * math.cos(angle_y) + z1 * math.sin(angle_y)
                y2 = y1
                z2 = -x1 * math.sin(angle_y) + z1 * math.cos(angle_y)
                
                if abs(dx) > 0.22:
                    fw = min(1.0, (abs(dx) - 0.22) / 0.18)
                    z2 += 0.06 * fw
                    y2 += 0.02 * fw
                    
                x = sx + x2
                y = sy + y2
                z = sz + z2
            
            # Natural slight forward lean for head & upper torso
            if y > 0.40:
                lean = (y - 0.40) * 0.08
                z += lean
            
            out_lines.append(f"v {x:.4f} {y:.4f} {z:.4f}\n")
        else:
            out_lines.append(line)
            
    with open(out_path, 'w', encoding='utf-8') as f:
        f.writelines(out_lines)
    print(f"Properly wrote posed mesh: {out_path}")

pose_obj(r"CustomEngine\extracted_assets\meshes\GM_3D_Character_MainA.obj", r"CustomEngine\extracted_assets\meshes\GM_3D_Character_SeatedA.obj")
pose_obj(r"CustomEngine\extracted_assets\meshes\GM_3D_Character_MainB.obj", r"CustomEngine\extracted_assets\meshes\GM_3D_Character_SeatedB.obj")
pose_obj(r"CustomEngine\extracted_assets\meshes\GM_3D_Character_Shirt.obj", r"CustomEngine\extracted_assets\meshes\GM_3D_Character_SeatedShirt.obj", is_shirt=True)

