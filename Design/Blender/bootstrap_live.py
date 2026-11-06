import bpy
import addon_utils

# Enable the Blender MCP bridge addon in this GUI session and persist the preference,
# so it appears in Edit > Preferences > Add-ons (Enabled Add-ons Only).
try:
    addon_utils.enable("blender_bridge_addon", default_set=True)
    print("BRIDGE_ADDON_ENABLED")
except Exception as e:
    print("BRIDGE_ADDON_ERR", repr(e))

try:
    bpy.ops.mcp.start_bridge()
    print("BRIDGE_STARTED")
except Exception as e:
    print("BRIDGE_START_ERR", repr(e))

try:
    bpy.ops.wm.save_userpref()
    print("USERPREF_SAVED")
except Exception as e:
    print("USERPREF_ERR", repr(e))

# Material preview viewport for editing
for area in bpy.context.screen.areas:
    if area.type == "VIEW_3D":
        for space in area.spaces:
            if space.type == "VIEW_3D":
                space.shading.type = "MATERIAL"
print("LIVE_BOOTSTRAP_OK")
