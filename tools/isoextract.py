"""Extract the ISO 9660 data track from a raw MODE1/2352 (or 2048) image into a directory.
usage: isoextract.py <image> <outdir> [sector_size] [header bytes: 16 MODE1, 24 MODE2 form 1]
"""
import os, struct, sys

def main():
    img, out = sys.argv[1], sys.argv[2]
    ss = int(sys.argv[3]) if len(sys.argv) > 3 else 2352
    hdr = int(sys.argv[4]) if len(sys.argv) > 4 else (16 if ss == 2352 else 0)
    f = open(img, "rb")
    def sector(n):
        f.seek(n * ss + hdr); return f.read(2048)
    pvd = sector(16)
    assert pvd[1:6] == b"CD001", "no PVD"
    root = pvd[156:190]
    n = 0; total = 0
    def walk(rec, path):
        nonlocal n, total
        lba = struct.unpack("<I", rec[2:6])[0]; size = struct.unpack("<I", rec[10:14])[0]
        data = b"".join(sector(lba + i) for i in range((size + 2047) // 2048))[:size]
        pos = 0
        while pos < len(data):
            ln = data[pos]
            if ln == 0:
                pos = (pos // 2048 + 1) * 2048; continue
            r = data[pos:pos + ln]; pos += ln
            nl = r[32]; name = r[33:33 + nl]
            if name in (b"\x00", b"\x01"): continue
            name = name.decode("latin1").split(";")[0]
            flags = r[25]
            if flags & 2:
                os.makedirs(os.path.join(path, name), exist_ok=True); walk(r, os.path.join(path, name))
            else:
                l2 = struct.unpack("<I", r[2:6])[0]; s2 = struct.unpack("<I", r[10:14])[0]
                buf = b"".join(sector(l2 + i) for i in range((s2 + 2047) // 2048))[:s2]
                open(os.path.join(path, name), "wb").write(buf); n += 1; total += s2
    os.makedirs(out, exist_ok=True)
    walk(root, out)
    print(f"{n} files, {total/1e6:.1f} MB -> {out}")

main()
