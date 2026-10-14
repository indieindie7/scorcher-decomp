# manx2ting.py: put the Manx TT player bike (BHONDA1 high LOD body + both tyres) and rider (PRIDERS row 6, RIDER_M clip 1 key 7)
# into Scorcher's player bike world "ting": writes patched ting.wl3 + TING.0 (Saturn) under extracted/bike/manx/. Offline only:
# the originals are never touched and all outputs are ripped data that stays local (never commit).
# Run: py -3.13 -I tools/manx2ting.py [--maxtex 16] [--length 113600] [--selftest] [--out DIR]
# --pc: write the PC (SCORWIN) form instead (extracted/bike/manx_pc/ting.wl3 + ting.0, base = pc/extracted/TAGDEN/scorpc stock files,
#       default --maxtex 32); wl3 differs by the per-corner normals stream at record +0x10 and a larger colour table, TING.0 has the same layout.
# Formats: NOTES.md "Bike mesh for Manx swap (10-13)" and "Manx -> ting converter (10-13)"; BIGS writer in tools/bigs_ting.py.
import sys, os, struct, math, importlib.util, json
import numpy as np

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
MANX = 'C:/Users/john/Documents/ManxTT'
PC = '--pc' in sys.argv
if PC:
    WL3 = os.path.join(ROOT, 'pc', 'extracted', 'TAGDEN', 'scorpc', 'worlds', 'ting', 'ting.wl3')
    BIGS = os.path.join(ROOT, 'pc', 'extracted', 'TAGDEN', 'scorpc', 'bigs', 'ting.0')
    COL = (0x1ed4, 0x1ed4 + 172)        # colour table (be16 count 0x54 + RGB555), last thing in the file
else:
    WL3 = os.path.join(ROOT, 'extracted', 'LEV1', 'scorsat', 'worlds', 'ting', 'ting.wl3')
    BIGS = os.path.join(ROOT, 'saturn', 'cd', 'SCORSAT', 'GAME', 'SCORSAT', 'BIGS', 'TING.0')
    COL = (0x15ec, 0x168c)
sys.path.insert(0, HERE)
from bigs_ting import Bigs   # noqa: E402


def load(name, path):
    s = importlib.util.spec_from_file_location(name, path)
    m = importlib.util.module_from_spec(s)
    sys.modules[name] = m
    s.loader.exec_module(m)
    return m


def arg(name, default):
    return type(default)(sys.argv[sys.argv.index(name) + 1]) if name in sys.argv else default


MAXTEX = arg('--maxtex', 32 if PC else 16)
OPAQUE = PC and '--keep-transparent' not in sys.argv
LENGTH = arg('--length', 113600)
OUT = arg('--out', os.path.join(ROOT, 'extracted', 'bike', 'manx_pc' if PC else 'manx'))
SELFTEST = '--selftest' in sys.argv

# ---------------------------------------------------------------------------------------------------- Manx side
rb = load('rbh2obj', MANX + '/tools/rbh2obj.py')
rp = load('riderpose', MANX + '/tools/riderpose.py')
EX = MANX + '/extracted/'


def roots(path):
    r = rb.Rbh(path)
    objs = rb.find_objects(r)
    parsed = {o: rb.parse_object(r, o) for o in objs}
    B = r.blocks[1]
    rt = [rb.u32(r.blocks[0], i) for i in range(0, len(r.blocks[0]), 4)]
    if all(x in parsed for x in rt):
        rows = {i: [x] for i, x in enumerate(rt)}
    else:
        rl = min(rb.u32(B, o) for o in objs) // 4 // len(rt)
        rows = {i: [rb.u32(B, x + 4 * k) for k in range(rl)] for i, x in enumerate(rt)}
    return rows, parsed


