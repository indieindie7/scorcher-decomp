# Posts keys to a window (no focus needed): py -3.13 -I sendkeys.py CLASSNAME STEP...
# STEP = VK[:hold_ms] (VK as hex like 0d, or a name: enter up down left right esc space ctrl alt)
#        | wait:MS | shot:FILE (needs capwin.py next to this file)
import ctypes, os, subprocess, sys, time

u = ctypes.WinDLL('user32')
u.FindWindowA.restype = ctypes.c_void_p
NAMES = {'enter': 0x0d, 'esc': 0x1b, 'space': 0x20, 'left': 0x25, 'up': 0x26, 'right': 0x27,
         'down': 0x28, 'ctrl': 0x11, 'alt': 0x12, 'shift': 0x10}
EXT = {0x25, 0x26, 0x27, 0x28}
cls = sys.argv[1]
hw = u.FindWindowA(cls.encode(), None)
if not hw:
    sys.exit('no window ' + cls)
for step in sys.argv[2:]:
    k, _, v = step.partition(':')
    if k == 'wait':
        time.sleep(int(v) / 1000)
    elif k == 'shot':
        subprocess.run([sys.executable, '-I', os.path.join(os.path.dirname(__file__), 'capwin.py'), cls, v])
    else:
        vk = NAMES.get(k) or int(k, 16)
        sc = u.MapVirtualKeyA(vk, 0)
        lp = 1 | (sc << 16) | ((1 << 24) if vk in EXT else 0)
        u.PostMessageA(ctypes.c_void_p(hw), 0x100, vk, lp)
        time.sleep(int(v or 80) / 1000)
        u.PostMessageA(ctypes.c_void_p(hw), 0x101, vk, lp | 0xC0000000)
        time.sleep(0.15)
