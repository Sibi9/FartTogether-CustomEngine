import bpy

print("LIVE_ADDONS", [a.module for a in bpy.context.preferences.addons])

for area in bpy.context.screen.areas:
    if area.type == "VIEW_3D":
        for space in area.spaces:
            if space.type == "VIEW_3D":
                space.shading.type = "MATERIAL"
                space.region_3d.view_perspective = "CAMERA"

bpy.ops.wm.save_mainfile()
print("LIVE_VIEW_OK")
