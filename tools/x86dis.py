import os
import sys, struct, capstone
# x86dis.py <dos|win> <addr> <n>
which, a, n = sys.argv[1], int(sys.argv[2], 16), int(sys.argv[3])
if which == "dos":
    d = open(os.environ.get("SCORCHER_DIR", ".") + "/" + "pc/flat/scordos.bin", "rb").read(); o = a - 0x10000
else:
    d = open(os.environ.get("SCORCHER_DIR", ".") + "/" + "pc/cd/SCORCHER/SCORWIN.EXE", "rb").read()
    pe = struct.unpack("<I", d[0x3c:0x40])[0]; ns = struct.unpack("<H", d[pe+6:pe+8])[0]
    oh = struct.unpack("<H", d[pe+20:pe+22])[0]; base = struct.unpack("<I", d[pe+52:pe+56])[0]; o = None
    for i in range(ns):
        s = pe + 24 + oh + 40 * i; va, vs, ro = struct.unpack("<I", d[s+12:s+16])[0], struct.unpack("<I", d[s+8:s+12])[0], struct.unpack("<I", d[s+20:s+24])[0]
        if base + va <= a < base + va + max(vs, struct.unpack("<I", d[s+16:s+20])[0]): o = a - base - va + ro
md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
for i in md.disasm(d[o:o+n], a): print("%x %s %s" % (i.address, i.mnemonic, i.op_str))
