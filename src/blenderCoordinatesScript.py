import bpy
import mathutils

# Get the active object (or replace with specific object like bpy.data.objects['bed frame'])
obj = bpy.context.active_object

# Get the 8 corners of the bounding box in local space
local_bbox_corners = [mathutils.Vector(corner) for corner in obj.bound_box]

# Transform them to world space
world_bbox_corners = [obj.matrix_world @ corner for corner in local_bbox_corners]

# Get min and max coordinates
min_corner = mathutils.Vector((min(v.x for v in world_bbox_corners),
                               min(v.y for v in world_bbox_corners),
                               min(v.z for v in world_bbox_corners)))

max_corner = mathutils.Vector((max(v.x for v in world_bbox_corners),
                               max(v.y for v in world_bbox_corners),
                               max(v.z for v in world_bbox_corners)))

print("Min corner:", min_corner)
print("Max corner:", max_corner)
