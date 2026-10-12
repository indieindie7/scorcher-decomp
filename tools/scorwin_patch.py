# Builds the playable SCORWIN.EXE for modern Windows from the disc copy:
#   py -3.13 -I tools\scorwin_patch.py pc\cd\SCORCHER\SCORWIN.EXE pcwin\SCORWIN.EXE [--devlog]
# 1. fixwatcompe.fix: the section header fix so Windows 10/11 loads it at all.
# 2. The byte patches below, each checked against the original bytes first.
# Addresses are SCORWIN virtual addresses (BEGTEXT: file offset = va - 0x401000 + 0x400).
import os, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import fixwatcompe

PATCHES = [
    # (the old cd_Init skip at 0x00432670 is gone: pcwin\ddraw.dll now fakes the MCI cdaudio device
    # and plays music\trackNN.wav; to run without music again: 0x00432670 6a00b8bce04300 -> eb219090909090)
    # cd_Init 0x0043265d: the developer bypass (c:\3dzwork\nocd.txt exists -> return 0, no CD
    # audio) taken always: the _lopen call becomes a jump to the "return 0" store. Without it the
    # game demands the disc (MCI cdaudio, track 2 length 0x1a3 s) and quits.
]
# Run as: SCORWIN.EXE l:\scorpc\ <path>\TAGDEN.BIN
# The first word becomes the file-key prefix (0x0043af5f); keys = prefix + "game" + name without
# drive, and TAGDEN's entries are named l:\scorpc\game\... The second word is the TAGDEN path.

# --devlog: the developer log (dbg_Printf, sys_Fatal messages) written to outlort.txt / outlort.err
# in the working folder. dbg_InitDevLogs 0x004317f3 normally opens them only when
# c:\3dzwork\default.pal exists; the strings c:\3dzwork\outlort.* become plain names.
DEVLOG = [
    (0x00431801, '6a00b816e043', 'eb2190909090', 'dbg_InitDevLogs: always open the logs'),
    (0x0043e36e, '633a5c33647a776f726b5c', '6f75746c6f72742e747874', 'log name outlort.txt'),
    (0x0043e379, '6f75746c6f72742e747874', '0000000000000000000000', ''),
    (0x0043e385, '633a5c33647a776f726b5c', '6f75746c6f72742e657272', 'log name outlort.err'),
    (0x0043e390, '6f75746c6f72742e657272', '0000000000000000000000', ''),
]


def va_to_off(d, va):
    import struct
    pe = struct.unpack_from('<I', d, 0x3C)[0]
    n = struct.unpack_from('<H', d, pe + 6)[0]
    base = struct.unpack_from('<I', d, pe + 24 + 28)[0]
    sec = pe + 24 + struct.unpack_from('<H', d, pe + 20)[0]
    for i in range(n):
        vs, rva, rs, rp = struct.unpack_from('<IIII', d, sec + 40 * i + 8)
        if rva <= va - base < rva + rs:
            return va - base - rva + rp
    raise ValueError('va %x not in file' % va)


def main(src, dst, devlog=False):
    d = bytearray(open(src, 'rb').read())
    fixwatcompe.fix(d, log=lambda s: None)
    for va, old, new, what in PATCHES + (DEVLOG if devlog else []):
        o = va_to_off(d, va)
        old, new = bytes.fromhex(old), bytes.fromhex(new)
        assert len(old) == len(new), what
        if d[o:o + len(old)] != old:
            sys.exit('%s: bytes at %08x are %s, not %s' % (what, va, d[o:o + len(old)].hex(), old.hex()))
        d[o:o + len(new)] = new
        if what:
            print('patched %08x %s' % (va, what))
    open(dst, 'wb').write(d)
    print('wrote', dst)


if __name__ == '__main__':
    args = [a for a in sys.argv[1:] if not a.startswith('--')]
    main(args[0], args[1], '--devlog' in sys.argv)
