# bikegen.py: original low-poly "chunky" player bike for Scorcher (our own design in the general style of 90s anime
# chopper/racer bikes: fat balloon tyres, bulbous off-white engine pods with red trim, exposed gunmetal/copper pipes, rear turbine).
# Design frame: X right, h = height above the ground, Z forward (units = 1000 world units); the Scorcher frame is y down, +z forward.
# build() -> (verts [(X,h,Z)], faces [(idx list, colour name)]) ; faces are CCW-outward in the dump frame (x, -y_world, -z) = (X, h-49.824, -Z),
# the same winding as the stock ting bike (63 of 63 faces outward in that frame).  Writes OBJ+MTL when run directly.
# Run: py -3.13 -I tools/bikegen.py [outdir]
import sys, os, math

GROUND_Y = 49.824      # world y (k units) of the ground = stock wheel bottom
COL = {                # name -> sRGB 0..255
    'white': (232, 230, 222), 'grey': (186, 190, 196), 'red': (206, 34, 34), 'gun': (62, 66, 76), 'gun2': (90, 94, 104),
    'copper': (178, 108, 60), 'tyre': (26, 26, 30), 'tyre2': (46, 46, 52), 'dark': (14, 14, 18),
}


class B:
    def __init__(self):
        self.v, self.f = [], []

    def vert(self, X, h, Z):
        self.v.append((float(X), float(h), float(Z)))
        return len(self.v) - 1

    def _o(self, i):
        X, h, Z = self.v[i]
        return (X, h - GROUND_Y, -Z)

    def face(self, idx, col, inside):
        """idx corner list (3 or 4); inside = design-frame point inside the solid: fixes the winding to outward."""
        P = [self._o(i) for i in idx]
        n = [0.0, 0.0, 0.0]
        for a in range(len(P)):        # Newell normal
            x1, y1, z1 = P[a]
            x2, y2, z2 = P[(a + 1) % len(P)]
            n[0] += (y1 - y2) * (z1 + z2)
            n[1] += (z1 - z2) * (x1 + x2)
            n[2] += (x1 - x2) * (y1 + y2)
        c = [sum(p[k] for p in P) / len(P) for k in range(3)]
        ci = (inside[0], inside[1] - GROUND_Y, -inside[2])
        if sum(n[k] * (c[k] - ci[k]) for k in range(3)) < 0:
            idx = idx[::-1]
        self.f.append((list(idx), col))

    def centroid(self, ids):
        return tuple(sum(self.v[i][k] for i in ids) / len(ids) for k in range(3))

    # ---- parts
    def tyre(self, cz, ch, R, hw, n, cols, cap_cols, sx=1.0):
        """n-sided prism along X, flat on the ground when n is 8 (vertices at 22.5 + 45k deg), x from -hw to +hw."""
        ring = []
        for sgn in (-1, 1):
            r = []
            for k in range(n):
                a = math.radians((22.5 if n == 8 else 0.0) + 360.0 * k / n)
                r.append(self.vert(sgn * hw, ch + R * math.sin(a), cz + R * math.cos(a)))
            ring.append(r)
        ctr = (0, ch, cz)
        for k in range(n):
            k2 = (k + 1) % n
            self.face([ring[0][k], ring[0][k2], ring[1][k2], ring[1][k]], cols[k % len(cols)], ctr)
        for s in (0, 1):
            r = ring[s]
            fan = [[r[0], r[1], r[2], r[3]], [r[0], r[3], r[4], r[5]], [r[0], r[5], r[6], r[7]]] if n == 8 else \
                  [[r[0], r[1], r[2], r[3]], [r[0], r[3], r[4], r[5]]]
            for j, q in enumerate(fan):
                self.face(q, cap_cols[j % len(cap_cols)], ctr)

    def hexring(self, X, h, Z, R):
        """hexagon in the X-h plane, vertices at 30+60k deg: vertical flats on the outer/inner sides, ridge at the top."""
        return [self.vert(X + R * math.cos(math.radians(30 + 60 * k)), h + R * math.sin(math.radians(30 + 60 * k)), Z) for k in range(6)]

    def pod(self, side, X0, h0, stations, rear_cap, cols, mid_col):
        """Bulbous engine pod: rings (Z, R) lofted; the hexagon's inner flat (index 2 = 150..210 deg) and the two lower faces are dropped
        (hidden against the hull / never seen from above).  side = +1/-1 mirrors in X; X0 = ring centre X."""
        rings = [self.hexring(side * X0, h0, Z, R) for (Z, R) in stations]
        allv = [i for r in rings for i in r]
        ctr = self.centroid(allv)
        # hexagon vertices: 0:30 1:90 2:150 3:210 4:270 5:330 -> edges (k,k+1): 0-1 NE, 1-2 NW, 2-3 W(inner flat for side=+1), 3-4 SW, 4-5 SE, 5-0 E (outer flat)
        keep = (0, 1, 4, 5) if side > 0 else (0, 1, 4, 5)   # mirrored pod keeps the same edge numbers except inner/outer swap: handled below
        for s in range(len(rings) - 1):
            a, b = rings[s], rings[s + 1]
            for e in range(6):
                if side > 0 and e in (2, 3):     # drop inner flat W and SW
                    continue
                if side < 0 and e in (5, 4):     # mirrored: inner flat is the E-edge (5-0), lower inner is SE (4-5)
                    continue
                self.face([a[e], a[(e + 1) % 6], b[(e + 1) % 6], b[e]], cols[e % len(cols)] if s != 0 or True else mid_col, ctr)
        for r in ((rings[0], rings[-1]) if rear_cap else ()):
            self.face([r[0], r[1], r[2], r[3]], 'red', ctr)
            self.face([r[0], r[3], r[4], r[5]], 'red', ctr)
        return rings

    def tube3(self, pts, w, cols, end_col):
        """triangular tube (apex up) following pts [(X,h,Z)] (rings translated, parallelogram sides); drops the inner side face."""
        rings = []
        for (X, h, Z) in pts:
            rings.append([self.vert(X - w, h - w * 0.6, Z), self.vert(X + w, h - w * 0.6, Z), self.vert(X, h + w * 0.9, Z)])
        for s in range(len(rings) - 1):
            a, b = rings[s], rings[s + 1]
            ctr = tuple((sum(self.v[i][k] for i in a + b)) / 6 for k in range(3))
            sgnx = 1 if pts[0][0] > 0 else -1
            for e in range(3):
                # edges: 0-1 bottom, 1-2 right side, 2-0 left side; outer side = away from the centre line (X sign), inner dropped
                if e == 0:
                    continue
                inner = (e == 2 and sgnx > 0) or (e == 1 and sgnx < 0)
                if inner:
                    continue
                self.face([a[e], a[(e + 1) % 3], b[(e + 1) % 3], b[e]], cols[s % len(cols)], ctr)
        r = rings[-1]
        ctr = tuple(sum(self.v[i][k] for i in r + rings[-2]) / 6 for k in range(3))
        self.face(r, end_col, ctr)
        return rings

    def cone(self, Z0, h, R0, Z1, cols):
        """rear turbine cone: hexagon at Z0 (radius R0) closing to an apex at Z1 (behind the bike), 6 triangles."""
        r0 = [self.vert(R0 * math.cos(math.radians(60 * k)), h + R0 * math.sin(math.radians(60 * k)), Z0) for k in range(6)]
        apex = self.vert(0, h, Z1)
        ctr = (0, h, (Z0 + Z1) / 2 + 1.0)
        for k in range(6):
            self.face([r0[k], r0[(k + 1) % 6], apex], cols[k % len(cols)], ctr)


