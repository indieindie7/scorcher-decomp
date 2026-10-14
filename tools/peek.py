# Reads dwords from a running process: py -3.13 -I peek.py EXE ADDR [COUNT]
import ctypes, subprocess, sys

k = ctypes.WinDLL('kernel32')
k.OpenProcess.restype = ctypes.c_void_p
exe, addr = sys.argv[1], int(sys.argv[2], 16)
cnt = int(sys.argv[3]) if len(sys.argv) > 3 else 1
out = subprocess.run(['tasklist', '/fi', 'imagename eq ' + exe, '/fo', 'csv', '/nh'], capture_output=True, text=True).stdout
pid = int(out.split(',')[1].strip('"'))
h = k.OpenProcess(0x10, False, pid)
buf = (ctypes.c_int32 * cnt)()
n = ctypes.c_size_t()
if not k.ReadProcessMemory(ctypes.c_void_p(h), ctypes.c_void_p(addr), buf, cnt * 4, ctypes.byref(n)):
    sys.exit('read failed')
for i in range(cnt):
    print('%08x: %d (0x%x)' % (addr + i * 4, buf[i], buf[i] & 0xffffffff))
