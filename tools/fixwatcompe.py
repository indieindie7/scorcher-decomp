# Makes an early Watcom Win32 PE (SCORWIN.EXE, 1994 linker 2.18) loadable on modern Windows.
# The linker left every section's VirtualSize at 0 and gave .bss a SizeOfRawData with no file
# data; Windows 95 accepted that, NT refuses ("not a valid application for this platform").
# Each section's VirtualSize becomes the distance to the next section (covering the gaps the NT
# loader does not allow), .bss gets SizeOfRawData 0, the OS version becomes 4.0.
#   py -3.13 fixwatcompe.py IN.EXE OUT.EXE
import struct, sys


def fix(d, log=print):
    pe = struct.unpack_from('<I', d, 0x3C)[0]
    assert d[pe:pe + 4] == b'PE\0\0'
    n = struct.unpack_from('<H', d, pe + 6)[0]
    opt = pe + 24
    image = struct.unpack_from('<I', d, opt + 56)[0]
    struct.pack_into('<HH', d, opt + 40, 4, 0)             # Major/MinorOperatingSystemVersion
    sec = opt + struct.unpack_from('<H', d, pe + 20)[0]
    vas = [struct.unpack_from('<I', d, sec + 40 * i + 12)[0] for i in range(n)] + [image]
    for i in range(n):
        o = sec + 40 * i
        name = bytes(d[o:o + 8]).rstrip(b'\0').decode()
        vs, va, rs, rp, _, _, _, _, ch = struct.unpack_from('<IIIIIIHHI', d, o + 8)
        if rp == 0 and rs:                                  # uninitialised data with a raw size
            rs = 0
            struct.pack_into('<I', d, o + 16, 0)
        vs = vas[i + 1] - va
        struct.pack_into('<I', d, o + 8, vs)
        log('%-8s va %06x vsize %06x raw %06x @ %06x ch %08x' % (name, va, vs, rs, rp, ch))


if __name__ == '__main__':
    d = bytearray(open(sys.argv[1], 'rb').read())
    fix(d)
    open(sys.argv[2], 'wb').write(d)
    print('wrote', sys.argv[2])