def build():
    b = B()
    # --- tyres (rear fatter than front; rear 8-sided, front 6-sided to save faces)
    b.tyre(cz=-33.0, ch=0.92388 * 22.5, R=22.5, hw=12.0, n=8, cols=['tyre', 'tyre2'], cap_cols=['tyre', 'tyre2', 'tyre'])
    b.tyre(cz=38.5, ch=0.866 * 19.5, R=19.5, hw=8.5, n=6, cols=['tyre', 'tyre2'], cap_cols=['tyre', 'tyre2'])
    # --- hull: rings of 4 (top width wt, top h, bottom h; bottom width follows a constant side slope so every side quad is planar)
    C = 0.35

    def hullring(Z, wt, ht, hb):
        wb = wt - (ht - hb) * C
        return [b.vert(-wt, ht, Z), b.vert(wt, ht, Z), b.vert(wb, hb, Z), b.vert(-wb, hb, Z)]
    st = [hullring(-49.0, 7.0, 47.0, 41.0), hullring(-28.0, 9.0, 44.0, 33.0), hullring(0.0, 12.0, 47.0, 20.0), hullring(34.0, 10.0, 54.0, 38.0)]
    allh = [i for r in st for i in r]
    hc = b.centroid(allh)
    cols_h = [('white', 'white', 'grey'), ('white', 'white', 'grey'), ('white', 'grey', 'grey')]
    for s in range(3):
        a, c = st[s], st[s + 1]
        top, left, right = cols_h[s]
        b.face([a[0], a[1], c[1], c[0]], top, hc)
        b.face([a[3], a[0], c[0], c[3]], left, hc)
        b.face([a[1], a[2], c[2], c[1]], right, hc)
    # --- engine pods (bulbous hex barrels, red rear cap, red front-most band via the first section colours)
    for side in (1, -1):
        b.pod(side, 18.0, 30.0, [(-14.0, 7.0), (4.0, 13.5), (26.0, 8.5)], True, ['white', 'white', 'red', 'white', 'grey', 'white'], 'white')
    # --- exhaust pipes (gunmetal, copper tip) running along the outside of the pods and out the back
    for side in (1, -1):
        b.tube3([(side * 30.0, 19.0, 26.0), (side * 33.0, 22.0, -2.0), (side * 23.0, 34.0, -30.0)], 3.8, ['copper', 'gun'], 'dark')
    # --- rear turbine
    b.cone(-49.0, 43.5, 6.5, -55.0, ['dark', 'red'])
    # --- fork plates
    for side in (1, -1):
        x = side * 10.6
        q = [b.vert(x, 54.0, 32.0), b.vert(x, 16.9, 40.5), b.vert(x, 16.9, 36.5)]
        b.face(q, 'gun2', (0.0, 30.0, 38.0))
    # --- front fender (white wedge over the front tyre)
    def fendring(Z, wt, ht, hb):
        return [b.vert(-wt, ht, Z), b.vert(wt, ht, Z), b.vert(wt, hb, Z), b.vert(-wt, hb, Z)]
    f0, f1 = fendring(30.0, 6.0, 41.5, 38.0), fendring(53.0, 6.0, 37.0, 34.0)
    fc = b.centroid(f0 + f1)
    b.face([f0[0], f0[1], f1[1], f1[0]], 'white', fc)
    b.face([f0[3], f0[0], f1[0], f1[3]], 'red', fc)
    b.face([f0[1], f0[2], f1[2], f1[1]], 'red', fc)
    return b.v, b.f


