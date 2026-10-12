# Saves a window's picture as PNG: py -3.13 -I capwin.py CLASSNAME out.png
# PrintWindow (PW_RENDERFULLCONTENT) works for GDI-drawn windows such as our ddraw.dll's.
import ctypes, sys
from ctypes import wintypes as W
from PIL import Image

u, g = ctypes.WinDLL('user32'), ctypes.WinDLL('gdi32')
u.FindWindowA.restype = W.HWND
hw = u.FindWindowA(sys.argv[1].encode(), None)
if not hw:
    sys.exit('no window ' + sys.argv[1])
r = W.RECT(); u.GetClientRect(hw, ctypes.byref(r))
w, h = r.right, r.bottom
wdc = u.GetDC(hw); mdc = g.CreateCompatibleDC(wdc)
bmp = g.CreateCompatibleBitmap(wdc, w, h); g.SelectObject(mdc, bmp)
u.PrintWindow(hw, mdc, 3)  # PW_CLIENTONLY | PW_RENDERFULLCONTENT
bi = (ctypes.c_uint32 * 10)(40, w, (-h) & 0xffffffff, 1 | (32 << 16), 0, 0, 0, 0, 0, 0)
buf = ctypes.create_string_buffer(w * h * 4)
g.GetDIBits(mdc, bmp, 0, h, buf, bi, 0)
Image.frombuffer('RGBA', (w, h), buf, 'raw', 'BGRA', 0, 1).convert('RGB').save(sys.argv[2])
print('saved', sys.argv[2], w, h)
