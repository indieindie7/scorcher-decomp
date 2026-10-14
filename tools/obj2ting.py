# obj2ting.py: put our original chunky bike (tools/bikegen.py) into Scorcher's player bike world "ting" as obj2, keep the converted Manx
# rider (manx2ting's rider path) as obj1 seated on it, keep the stock sphere obj0.  Writes ting.wl3 + TING.0 (Saturn) under
# extracted/bike/custom/ and a coloured parse-back OBJ for rendering.  Offline only; ripped data stays local.
# Run: py -3.13 -I tools/obj2ting.py [--out DIR] [--rider-dx N --rider-dy N --rider-dz N --rider-sx F]
# Uses the writer functions of tools/manx2ting.py and tools/bigs_ting.py unchanged (importing manx2ting runs its --selftest main, no writes).
# Formats: NOTES.md "Bike mesh for Manx swap (10-13)", "Manx -> ting converter (10-13)", "Custom chunky bike (10-13)".
import sys, os, io, struct, math
import numpy as np

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
sys.path.insert(0, HERE)


def arg(name, default):
    return type(default)(sys.argv[sys.argv.index(name) + 1]) if name in sys.argv else default


OUT = arg('--out', os.path.join(ROOT, 'extracted', 'bike', 'custom'))
RDX, RDY, RDZ, RSX = arg('--rider-dx', 0.0), arg('--rider-dy', -1500.0), arg('--rider-dz', 3500.0), arg('--rider-sx', 0.8)
_argv, _out = sys.argv, sys.stdout
sys.argv, sys.stdout = ['manx2ting', '--selftest'], io.StringIO()      # its main() would otherwise write extracted/bike/manx
import manx2ting as m                                                 # noqa: E402
sys.argv, sys.stdout = _argv, _out
import bikegen                                                        # noqa: E402
from bigs_ting import Bigs                                            # noqa: E402

RGB = bikegen.COL


def rgb555(c):
    return ((c[0] >> 3) | ((c[1] >> 3) << 5) | ((c[2] >> 3) << 10)) & 0x7fff


def main():
    d = open(m.WL3, 'rb').read()
    st = m.Stock(d)
    bg = Bigs(open(m.BIGS, 'rb').read())
    term = {i: st.recs[i]['circ'][-2:] for i in range(3)}
    os.makedirs(OUT, exist_ok=True)
    # ---- our bike in world units (design frame X right, h up, Z forward, units 1000 -> world y = (49.824 - h) * 1000, y down)
    v, f = bikegen.build()
    bike = m.Mesh()
    bike.v = [(X * 1000.0, (bikegen.GROUND_Y - h) * 1000.0, Z * 1000.0) for X, h, Z in v]
    for idx, col in f:
        # the generator's winding is CCW-outward in the dump frame (x, -y, -z), a proper rotation of the game frame: keep as is
        bike.f.append(dict(idx=list(idx), tex=('f', rgb555(RGB[col])), two=False))
    # ---- rider: manx2ting's rider path (PRIDERS row 6 posed RIDER_M clip 1 key 7), same raw -> world frame as the Manx swap, then offset
    mbike, rider = m.manx_meshes()
    allv = np.array(mbike.v)
    mn, mx = allv.min(0), allv.max(0)
    s = m.LENGTH / (mx[2] - mn[2])
    off = np.array([-s * (mn[0] + mx[0]) / 2, 49824.0 - s * mx[1], 1921.0 - s * (mn[2] + mx[2]) / 2])
    rxf = lambda p: np.array([(p[0] * s + off[0]) * RSX + RDX, p[1] * s + off[1] + RDY, p[2] * s + off[2] + RDZ])
    ident = lambda p: np.array(p)
    rider_o = m.make_obj(rider, st.recs[1]['pos'], rxf)
    bike_o = m.make_obj(bike, st.recs[2]['pos'], ident)
    for nm, o in (('bike', bike_o), ('rider', rider_o)):
        print(nm, 'verts', o['nv'], 'faces', o['nf'], 'tri', o['n1'], 'quad', o['n2'], 's16 range', o['range'], 'maxd %.0f' % o['maxd'])
    Bw = np.array(bike.v)
    pl = 0.0
    for fc in bike_o['faces']:
        if len(fc['idx']) == 4:
            P = np.array([bike_o['v16'][i] for i in fc['idx']], float) * 8
            n = np.cross(P[1] - P[0], P[3] - P[0])
            pl = max(pl, abs(np.dot(P[2] - P[0], n)) / (np.linalg.norm(n) or 1))
    print('bike max quad non-planarity after s16 rounding: %.0f world units (%.2f of the bike length 1e-3)' % (pl, pl / 113.6))
    print('bike world bbox', Bw.min(0).round(), Bw.max(0).round(), 'length', round(Bw[:, 2].max() - Bw[:, 2].min()))
    Rw = np.array([rxf(np.array(p)) for p in rider.v])
    print('rider world bbox', Rw.min(0).round(), Rw.max(0).round())
    sphere = dict(nv=st.recs[0]['nv'], nf=st.recs[0]['nf'], verts=st.recs[0]['verts'], idx=st.recs[0]['idx'],
                  flags=st.recs[0]['flags'], circ=st.recs[0]['circ'], norm=st.recs[0]['norm'])
    rider_o['circ'] = rider_o['circ_list']
    bike_o['circ'] = bike_o['circ_list']
    new_wl3, place = m.build_wl3(st, {0: sphere, 1: rider_o, 2: bike_o}, term)
    vram = m.rb.Vram([m.EX + '073_COMMON.VRI.chunks', m.EX + '032_BHONDA1.VRI.chunks'])
    report = {}
    new_bigs = m.build_bigs(bg, bike_o, rider_o, vram, 16, report)
    open(os.path.join(OUT, 'ting.wl3'), 'wb').write(new_wl3)
    open(os.path.join(OUT, 'TING.0'), 'wb').write(new_bigs)
    print('ting.wl3 %d bytes (stock %d)   TING.0 %d bytes (stock %d)' % (len(new_wl3), len(d), len(new_bigs), os.path.getsize(m.BIGS)))
    print('texture stats', report['tex'])
    info = parse_back(os.path.join(OUT, 'ting.wl3'), os.path.join(OUT, 'TING.0'), os.path.join(OUT, 'custom_ting.obj'))
    print('parsed back', info)


