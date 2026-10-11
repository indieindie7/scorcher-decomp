"""Flatten a DOS/4GW LE executable (Watcom) into a raw memory image for Ghidra.
Lays every object out at its load base, applies internal fixups (32-bit offset, 16-bit offset, 16:32 pointer),
and writes <out>.bin plus <out>.map listing the objects so a Ghidra script can create memory blocks.
usage: leflatten.py <exe> <out-prefix>
"""
import struct, sys

def u16(b, o): return struct.unpack_from("<H", b, o)[0]
def u32(b, o): return struct.unpack_from("<I", b, o)[0]

def main():
    exe, out = sys.argv[1], sys.argv[2]
    d = open(exe, "rb").read()
    le = u32(d, 0x3C)
    assert d[le:le + 2] == b"LE", "not LE"
    pagesize = u32(d, le + 0x28)
    npages = u32(d, le + 0x14)
    lastpage = u32(d, le + 0x2C)
    objtab = le + u32(d, le + 0x40); nobj = u32(d, le + 0x44)
    pagemap = le + u32(d, le + 0x48)
    fixpage = le + u32(d, le + 0x68); fixrec = le + u32(d, le + 0x6C)
    datapages = u32(d, le + 0x80)
    eipobj, eip = u32(d, le + 0x18), u32(d, le + 0x1C)
    espobj, esp = u32(d, le + 0x20), u32(d, le + 0x24)

    objs = []
    for i in range(nobj):
        o = objtab + 24 * i
        vsize, base, flags, pidx, pcnt = struct.unpack_from("<IIIII", d, o)
        objs.append((vsize, base, flags, pidx, pcnt))

    def page_bytes(pnum):   # pnum is 1-based
        if pnum > npages: return b""
        off = datapages + (pnum - 1) * pagesize
        n = lastpage if pnum == npages else pagesize
        return d[off:off + n]

    top = max(b + v for v, b, _, _, _ in objs)
    lo = min(b for _, b, _, _, _ in objs)
    img = bytearray(top - lo)
    for vsize, base, flags, pidx, pcnt in objs:
        for k in range(pcnt):
            pb = page_bytes(pidx + k)
            dst = base - lo + k * pagesize
            img[dst:dst + len(pb)] = pb

    # fixups: one record list per page
    nfix = 0; kinds = {}
    for vsize, base, flags, pidx, pcnt in objs:
        for k in range(pcnt):
            pnum = pidx + k
            start = fixrec + u32(d, fixpage + 4 * (pnum - 1)); end = fixrec + u32(d, fixpage + 4 * pnum)
            p = start
            while p < end:
                src, flg = d[p], d[p + 1]; p += 2
                stype = src & 0x0F; srclist = src & 0x20
                tflags = flg & 0x03
                srcoff = None
                if srclist:
                    cnt = d[p]; p += 1
                else:
                    srcoff = struct.unpack_from("<h", d, p)[0]; p += 2
                if tflags != 0:   # only internal references expected in DOS/4GW
                    raise SystemExit(f"unsupported target type {tflags} at page {pnum}")
                if flg & 0x40: tobj = u16(d, p); p += 2
                else: tobj = d[p]; p += 1
                if stype == 2:   # 16-bit selector only
                    toff = 0
                else:
                    if flg & 0x10: toff = u32(d, p); p += 4
                    else: toff = u16(d, p); p += 2
                srcs = []
                if srclist:
                    for _ in range(cnt): srcs.append(struct.unpack_from("<h", d, p)[0]); p += 2
                else: srcs = [srcoff]
                target = objs[tobj - 1][1] + toff
                pagebase = base + k * pagesize
                for so in srcs:
                    a = pagebase + so - lo
                    kinds[stype] = kinds.get(stype, 0) + 1; nfix += 1
                    if a < 0 or a >= len(img): continue   # fixup spilling before the page start
                    if stype == 7:      # 32-bit offset
                        struct.pack_into("<I", img, a, target & 0xFFFFFFFF)
                    elif stype == 5:    # 16-bit offset
                        struct.pack_into("<H", img, a, target & 0xFFFF)
                    elif stype == 6:    # 16:32 pointer
                        struct.pack_into("<IH", img, a, target & 0xFFFFFFFF, 0)
                    elif stype == 8:    # 32-bit self-relative
                        struct.pack_into("<i", img, a, target - (pagebase + so + 4))
                    elif stype == 2:    # selector: leave
                        pass
                    else:
                        raise SystemExit(f"unsupported source type {stype}")
    open(out + ".bin", "wb").write(img)
    with open(out + ".map", "w") as m:
        m.write(f"imagebase 0x{lo:x}\nentry 0x{objs[eipobj-1][1]+eip:x}\nstack 0x{objs[espobj-1][1]+esp:x}\n")
        for i, (vsize, base, flags, pidx, pcnt) in enumerate(objs):
            kind = "code" if flags & 4 else "data"
            m.write(f"obj{i+1} 0x{base:x} 0x{vsize:x} {kind} flags=0x{flags:x} filepages={pcnt}\n")
    print(f"objects {nobj}, image {len(img)} bytes from 0x{lo:x}, fixups {nfix} by type {kinds}")
    print(f"entry 0x{objs[eipobj-1][1]+eip:x}")

main()
