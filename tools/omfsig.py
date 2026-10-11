"""Match Open Watcom OMF library code against a Watcom-built x86 program (FLIRT-style, fixup bytes wildcarded),
then propagate names through the matched functions' fixups: a self-relative call fixup to an EXTDEF names the callee,
an absolute 32-bit fixup to an EXTDEF names the referenced data.

usage: py -3.13 -I omfsig.py <out.txt> <target> <lib.lib> [more .lib ...]
  <target> = a PE file (sections mapped by header) or "flat:<base>:<file>" for a raw image loaded at <base>.
Output lines: "<addr> <kind> <name> <module> <conf>" with kind = func | data,
  conf = full | prefixNN (body match) | callee | extern (propagated from a matched function).
Env SIGDEBUG=name1,name2 prints those signatures' bytes.
"""
import re, struct, sys, os
from collections import defaultdict, Counter

def idx(d, p):
    b = d[p]
    return (b, p + 1) if b < 0x80 else (((b & 0x7f) << 8) | d[p + 1], p + 2)

def parse_module(d, pos, end):
    """One OMF module. Returns (name, segments, extdefs): segment = dict(name, cls, data, mask, pubs[(off,name)],
    fixups[(off, size, selfrel, tkind, tindex)]) ; extdefs = [None, name1, name2, ...] (1-based index)."""
    lnames = [None]; segs = [None]; extdefs = [None]; modname = "?"
    last = None
    while pos < end:
        t = d[pos]; ln = struct.unpack_from("<H", d, pos + 1)[0]; r = d[pos + 3:pos + 3 + ln - 1]; pos += 3 + ln
        big = t & 1
        if t == 0x80: modname = r[1:1 + r[0]].decode("latin1")
        elif t == 0x96:
            p = 0
            while p < len(r): n = r[p]; lnames.append(r[p + 1:p + 1 + n].decode("latin1")); p += 1 + n
        elif t in (0x8C, 0xB4, 0xB5):
            p = 0
            while p < len(r):
                n = r[p]; extdefs.append(r[p + 1:p + 1 + n].decode("latin1")); p += 1 + n; _, p = idx(r, p)
        elif t == 0xB0:  # COMDEF: name, type idx, data type, length(s)
            p = 0
            while p < len(r):
                n = r[p]; extdefs.append(r[p + 1:p + 1 + n].decode("latin1")); p += 1 + n; _, p = idx(r, p)
                dt = r[p]; p += 1
                for _ in range(2 if dt == 0x61 else 1):
                    v = r[p]; p += 1
                    if v == 0x81: p += 2
                    elif v == 0x84: p += 3
                    elif v == 0x88: p += 4
        elif t in (0x98, 0x99):
            p = 0; acbp = r[p]; p += 1
            if (acbp >> 5) == 0: p += 3
            size = struct.unpack_from("<I" if big else "<H", r, p)[0]; p += 4 if big else 2
            sn, p = idx(r, p); cn, p = idx(r, p)
            segs.append(dict(name=lnames[sn], cls=lnames[cn], data=bytearray(size), mask=bytearray(size), pubs=[], fixups=[]))
        elif t in (0x90, 0x91, 0xB6, 0xB7):
            p = 0; g, p = idx(r, p); s, p = idx(r, p)
            if s == 0: p += 2
            while p < len(r):
                n = r[p]; name = r[p + 1:p + 1 + n].decode("latin1"); p += 1 + n
                off = struct.unpack_from("<I" if big else "<H", r, p)[0]; p += 4 if big else 2
                _, p = idx(r, p)
                if s < len(segs) and segs[s]: segs[s]["pubs"].append((off, name))
        elif t in (0xA0, 0xA1):
            p = 0; s, p = idx(r, p); off = struct.unpack_from("<I" if big else "<H", r, p)[0]; p += 4 if big else 2
            data = r[p:]
            if s < len(segs) and segs[s]:
                sg = segs[s]; sg["data"][off:off + len(data)] = data; last = (s, off)
            else: last = None
        elif t in (0xA2, 0xA3):
            last = None
        elif t in (0x9C, 0x9D) and last:
            s, base = last; sg = segs[s]; p = 0
            while p < len(r):
                b0 = r[p]
                if b0 & 0x80 == 0:  # thread: D(1) 0 method(3) thred(2); frame threads with method <4 carry an index
                    p += 1
                    method = (b0 >> 2) & 7
                    if (b0 & 0x40) == 0 or method < 4: _, p = idx(r, p)
                    continue
                locat = (b0 << 8) | r[p + 1]; p += 2
                loc = (locat >> 10) & 0xF; doff = locat & 0x3FF; selfrel = (locat & 0x4000) == 0
                fd = r[p]; p += 1
                if fd & 0x80 == 0 and (fd >> 4) & 7 < 4: _, p = idx(r, p)
                tkind = -1; tindex = 0
                if fd & 0x08 == 0:
                    tkind = fd & 3; tindex, p = idx(r, p)
                if fd & 0x04 == 0: p += 4 if big else 2
                size = {0: 1, 1: 2, 2: 2, 3: 4, 4: 1, 5: 2, 9: 4, 11: 6, 13: 4}.get(loc, 4)
                a = base + doff
                for i in range(a, min(a + size, len(sg["mask"]))): sg["mask"][i] = 1
                sg["fixups"].append((a, size, selfrel, tkind, tindex))
        elif t in (0x8A, 0x8B): break
    return modname, [s for s in segs if s], extdefs

