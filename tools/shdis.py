import os
import sys, capstone
d = open(os.environ.get("SCORCHER_DIR", ".") + "/" + "saturn/cd/LORT.BIN", "rb").read()
md = capstone.Cs(capstone.CS_ARCH_SH, capstone.CS_MODE_BIG_ENDIAN | capstone.CS_MODE_SH2)
a, n = int(sys.argv[1], 16), int(sys.argv[2])
o = a - 0x6004000
print(d[o:o+n].hex())
for i in md.disasm(d[o:o+n], a): print("%x %s %s" % (i.address, i.mnemonic, i.op_str))
