# Samples the EIP of every thread of a 32-bit process: py -3.13 -I eips.py EXE [SAMPLES]
import ctypes, ctypes.wintypes as w, subprocess, sys, time

k = ctypes.WinDLL('kernel32')
k.OpenThread.restype = ctypes.c_void_p
k.CreateToolhelp32Snapshot.restype = ctypes.c_void_p


class TE(ctypes.Structure):
    _fields_ = [('size', w.DWORD), ('usage', w.DWORD), ('tid', w.DWORD), ('pid', w.DWORD),
                ('base', w.LONG), ('delta', w.LONG), ('flags', w.DWORD)]


class CTX(ctypes.Structure):   # WOW64_CONTEXT
    _fields_ = [('flags', w.DWORD), ('dr', w.DWORD * 6), ('fs', ctypes.c_byte * 112),
                ('seg', w.DWORD * 4), ('edi', w.DWORD), ('esi', w.DWORD), ('ebx', w.DWORD),
                ('edx', w.DWORD), ('ecx', w.DWORD), ('eax', w.DWORD), ('ebp', w.DWORD),
                ('eip', w.DWORD), ('cs', w.DWORD), ('eflags', w.DWORD), ('esp', w.DWORD),
                ('ss', w.DWORD), ('ext', ctypes.c_byte * 512)]


out = subprocess.run(['tasklist', '/fi', 'imagename eq ' + sys.argv[1], '/fo', 'csv', '/nh'],
                     capture_output=True, text=True).stdout
pid = int(out.split(',')[1].strip('"'))
n = int(sys.argv[2]) if len(sys.argv) > 2 else 5
snap = k.CreateToolhelp32Snapshot(4, 0)
te = TE(); te.size = ctypes.sizeof(TE)
tids = []
ok = k.Thread32First(ctypes.c_void_p(snap), ctypes.byref(te))
while ok:
    if te.pid == pid:
        tids.append(te.tid)
    ok = k.Thread32Next(ctypes.c_void_p(snap), ctypes.byref(te))
for s in range(n):
    row = []
    for t in tids:
        h = k.OpenThread(0x0002 | 0x0008 | 0x0040, False, t)
        c = CTX(); c.flags = 0x10001
        k.Wow64SuspendThread(ctypes.c_void_p(h))
        k.Wow64GetThreadContext(ctypes.c_void_p(h), ctypes.byref(c))
        k.ResumeThread(ctypes.c_void_p(h))
        k.CloseHandle(ctypes.c_void_p(h))
        row.append('%d:%08x' % (t, c.eip))
    print(' '.join(row))
    time.sleep(0.2)
