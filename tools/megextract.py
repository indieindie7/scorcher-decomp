"""Extract Zyrinx .MEG ("MEGAFILE") archives from Scorcher (Saturn).

Layout (big-endian): a run of entries [u32 kind][u32 data_offset][u32 data_size][u32 entry_len][name bytes]  (entry_len covers the 16-byte header too),
followed by the file data. Names are full dev paths like t:\\scorsat\\game\\...; the table ends where the
first entry's data begins.

usage: megextract.py <file.meg|dir> <outdir>
"""
import os, re, struct, sys


def parse(data):
    entries = []
    pos = 0
    first_data = None
    while first_data is None or pos < first_data:
        if pos + 16 > len(data):
            break
        kind, off, size, nlen = struct.unpack(">IIII", data[pos:pos + 16])
        name = data[pos + 16:pos + nlen].split(b"\0", 1)[0].decode("latin1")
        if kind == 0xFFFFFFFF or nlen < 16:   # 0xFFFFFFFF terminates the table
            entries.pop()
            break
        entries.append((kind, off, size, name))
        if first_data is None:
            first_data = off
        pos += nlen
    return entries


def extract(path, outdir):
    data = open(path, "rb").read()
    entries = parse(data)
    base = os.path.join(outdir, os.path.splitext(os.path.basename(path))[0])
    os.makedirs(base, exist_ok=True)
    expect = entries[0][1] if entries else 0
    gaps = 0
    with open(os.path.join(base, "_index.txt"), "w") as idx:
        for kind, off, size, name in entries:
            if off != expect:
                gaps += 1
            expect = off + size
            rel = name.replace("\\", "/").split(":/", 1)[-1]
            rel = re.sub(r"^scor(sat|pc)/game/", "", rel) or f"_unnamed_{off:08x}.bin"
            dst = os.path.join(base, rel)
            os.makedirs(os.path.dirname(dst), exist_ok=True)
            open(dst, "wb").write(data[off:off + size])
            idx.write(f"{kind:3d} {off:8d} {size:8d} {name}\n")
    kinds = sorted({e[0] for e in entries})
    tail = len(data) - expect
    print(f"{os.path.basename(path):14s} {len(entries):3d} files, kinds {kinds}, gaps {gaps}, trailing bytes {tail}")
    return entries


if __name__ == "__main__":
    src, out = sys.argv[1], sys.argv[2]
    files = [os.path.join(src, f) for f in sorted(os.listdir(src)) if f.lower().endswith(".meg")] if os.path.isdir(src) else [src]
    for f in files:
        extract(f, out)