class Mesh:
    def __init__(self):
        self.v = []       # (x, y, z) floats, Manx raw units (y down, +z forward: same frame as Scorcher)
        self.f = []       # dict(idx=[..3|4..], tex=('t', srca, size, colr) | ('f', rgb555))

    def add(self, obj, M=None):
        base = len(self.v)
        for x, y, z in obj['verts']:
            p = np.array([x, y, z, 1.0])
            self.v.append(tuple((M @ p)[:3]) if M is not None else (x, y, z))
        for f in obj['faces']:
            idx = list(dict.fromkeys(f['idx']))            # a repeated corner = triangle
            if len(idx) < 3:
                continue
            tex = ('t', f['srca'], f['size'], f['colr']) if f['srca'] is not None else ('f', f['colr'] & 0x7fff)
            self.f.append(dict(idx=[base + i for i in idx], tex=tex, two=f['two']))


def rider_mesh(key, clip=1):
    prows, pp = roots(EX + '079_PRIDERS.RBH')
    n, pose, kxyz = rp.clip_key(rp.rbh_block0(EX + '080_RIDER_M.RBH'), clip, key)
    mats = rp.part_mats(pose)
    root = rp.mat(t=(0, -22.0, 0)) @ rp.mat(rp.ry(-2048)) @ rp.mat(t=tuple(4 * v for v in kxyz))
    rider = Mesh()
    for i, off in enumerate(prows[6]):
        rider.add(pp[off], root @ mats[i])
    return n, rider


def write_poses(path, place, rider_o, bike_o, xf, nbody, ntyre):
    """pcwin/manx/rider.bin for ddraw.c (user 10-13 "lets keep going until the import is fully done"): Manx RIDER_M clip 1 =
    15 lean keys (key 7 upright, key = 7 + roll * 0.01231 per 12-bit unit = 7.1 deg per key); every key is the same rider mesh
    (same vertex order) posed, written as v16 like make_obj. Also the tyre vertex ranges of the bike object and their axles,
    so ddraw.c can spin the wheels. Layout (little-endian int32 header): 'MXR1', nkeys, rider nv, rider verts offset in
    ting.wl3, bike nv, bike verts offset, front first, front count, rear first, rear count, front axle y z, rear axle y z
    (v16 units), key14 side (+1 if key 14 leans to +x), nidle, then nkeys * nv * 3 s16 lean keys and nidle * nv * 3 s16
    keys of RIDER_M clip 2 (STOPPED: Manx holds key 4.0 below speed 0.3 and plays it back to 0 before leaning again).
    Magic 'MXR2' since the idle keys (was 'MXR1' with 14 ints)."""
    keys = []
    n = 15
    for k in range(n):
        n, m = rider_mesh(k)
        W = [xf(np.array(q)) for q in m.v]
        keys.append([tuple(int(round((w[c] - rider_o['pos'][c]) / 8)) for c in range(3)) for w in W])
    assert all(len(k) == rider_o['nv'] for k in keys), 'pose vertex count differs'
    assert keys[7] == list(rider_o['v16']), 'key 7 differs from the wl3 rider'
    nidle = rider_mesh(0, 2)[0]
    idle = []
    for k in range(nidle):
        _, m = rider_mesh(k, 2)
        W = [xf(np.array(q)) for q in m.v]
        idle.append([tuple(int(round((w[c] - rider_o['pos'][c]) / 8)) for c in range(3)) for w in W])
    assert all(len(k) == rider_o['nv'] for k in idle), 'idle vertex count differs'
    side = 1 if np.mean([v[0] for v in keys[14]]) > np.mean([v[0] for v in keys[7]]) else -1
    ax = lambda raw: [int(round((xf(np.array(raw))[c] - bike_o['pos'][c]) / 8)) for c in (1, 2)]
    hdr = struct.pack('<4s15i', b'MXR2', n, rider_o['nv'], place[('verts', 1)], bike_o['nv'], place[('verts', 2)],
                      nbody, ntyre[0], nbody + ntyre[0], ntyre[1], *ax((0, 0.8, 56.4)), *ax((0, 0, 0)), side, nidle)
    body = b''.join(struct.pack('<3h', *v) for k in keys + idle for v in k)
    open(path, 'wb').write(hdr + body)
    print('rider.bin', len(hdr + body), 'bytes, keys', n, 'idle', nidle, 'nv', rider_o['nv'], 'key14 side', side, 'axles', ax((0, 0.8, 56.4)), ax((0, 0, 0)))


