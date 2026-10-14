# wl3obj.py <file.wl3> <out.obj>: the meshes of a .wl3 world as one Wavefront OBJ (a group per object)
# layout: NOTES.md ".wl3 octree at runtime" (object mesh record)
import sys, struct
d = open(sys.argv[1], 'rb').read()
be16 = lambda o: struct.unpack_from('>H', d, o)[0]
s16 = lambda o: struct.unpack_from('>h', d, o)[0]
be32 = lambda o: struct.unpack_from('>I', d, o)[0]
s32 = lambda o: struct.unpack_from('>i', d, o)[0]
t = be32(0); n = be32(0x10)
out = open(sys.argv[2], 'w'); base = 1
for i in range(n):
    o = t + be32(t + 4 * i)
    nv, nf = be32(o + 0x18), be32(o + 0x1c)
    scale = be32(o + 0x28) / 65536.0
    px, py, pz = s32(o + 0x2c), s32(o + 0x30), s32(o + 0x34)
    io, vo = o + be32(o), o + be32(o + 4)
    verts = [(s16(vo + 6 * k), s16(vo + 6 * k + 2), s16(vo + 6 * k + 4)) for k in range(nv)]
    n1 = be16(io); tris = [[be16(io + 2 + 6 * k + 2 * j) for j in range(3)] for k in range(n1)]
    q = io + 2 + 6 * n1; n2 = be16(q); quads = [[be16(q + 2 + 8 * k + 2 * j) for j in range(4)] for k in range(n2)]
    faces = tris + quads
    mx = max(max(f) for f in faces) if faces else 0
    div = 2 if mx >= nv else 1
    print(f'obj {i}: verts {nv} faces {nf} (tris {n1} quads {n2}) maxidx {mx} div {div} scale {scale} pos {px},{py},{pz} prio {be16(o+0x48)}')
    out.write(f'o {sys.argv[1].split("/")[-1].split(".")[0]}_{i}\n')
    k = scale * 4
    for x, y, z in verts:   # Saturn y is down: flip y (and z to keep handedness)
        out.write(f'v {(x * k + px) / 1000:.4f} {-(y * k + py) / 1000:.4f} {-(z * k + pz) / 1000:.4f}\n')
    for f in faces:
        out.write('f ' + ' '.join(str(base + v // div) for v in f) + '\n')
    base += nv
