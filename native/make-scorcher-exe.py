# Makes native\Scorcher\Scorcher.exe: a copy of build\recomp-build\saturn.exe to pin to the taskbar.
# The copy gets the Windows GUI subsystem (no console window), the icon scorcher.ico, the DLLs it
# needs and saturn-args.txt (saturn.exe reads it when started with no arguments). Saves go to
# build\play like play-scorcher.bat. Run again after a rebuild:  py -3.13 make-scorcher-exe.py
import ctypes, os, shutil, struct, subprocess, sys
from ctypes import wintypes

NATIVE = os.path.dirname(os.path.abspath(__file__))
DIST = os.path.join(NATIVE, 'Scorcher')
SRC = os.path.join(NATIVE, 'build', 'recomp-build', 'saturn.exe')
DLLS = [os.path.join(NATIVE, 'build', 'recomp-build', 'SDL3.dll')] + [
    os.path.join('C:/msys64/mingw64/bin', d) for d in ('libgcc_s_seh-1.dll', 'libwinpthread-1.dll', 'libstdc++-6.dll')]
EXE = os.path.join(DIST, 'Scorcher.exe')
ICO = os.path.join(DIST, 'scorcher.ico')
ARGS = '--cue "../../saturn/image/Scorcher.cue" --fullscreen --out ../build/play\n'


def set_gui_subsystem(path):
    with open(path, 'r+b') as f:
        data = f.read(0x400)
        pe = struct.unpack_from('<I', data, 0x3C)[0]
        assert data[pe:pe + 4] == b'PE\0\0', 'not a PE file'
        f.seek(pe + 24 + 68)                       # optional header + Subsystem
        f.write(struct.pack('<H', 2))              # IMAGE_SUBSYSTEM_WINDOWS_GUI


def add_icon(path, ico):
    raw = open(ico, 'rb').read()
    _, _, count = struct.unpack_from('<HHH', raw, 0)
    k = ctypes.WinDLL('kernel32', use_last_error=True)
    k.BeginUpdateResourceW.restype = wintypes.HANDLE
    k.BeginUpdateResourceW.argtypes = [wintypes.LPCWSTR, wintypes.BOOL]
    k.UpdateResourceW.argtypes = [wintypes.HANDLE, ctypes.c_void_p, ctypes.c_void_p, wintypes.WORD,
                                  ctypes.c_void_p, wintypes.DWORD]
    k.EndUpdateResourceW.argtypes = [wintypes.HANDLE, wintypes.BOOL]
    h = k.BeginUpdateResourceW(path, False)
    if not h:
        sys.exit('BeginUpdateResource failed %d' % ctypes.get_last_error())
    group = struct.pack('<HHH', 0, 1, count)
    for i in range(count):
        w, hgt, colors, res, planes, bits, size, off = struct.unpack_from('<BBBBHHII', raw, 6 + 16 * i)
        img = raw[off:off + size]
        if not k.UpdateResourceW(h, 3, i + 1, 0, img, len(img)):          # RT_ICON
            sys.exit('UpdateResource icon failed %d' % ctypes.get_last_error())
        group += struct.pack('<BBBBHHIH', w, hgt, colors, res, planes, bits, size, i + 1)
    if not k.UpdateResourceW(h, 14, 1, 0, group, len(group)):             # RT_GROUP_ICON
        sys.exit('UpdateResource group failed %d' % ctypes.get_last_error())
    if not k.EndUpdateResourceW(h, False):
        sys.exit('EndUpdateResource failed %d' % ctypes.get_last_error())


os.makedirs(DIST, exist_ok=True)
shutil.copy2(SRC, EXE)
subprocess.run(['C:/msys64/mingw64/bin/strip.exe', EXE], check=True)   # UpdateResource breaks the debug sections
for d in DLLS:
    shutil.copy2(d, DIST)
add_icon(EXE, ICO)
set_gui_subsystem(EXE)
with open(os.path.join(DIST, 'saturn-args.txt'), 'w') as f:
    f.write(ARGS)
os.makedirs(os.path.join(NATIVE, 'build', 'play'), exist_ok=True)
if not os.path.exists(os.path.join(NATIVE, 'build', 'play', 'backup.bin')):
    shutil.copy2(os.path.join(NATIVE, 'build', 'run4', 'backup.bin'), os.path.join(NATIVE, 'build', 'play'))
print('made', EXE)
