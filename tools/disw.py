# Disassembles SCORWIN.EXE at a virtual address: py -3.13 -I disw.py EXE VA COUNT
import os, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from scorwin_patch import va_to_off

d = open(sys.argv[1], 'rb').read()
va = int(sys.argv[2], 16)
o = va_to_off(d, va)
for n, i in enumerate(Cs(CS_ARCH_X86, CS_MODE_32).disasm(d[o:o + 0x1000], va)):
    if n >= int(sys.argv[3]):
        break
    print('%08x %-24s %s %s' % (i.address, i.bytes.hex(), i.mnemonic, i.op_str))
