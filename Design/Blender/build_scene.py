import bpy
import math
import os

BASE = r"G:\Steam\steamapps\common\Fart Together Demo"
OUT = r"P:\Blender\pirate_design"
os.makedirs(OUT, exist_ok=True)

# Empty scene
bpy.ops.wm.read_factory_settings(use_empty=True)


def import_obj(path, name):
    before = set(bpy.data.objects)
    bpy.ops.wm.obj_import(filepath=path)
    new = [o for o in bpy.data.objects if o not in before]
    root = new[0] if len(new) == 1 else None
    for o in new:
        o.name = name if root is None or o == root else name + "_part"
    return new[0] if len(new) == 1 else new


boat = import_obj(os.path.join(BASE, r"Models_Export\Boat\BoatMain_ThreeP.obj"), "BoatMain_ThreeP")
boat.location = (0, 0, 0)

# Two standing pirates on the rim, facing each other (T-pose starting point)
A = import_obj(os.path.join(BASE, r"Models_Export\Pirates\Pirate_Standing_MainA.obj"), "Pirate_A")
A.location = (-0.98, 0.0, 0.49)
A.rotation_euler = (math.radians(90), 0, math.radians(90))

C = import_obj(os.path.join(BASE, r"Models_Export\Pirates\Pirate_Standing_MainC.obj"), "Pirate_C")
C.location = (0.98, 0.0, 0.49)
C.rotation_euler = (math.radians(90), 0, math.radians(-90))

# Camera (3/4 high, matching the reference framing)
bpy.ops.object.empty_add(location=(0.0, 0.0, 0.45))
cam_target = bpy.context.object
cam_target.name = "CameraTarget"
bpy.ops.object.camera_add(location=(0.0, -5.2, 3.2))
cam = bpy.context.object
cam.data.lens = 45
con = cam.constraints.new("TRACK_TO")
con.target = cam_target
con.track_axis = "TRACK_NEGATIVE_Z"
con.up_axis = "UP_Y"
bpy.context.scene.camera = cam

# Lights
bpy.ops.object.light_add(type="SUN", location=(2.5, -3.0, 4.0))
sun = bpy.context.object
sun.data.energy = 3.0
sun.rotation_euler = (math.radians(50), 0, math.radians(30))

# World
world = bpy.data.worlds.new("World")
world.use_nodes = True
world.node_tree.nodes["Background"].inputs[0].default_value = (0.05, 0.30, 0.55, 1)
world.node_tree.nodes["Background"].inputs[1].default_value = 0.6
bpy.context.scene.world = world

# Render settings (Cycles CPU — reliable headless on this machine)
s = bpy.context.scene
s.render.engine = "CYCLES"
s.cycles.samples = 24
s.cycles.device = "CPU"
s.render.resolution_x = 1024
s.render.resolution_y = 576
s.render.resolution_percentage = 100
s.render.film_transparent = False
s.render.filepath = os.path.join(OUT, "pirate_boat_scene.png")

# Viewport shading: material preview for editing
for area in bpy.context.screen.areas if bpy.context.screen else []:
    if area.type == "VIEW_3D":
        for space in area.spaces:
            if space.type == "VIEW_3D":
                space.shading.type = "MATERIAL"

bpy.ops.wm.save_as_mainfile(filepath=os.path.join(OUT, "pirate_boat.blend"))
print("SCENE_SAVED", os.path.join(OUT, "pirate_boat.blend"))
bpy.ops.render.render(write_still=True)
print("SCENE_RENDER", s.render.filepath)
