# Bike select turntables for the SCORWIN pre-race screen (pcwin\ddraw\ddraw.c bikesel_*), 10-13.
# 1) blender -b -P bikesel_render.py -- render <outdir> <frames> <size> <name>=<in.obj>[:<tint r,g,b>] ...
#    per bike <outdir>\<name>_NN.png: transparent turntable, one turn over <frames> frames, neon rim light in the tint.
# 2) py -3.13 -I bikesel_render.py pack <outdir> <out.bin> <frames> <name> ...
#    -> "BSL1", w, h, frames, bikes (int32) then BGRA premultiplied rows, bike-major. Ripped models: local only.
import sys, os, math

def render(outd, frames, size, specs):
    import bpy, mathutils
    os.makedirs(outd, exist_ok=True)
    for spec in specs:
        name, src = spec.split('=', 1)
        tint = (0.2, 0.9, 1.0)
        if ':' in src[2:]:
            src, t = src.rsplit(':', 1); tint = tuple(float(x) for x in t.split(','))
        bpy.ops.wm.read_factory_settings(use_empty=True)
        bpy.ops.wm.obj_import(filepath=src, forward_axis='NEGATIVE_Z', up_axis='Y')
        objs = [o for o in bpy.context.scene.objects if o.type == 'MESH' and 'sphere' not in o.name]
        for o in bpy.context.scene.objects:
            if 'sphere' in o.name: bpy.data.objects.remove(o)
        sc = bpy.context.scene
        lo = [1e9] * 3; hi = [-1e9] * 3
        for o in objs:
            used = {i for p in o.data.polygons for i in p.vertices}
            for v in o.data.vertices:
                if v.index not in used: continue
                w = o.matrix_world @ v.co
                for i in range(3): lo[i] = min(lo[i], w[i]); hi[i] = max(hi[i], w[i])
        ctr = [(lo[i] + hi[i]) / 2 for i in range(3)]
        rad = max(hi[i] - lo[i] for i in range(3)) / 2
        piv = bpy.data.objects.new('piv', None); sc.collection.objects.link(piv)
        for o in objs:
            o.location = [o.location[i] - ctr[i] for i in range(3)]
            o.parent = piv
        piv.scale = (1 / rad,) * 3
        body = bpy.data.materials.new('body'); body.use_nodes = True
        b = body.node_tree.nodes['Principled BSDF']
        b.inputs['Base Color'].default_value = (0.32, 0.33, 0.37, 1)
        b.inputs['Metallic'].default_value = 0.6; b.inputs['Roughness'].default_value = 0.35
        glass = bpy.data.materials.new('glass'); glass.use_nodes = True
        g = glass.node_tree.nodes['Principled BSDF']
        g.inputs['Base Color'].default_value = (0, 0, 0, 1)
        g.inputs['Emission Color'].default_value = tint + (1,); g.inputs['Emission Strength'].default_value = 2.5
        for o in objs:
            o.data.materials.clear()
            if 'sphere' in o.name:
                o.hide_render = True   # user 10-13: no wireframe sphere
            else:
                o.data.materials.append(body)
                for p in o.data.polygons: p.use_smooth = False
        sc.render.engine = 'BLENDER_EEVEE'
        sc.render.resolution_x = sc.render.resolution_y = size
        sc.render.film_transparent = True
        sc.view_settings.view_transform = 'Standard'
        w = bpy.data.worlds.new('w'); sc.world = w; w.use_nodes = True
        w.node_tree.nodes['Background'].inputs[0].default_value = (0.05, 0.05, 0.08, 1)
        def light(kind, rot, energy, col, loc=(0, 0, 0)):
            l = bpy.data.objects.new(kind, bpy.data.lights.new(kind, 'SUN')); sc.collection.objects.link(l)
            l.rotation_euler = rot; l.data.energy = energy; l.data.color = col
        light('key', (0.7, 0.1, 0.5), 3.0, (1, 0.97, 0.92))
        light('rim1', (-1.9, 0, 2.6), 6.0, tint)
        light('rim2', (-1.9, 0, -0.4), 4.0, tint)
        light('fill', (2.6, 0, 0), 0.6, (0.6, 0.6, 1))
        cam = bpy.data.objects.new('cam', bpy.data.cameras.new('cam')); sc.collection.objects.link(cam); sc.camera = cam
        cam.data.lens = 50
        el = math.radians(14); d = 3.7
        cam.location = (0, -d * math.cos(el), d * math.sin(el))
        cam.rotation_euler = (math.pi / 2 - el, 0, 0)
        for f in range(frames):
            piv.rotation_euler = (0, 0, 2 * math.pi * f / frames)
            sc.render.filepath = os.path.join(outd, '%s_%02d.png' % (name, f))
            bpy.ops.render.render(write_still=True)

def pack(outd, out, frames, names):
    from PIL import Image
    import struct, numpy as np
    first = Image.open(os.path.join(outd, '%s_00.png' % names[0]))
    w, h = first.size
    with open(out, 'wb') as fo:
        fo.write(b'BSL1' + struct.pack('<4i', w, h, frames, len(names)))
        for n in names:
            for f in range(frames):
                a = np.asarray(Image.open(os.path.join(outd, '%s_%02d.png' % (n, f))).convert('RGBA')).astype(np.uint32)
                al = a[..., 3]
                px = (al << 24) | ((a[..., 0] * al // 255) << 16) | ((a[..., 1] * al // 255) << 8) | (a[..., 2] * al // 255)
                fo.write(px.astype('<u4').tobytes())
    print('packed', out, w, h, frames, len(names))

if '--' in sys.argv:
    a = sys.argv[sys.argv.index('--') + 1:]
    if a[0] == 'render': render(a[1], int(a[2]), int(a[3]), a[4:])
elif len(sys.argv) > 1 and sys.argv[1] == 'pack':
    pack(sys.argv[2], sys.argv[3], int(sys.argv[4]), sys.argv[5:])