def parse_lib(path):
    d = open(path, "rb").read()
    assert d[0] == 0xF0, "not an OMF library"
    page = struct.unpack_from("<H", d, 1)[0] + 3
    pos = page; mods = []
    while pos < len(d) and d[pos] == 0x80:
        mods.append(parse_module(d, pos, len(d)))
        p = pos
        while p < len(d):
            t = d[p]; ln = struct.unpack_from("<H", d, p + 1)[0]; p += 3 + ln
            if t in (0x8A, 0x8B): break
        pos = (p + page - 1) // page * page
    return mods

def load_target(spec):
    if spec.startswith("flat:"):
        _, base, path = spec.split(":", 2); return [(int(base, 0), open(path, "rb").read())]
    d = open(spec, "rb").read(); pe = struct.unpack_from("<I", d, 0x3C)[0]
    assert d[pe:pe + 4] == b"PE\0\0"
    nsec = struct.unpack_from("<H", d, pe + 6)[0]; opt = struct.unpack_from("<H", d, pe + 20)[0]
    ibase = struct.unpack_from("<I", d, pe + 24 + 28)[0]; regs = []
    for i in range(nsec):
        s = pe + 24 + opt + i * 40
        va = struct.unpack_from("<I", d, s + 12)[0]; rsz = struct.unpack_from("<I", d, s + 16)[0]
        rptr = struct.unpack_from("<I", d, s + 20)[0]; chars = struct.unpack_from("<I", d, s + 36)[0]
        if chars & 0x20000000: regs.append((ibase + va, d[rptr:rptr + rsz]))
    return regs

def main():
    out, target, libs = sys.argv[1], sys.argv[2], sys.argv[3:]
    regions = load_target(target)
    def read32(addr):
        for base, blob in regions:
            if base <= addr and addr + 4 <= base + len(blob): return struct.unpack_from("<I", blob, addr - base)[0]
        return None
    sigs = []
    for lib in libs:
        for mod, segs, extdefs in parse_lib(lib):
            for sg in segs:
                if "CODE" not in sg["cls"].upper() and not sg["name"].upper().endswith("TEXT"): continue
                ps = sorted(set(sg["pubs"]))
                for i, (off, name) in enumerate(ps):
                    end = ps[i + 1][0] if i + 1 < len(ps) else len(sg["data"])
                    if end <= off: end = len(sg["data"])
                    fx = [(a - off, sz, sr, tk, ti) for (a, sz, sr, tk, ti) in sg["fixups"] if off <= a < end]
                    sigs.append((name, mod, os.path.basename(lib), sg["data"][off:end], sg["mask"][off:end], fx, extdefs))
    print("signatures", len(sigs), "from", len(libs), "libs")
    if os.environ.get("SIGDEBUG"):
        want = os.environ["SIGDEBUG"].split(",")
        for s in sigs:
            if s[0] in want:
                print(s[0], s[1], len(s[3]), "bytes:", " ".join("??" if m else f"{c:02x}" for c, m in zip(s[3][:48], s[4][:48])))
    results = {}; tried = 0; multi = 0
    def rx(data, mask):
        return re.compile(b"".join(b"." if m else re.escape(bytes([c])) for c, m in zip(data, mask)), re.S)
    for name, mod, lib, data, mask, fx, extdefs in sigs:
        if sum(1 for m in mask if not m) < 12 or len(data) < 8: continue
        tries = [("full", data, mask)]
        for n in (32, 24, 16):
            if len(data) > n: tries.append((f"prefix{n}", data[:n], mask[:n]))
        tried += 1
        for conf, dd, mm in tries:
            if sum(1 for m in mm if not m) < (12 if conf == "full" else 14): continue
            r = rx(dd, mm); hits = []
            for base, blob in regions:
                hits += [base + m.start() for m in r.finditer(blob)]
                if len(hits) > 4: break
            if len(hits) > 4: multi += 1
            if 1 <= len(hits) <= 4:
                rank = {"full": 4, "prefix32": 3, "prefix24": 2, "prefix16": 1}[conf]
                for h in hits:
                    prev = results.get(h)
                    if prev is None or prev["rank"] < rank:
                        results[h] = dict(kind="func", name=name, mod=mod, conf=conf, rank=rank, span=len(dd), fx=fx, ext=extdefs)
                break
    # propagate through fixups of matched functions
    prop = {}
    for addr, m in list(results.items()):
        for (o, sz, selfrel, tk, ti) in m["fx"]:
            if tk != 2 or sz != 4 or o + 4 > m["span"] or ti >= len(m["ext"]): continue
            v = read32(addr + o)
            if v is None: continue
            ename = m["ext"][ti]
            if selfrel:
                t = (addr + o + 4 + v) & 0xffffffff
                if read32(t) is None: continue
                prop.setdefault(t, dict(kind="func", name=ename, mod=m["mod"], conf="callee", rank=0))
            else:
                if read32(v) is None and not any(b <= v < b + len(bl) + 0x100000 for b, bl in regions): continue
                prop.setdefault(v, dict(kind="data", name=ename, mod=m["mod"], conf="extern", rank=0))
    for a, m in prop.items():
        if a not in results: results[a] = m
    with open(out, "w") as f:
        for a in sorted(results):
            m = results[a]; f.write(f"{a:#x} {m['kind']} {m['name']} {m['mod']} {m['conf']}\n")
    print("tried", tried, "sigs; multi-hit", multi, "; matched", len(results), "addresses:", dict(Counter(v["conf"] for v in results.values())), "->", out)

main()
