# blender -b -P obj2fbx_render.py -- <in.obj> <outdir>: FBX export + 4 clay renders (side, front, back, 3/4)
import bpy, sys, math, os
a = sys.argv[sys.argv.index('--') + 1:]; src, outd = a[0], a[1]; name = os.path.splitext(os.path.basename(src))[0]
bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.wm.obj_import(filepath=src, forward_axis='NEGATIVE_Z', up_axis='Y')
objs = [o for o in bpy.context.scene.objects if o.type == 'MESH']
cols = [(0.55, 0.12, 0.1), (0.15, 0.15, 0.18), (0.6, 0.6, 0.62), (0.2, 0.5, 0.2)]
for i, o in enumerate(objs):
    m = bpy.data.materials.new(o.name); m.diffuse_color = (*cols[(i + 1) % 4], 1)
    m.use_nodes = True; m.node_tree.nodes['Principled BSDF'].inputs['Base Color'].default_value = (*cols[(i + 1) % 4], 1)
    o.data.materials.append(m)
bpy.ops.export_scene.fbx(filepath=os.path.join(outd, name + '.fbx'))
# frame: the shield (object _0 of ting) hides the bike, so leave it out of the renders
for o in objs:
    if o.name.endswith('_0') and len(objs) == 3: o.hide_render = True
sc = bpy.context.scene; sc.render.engine = 'BLENDER_EEVEE'; sc.render.resolution_x = sc.render.resolution_y = 1024
sc.render.film_transparent = False
w = bpy.data.worlds.new('w'); sc.world = w; w.use_nodes = True; w.node_tree.nodes['Background'].inputs[0].default_value = (0.35, 0.35, 0.38, 1)
sun = bpy.data.objects.new('sun', bpy.data.lights.new('sun', 'SUN')); sc.collection.objects.link(sun); sun.rotation_euler = (0.8, 0.2, 0.6); sun.data.energy = 3
cam = bpy.data.objects.new('cam', bpy.data.cameras.new('cam')); sc.collection.objects.link(cam); sc.camera = cam
cam.data.type = 'ORTHO'
import mathutils
pts = [o.matrix_world @ mathutils.Vector(c) for o in objs if not o.hide_render for c in o.bound_box]
lo = mathutils.Vector([min(p[i] for p in pts) for i in range(3)]); hi = mathutils.Vector([max(p[i] for p in pts) for i in range(3)])
ctr = (lo + hi) / 2; size = (hi - lo).length
cam.data.ortho_scale = size * 1.05; cam.data.clip_end = size * 20
for tag, az, el in [('side', 90, 5), ('front', 180, 5), ('back', 0, 10), ('34', 135, 25)]:
    r = size * 4; azr, elr = math.radians(az), math.radians(el)
    cam.location = (r * math.sin(azr) * math.cos(elr), -r * math.cos(azr) * math.cos(elr), r * math.sin(elr))
    cam.location = cam.location + ctr; d = ctr - cam.location
    cam.rotation_euler = d.to_track_quat('-Z', 'Y').to_euler()
    sc.render.filepath = os.path.join(outd, f'{name}_{tag}.png'); bpy.ops.render.render(write_still=True)
