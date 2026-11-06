import bpy
import math
from mathutils import Vector

REST = {
    "hips":        ((0.00, 0.00, 0.00), (0.00, 0.00, 0.10)),
    "spine":       ((0.00, 0.00, 0.10), (0.00, 0.00, 0.22)),
    "chest":       ((0.00, 0.00, 0.22), (0.00, 0.00, 0.35)),
    "neck":        ((0.00, 0.00, 0.35), (0.03, 0.00, 0.47)),
    "head":        ((0.03, 0.00, 0.47), (0.08, 0.00, 0.65)),
    "shoulder.L":  ((0.00, 0.00, 0.32), (0.00, 0.22, 0.30)),
    "upper_arm.L": ((0.00, 0.22, 0.30), (0.00, 0.44, 0.30)),
    "forearm.L":   ((0.00, 0.44, 0.30), (0.00, 0.60, 0.30)),
    "hand.L":      ((0.00, 0.60, 0.30), (0.00, 0.675, 0.30)),
    "shoulder.R":  ((0.00, 0.00, 0.32), (0.00, -0.22, 0.30)),
    "upper_arm.R": ((0.00, -0.22, 0.30), (0.00, -0.44, 0.30)),
    "forearm.R":   ((0.00, -0.44, 0.30), (0.00, -0.60, 0.30)),
    "hand.R":      ((0.00, -0.60, 0.30), (0.00, -0.675, 0.30)),
    "thigh.L":     ((0.00, 0.15, 0.00), (0.00, 0.17, -0.16)),
    "shin.L":      ((0.00, 0.17, -0.16), (0.00, 0.17, -0.31)),
    "foot.L":      ((0.00, 0.17, -0.31), (0.15, 0.17, -0.33)),
    "thigh.R":     ((0.00, -0.15, 0.00), (0.00, -0.17, -0.16)),
    "shin.R":      ((0.00, -0.17, -0.16), (0.00, -0.17, -0.31)),
    "foot.R":      ((0.00, -0.17, -0.31), (0.15, -0.17, -0.33)),
}
PARENT = {
    "spine": "hips", "chest": "spine", "neck": "chest", "head": "neck",
    "shoulder.L": "chest", "upper_arm.L": "shoulder.L", "forearm.L": "upper_arm.L", "hand.L": "forearm.L",
    "shoulder.R": "chest", "upper_arm.R": "shoulder.R", "forearm.R": "upper_arm.R", "hand.R": "forearm.R",
    "thigh.L": "hips", "shin.L": "thigh.L", "foot.L": "shin.L",
    "thigh.R": "hips", "shin.R": "thigh.R", "foot.R": "shin.R",
}


def rig_pirate(obj_name, x_side, facing, lean_deg=33.0):
    A = bpy.data.objects[obj_name]

    bpy.ops.object.select_all(action="DESELECT")
    A.select_set(True)
    bpy.context.view_layer.objects.active = A
    bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)

    old = bpy.data.objects.get("Rig_" + obj_name)
    if old:
        bpy.data.objects.remove(old, do_unlink=True)
    for o in list(bpy.data.objects):
        if o.name.startswith("IK_" + obj_name):
            bpy.data.objects.remove(o, do_unlink=True)

    HIP = Vector((x_side * 0.98, 0.0, 0.82))
    bpy.ops.object.armature_add(enter_editmode=True, location=HIP)
    arm = bpy.context.object
    arm.name = "Rig_" + obj_name
    eb = arm.data.edit_bones
    for b in list(eb):
        eb.remove(b)

    def mx(p):
        return (facing * p[0], p[1], p[2])

    for n, (h, t) in REST.items():
        b = eb.new(n)
        b.head = mx(h)
        b.tail = mx(t)
    for n, p in PARENT.items():
        eb[n].parent = eb[p]
        eb[n].use_connect = False
    bpy.ops.object.mode_set(mode="OBJECT")

    bpy.ops.object.select_all(action="DESELECT")
    A.select_set(True)
    arm.select_set(True)
    bpy.context.view_layer.objects.active = arm
    bpy.ops.object.parent_set(type="ARMATURE_AUTO")

    def mk_empty(suffix, loc):
        e = bpy.data.objects.new("IK_" + obj_name + "." + suffix, None)
        bpy.context.scene.collection.objects.link(e)
        e.location = loc
        e.empty_display_size = 0.03
        return e

    wtL = mk_empty("wrist.L", (x_side * 0.52, 0.20, 0.55))
    wtR = mk_empty("wrist.R", (x_side * 0.52, -0.20, 0.55))
    anL = mk_empty("ankle.L", (x_side * 0.88, 0.17, 0.51))
    anR = mk_empty("ankle.R", (x_side * 0.88, -0.17, 0.51))

    bpy.context.view_layer.objects.active = arm
    bpy.ops.object.mode_set(mode="POSE")
    for bone, target in (("forearm.L", wtL), ("forearm.R", wtR), ("shin.L", anL), ("shin.R", anR)):
        pb = arm.pose.bones[bone]
        c = pb.constraints.new("IK")
        c.target = target
        c.chain_count = 2
        c.use_tail = True
    # keep head up
    for bone, ang in (("head", -25.0), ("neck", -10.0)):
        pb = arm.pose.bones[bone]
        pb.rotation_mode = "XYZ"
        pb.rotation_euler.y = math.radians(facing * ang)
    bpy.ops.object.mode_set(mode="OBJECT")

    arm.location = (x_side * 0.93, 0.0, 0.54)
    arm.rotation_euler = (0.0, math.radians(facing * lean_deg), 0.0)
    bpy.context.view_layer.update()
    print("RIGGED", obj_name, "x_side", x_side, "facing", facing)


rig_pirate("Pirate_A", -1, +1, 33.0)
rig_pirate("Pirate_C", +1, -1, 33.0)

bpy.context.scene.render.filepath = r"P:\Blender\pirate_design\rig_both.png"
bpy.ops.render.render(write_still=True)
bpy.ops.wm.save_as_mainfile(filepath=r"P:\Blender\pirate_design\pirate_boat_rigged.blend")
print("RIG_BOTH_OK")