def planarity(v, f):
    worst = 0.0
    for idx, col in f:
        if len(idx) != 4:
            continue
        P = [v[i] for i in idx]
        d1 = [P[1][k] - P[0][k] for k in range(3)]
        d2 = [P[3][k] - P[0][k] for k in range(3)]
        n = [d1[1] * d2[2] - d1[2] * d2[1], d1[2] * d2[0] - d1[0] * d2[2], d1[0] * d2[1] - d1[1] * d2[0]]
        ln = math.sqrt(sum(c * c for c in n)) or 1.0
        d = abs(sum((P[2][k] - P[0][k]) * n[k] for k in range(3))) / ln
        worst = max(worst, d)
    return worst


def write_obj(path, v, f):
    base = os.path.splitext(path)[0]
    with open(base + '.mtl', 'w') as m:
        for name, c in COL.items():
            m.write(f'newmtl {name}\nKd {c[0] / 255:.3f} {c[1] / 255:.3f} {c[2] / 255:.3f}\n')
    with open(path, 'w') as o:
        o.write(f'mtllib {os.path.basename(base)}.mtl\no custom_bike\n')
        for X, h, Z in v:
            o.write(f'v {X:.4f} {h - GROUND_Y:.4f} {-Z:.4f}\n')
        cur = None
        for idx, col in f:
            if col != cur:
                o.write(f'usemtl {col}\n')
                cur = col
            o.write('f ' + ' '.join(str(i + 1) for i in idx) + '\n')


if __name__ == '__main__':
    out = sys.argv[1] if len(sys.argv) > 1 else os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), 'extracted', 'bike', 'custom')
    os.makedirs(out, exist_ok=True)
    v, f = build()
    write_obj(os.path.join(out, 'custom_bike.obj'), v, f)
    print('verts', len(v), 'faces', len(f), 'tris', sum(1 for x in f if len(x[0]) == 3), 'quads', sum(1 for x in f if len(x[0]) == 4),
          'max quad non-planarity (k units) %.4f' % planarity(v, f))
