# bigs_ting.py: read/compose/write the Scorcher BIGS texture stream (TING.0 layout) - helper for manx2ting.py (local use only).
# Layout (big-endian), see NOTES.md "Bike mesh for Manx swap (10-13)" and "Manx -> ting converter (10-13)":
#   0x00 u32 nobj, nfaces_total, npal, nA, nB, objtab(0x28), paltab, Atab, Btab; 0x24 u32 nfaces_total again (cache record count);
#   obj table: 3 u32 per object {face count, first face index, self-relative offset to the 8-byte descriptors};
#   descriptors start at 0x4c with one dummy (0,0,0,0,2), real face k is at 0x54+8k with cache index k+1 (the stock sphere's last
#   descriptor overlaps the palette table; sphere faces are flat colour so it is never used);
#   descriptor 8 bytes: u16 A image, u16 B image, u16 cache index, byte flip flags, byte flags;
#   palette table: u32 per palette, offset relative to the entry itself -> RGB555 u16 list (entry 0 usually 0xffff = transparent);
#   image record 8 bytes: u8 w, u8 h, u16 palette (0xffff none), u32 pixel offset relative to the record's +4 field; 8 bit indices.
#   Face colour = A texel * B texel (5-bit lanes), dest size = max of both per axis (tex_DepackMul 0x16, NOTES).
import struct


class Bigs:
    def __init__(self, data):
        d = data
        self.d = d
        u32 = lambda o: struct.unpack_from('>I', d, o)[0]
        u16 = lambda o: struct.unpack_from('>H', d, o)[0]
        self.nobj, self.nfaces, npal, na, nb, ot, pt, at, bt = (u32(4 * i) for i in range(9))
        self.objs = []          # list of (first, [descriptor tuples])
        for i in range(self.nobj):
            n, first, off = (u32(ot + 12 * i + 4 * k) for k in range(3))
            p = ot + 12 * i + 8 + off
            self.objs.append([struct.unpack_from('>HHHBB', d, p + 8 * k) for k in range(n)])
        self.pals = []
        for i in range(npal):
            p = pt + 4 * i + u32(pt + 4 * i)
            self.pals.append(p)
        # palette lengths are not stored: derive from the next start in sorted order of all pixel/palette addresses
        self.A = [self._rec(at, i) for i in range(na)]
        self.B = [self._rec(bt, i) for i in range(nb)]
        starts = sorted(set(self.pals + [r[3] for r in self.A + self.B]) | {len(d)})
        nxt = {s: starts[starts.index(s) + 1] for s in starts[:-1]}
        self.palraw = [d[p:p + min(nxt[p] - p, 512)] for p in self.pals]
        self.pixraw = {}
        for r in self.A + self.B:
            self.pixraw[r[3]] = d[r[3]:r[3] + r[0] * r[1]]

    def _rec(self, base, i):
        d = self.d
        w, h, pal, off = struct.unpack_from('>BBHI', d, base + 8 * i)
        return (w, h, pal, base + 8 * i + 4 + off)

    def palette(self, idx):
        if idx == 0xffff:
            return None
        raw = self.palraw[idx]
        return [struct.unpack_from('>H', raw, 2 * k)[0] for k in range(len(raw) // 2)]

    def image(self, rec):
        w, h, pal, off = rec
        px = self.d[off:off + w * h]
        p = self.palette(pal)
        return w, h, px, p

    def compose(self, desc):
        """Return (w, h, rows of (r,g,b,a)) for a descriptor, per the NOTES depack rules (nearest sampling)."""
        A = self.image(self.A[desc[0]])
        B = self.image(self.B[desc[1]]) if desc[1] != 0xffff else None
        W = max(A[0], B[0] if B else 1)
        H = max(A[1], B[1] if B else 1)

        def tex(im, x, y):
            if im is None:
                return None
            w, h, px, pal = im
            v = px[(y * h // H) * w + (x * w // W)] if (w, h) != (1, 1) else px[0]
            if pal is None:
                return (31, 31, 31)
            c = pal[v] if v < len(pal) else 0xffff
            if c == 0xffff:
                return None
            return (c & 31, c >> 5 & 31, c >> 10 & 31)
        rows = []
        for y in range(H):
            row = []
            for x in range(W):
                a, b = tex(A, x, y), tex(B, x, y)
                if a is None:
                    row.append((0, 0, 0, 0))
                    continue
                if b is None:
                    b = (31, 31, 31)
                row.append(tuple(min(255, (a[k] * b[k] * 255) // (31 * 31)) for k in range(3)) + (255,))
            rows.append(row)
        return W, H, rows

    # ------------------------------------------------------------------ writer
    @staticmethod
    def build(obj_descs, A_imgs, B_imgs, pals, ):
        """obj_descs: list per bigs object of list of (A, B, flip, flags) with A/B indices into A_imgs/B_imgs;
        A_imgs/B_imgs: lists of (w, h, palette index or 0xffff, bytes); pals: list of lists of RGB555 u16.
        Returns the file bytes in the stock order: header, object table, descriptors, palette table, A table, B table, palettes, pixels."""
        nobj = len(obj_descs)
        nf = sum(len(x) for x in obj_descs)
        ot = 0x28
        desc0 = ot + 12 * nobj
        pt = desc0 + 8 + 8 * nf          # stock has one dummy descriptor (0,0,0,0,2) first; real face k has cache index k+1
        pt = (pt + 3) & ~3
        at = pt + 4 * len(pals)
        bt = at + 8 * len(A_imgs)
        pal0 = bt + 8 * len(B_imgs)
        out = bytearray(pal0)
        palpos = []
        blob = bytearray()
        for p in pals:
            palpos.append(pal0 + len(blob))
            blob += b''.join(struct.pack('>H', c) for c in p)
        if len(blob) & 1:
            blob += b'\0'
        pix0 = pal0 + len(blob)
        pixblob = bytearray()
        pixpos = {}

        def put(data):
            key = bytes(data)
            if key not in pixpos:
                pixpos[key] = pix0 + len(pixblob)
                pixblob.extend(key)
            return pixpos[key]
        hdr = [nobj, nf, len(pals), len(A_imgs), len(B_imgs), ot, pt, at, bt, nf]
        for i, v in enumerate(hdr):
            struct.pack_into('>I', out, 4 * i, v)
        struct.pack_into('>HHHBB', out, desc0, 0, 0, 0, 0, 2)
        first = 0
        for i, descs in enumerate(obj_descs):
            dp = desc0 + 8 + 8 * first
            struct.pack_into('>III', out, ot + 12 * i, len(descs), first, dp - (ot + 12 * i + 8))
            for k, (a, b, flip, fl) in enumerate(descs):
                struct.pack_into('>HHHBB', out, dp + 8 * k, a, b, first + k + 1, flip, fl)
            first += len(descs)
        for i, pp in enumerate(palpos):
            struct.pack_into('>I', out, pt + 4 * i, pp - (pt + 4 * i))
        for tab, imgs in ((at, A_imgs), (bt, B_imgs)):
            for i, (w, h, pal, px) in enumerate(imgs):
                pos = put(px)
                struct.pack_into('>BBHI', out, tab + 8 * i, w, h, pal, pos - (tab + 8 * i + 4))
        return bytes(out) + bytes(blob) + bytes(pixblob)