def parse_back(path_wl3, path_bigs, path_obj):
    """wl3bike2obj / manx2ting.dump_obj logic plus the per-face colour composed from the parsed TING.0 (bigs obj index = n-1-wl3 index)."""
    d = open(path_wl3, 'rb').read()
    bg = Bigs(open(path_bigs, 'rb').read())
    be16 = lambda o: struct.unpack_from('>H', d, o)[0]
    s16 = lambda o: struct.unpack_from('>h', d, o)[0]
    be32 = lambda o: struct.unpack_from('>I', d, o)[0]
    s32 = lambda o: struct.unpack_from('>i', d, o)[0]
    t, n = be32(0), be32(0x10)
    lines, base, info, mats = [], 1, [], {}
    for i in range(n):
        o = t + be32(t + 4 * i)
        nv, nf = be32(o + 0x18), be32(o + 0x1c)
        pos = [s32(o + 0x2c + 4 * k) for k in range(3)]
        io, vo = (o + be32(o + k) for k in (0, 4))
        verts = [(s16(vo + 6 * k), s16(vo + 6 * k + 2), s16(vo + 6 * k + 4)) for k in range(nv)]
        n1 = be16(io)
        tris = [[be16(io + 2 + 6 * k + 2 * j) // 2 for j in range(3)] for k in range(n1)]
        q = io + 2 + 6 * n1
        n2 = be16(q)
        quads = [[be16(q + 2 + 8 * k + 2 * j) // 2 for j in range(4)] for k in range(n2)]
        assert n1 + n2 == nf and all(max(fc) < nv for fc in tris + quads), i
        role = 'sphere' if i == 0 else ('rider' if i == 1 else 'bike')
        info.append((role, nv, nf, n1, n2, pos))
        if i == 0:
            continue
        descs = bg.objs[n - 1 - i]
        assert len(descs) == nf, (role, len(descs), nf)
        lines.append(f'o ting_{i}_{role}')
        for x, y, z in verts:
            lines.append(f'v {(x * 8 + pos[0]) / 1000:.4f} {-(y * 8 + pos[1]) / 1000:.4f} {-(z * 8 + pos[2]) / 1000:.4f}')
        for k, fc in enumerate(tris + quads):
            w, h, rows = bg.compose(descs[k])
            px = [p for r in rows for p in r if p[3]]
            c = tuple(int(sum(p[j] for p in px) / len(px)) for j in range(3)) if px else (255, 0, 255)
            name = 'c%02x%02x%02x' % c
            mats[name] = c
            lines.append(f'usemtl {name}')
            lines.append('f ' + ' '.join(str(base + vv) for vv in fc))
        base += nv
    mtl = os.path.splitext(path_obj)[0] + '.mtl'
    with open(mtl, 'w') as fm:
        for name, c in mats.items():
            fm.write(f'newmtl {name}\nKd {c[0] / 255:.3f} {c[1] / 255:.3f} {c[2] / 255:.3f}\n')
    open(path_obj, 'w').write(f'mtllib {os.path.basename(mtl)}\n' + '\n'.join(lines) + '\n')
    return info


main()
