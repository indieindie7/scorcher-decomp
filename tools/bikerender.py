# (copy of CyberSpeedway vehrender.py with a chase view) blender -b -P bikerender.py -- <outdir> <in.obj>...: per model an FBX and 4 renders (side, front, back, 34, top; VEHVIEWS=top,side limits them), 1024x1024 Eevee.
# Keeps the OBJ's MTL colours (Cyber Speedway); models without an MTL get a light clay colour (Cyber Sled). 10-13.
import bpy, sys, math, os, mathutils
a = sys.argv[sys.argv.index('--') + 1:]; outd, srcs = a[0], a[1:]
os.makedirs(outd, exist_ok=True)
for src in srcs:
    name = os.path.splitext(os.path.basename(src))[0]
    bpy.ops.wm.read_factory_settings(use_empty=True)
    bpy.ops.wm.obj_import(filepath=src, forward_axis='NEGATIVE_Z', up_axis='Y')
    objs = [o for o in bpy.context.scene.objects if o.type == 'MESH']
    for o in objs:
        if not o.data.materials:
            m = bpy.data.materials.new('clay'); m.use_nodes = True
            m.node_tree.nodes['Principled BSDF'].inputs['Base Color'].default_value = (0.62, 0.62, 0.66, 1)
            o.data.materials.append(m)
        for m in o.data.materials:
            if m and m.use_nodes:
                b = m.node_tree.nodes.get('Principled BSDF')
                if b: b.inputs['Roughness'].default_value = 0.95; b.inputs['Metallic'].default_value = 0.0; b.inputs['Specular IOR Level'].default_value = 0.0
            if m: m.use_backface_culling = bool(os.environ.get('CULL'))
    bpy.ops.export_scene.fbx(filepath=os.path.join(outd, name + '.fbx'))
    sc = bpy.context.scene; sc.render.engine = 'BLENDER_EEVEE'; sc.render.resolution_x = sc.render.resolution_y = 1024
    sc.view_settings.view_transform = 'Standard'
    w = bpy.data.worlds.new('w'); sc.world = w; w.use_nodes = True
    w.node_tree.nodes['Background'].inputs[0].default_value = (0.3, 0.32, 0.36, 1)
    w.node_tree.nodes['Background'].inputs[1].default_value = 0.5
    sun = bpy.data.objects.new('sun', bpy.data.lights.new('sun', 'SUN')); sc.collection.objects.link(sun)
    sun.rotation_euler = (0.8, 0.2, 0.6); sun.data.energy = 3.5
    cam = bpy.data.objects.new('cam', bpy.data.cameras.new('cam')); sc.collection.objects.link(cam); sc.camera = cam
    cam.data.type = 'ORTHO'
    pts = [o.matrix_world @ mathutils.Vector(c) for o in objs for c in o.bound_box]
    lo = mathutils.Vector([min(p[i] for p in pts) for i in range(3)]); hi = mathutils.Vector([max(p[i] for p in pts) for i in range(3)])
    ctr = (lo + hi) / 2; size = (hi - lo).length
    cam.data.ortho_scale = size * 1.05; cam.data.clip_end = size * 20
    views = [("chase", 0, 20), ("chase2", 25, 22), ("side", 90, 5), ('front', 180, 5), ('back', 0, 10), ('34', 135, 25), ('top', 120, 70)]
    only = os.environ.get('VEHVIEWS')  # e.g. VEHVIEWS=top renders only those tags (10-13)
    for tag, az, el in [v for v in views if not only or v[0] in only.split(',')]:
        persp = tag.startswith('chase')       # chase views use a perspective camera like the game
        cam.data.type = 'PERSP' if persp else 'ORTHO'; cam.data.lens = 40
        r = size * (1.45 if persp else 4); azr, elr = math.radians(az), math.radians(el)
        cam.location = ctr + mathutils.Vector((r * math.sin(azr) * math.cos(elr), -r * math.cos(azr) * math.cos(elr), r * math.sin(elr)))
        cam.rotation_euler = (ctr - cam.location).to_track_quat('-Z', 'Y').to_euler()
        sc.render.filepath = os.path.join(outd, f'{name}_{tag}.png'); bpy.ops.render.render(write_still=True)
