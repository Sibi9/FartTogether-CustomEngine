import bpy
import math

# Clean scene
bpy.ops.wm.read_factory_settings(use_empty=True)

# Sphere
bpy.ops.mesh.primitive_uv_sphere_add(segments=64, ring_count=32, radius=1.0, location=(0, 0, 0))
obj = bpy.context.object
bpy.ops.object.shade_smooth()
mat = bpy.data.materials.new("TestMat")
mat.use_nodes = True
bsdf = mat.node_tree.nodes["Principled BSDF"]
bsdf.inputs["Base Color"].default_value = (0.08, 0.55, 0.30, 1)
bsdf.inputs["Roughness"].default_value = 0.28
obj.data.materials.append(mat)

# Ground
bpy.ops.mesh.primitive_plane_add(size=10, location=(0, 0, -1))

# Camera
bpy.ops.object.camera_add(location=(3.6, -3.6, 2.6), rotation=(math.radians(64), 0, math.radians(45)))
cam = bpy.context.object
bpy.context.scene.camera = cam

# Light
bpy.ops.object.light_add(type="SUN", location=(3, -3, 5))
bpy.context.object.data.energy = 4.0

# World
world = bpy.data.worlds.new("W")
world.use_nodes = True
world.node_tree.nodes["Background"].inputs[0].default_value = (0.04, 0.07, 0.11, 1)
bpy.context.scene.world = world

# Render
s = bpy.context.scene
s.render.engine = "CYCLES"
s.cycles.samples = 8
s.cycles.device = "CPU"
s.render.resolution_x = 640
s.render.resolution_y = 480
s.render.resolution_percentage = 100
s.render.filepath = r"P:\Blender\blender-mcp\logs\mcp_connection_test.png"
bpy.ops.render.render(write_still=True)
print("RENDER_OK", s.render.filepath)
