# wl3bike2obj.py [outdir]: dump the bike worlds (player "ting", enemies "fjende1..5", ebike) of the Saturn (extracted/LEVn)
# and PC (pc/extracted/TAGDEN/scorpc/worlds) builds as Wavefront OBJ, one OBJ per world, one group per object with a role
# name, plus <name>.json with per-face data (flags u16, 4-byte circle record, index record).  Run: py -3.13 -I wl3bike2obj.py
# Layout: NOTES.md "Bike mesh for Manx swap (10-13)" and "wl3 octree at runtime" (object mesh record).
# Output is ripped game data: keep it local (extracted/ is not for any public repo).
import sys, os, struct, json

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUT = sys.argv[1] if len(sys.argv) > 1 else os.path.join(ROOT, 'extracted', 'bike')
SRC = [('sat', os.path.join(ROOT, 'extracted', 'LEV1', 'scorsat', 'worlds', '%s', '%s.wl3')),
       ('pc', os.path.join(ROOT, 'pc', 'extracted', 'TAGDEN', 'scorpc', 'worlds', '%s', '%s.wl3'))]
ENDSEQ = os.path.join(ROOT, 'extracted', 'ENDSEQ', 'scorsat', 'worlds', 'ebike', 'ebike.wl3')
# role = by object origin (Saturn y is down): y > 0 below = bike, y < 0 above = rider; 30-vert sphere at 0,0,0; 16-vert box


def dump(path, name, tag):
    d = open(path, 'rb').read()
    be16 = lambda o: struct.unpack_from('>H', d, o)[0]
    s16 = lambda o: struct.unpack_from('>h', d, o)[0]
    be32 = lambda o: struct.unpack_from('>I', d, o)[0]
    s32 = lambda o: struct.unpack_from('>i', d, o)[0]
    t, n = be32(0), be32(0x10)
    obj = open(os.path.join(OUT, f'{tag}_{name}.obj'), 'w')
    info, base = [], 1
    for i in range(n):
        o = t + be32(t + 4 * i)
        nv, nf = be32(o + 0x18), be32(o + 0x1c)
        sc = be32(o + 0x28) / 65536.0                       # 2.0 for every bike object
        pos = [s32(o + 0x2c), s32(o + 0x30), s32(o + 0x34)]
        io, vo, co, fo = (o + be32(o + k) for k in (0, 4, 8, 0x14))
        verts = [(s16(vo + 6 * k), s16(vo + 6 * k + 2), s16(vo + 6 * k + 4)) for k in range(nv)]
        n1 = be16(io)
        tris = [[be16(io + 2 + 6 * k + 2 * j) // 2 for j in range(3)] for k in range(n1)]
        q = io + 2 + 6 * n1
        n2 = be16(q)
        quads = [[be16(q + 2 + 8 * k + 2 * j) // 2 for j in range(4)] for k in range(n2)]
        faces = tris + quads                                  # face k <-> flags[k] <-> circle[k]
        assert len(faces) == nf and all(max(f) < nv for f in faces), (name, i)
        flags = [be16(fo + 2 * k) for k in range(nf)]
        circ = [list(struct.unpack_from('>4b', d, co + 4 * k)) for k in range(nf)]
        role = ('sphere30' if nv == 30 and pos == [0, 0, 0] else 'box12' if nv == 16 and n1 == 0 else
                'bike' if pos[1] > 0 else 'rider') if name != 'ebike' else ('part' if i == 0 else 'bike')
        obj.write(f'o {name}_{i}_{role}\n')
        k = sc * 4        # world units = s16 * 8 (+ object pos); y is down on the Saturn: flip y and z for OBJ
        for x, y, z in verts:
            obj.write(f'v {(x * k + pos[0]) / 1000:.4f} {-(y * k + pos[1]) / 1000:.4f} {-(z * k + pos[2]) / 1000:.4f}\n')
        for f in faces:   # winding reversed by the double flip is kept as in tools/wl3obj.py
            obj.write('f ' + ' '.join(str(base + v) for v in f) + '\n')
        base += nv
        info.append(dict(index=i, role=role, verts=nv, faces=nf, tris=n1, quads=n2, scale_raw=be32(o + 0x28), pos=pos,
                         prio=be16(o + 0x48), face_flags=flags, face_circle=circ))
        print(f'{tag} {name} obj{i} {role}: {nv} verts, {nf} faces ({n1} tri + {n2} quad), pos {pos}')
    json.dump(info, open(os.path.join(OUT, f'{tag}_{name}.json'), 'w'))


os.makedirs(OUT, exist_ok=True)
for tag, pat in SRC:
    for w in ('ting', 'fjende1', 'fjende2', 'fjende3', 'fjende4', 'fjende5'):
        p = pat % (w, w)
        if os.path.exists(p):
            dump(p, w, tag)
    if tag == 'pc':
        dump(pat % ('ebike', 'ebike'), 'ebike', tag)
if os.path.exists(ENDSEQ):
    dump(ENDSEQ, 'ebike', 'sat')
