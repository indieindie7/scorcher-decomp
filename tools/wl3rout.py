"""List the ROUT chunks of Scorcher .wl3 worlds: routine name, alloc size, VALS pokes, STRN strings, LOAD files.
usage: py -3.13 -I wl3rout.py <worlds dir> [routine-name filter]
Chunk = {u32 tag; u16 len; u16 count; data} big-endian. ROUT data = count entries of {u16 len; u16 count; chunks}.
NAME data = char name[16] + u32 allocsize + u32 x. VALS = u32 n + [ptrref, u32 value, u32 size]*; STRN = u32 n + [ptrref, cstr pad4]*.
ptrref = u32 kind (0 none, 1 WROUT instance block, 2 object, 3 OROUT instance block; Watcom regs EBX/ECX/stack of wl3_Bind*Routine) + u32 nofs + u32 ofs[nofs] (chain: deref all but last)."""
import struct, sys, os, glob
from collections import defaultdict
KIND = {0: "none", 1: "wrout", 2: "obj", 3: "orout"}
def u32(d, o): return struct.unpack_from(">I", d, o)[0]
def u16(d, o): return struct.unpack_from(">H", d, o)[0]
def ptrref(d, o):
    kind, n = u32(d, o), u32(d, o + 4); ofs = [u32(d, o + 8 + 4 * i) for i in range(n)]
    s = KIND.get(kind, str(kind)) + "".join(("->" if i < n - 1 else "+") + f"{x:#x}" for i, x in enumerate(ofs))
    return s, o + 8 + 4 * n
def chunks(d, o, end):
    while o + 8 <= end:
        tag, ln, cnt = d[o:o+4], u16(d, o + 4), u16(d, o + 6)
        if ln < 8: break
        yield tag, cnt, d[o+8:o+ln]; o += ln
def parse_rout(d, o):
    ln, cnt = u16(d, o + 4), u16(d, o + 6); p = o + 8; out = []
    for _ in range(cnt):
        eln, ecnt = u16(d, p), u16(d, p + 2)
        ent = dict(name="?", vals=[], strs=[], loads=[], other=[])
        for tag, c, data in chunks(d, p + 4, p + eln):
            if tag == b"NAME": ent["name"] = data[:16].split(b"\0")[0].decode(); ent["size"] = u32(data, 16); ent["x"] = u32(data, 20)
            elif tag == b"VALS":
                n = u32(data, 0); q = 4
                while q < 4 + 4 * n:
                    r, q = ptrref(data, q); v, sz = u32(data, q), u32(data, q + 4); q += 8
                    if sz == 2 and v >= 0x8000: v -= 0x10000
                    if sz == 4 and v >= 0x80000000: v -= 0x100000000
                    ent["vals"].append(f"{r}={v}")
            elif tag == b"STRN":
                n = u32(data, 0); q = 4
                while q < 4 + 4 * n:
                    r, q = ptrref(data, q); s = data[q:data.index(b"\0", q)]; q += (len(s) + 4) & ~3
                    ent["strs"].append(f"{r}={s.decode()!r}")
            elif tag == b"LOAD":
                ent["loads"].append(repr(bytes(x for x in data if 32 <= x < 127).decode()))
            else: ent["other"].append(tag.decode() + f"[{c}]")
        out.append(ent); p += eln
    return out
def main():
    root = sys.argv[1]; filt = sys.argv[2] if len(sys.argv) > 2 else None
    uses = defaultdict(list); fields = defaultdict(set)
    for f in sorted(glob.glob(os.path.join(root, "*", "*.wl3"))):
        d = open(f, "rb").read(); w = os.path.basename(f)[:-4]
        o = d.find(b"ROUT")
        while o >= 0:
            for ent in parse_rout(d, o):
                uses[ent["name"]].append(w)
                for v in ent["vals"] + ent["strs"]: fields[ent["name"]].add(v.split("=")[0])
                if not filt or filt == ent["name"]:
                    print(f"{w:9s} {ent['name']:12s} size={ent.get('size',0):#x} " + " ".join(ent["vals"] + ent["strs"] + ent["loads"] + ent["other"]))
            o = d.find(b"ROUT", o + 8)
    print("\n== routine -> worlds")
    for n in sorted(uses): print(f"{n:12s} {len(uses[n]):3d}  " + " ".join(sorted(set(uses[n]))))
    print("\n== routine -> poked fields")
    for n in sorted(fields): print(f"{n:12s} " + " ".join(sorted(fields[n])))
main()
