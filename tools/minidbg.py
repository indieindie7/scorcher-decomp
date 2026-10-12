# A tiny Win32 debugger: runs a program and logs exceptions (code, address, the 32-bit
# registers through Wow64GetThreadContext), OutputDebugString text, DLL loads and the exit code.
#   py -3.13 -I minidbg.py [--secs N] EXE [ARGS...]      (cwd = the exe's folder)
# First-chance exceptions are passed back to the program (DBG_EXCEPTION_NOT_HANDLED).
import ctypes, os, subprocess, sys, time
from ctypes import wintypes as W

k = ctypes.WinDLL('kernel32', use_last_error=True)
DEBUG_ONLY_THIS_PROCESS = 2
DBG_CONTINUE, DBG_NOT_HANDLED = 0x00010002, 0x80010001


class EXCEPTION_RECORD(ctypes.Structure):
    _fields_ = [('code', W.DWORD), ('flags', W.DWORD), ('rec', ctypes.c_void_p), ('addr', ctypes.c_void_p),
                ('n', W.DWORD), ('info', ctypes.c_size_t * 15)]


class EXC(ctypes.Structure):
    _fields_ = [('rec', EXCEPTION_RECORD), ('first', W.DWORD)]


class ODS(ctypes.Structure):
    _fields_ = [('ptr', ctypes.c_void_p), ('unicode', W.WORD), ('len', W.WORD)]


class LOADDLL(ctypes.Structure):
    _fields_ = [('file', W.HANDLE), ('base', ctypes.c_void_p), ('off', W.DWORD), ('size', W.DWORD),
                ('name', ctypes.c_void_p), ('unicode', W.WORD)]


class U(ctypes.Union):
    _fields_ = [('exc', EXC), ('ods', ODS), ('dll', LOADDLL), ('exitcode', W.DWORD), ('pad', ctypes.c_byte * 160)]


class DEBUG_EVENT(ctypes.Structure):
    _fields_ = [('code', W.DWORD), ('pid', W.DWORD), ('tid', W.DWORD), ('u', U)]


class WOW64_CONTEXT(ctypes.Structure):
    _fields_ = [('flags', W.DWORD), ('dr', W.DWORD * 6), ('fsave', ctypes.c_byte * 112),
                ('gs', W.DWORD), ('fs', W.DWORD), ('es', W.DWORD), ('ds', W.DWORD),
                ('edi', W.DWORD), ('esi', W.DWORD), ('ebx', W.DWORD), ('edx', W.DWORD), ('ecx', W.DWORD),
                ('eax', W.DWORD), ('ebp', W.DWORD), ('eip', W.DWORD), ('cs', W.DWORD), ('eflags', W.DWORD),
                ('esp', W.DWORD), ('ss', W.DWORD), ('ext', ctypes.c_byte * 512)]


def read(h, addr, n):
    buf = ctypes.create_string_buffer(n)
    got = ctypes.c_size_t()
    k.ReadProcessMemory(h, ctypes.c_void_p(addr), buf, n, ctypes.byref(got))
    return buf.raw[:got.value]


def main():
    args = sys.argv[1:]
    secs = 30
    if args[0] == '--secs':
        secs, args = float(args[1]), args[2:]
    exe = os.path.abspath(args[0])
    cmd = subprocess.list2cmdline([exe] + args[1:])
    si = (ctypes.c_byte * 104)(); ctypes.memmove(si, ctypes.byref(ctypes.c_uint32(104)), 4)
    pi = (ctypes.c_size_t * 3)()
    if not k.CreateProcessW(None, ctypes.create_unicode_buffer(cmd), None, None, False, DEBUG_ONLY_THIS_PROCESS,
                            None, os.path.dirname(exe), ctypes.byref(si), ctypes.byref(pi)):
        sys.exit('CreateProcess failed %d' % ctypes.get_last_error())
    hproc = ctypes.c_void_p(pi[0])
    threads = {}
    ev = DEBUG_EVENT()
    end = time.time() + secs
    nexc = 0
    while True:
        if not k.WaitForDebugEvent(ctypes.byref(ev), 500):
            if time.time() > end:
                print('timeout: still running, killing')
                k.TerminateProcess(hproc, 1)
                end = time.time() + 5
            continue
        cont = DBG_CONTINUE
        if ev.code == 1:                                   # EXCEPTION
            r = ev.u.exc.rec
            nexc += 1
            msg = 'exc %08x at %08x first=%d' % (r.code, r.addr or 0, ev.u.exc.first)
            if r.n:
                msg += ' info ' + ' '.join('%x' % r.info[i] for i in range(min(r.n, 3)))
            if r.code not in (0x80000003, 0x4000001f) or nexc < 4:
                th = threads.get(ev.tid)
                if th:
                    c = WOW64_CONTEXT(); c.flags = 0x1001f
                    if k.Wow64GetThreadContext(th, ctypes.byref(c)):
                        msg += '\n   eax %08x ebx %08x ecx %08x edx %08x esi %08x edi %08x ebp %08x esp %08x eip %08x' % (
                            c.eax, c.ebx, c.ecx, c.edx, c.esi, c.edi, c.ebp, c.esp, c.eip)
                        st = read(hproc, c.esp, 64)
                        msg += '\n   stack ' + ' '.join('%08x' % int.from_bytes(st[i:i + 4], 'little') for i in range(0, len(st), 4))
                print(msg)
            if r.code not in (0x80000003, 0x4000001f) and ev.u.exc.first:
                cont = DBG_NOT_HANDLED
        elif ev.code == 2:                                 # CREATE_THREAD
            threads[ev.tid] = ctypes.c_void_p(ctypes.cast(ctypes.byref(ev.u), ctypes.POINTER(ctypes.c_void_p))[0])
        elif ev.code == 3:                                 # CREATE_PROCESS: hFile, hProcess, hThread
            p = ctypes.cast(ctypes.byref(ev.u), ctypes.POINTER(ctypes.c_void_p))
            threads[ev.tid] = ctypes.c_void_p(p[2])
        elif ev.code == 5:
            print('exit code %08x' % ev.u.exitcode)
            break
        elif ev.code == 6:                                 # LOAD_DLL
            d = ev.u.dll
            name = ''
            if d.name:
                pp = read(hproc, d.name, 8)
                ptr = int.from_bytes(pp, 'little')
                if ptr:
                    raw = read(hproc, ptr, 520)
                    name = raw.decode('utf-16-le' if d.unicode else 'latin-1', 'replace').split('\0')[0]
            print('dll %08x %s' % (d.base or 0, name))
        elif ev.code == 8:                                 # OUTPUT_DEBUG_STRING
            o = ev.u.ods
            raw = read(hproc, o.ptr, o.len * (2 if o.unicode else 1))
            print('ods', raw.decode('utf-16-le' if o.unicode else 'latin-1', 'replace').rstrip('\0\n'))
        k.ContinueDebugEvent(ev.pid, ev.tid, cont)


main()