def manx_meshes():
    brows, bp = roots(EX + '031_BHONDA1.RBH')
    trows, tp = roots(EX + '077_TYRES.RBH')
    prows, pp = roots(EX + '079_PRIDERS.RBH')
    bike = Mesh()
    bike.add(bp[brows[0][0]])                                                   # high LOD body
    bike.add(tp[trows[0][0]], rp.mat(t=(0, 0.8, 56.4)))                         # front tyre (root[LOD]) at T(0,0.8,56.4)
    bike.add(tp[trows[2][0]])                                                   # rear tyre (root[LOD+2]) at the origin
    bike.counts = (len(bp[brows[0][0]]['verts']), len(tp[trows[0][0]]['verts']), len(tp[trows[2][0]]['verts']))
    n, pose, kxyz = rp.clip_key(rp.rbh_block0(EX + '080_RIDER_M.RBH'), 1, 7)
    mats = rp.part_mats(pose)
    root = rp.mat(t=(0, -22.0, 0)) @ rp.mat(rp.ry(-2048)) @ rp.mat(t=tuple(4 * v for v in kxyz))
    rider = Mesh()
    for i, off in enumerate(prows[6]):
        rider.add(pp[off], root @ mats[i])
    return bike, rider


def tex_image(vram, tex, cap):
    """-> (w, h, palette(list of RGB555 u16, entry 0 = 0xffff), pixels bytes, n_transparent) for a Manx face texture."""
    if tex[0] == 'f':
        return 1, 1, [0xffff, tex[1]], bytes([1]), 0
    _, srca, size, colr = tex
    w, h = ((size >> 8) & 0x3f) * 8, size & 0xff
    a = srca * 8
    px = []
    for y in range(h):
        for x in range(w):
            byte = vram.mem[a + (y * w + x) // 2]
            px.append((byte >> 4) if x % 2 == 0 else (byte & 15))
    pal = [0xffff] + [rb.u16(vram.mem, colr * 8 + 2 * i) & 0x7fff for i in range(1, 16)]
    if OPAQUE:
        # 10-13: SCORWIN draws Manx colour 0 (VDP1 transparent) magenta through tex_DepackMul; fill every 0 texel
        # with its nearest non-zero texel (breadth-first) so the faces are opaque.
        if not any(px):
            px = [1] * len(px)
        else:
            front = [i for i, p in enumerate(px) if p]
            while front:
                nxt = []
                for i in front:
                    y, x = divmod(i, w)
                    for j in ((i - w) if y else -1, (i + w) if y < h - 1 else -1, (i - 1) if x else -1, (i + 1) if x < w - 1 else -1):
                        if j >= 0 and not px[j]:
                            px[j] = px[i]
                            nxt.append(j)
                front = nxt
    k = min(1.0, cap / max(w, h))
    w2, h2 = max(1, round(w * k)), max(1, round(h * k))
    out = bytes(px[min(h - 1, int((y + 0.5) * h / h2)) * w + min(w - 1, int((x + 0.5) * w / w2))]
                for y in range(h2) for x in range(w2))
    return w2, h2, pal, out, sum(1 for p in px if p == 0)


# ---------------------------------------------------------------------------------------------------- wl3 side
class Stock:
    def __init__(self, d):
        self.d = d
        u32 = lambda o: struct.unpack_from('>I', d, o)[0]
        u16 = lambda o: struct.unpack_from('>H', d, o)[0]
        self.recs = {}
        for i, (o, ln) in enumerate(((0x1bc, 0xc0), (0x27c, 0x68), (0x2e4, 0x68))):
            io, vo, co, no, fo = (o + u32(o + k) for k in (0, 4, 8, 0xc, 0x14))
            cno = o + u32(o + 0x10)                  # PC: per-corner normals (Saturn: points at the record itself)
            nv, nf = u32(o + 0x18), u32(o + 0x1c)
            n1 = u16(io)
            q = io + 2 + 6 * n1
            n2 = u16(q)
            self.recs[i] = dict(off=o, len=ln, rec=bytearray(d[o:o + ln]), nv=nv, nf=nf, n1=n1, n2=n2,
                                verts=d[vo:vo + 6 * nv], idx=d[io:q + 2 + 8 * n2], flags=d[fo:fo + 2 * nf],
                                circ=d[co:co + 4 * nf + 2], norm=d[no:no + 3 * nf],
                                cnorm=d[cno:cno + 3 * (3 * n1 + 4 * n2)] if PC else b'',
                                pos=[struct.unpack_from('>i', d, o + 0x2c + 4 * k)[0] for k in range(3)])


def face_data(verts16, tris, quads, tex_circle=True):
    """flags / circle / normals for faces ordered tris then quads (verts16: s16 triples)."""
    V = np.array(verts16, float)
    flags, circ, norm = [], [], []
    for f in tris + quads:
        P = V[f]
        if len(f) == 3:
            n = -np.cross(P[1] - P[0], P[2] - P[0])
        else:
            n = -np.cross(P[2] - P[0], P[3] - P[1])
        ln = np.linalg.norm(n)
        n = n / ln * 64 if ln else n
        norm += [int(max(-127, min(127, round(c)))) for c in n]
        ctr = (P.min(0) + P.max(0)) / 2
        r = np.linalg.norm(P - ctr, axis=1).max()
        cl = lambda v: int(max(-128, min(127, round(v))))
        circ.append(struct.pack('>4b', cl(ctr[2] / 256), cl(math.ceil(r * 1.3 / 256) + 1), cl(ctr[0] / 256), cl(ctr[1] / 256)))
        flags.append(0x0007 if len(f) == 3 else 0x000f)
    return flags, circ, norm


def corner_normals(verts16, tris, quads, thresh_deg=80.0):
    """PC per-corner normals (3 s8 per corner, length 64, same sign as the +0xc face normals): average of the unit face normals
    around the vertex that lie within thresh_deg of this face (stock looks smoothed with hard edges: bike mean cos 0.98, rider 0.92)."""
    V = np.array(verts16, float)
    faces = tris + quads
    F = []
    for f in faces:
        P = V[f]
        n = -np.cross(P[1] - P[0], P[2] - P[0]) if len(f) == 3 else -np.cross(P[2] - P[0], P[3] - P[1])
        ln = np.linalg.norm(n)
        F.append(n / ln if ln else n)
    at = {}
    for fi, f in enumerate(faces):
        for v in f:
            at.setdefault(v, []).append(fi)
    ct = math.cos(math.radians(thresh_deg))
    out = []
    for fi, f in enumerate(faces):
        for v in f:
            if np.linalg.norm(F[fi]) == 0:                 # degenerate face: use all faces at the vertex
                n = sum(F[g] for g in at[v])
            else:
                n = sum(F[g] for g in at[v] if F[g] @ F[fi] >= ct)
            ln = np.linalg.norm(n)
            n = n / ln * 64 if ln else n
            out += [int(max(-127, min(127, round(c)))) for c in n]
    return out


def make_obj(mesh, pos, xf):
    """mesh Mesh (Manx raw), xf raw->world (function), pos s32 triple -> dict for the wl3 writer + faces in tris-then-quads order."""
    W = [xf(np.array(p)) for p in mesh.v]
    v16 = []
    for w in W:
        v16.append(tuple(int(round((w[k] - pos[k]) / 8)) for k in range(3)))
    lo = min(min(v) for v in v16)
    hi = max(max(v) for v in v16)
    assert -32768 <= lo and hi <= 32767, ('s16 overflow', lo, hi)
    tris = [f for f in mesh.f if len(f['idx']) == 3]
    quads = [f for f in mesh.f if len(f['idx']) == 4]
    order = tris + quads
    maxd = max(math.sqrt(sum((c * 8) ** 2 for c in v)) for v in v16)
    flags, circ, norm = face_data(v16, [f['idx'] for f in tris], [f['idx'] for f in quads])
    idx = struct.pack('>H', len(tris)) + b''.join(struct.pack('>3H', *[2 * i for i in f['idx']]) for f in tris) \
        + struct.pack('>H', len(quads)) + b''.join(struct.pack('>4H', *[2 * i for i in f['idx']]) for f in quads)
    return dict(nv=len(v16), nf=len(order), n1=len(tris), n2=len(quads), v16=v16, faces=order, maxd=maxd, range=(lo, hi),
                verts=b''.join(struct.pack('>3h', *v) for v in v16), idx=idx,
                flags=b''.join(struct.pack('>H', f) for f in flags), circ_list=circ,
                norm=b''.join(struct.pack('>b', n) for n in norm), pos=pos,
                cnorm=b''.join(struct.pack('>b', n) for n in corner_normals(v16, [f['idx'] for f in tris], [f['idx'] for f in quads])))


def build_wl3(st, objs, term):
    """objs: {0: sphere(stock dict), 1: rider dict, 2: bike dict}; term = stock circle terminators. Returns bytes."""
    d = st.d
    out = bytearray(d[:0x3b0])
    r = {i: bytearray(st.recs[i]['rec']) for i in range(3)}
    start = {}                                    # first bigs descriptor of each wl3 object: bike 0, rider, sphere
    start[2] = 0
    start[1] = objs[2]['nf']
    start[0] = start[1] + objs[1]['nf']
    place = {}
    for kind in (('verts', 'idx', 'flags', 'cnorm', 'circ', 'norm') if PC else ('verts', 'idx', 'flags', 'circ', 'norm')):
        for i in range(3):
            o = objs[i]
            place[(kind, i)] = len(out)
            if kind == 'circ':
                out += (o['circ'] if isinstance(o['circ'], (bytes, bytearray)) else b''.join(o['circ'])) + (term[i] if 'circ_list' in o else b'')
            else:
                out += o[kind]
    while len(out) % 4:
        out += b'\0'
    colour = len(out)
    out += d[COL[0]:COL[1]]
    tv = tf = 0
    for i in range(3):
        o, rec, ro = objs[i], r[i], st.recs[i]['off']
        for kind, f in (('idx', 0), ('verts', 4), ('circ', 8), ('norm', 0xc), ('flags', 0x14)) + ((('cnorm', 0x10),) if PC else ()):
            struct.pack_into('>I', rec, f, place[(kind, i)] - ro)
        struct.pack_into('>II', rec, 0x18, o['nv'], o['nf'])
        if 'maxd' in o:
            struct.pack_into('>I', rec, 0x24, math.ceil(o['maxd'] / 4) + 2)
        struct.pack_into('>I', rec, 0x54, start[i])
        out[ro:ro + st.recs[i]['len']] = rec
        tv += o['nv']
        tf += o['nf']
    struct.pack_into('>III', out, 0xc, colour, 3, tv)
    struct.pack_into('>I', out, 0x18, tf)
    # hierarchy (5 words/node): leaf radius = rec[+0x24]*4+3; parent = dist(parent centre, child pos) + child radius
    nodes = {n: list(struct.unpack_from('>5i', out, 0x34c + 20 * n)) for n in range(5)}
    for leaf, par, i in ((1, 0, 1), (3, 2, 2)):
        rad = struct.unpack_from('>I', r[i], 0x24)[0] * 4 + 3
        nodes[leaf][1] = rad
        c = nodes[par]
        nodes[par][1] = int(math.dist(c[2:5], nodes[leaf][2:5])) + rad
    for n in range(5):
        struct.pack_into('>5i', out, 0x34c + 20 * n, *nodes[n])
    return bytes(out), place


# ---------------------------------------------------------------------------------------------------- BIGS side
def build_bigs(bg, bike_o, rider_o, vram, cap, report):
    """Keep the stock sphere (bigs obj2) images; rewrite bike (bigs obj0) and rider (bigs obj1) as A=white x B=Manx texture."""
    A, B, pals = [], [], []
    palmap, amap, bmap = {}, {}, {}

    def keep_pal(i, need):
        if i == 0xffff:
            return 0xffff
        p = bg.palette(i)[:max(need, 1)]
        if (i, len(p)) not in palmap:
            palmap[(i, len(p))] = len(pals)
            pals.append(p)
        return palmap[(i, len(p))]

    def keep_img(rec, table, tmap, out):
        key = rec[3]
        if key not in tmap:
            w, h, pal, off = rec
            px = bg.pixraw[off]
            need = (max(px) + 1) if px else 1
            if pal != 0xffff:
                need = max(need, 1)
            tmap[key] = len(out)
            out.append((w, h, keep_pal(pal, need) if pal != 0xffff else 0xffff, px))
        return tmap[key]
    # white A image (1x1): palette [transparent, white]
    keep_img(bg.A[0], 'A', amap, A)           # image 0 of each table = the dummy descriptor's images (kept from stock)
    keep_img(bg.B[0], 'B', bmap, B)
    wp = len(pals)
    pals.append([0xffff, 0x7fff])
    A.append((1, 1, wp, bytes([1])))
    WHITE = len(A) - 1
    # 10-13: SCORWIN's tex_DepackMul reads A at the full face size (stock A = the texture, B = colour), so a 1x1 white A
    # read garbage past its end (magenta/white/black stripes); a 1x1 white B (Tint path) drew flat green.
    # PC: Manx texture as A, an all-white B of the same size (stock textured faces: A and B both full images, Mul).
    whiteb = {}

    def white_b(w, h):
        if (w, h) not in whiteb:
            whiteb[(w, h)] = len(B)
            B.append((w, h, wp, bytes([1]) * (w * h)))
        return whiteb[(w, h)]
    sphere = []
    for (a, b, c, flip, fl) in bg.objs[2]:
        if a >= len(bg.A) or (b != 0xffff and b >= len(bg.B)):      # stock quirk: the 36th sphere descriptor overlaps the palette table
            sphere.append((0, 0, 0, 2))
            continue
        ia = keep_img(bg.A[a], 'A', amap, A)
        ib = keep_img(bg.B[b], 'B', bmap, B) if b != 0xffff else 0xffff
        sphere.append((ia, ib, flip, fl))
    cache = {}
    stats = dict(transparent_faces=0, faces=0, sizes={})

    def manx_descs(obj):
        ds = []
        for f in obj['faces']:
            tex = f['tex']
            key = (tex, cap)
            if key not in cache:
                w, h, pal, px, ntr = tex_image(vram, tex, cap)
                pi = len(pals)
                pals.append(pal)
                cache[key] = ((len(A), white_b(w, h)) if PC else len(B), ntr)
                (A if PC else B).append((w, h, pi, px))
                stats['sizes'][(w, h)] = stats['sizes'].get((w, h), 0) + 1
            bi, ntr = cache[key]
            stats['faces'] += 1
            stats['transparent_faces'] += 1 if ntr else 0
            ds.append((bi[0], bi[1], 0, 0) if PC else (WHITE, bi, 0, 0))
        return ds
    bike_d, rider_d = manx_descs(bike_o), manx_descs(rider_o)
    report['tex'] = stats
    return Bigs.build([bike_d, rider_d, sphere], A, B, pals)


# ---------------------------------------------------------------------------------------------------- parse back
def dump_obj(path_wl3, path_obj, skip_sphere=False):
    d = open(path_wl3, 'rb').read()
    be16 = lambda o: struct.unpack_from('>H', d, o)[0]
    s16 = lambda o: struct.unpack_from('>h', d, o)[0]
    be32 = lambda o: struct.unpack_from('>I', d, o)[0]
    s32 = lambda o: struct.unpack_from('>i', d, o)[0]
    t, n = be32(0), be32(0x10)
    lines, base, info = [], 1, []
    for i in range(n):
        o = t + be32(t + 4 * i)
        nv, nf = be32(o + 0x18), be32(o + 0x1c)
        pos = [s32(o + 0x2c + 4 * k) for k in range(3)]
        io, vo, fo = (o + be32(o + k) for k in (0, 4, 0x14))
        verts = [(s16(vo + 6 * k), s16(vo + 6 * k + 2), s16(vo + 6 * k + 4)) for k in range(nv)]
        n1 = be16(io)
        tris = [[be16(io + 2 + 6 * k + 2 * j) // 2 for j in range(3)] for k in range(n1)]
        q = io + 2 + 6 * n1
        n2 = be16(q)
        quads = [[be16(q + 2 + 8 * k + 2 * j) // 2 for j in range(4)] for k in range(n2)]
        assert n1 + n2 == nf and all(max(f) < nv for f in tris + quads), i
        if PC:      # per-corner normals: 3 s8 per corner of the index stream, length ~64, inside the file
            cno = o + be32(o + 0x10)
            nc = 3 * n1 + 4 * n2
            assert cno + 3 * nc <= len(d) and cno != o, (i, 'corner normal stream')
            cl = [math.sqrt(sum(c * c for c in struct.unpack_from('>3b', d, cno + 3 * k))) for k in range(nc)]
            assert all(56 <= x <= 68 for x in cl), (i, min(cl), max(cl))
            fo2 = o + be32(o + 0xc)
            fl = [struct.unpack_from('>3b', d, fo2 + 3 * k) for k in range(nf)]
            cn0 = struct.unpack_from('>%db' % (3 * nc), d, cno)
            c = 0                                                  # every corner normal must lean the same way as its face normal
            for k in range(nf):
                for _ in range(3 if k < n1 else 4):
                    cc = cn0[3 * c:3 * c + 3]
                    assert sum(a * b for a, b in zip(cc, fl[k])) >= 0, (i, 'corner normal vs face normal', k)
                    c += 1
            print('pc corner normals obj%d: %d corners ok, len %.1f..%.1f' % (i, nc, min(cl), max(cl)))
        role = 'sphere' if i == 0 else ('rider' if i == 1 else 'bike')
        info.append((role, nv, nf, n1, n2, pos))
        if skip_sphere and i == 0:
            continue
        lines.append(f'o ting_{i}_{role}')
        for x, y, z in verts:
            lines.append(f'v {(x * 8 + pos[0]) / 1000:.4f} {-(y * 8 + pos[1]) / 1000:.4f} {-(z * 8 + pos[2]) / 1000:.4f}')
        for f in tris + quads:
            lines.append('f ' + ' '.join(str(base + v) for v in f))
        base += nv
    open(path_obj, 'w').write('\n'.join(lines) + '\n')
    return info


def bg_build_stock(bg):
    return Bigs.build([[(a, b, fl, f) for (a, b, c, fl, f) in o] for o in bg.objs],
                      [(w, h, p, bg.pixraw[off]) for (w, h, p, off) in bg.A], [(w, h, p, bg.pixraw[off]) for (w, h, p, off) in bg.B],
                      [bg.palette(i) for i in range(len(bg.pals))])


def main():
    d = open(WL3, 'rb').read()
    st = Stock(d)
    bg = Bigs(open(BIGS, 'rb').read())
    term = {i: st.recs[i]['circ'][-2:] for i in range(3)}
    report = {}
    if SELFTEST:
        # stock streams through the writer must reproduce the original file byte for byte
        # circle terminators are part of the stock circ bytes in selftest: pass circ with terminator, no extra term
        objs = {i: dict(nv=st.recs[i]['nv'], nf=st.recs[i]['nf'], verts=st.recs[i]['verts'], idx=st.recs[i]['idx'],
                        flags=st.recs[i]['flags'], circ=st.recs[i]['circ'], norm=st.recs[i]['norm'],
                        cnorm=st.recs[i]['cnorm']) for i in range(3)}
        new, _ = build_wl3(st, objs, term)
        print('selftest wl3 identical:', new == d, len(new), len(d))
        if new != d:
            print('differing bytes:', [hex(i) for i in range(len(d)) if i < len(new) and new[i] != d[i]][:20],
                  '(0x37b = hierarchy node 2 radius byte: the stock value is 2 units smaller than the recomputed dist+leaf radius)')
        if PC:
            # stock sphere must come through build_bigs unchanged; the BIGS layout is identical to Saturn (Bigs.build does not
            # reproduce either stock file byte for byte: palette lengths are not stored, pixel order differs)
            b2 = Bigs(bg_build_stock(bg))
            same = all(bg.compose(x) == b2.compose(y) for x, y in zip(bg.objs[2], b2.objs[2]) if x[0] < len(bg.A) and (x[1] == 0xffff or x[1] < len(bg.B)))
            print('selftest ting.0 sphere faces compose identical after rebuild:', same, '| objs', [len(o) for o in b2.objs])
        return
    os.makedirs(OUT, exist_ok=True)
    bike, rider = manx_meshes()
    allv = np.array(bike.v)
    mn, mx = allv.min(0), allv.max(0)
    s = LENGTH / (mx[2] - mn[2])
    zc_stock, ybot_stock = 1921.0, 49824.0
    off = np.array([0.0, ybot_stock - s * mx[1], zc_stock - s * (mn[2] + mx[2]) / 2])
    xoff = -s * (mn[0] + mx[0]) / 2
    off[0] = xoff
    xf = lambda p: p * s + off
    print('manx bike raw bbox', mn, mx, 'scale %.2f world units per raw unit' % s)
    rider_o = make_obj(rider, st.recs[1]['pos'], xf)
    bike_o = make_obj(bike, st.recs[2]['pos'], xf)
    for nm, o in (('bike', bike_o), ('rider', rider_o)):
        print(nm, 'verts', o['nv'], 'faces', o['nf'], 'tri', o['n1'], 'quad', o['n2'], 's16 range', o['range'], 'maxd %.0f' % o['maxd'])
    Rw = np.array([xf(np.array(p)) for p in rider.v])
    print('rider world bbox', Rw.min(0).round(), Rw.max(0).round())
    Bw = np.array([xf(np.array(p)) for p in bike.v])
    print('bike world bbox', Bw.min(0).round(), Bw.max(0).round())
    # user 10-13 "drop the ball thing": the wireframe sphere keeps its record and faces (layout unchanged) but all its
    # vertices collapse to the origin, so it draws nothing; --keep-sphere restores it
    sverts = st.recs[0]['verts'] if '--keep-sphere' in sys.argv else bytes(6 * st.recs[0]['nv'])
    sphere = dict(nv=st.recs[0]['nv'], nf=st.recs[0]['nf'], verts=sverts, idx=st.recs[0]['idx'],
                  flags=st.recs[0]['flags'], circ=st.recs[0]['circ'], norm=st.recs[0]['norm'], cnorm=st.recs[0]['cnorm'])
    rider_o['circ'] = rider_o['circ_list']
    bike_o['circ'] = bike_o['circ_list']
    new_wl3, place = build_wl3(st, {0: sphere, 1: rider_o, 2: bike_o}, term)
    vram = rb.Vram([EX + '073_COMMON.VRI.chunks', EX + '032_BHONDA1.VRI.chunks'])
    new_bigs = build_bigs(bg, bike_o, rider_o, vram, MAXTEX, report)
    open(os.path.join(OUT, 'ting.wl3'), 'wb').write(new_wl3)
    open(os.path.join(OUT, 'ting.0' if PC else 'TING.0'), 'wb').write(new_bigs)
    print('ting.wl3 %d bytes (stock %d)   TING.0 %d bytes (stock %d)' % (len(new_wl3), len(d), len(new_bigs), os.path.getsize(BIGS)))
    print('stream offsets', {f'{k}{i}': hex(v) for (k, i), v in place.items()})
    if PC:
        write_poses(os.path.join(OUT, 'rider.bin'), place, rider_o, bike_o, xf, bike.counts[0], bike.counts[1:])
    print('texture stats', report['tex'])
    info = dump_obj(os.path.join(OUT, 'ting.wl3'), os.path.join(OUT, 'manx_ting.obj'))
    dump_obj(os.path.join(OUT, 'ting.wl3'), os.path.join(OUT, 'manx_ting_nosphere.obj'), skip_sphere=True)
    print('parsed back', info)
    b2 = Bigs(new_bigs)
    print('bigs back: objects', [len(o) for o in b2.objs], 'wl3 faces by obj', [i[2] for i in info], '(bigs obj index = n-1-wl3 index)')


main()
