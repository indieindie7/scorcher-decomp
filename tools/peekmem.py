# Reads memory of a running 32-bit process: py -3.13 -I peekmem.py PROCNAME ADDR[:LEN|s] ...
# ADDR:LEN dumps hex, ADDR:s prints a NUL-terminated string, ADDR:p follows a pointer to a string.
import ctypes, subprocess, sys

k = ctypes.WinDLL('kernel32', use_last_error=True)
name = sys.argv[1]
out = subprocess.run(['tasklist', '/fi', 'imagename eq %s' % name, '/fo', 'csv', '/nh'], capture_output=True, text=True).stdout
pid = int(out.split('","')[1])
h = k.OpenProcess(0x0410, False, pid)


def rd(a, n):
    b = ctypes.create_string_buffer(n); g = ctypes.c_size_t()
    k.ReadProcessMemory(h, ctypes.c_void_p(a), b, n, ctypes.byref(g))
    return b.raw[:g.value]


for arg in sys.argv[2:]:
    a, _, m = arg.partition(':')
    a = int(a, 16)
    if m == 's':
        print(hex(a), rd(a, 200).split(b'\0')[0])
    elif m == 'p':
        p = int.from_bytes(rd(a, 4), 'little'); print(hex(a), '->', hex(p), rd(p, 200).split(b'\0')[0])
    else:
        print(hex(a), rd(a, int(m or '32')).hex())
