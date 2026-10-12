/* Our own DirectDraw 1 for SCORWIN.EXE (Scorcher, 1995) on modern Windows.
   The game asks for 320x200x8 / 640x480x16 exclusive modes; Windows 11 desktops are 32 bpp.
   Every surface here is a top-down DIB section in system memory; the primary surface is shown
   in the game's window, scaled to 4:3 with StretchDIBits on Unlock / Flip / Blt / palette change.
   Only the IDirectDraw v1 interfaces SCORWIN uses (no QueryInterface for later versions).
   Build: build.bat (MSVC x86). Log: ddraw.log next to the exe when SCORCHER_DDLOG=1. */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#include <ddraw.h>
#include <stdio.h>
#include <stdarg.h>

typedef struct Surf Surf;
typedef struct Pal { void **vt; LONG ref; PALETTEENTRY e[256]; } Pal;
typedef struct Clip { void **vt; LONG ref; HWND hwnd; } Clip;
struct Surf {
    void **vt; LONG ref;
    int w, h, bpp, pitch, primary;
    BYTE *bits; HBITMAP dib; HDC dc; HGDIOBJ olddib;
    Surf *back; Pal *pal; Clip *clip;
    DWORD caps; DDCOLORKEY ck; int hasck;
};
typedef struct DD { void **vt; LONG ref; HWND hwnd; DWORD coop; int w, h, bpp, modeset; } DD;

static DD *g_dd;
static Surf *g_primary;
static FILE *g_log;
static LARGE_INTEGER g_qpf, g_t0;

static void lg(const char *f, ...)
{
    va_list a;
    if (!g_log) return;
    va_start(a, f); vfprintf(g_log, f, a); va_end(a);
    fputc('\n', g_log); fflush(g_log);
}

/* ---------------------------------------------------------------- presenting */
static void client_fit(int cw, int ch, RECT *r)
{
    int w = cw, h = cw * 3 / 4;
    if (h > ch) { h = ch; w = ch * 4 / 3; }
    r->left = (cw - w) / 2; r->top = (ch - h) / 2; r->right = r->left + w; r->bottom = r->top + h;
}

static void present(void)
{
    Surf *s = g_primary;
    HWND hw = g_dd ? g_dd->hwnd : 0;
    struct { BITMAPINFOHEADER h; RGBQUAD c[256]; } bi;
    RECT cr, r;
    HDC dc;
    int i;
    { static int n; if (n++ % 300 == 0) lg("present #%d %p %dx%dx%d", n, s, s ? s->w : 0, s ? s->h : 0, s ? s->bpp : 0); }
    if (!s || !hw || !s->bits) return;
    memset(&bi, 0, sizeof bi);
    bi.h.biSize = sizeof bi.h; bi.h.biWidth = s->pitch / (s->bpp / 8); bi.h.biHeight = -s->h;
    bi.h.biPlanes = 1; bi.h.biBitCount = (WORD)s->bpp;
    if (s->bpp == 8) {
        Pal *p = s->pal;
        for (i = 0; i < 256; i++) if (p) {
            bi.c[i].rgbRed = p->e[i].peRed; bi.c[i].rgbGreen = p->e[i].peGreen; bi.c[i].rgbBlue = p->e[i].peBlue;
        }
    } else if (s->bpp == 16) {
        DWORD *m = (DWORD *)bi.c;
        bi.h.biCompression = BI_BITFIELDS; m[0] = 0xf800; m[1] = 0x07e0; m[2] = 0x001f;
    }
    GetClientRect(hw, &cr);
    if (cr.right <= 0 || cr.bottom <= 0) return;
    client_fit(cr.right, cr.bottom, &r);
    dc = GetDC(hw);
    SetStretchBltMode(dc, COLORONCOLOR);
    StretchDIBits(dc, r.left, r.top, r.right - r.left, r.bottom - r.top, 0, 0, s->w, s->h,
                  s->bits, (BITMAPINFO *)&bi, DIB_RGB_COLORS, SRCCOPY);
    if (r.left > 0) { RECT a = {0, 0, r.left, cr.bottom}, b = {r.right, 0, cr.right, cr.bottom};
        FillRect(dc, &a, GetStockObject(BLACK_BRUSH)); FillRect(dc, &b, GetStockObject(BLACK_BRUSH)); }
    if (r.top > 0) { RECT a = {0, 0, cr.right, r.top}, b = {0, r.bottom, cr.right, cr.bottom};
        FillRect(dc, &a, GetStockObject(BLACK_BRUSH)); FillRect(dc, &b, GetStockObject(BLACK_BRUSH)); }
    ReleaseDC(hw, dc);
}

/* The game's window procedure, wrapped: W A S D also act as the arrow keys (the game's key table
   DAT_004571e0 is indexed by VK), and WM_PAINT shows the last frame again. */
static WNDPROC g_oldproc;
static LRESULT CALLBACK wrap_proc(HWND hw, UINT m, WPARAM wp, LPARAM lp)
{
    if (m == WM_KEYDOWN || m == WM_KEYUP) {
        switch (wp) {
        case 'W': wp = VK_UP; break;
        case 'A': wp = VK_LEFT; break;
        case 'S': wp = VK_DOWN; break;
        case 'D': wp = VK_RIGHT; break;
        }
    } else if (m == WM_PAINT) {
        PAINTSTRUCT ps;
        BeginPaint(hw, &ps); EndPaint(hw, &ps);
        present();
        return 0;
    } else if (m == WM_ERASEBKGND)
        return 1;
    return CallWindowProcA(g_oldproc, hw, m, wp, lp);
}

/* Window for a mode: a normal captioned window, 4:3 client, centred on the work area. */
static void size_window(int w, int h)
{
    HWND hw = g_dd ? g_dd->hwnd : 0;
    RECT wa, r;
    int cw, ch, scale;
    char env[16];
    if (!hw) return;
    SystemParametersInfoA(SPI_GETWORKAREA, 0, &wa, 0);
    scale = (w <= 320) ? 3 : 2;
    if (GetEnvironmentVariableA("SCORCHER_SCALE", env, sizeof env)) scale = atoi(env);
    cw = (w <= 320 ? 320 : 640) * scale / (w <= 320 ? 1 : 2);
    if (w > 320) cw = 640 * scale / 2 * 1;
    ch = cw * 3 / 4;
    while ((ch > (wa.bottom - wa.top) * 9 / 10 || cw > (wa.right - wa.left) * 9 / 10) && cw > 320) { cw -= 64; ch = cw * 3 / 4; }
    SetWindowLongA(hw, GWL_STYLE, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_THICKFRAME | WS_VISIBLE);
    r.left = 0; r.top = 0; r.right = cw; r.bottom = ch;
    AdjustWindowRect(&r, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME, FALSE);
    if (GetEnvironmentVariableA("SCORCHER_PARK", env, sizeof env)) {
        /* test runs: off-screen (left of the virtual screen) and never activated */
        SetWindowPos(hw, HWND_BOTTOM, GetSystemMetrics(SM_XVIRTUALSCREEN) - (r.right - r.left) - 200, 0,
                     r.right - r.left, r.bottom - r.top, SWP_FRAMECHANGED | SWP_NOACTIVATE);
    } else {
        /* activated: DirectSound mutes a window that does not have the focus */
        SetWindowPos(hw, HWND_NOTOPMOST, wa.left + ((wa.right - wa.left) - (r.right - r.left)) / 2,
                     wa.top + ((wa.bottom - wa.top) - (r.bottom - r.top)) / 2, r.right - r.left, r.bottom - r.top,
                     SWP_FRAMECHANGED);
        SetForegroundWindow(hw);
    }
    lg("window %dx%d client for mode %dx%d", cw, ch, w, h);
}

/* ---------------------------------------------------------------- surfaces */
extern void *surf_vt[];
static Surf *surf_new(int w, int h, int bpp, DWORD caps)
{
    struct { BITMAPINFOHEADER h; RGBQUAD c[256]; } bi;
    Surf *s = (Surf *)calloc(1, sizeof *s);
    int i;
    s->vt = surf_vt; s->ref = 1; s->w = w; s->h = h; s->bpp = bpp; s->caps = caps;
    memset(&bi, 0, sizeof bi);
    bi.h.biSize = sizeof bi.h; bi.h.biWidth = w; bi.h.biHeight = -h; bi.h.biPlanes = 1; bi.h.biBitCount = (WORD)bpp;
    if (bpp == 8) for (i = 0; i < 256; i++) bi.c[i].rgbRed = bi.c[i].rgbGreen = bi.c[i].rgbBlue = (BYTE)i;
    if (bpp == 16) { DWORD *m = (DWORD *)bi.c; bi.h.biCompression = BI_BITFIELDS; m[0] = 0xf800; m[1] = 0x07e0; m[2] = 0x001f; }
    s->dib = CreateDIBSection(0, (BITMAPINFO *)&bi, DIB_RGB_COLORS, (void **)&s->bits, 0, 0);
    s->pitch = ((w * (bpp / 8)) + 3) & ~3;
    if (s->bits) memset(s->bits, 0, s->pitch * h);
    lg("surface %p %dx%dx%d caps %08x", s, w, h, bpp, caps);
    return s;
}

static void fill_pf(DDPIXELFORMAT *pf, int bpp)
{
    memset(pf, 0, sizeof *pf);
    pf->dwSize = sizeof *pf;
    pf->dwRGBBitCount = bpp;
    if (bpp == 8) pf->dwFlags = DDPF_RGB | DDPF_PALETTEINDEXED8;
    else { pf->dwFlags = DDPF_RGB; pf->dwRBitMask = 0xf800; pf->dwGBitMask = 0x07e0; pf->dwBBitMask = 0x001f; }
}

static void fill_desc(Surf *s, DDSURFACEDESC *d)
{
    DWORD size = d->dwSize ? d->dwSize : sizeof *d;
    memset(d, 0, size);
    d->dwSize = size;
    d->dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PITCH | DDSD_PIXELFORMAT;
    d->dwWidth = s->w; d->dwHeight = s->h; d->lPitch = s->pitch;
    d->ddsCaps.dwCaps = s->caps;
    if (s->back) { d->dwFlags |= DDSD_BACKBUFFERCOUNT; d->dwBackBufferCount = 1; }
    fill_pf(&d->ddpfPixelFormat, s->bpp);
}

static ULONG WINAPI s_AddRef(Surf *s) { return ++s->ref; }
static ULONG WINAPI s_Release(Surf *s)
{
    if (--s->ref > 0) return s->ref;
    lg("surface %p released", s);
    if (s == g_primary) g_primary = 0;
    if (s->back) { Surf *b = s->back; s->back = 0; s_Release(b); }
    if (s->dc) { SelectObject(s->dc, s->olddib); DeleteDC(s->dc); }
    if (s->dib) DeleteObject(s->dib);
    free(s);
    return 0;
}
static HRESULT WINAPI s_QI(Surf *s, REFIID r, void **o) { lg("surface QI"); *o = 0; return E_NOINTERFACE; }
static HRESULT WINAPI s_AddAttached(Surf *s, Surf *a) { if (!s->back) { s->back = a; a->ref++; } return DD_OK; }
static HRESULT WINAPI s_AddOverlayDirtyRect(Surf *s, LPRECT r) { return DDERR_UNSUPPORTED; }

static int clip_rect(Surf *s, RECT *r)
{
    if (r->left < 0) r->left = 0;
    if (r->top < 0) r->top = 0;
    if (r->right > s->w) r->right = s->w;
    if (r->bottom > s->h) r->bottom = s->h;
    return r->right > r->left && r->bottom > r->top;
}

static void do_fill(Surf *d, RECT *r, DWORD c)
{
    int x, y, bp = d->bpp / 8;
    for (y = r->top; y < r->bottom; y++) {
        BYTE *p = d->bits + y * d->pitch + r->left * bp;
        if (bp == 1) memset(p, (BYTE)c, r->right - r->left);
        else if (bp == 2) for (x = r->left; x < r->right; x++, p += 2) *(WORD *)p = (WORD)c;
        else for (x = r->left; x < r->right; x++, p += 4) *(DWORD *)p = c;
    }
}

/* Copy (with nearest-neighbour stretch and an optional source colour key). */
static void do_copy(Surf *d, RECT *dr, Surf *s, RECT *sr, int key)
{
    int bp = d->bpp / 8, dw = dr->right - dr->left, dh = dr->bottom - dr->top;
    int sw = sr->right - sr->left, sh = sr->bottom - sr->top, x, y;
    DWORD kl = s->ck.dwColorSpaceLowValue, kh = s->ck.dwColorSpaceHighValue;
    if (dw <= 0 || dh <= 0 || sw <= 0 || sh <= 0 || s->bpp != d->bpp) return;
    if (!key && dw == sw && dh == sh) {
        if (s == d && sr->top < dr->top) {
            for (y = dh - 1; y >= 0; y--)
                memmove(d->bits + (dr->top + y) * d->pitch + dr->left * bp, s->bits + (sr->top + y) * s->pitch + sr->left * bp, dw * bp);
        } else
            for (y = 0; y < dh; y++)
                memmove(d->bits + (dr->top + y) * d->pitch + dr->left * bp, s->bits + (sr->top + y) * s->pitch + sr->left * bp, dw * bp);
        return;
    }
    for (y = 0; y < dh; y++) {
        BYTE *dp = d->bits + (dr->top + y) * d->pitch + dr->left * bp;
        BYTE *sp = s->bits + (sr->top + y * sh / dh) * s->pitch;
        for (x = 0; x < dw; x++, dp += bp) {
            int sx = sr->left + x * sw / dw;
            DWORD v = bp == 1 ? sp[sx] : bp == 2 ? ((WORD *)sp)[sx] : ((DWORD *)sp)[sx];
            if (key && v >= kl && v <= kh) continue;
            if (bp == 1) *dp = (BYTE)v; else if (bp == 2) *(WORD *)dp = (WORD)v; else *(DWORD *)dp = v;
        }
    }
}

static HRESULT WINAPI s_Blt(Surf *d, LPRECT dr0, Surf *s, LPRECT sr0, DWORD fl, LPDDBLTFX fx)
{
    RECT dr, sr;
    if (dr0) dr = *dr0; else SetRect(&dr, 0, 0, d->w, d->h);
    if (fl & DDBLT_COLORFILL) {
        if (clip_rect(d, &dr)) do_fill(d, &dr, fx ? fx->dwFillColor : 0);
    } else if (s) {
        if (sr0) sr = *sr0; else SetRect(&sr, 0, 0, s->w, s->h);
        if (dr.right - dr.left == sr.right - sr.left && dr.bottom - dr.top == sr.bottom - sr.top) {
            /* same size: clip both together */
            if (dr.left < 0) { sr.left -= dr.left; dr.left = 0; }
            if (dr.top < 0) { sr.top -= dr.top; dr.top = 0; }
            if (dr.right > d->w) { sr.right -= dr.right - d->w; dr.right = d->w; }
            if (dr.bottom > d->h) { sr.bottom -= dr.bottom - d->h; dr.bottom = d->h; }
        }
        if (clip_rect(d, &dr) && clip_rect(s, &sr)) do_copy(d, &dr, s, &sr, (fl & DDBLT_KEYSRC) != 0);
    }
    if (d->primary) present();
    return DD_OK;
}
static HRESULT WINAPI s_BltBatch(Surf *s, LPDDBLTBATCH b, DWORD n, DWORD f) { return DDERR_UNSUPPORTED; }
static HRESULT WINAPI s_BltFast(Surf *d, DWORD x, DWORD y, Surf *s, LPRECT sr0, DWORD tr)
{
    RECT sr, dr;
    if (!s) return DDERR_INVALIDPARAMS;
    if (sr0) sr = *sr0; else SetRect(&sr, 0, 0, s->w, s->h);
    SetRect(&dr, x, y, x + sr.right - sr.left, y + sr.bottom - sr.top);
    if (dr.right > d->w) { sr.right -= dr.right - d->w; dr.right = d->w; }
    if (dr.bottom > d->h) { sr.bottom -= dr.bottom - d->h; dr.bottom = d->h; }
    if (clip_rect(s, &sr) && clip_rect(d, &dr)) do_copy(d, &dr, s, &sr, (tr & DDBLTFAST_SRCCOLORKEY) != 0);
    if (d->primary) present();
    return DD_OK;
}
static HRESULT WINAPI s_DeleteAttached(Surf *s, DWORD f, Surf *a)
{
    if (s->back && (!a || a == s->back)) { Surf *b = s->back; s->back = 0; s_Release(b); }
    return DD_OK;
}
static HRESULT WINAPI s_EnumAttached(Surf *s, LPVOID ctx, LPDDENUMSURFACESCALLBACK cb)
{
    if (s->back) { DDSURFACEDESC d; d.dwSize = sizeof d; fill_desc(s->back, &d); s->back->ref++;
        cb((LPDIRECTDRAWSURFACE)s->back, &d, ctx); }
    return DD_OK;
}
static HRESULT WINAPI s_EnumOverlayZ(Surf *s, DWORD f, LPVOID c, LPDDENUMSURFACESCALLBACK cb) { return DD_OK; }
static HRESULT WINAPI s_Flip(Surf *s, Surf *t, DWORD f)
{
    Surf *b = t ? t : s->back;
    if (b && b->bits && s->bits && b->pitch == s->pitch && b->h == s->h) {
        /* swap the buffers like a real flip */
        BYTE *bits = s->bits; HBITMAP dib = s->dib; HDC dc = s->dc; HGDIOBJ od = s->olddib;
        s->bits = b->bits; s->dib = b->dib; s->dc = b->dc; s->olddib = b->olddib;
        b->bits = bits; b->dib = dib; b->dc = dc; b->olddib = od;
    }
    if (s->primary) present();
    return DD_OK;
}
static HRESULT WINAPI s_GetAttached(Surf *s, LPDDSCAPS c, Surf **o)
{
    if (!s->back) { *o = 0; return DDERR_NOTFOUND; }
    s->back->ref++; *o = s->back; return DD_OK;
}
static HRESULT WINAPI s_GetBltStatus(Surf *s, DWORD f) { return DD_OK; }
static HRESULT WINAPI s_GetCaps(Surf *s, LPDDSCAPS c) { c->dwCaps = s->caps; return DD_OK; }
static HRESULT WINAPI s_GetClipper(Surf *s, Clip **c) { *c = s->clip; if (s->clip) s->clip->ref++; return s->clip ? DD_OK : DDERR_NOCLIPPERATTACHED; }
static HRESULT WINAPI s_GetColorKey(Surf *s, DWORD f, LPDDCOLORKEY k) { if (!s->hasck) return DDERR_NOCOLORKEY; *k = s->ck; return DD_OK; }
static HRESULT WINAPI s_GetDC(Surf *s, HDC *o)
{
    if (!s->dc) { s->dc = CreateCompatibleDC(0); s->olddib = SelectObject(s->dc, s->dib); }
    if (s->bpp == 8 && s->pal) {
        RGBQUAD c[256]; int i;
        for (i = 0; i < 256; i++) { c[i].rgbRed = s->pal->e[i].peRed; c[i].rgbGreen = s->pal->e[i].peGreen; c[i].rgbBlue = s->pal->e[i].peBlue; c[i].rgbReserved = 0; }
        SetDIBColorTable(s->dc, 0, 256, c);
    }
    GdiFlush();
    *o = s->dc; return DD_OK;
}
static HRESULT WINAPI s_GetFlipStatus(Surf *s, DWORD f) { return DD_OK; }
static HRESULT WINAPI s_GetOverlayPos(Surf *s, LPLONG x, LPLONG y) { return DDERR_NOTAOVERLAYSURFACE; }
static HRESULT WINAPI s_GetPalette(Surf *s, Pal **p) { *p = s->pal; if (s->pal) s->pal->ref++; return s->pal ? DD_OK : DDERR_NOPALETTEATTACHED; }
static HRESULT WINAPI s_GetPixelFormat(Surf *s, LPDDPIXELFORMAT pf) { fill_pf(pf, s->bpp); return DD_OK; }
static HRESULT WINAPI s_GetSurfaceDesc(Surf *s, LPDDSURFACEDESC d) { fill_desc(s, d); return DD_OK; }
static HRESULT WINAPI s_Initialize(Surf *s, LPDIRECTDRAW dd, LPDDSURFACEDESC d) { return DDERR_ALREADYINITIALIZED; }
static HRESULT WINAPI s_IsLost(Surf *s) { return DD_OK; }
static HRESULT WINAPI s_Lock(Surf *s, LPRECT r, LPDDSURFACEDESC d, DWORD f, HANDLE e)
{
    { static int n; if (n++ < 6 || n % 1000 == 0) lg("Lock %p #%d flags %x", s, n, f); }
    fill_desc(s, d);
    d->dwFlags |= DDSD_LPSURFACE;
    d->lpSurface = s->bits + (r ? r->top * s->pitch + r->left * (s->bpp / 8) : 0);
    GdiFlush();
    return DD_OK;
}
static HRESULT WINAPI s_ReleaseDC(Surf *s, HDC dc) { GdiFlush(); if (s->primary) present(); return DD_OK; }
static HRESULT WINAPI s_Restore(Surf *s) { return DD_OK; }
static HRESULT WINAPI s_SetClipper(Surf *s, Clip *c) { s->clip = c; return DD_OK; }
static HRESULT WINAPI s_SetColorKey(Surf *s, DWORD f, LPDDCOLORKEY k) { if (k) { s->ck = *k; s->hasck = 1; } else s->hasck = 0; return DD_OK; }
static HRESULT WINAPI s_SetOverlayPos(Surf *s, LONG x, LONG y) { return DDERR_NOTAOVERLAYSURFACE; }
static HRESULT WINAPI s_SetPalette(Surf *s, Pal *p)
{
    if (p) p->ref++;
    s->pal = p;
    if (s->back) s->back->pal = p;
    if (s->primary) present();
    return DD_OK;
}
static HRESULT WINAPI s_Unlock(Surf *s, LPVOID p) { if (s->primary) present(); return DD_OK; }
static HRESULT WINAPI s_UpdateOverlay(Surf *s, LPRECT a, Surf *d, LPRECT b, DWORD f, LPDDOVERLAYFX fx) { return DDERR_UNSUPPORTED; }
static HRESULT WINAPI s_UpdateOverlayDisplay(Surf *s, DWORD f) { return DDERR_UNSUPPORTED; }
static HRESULT WINAPI s_UpdateOverlayZ(Surf *s, DWORD f, Surf *r) { return DDERR_UNSUPPORTED; }

void *surf_vt[] = {
    s_QI, s_AddRef, s_Release, s_AddAttached, s_AddOverlayDirtyRect, s_Blt, s_BltBatch, s_BltFast,
    s_DeleteAttached, s_EnumAttached, s_EnumOverlayZ, s_Flip, s_GetAttached, s_GetBltStatus, s_GetCaps,
    s_GetClipper, s_GetColorKey, s_GetDC, s_GetFlipStatus, s_GetOverlayPos, s_GetPalette, s_GetPixelFormat,
    s_GetSurfaceDesc, s_Initialize, s_IsLost, s_Lock, s_ReleaseDC, s_Restore, s_SetClipper, s_SetColorKey,
    s_SetOverlayPos, s_SetPalette, s_Unlock, s_UpdateOverlay, s_UpdateOverlayDisplay, s_UpdateOverlayZ,
};

/* ---------------------------------------------------------------- palette, clipper */
static HRESULT WINAPI p_QI(Pal *p, REFIID r, void **o) { *o = 0; return E_NOINTERFACE; }
static ULONG WINAPI p_AddRef(Pal *p) { return ++p->ref; }
static ULONG WINAPI p_Release(Pal *p) { if (--p->ref > 0) return p->ref; free(p); return 0; }
static HRESULT WINAPI p_GetCaps(Pal *p, LPDWORD c) { *c = DDPCAPS_8BIT | DDPCAPS_ALLOW256; return DD_OK; }
static HRESULT WINAPI p_GetEntries(Pal *p, DWORD f, DWORD b, DWORD n, LPPALETTEENTRY e)
{ if (b + n > 256) return DDERR_INVALIDPARAMS; memcpy(e, p->e + b, n * sizeof *e); return DD_OK; }
static HRESULT WINAPI p_Initialize(Pal *p, LPDIRECTDRAW d, DWORD f, LPPALETTEENTRY e) { return DDERR_ALREADYINITIALIZED; }
static HRESULT WINAPI p_SetEntries(Pal *p, DWORD f, DWORD b, DWORD n, LPPALETTEENTRY e)
{
    if (b + n > 256) return DDERR_INVALIDPARAMS;
    memcpy(p->e + b, e, n * sizeof *e);
    if (g_primary && g_primary->pal == p) present();
    return DD_OK;
}
static void *pal_vt[] = { p_QI, p_AddRef, p_Release, p_GetCaps, p_GetEntries, p_Initialize, p_SetEntries };

static HRESULT WINAPI c_QI(Clip *c, REFIID r, void **o) { *o = 0; return E_NOINTERFACE; }
static ULONG WINAPI c_AddRef(Clip *c) { return ++c->ref; }
static ULONG WINAPI c_Release(Clip *c) { if (--c->ref > 0) return c->ref; free(c); return 0; }
static HRESULT WINAPI c_GetClipList(Clip *c, LPRECT r, LPRGNDATA d, LPDWORD n) { return DDERR_UNSUPPORTED; }
static HRESULT WINAPI c_GetHWnd(Clip *c, HWND *h) { *h = c->hwnd; return DD_OK; }
static HRESULT WINAPI c_Initialize(Clip *c, LPDIRECTDRAW d, DWORD f) { return DDERR_ALREADYINITIALIZED; }
static HRESULT WINAPI c_IsClipListChanged(Clip *c, BOOL *b) { *b = FALSE; return DD_OK; }
static HRESULT WINAPI c_SetClipList(Clip *c, LPRGNDATA d, DWORD f) { return DD_OK; }
static HRESULT WINAPI c_SetHWnd(Clip *c, DWORD f, HWND h) { c->hwnd = h; return DD_OK; }
static void *clip_vt[] = { c_QI, c_AddRef, c_Release, c_GetClipList, c_GetHWnd, c_Initialize, c_IsClipListChanged, c_SetClipList, c_SetHWnd };

/* ---------------------------------------------------------------- IDirectDraw */
static const int modes[][3] = { {320,200,8}, {320,240,8}, {640,400,8}, {640,480,8}, {640,400,16}, {640,480,16} };

static HRESULT WINAPI d_QI(DD *d, REFIID r, void **o) { lg("dd QI"); *o = 0; return E_NOINTERFACE; }
static ULONG WINAPI d_AddRef(DD *d) { return ++d->ref; }
static ULONG WINAPI d_Release(DD *d) { if (--d->ref > 0) return d->ref; lg("dd released"); g_dd = 0; free(d); return 0; }
static HRESULT WINAPI d_Compact(DD *d) { return DD_OK; }
static HRESULT WINAPI d_CreateClipper(DD *d, DWORD f, Clip **o, IUnknown *u)
{ Clip *c = (Clip *)calloc(1, sizeof *c); c->vt = clip_vt; c->ref = 1; *o = c; return DD_OK; }
static HRESULT WINAPI d_CreatePalette(DD *d, DWORD f, LPPALETTEENTRY e, Pal **o, IUnknown *u)
{
    Pal *p = (Pal *)calloc(1, sizeof *p);
    p->vt = pal_vt; p->ref = 1;
    if (e) memcpy(p->e, e, sizeof p->e);
    *o = p; lg("palette %p flags %x", p, f); return DD_OK;
}
static HRESULT WINAPI d_CreateSurface(DD *d, LPDDSURFACEDESC desc, Surf **o, IUnknown *u)
{
    DWORD caps = (desc->dwFlags & DDSD_CAPS) ? desc->ddsCaps.dwCaps : 0;
    Surf *s;
    if (caps & DDSCAPS_PRIMARYSURFACE) {
        s = surf_new(d->w, d->h, d->bpp, caps | DDSCAPS_VISIBLE | DDSCAPS_FRONTBUFFER);
        s->primary = 1;
        if ((desc->dwFlags & DDSD_BACKBUFFERCOUNT) && desc->dwBackBufferCount)
            s->back = surf_new(d->w, d->h, d->bpp, DDSCAPS_BACKBUFFER | DDSCAPS_FLIP | DDSCAPS_COMPLEX | (caps & DDSCAPS_SYSTEMMEMORY));
        g_primary = s;
    } else {
        int bpp = d->bpp;
        if ((desc->dwFlags & DDSD_PIXELFORMAT) && desc->ddpfPixelFormat.dwRGBBitCount) bpp = desc->ddpfPixelFormat.dwRGBBitCount;
        s = surf_new(desc->dwWidth, desc->dwHeight, bpp, caps | DDSCAPS_OFFSCREENPLAIN);
    }
    lg("CreateSurface flags %08x caps %08x %dx%d -> %p back %p", desc->dwFlags, caps, desc->dwWidth, desc->dwHeight, s, s->back);
    *o = s; return s->bits ? DD_OK : DDERR_OUTOFMEMORY;
}
static HRESULT WINAPI d_DuplicateSurface(DD *d, Surf *s, Surf **o) { return DDERR_UNSUPPORTED; }
static HRESULT WINAPI d_EnumDisplayModes(DD *d, DWORD f, LPDDSURFACEDESC want, LPVOID ctx, LPDDENUMMODESCALLBACK cb)
{
    int i;
    for (i = 0; i < sizeof modes / sizeof modes[0]; i++) {
        DDSURFACEDESC m;
        memset(&m, 0, sizeof m); m.dwSize = sizeof m;
        m.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_PITCH | DDSD_PIXELFORMAT | DDSD_REFRESHRATE;
        m.dwWidth = modes[i][0]; m.dwHeight = modes[i][1]; m.lPitch = modes[i][0] * modes[i][2] / 8; m.dwRefreshRate = 60;
        fill_pf(&m.ddpfPixelFormat, modes[i][2]);
        if (want && (want->dwFlags & DDSD_WIDTH) && want->dwWidth != m.dwWidth) continue;
        if (want && (want->dwFlags & DDSD_HEIGHT) && want->dwHeight != m.dwHeight) continue;
        if (cb(&m, ctx) == DDENUMRET_CANCEL) break;
    }
    return DD_OK;
}
static HRESULT WINAPI d_EnumSurfaces(DD *d, DWORD f, LPDDSURFACEDESC s, LPVOID c, LPDDENUMSURFACESCALLBACK cb) { return DD_OK; }
static HRESULT WINAPI d_FlipToGDISurface(DD *d) { return DD_OK; }
static HRESULT WINAPI d_GetCaps(DD *d, LPDDCAPS a, LPDDCAPS b)
{
    LPDDCAPS c[2]; int i;
    c[0] = a; c[1] = b;
    for (i = 0; i < 2; i++) if (c[i]) {
        DWORD size = c[i]->dwSize ? c[i]->dwSize : sizeof(DDCAPS_DX1);
        memset(c[i], 0, size); c[i]->dwSize = size;
        c[i]->dwCaps = DDCAPS_BLT | DDCAPS_BLTCOLORFILL | DDCAPS_BLTSTRETCH | DDCAPS_COLORKEY | DDCAPS_PALETTE;
        c[i]->dwCKeyCaps = DDCKEYCAPS_SRCBLT;
        c[i]->dwPalCaps = DDPCAPS_8BIT;
        c[i]->dwVidMemTotal = c[i]->dwVidMemFree = 16 << 20;
        c[i]->ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_BACKBUFFER | DDSCAPS_FLIP | DDSCAPS_OFFSCREENPLAIN | DDSCAPS_PALETTE | DDSCAPS_SYSTEMMEMORY | DDSCAPS_VIDEOMEMORY;
    }
    return DD_OK;
}
static HRESULT WINAPI d_GetDisplayMode(DD *d, LPDDSURFACEDESC m)
{
    DWORD size = m->dwSize ? m->dwSize : sizeof *m;
    memset(m, 0, size); m->dwSize = size;
    m->dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_PITCH | DDSD_PIXELFORMAT | DDSD_REFRESHRATE;
    m->dwWidth = d->w; m->dwHeight = d->h; m->lPitch = d->w * d->bpp / 8; m->dwRefreshRate = 60;
    fill_pf(&m->ddpfPixelFormat, d->bpp);
    return DD_OK;
}
static HRESULT WINAPI d_GetFourCC(DD *d, LPDWORD n, LPDWORD c) { *n = 0; return DD_OK; }
static HRESULT WINAPI d_GetGDISurface(DD *d, Surf **o) { *o = g_primary; if (g_primary) g_primary->ref++; return g_primary ? DD_OK : DDERR_NOTFOUND; }
static HRESULT WINAPI d_GetMonitorFrequency(DD *d, LPDWORD f) { *f = 60; return DD_OK; }
static double now_frames(void)
{
    LARGE_INTEGER t; QueryPerformanceCounter(&t);
    return (double)(t.QuadPart - g_t0.QuadPart) * 60.0 / (double)g_qpf.QuadPart;
}
static HRESULT WINAPI d_GetScanLine(DD *d, LPDWORD l)
{
    double f = now_frames(); f -= (double)(LONGLONG)f;
    *l = (DWORD)(f * 525);
    return *l >= (DWORD)d->h ? DDERR_VERTICALBLANKINPROGRESS : DD_OK;
}
static HRESULT WINAPI d_GetVBStatus(DD *d, BOOL *b) { double f = now_frames(); f -= (double)(LONGLONG)f; *b = f > 0.92; return DD_OK; }
static HRESULT WINAPI d_Initialize(DD *d, GUID *g) { return DDERR_ALREADYINITIALIZED; }
static HRESULT WINAPI d_RestoreDisplayMode(DD *d) { return DD_OK; }
static HRESULT WINAPI d_SetCooperativeLevel(DD *d, HWND h, DWORD f)
{
    lg("SetCooperativeLevel hwnd %p flags %x", h, f);
    if (h) d->hwnd = h;
    if (h && !g_oldproc) g_oldproc = (WNDPROC)SetWindowLongPtrA(h, GWLP_WNDPROC, (LONG_PTR)wrap_proc);
    d->coop = f;
    return DD_OK;
}
static HRESULT WINAPI d_SetDisplayMode(DD *d, DWORD w, DWORD h, DWORD bpp)
{
    lg("SetDisplayMode %dx%dx%d", w, h, bpp);
    if (bpp != 8 && bpp != 16) return DDERR_INVALIDMODE;
    d->w = w; d->h = h; d->bpp = bpp; d->modeset = 1;
    size_window(w, h);
    return DD_OK;
}
static HRESULT WINAPI d_WaitForVerticalBlank(DD *d, DWORD f, HANDLE e)
{
    /* wait for the next 60 Hz boundary */
    double f0 = now_frames(), target = (double)(LONGLONG)f0 + 1.0;
    while (now_frames() < target) {
        double left = (target - now_frames()) * 1000.0 / 60.0;
        if (left > 2.0) Sleep((DWORD)(left - 1.5)); else Sleep(0);
    }
    return DD_OK;
}
static void *dd_vt[] = {
    d_QI, d_AddRef, d_Release, d_Compact, d_CreateClipper, d_CreatePalette, d_CreateSurface,
    d_DuplicateSurface, d_EnumDisplayModes, d_EnumSurfaces, d_FlipToGDISurface, d_GetCaps,
    d_GetDisplayMode, d_GetFourCC, d_GetGDISurface, d_GetMonitorFrequency, d_GetScanLine,
    d_GetVBStatus, d_Initialize, d_RestoreDisplayMode, d_SetCooperativeLevel, d_SetDisplayMode,
    d_WaitForVerticalBlank,
};

HRESULT WINAPI DirectDrawCreate(GUID *g, DD **o, IUnknown *u)
{
    char env[8], path[MAX_PATH], *p;
    if (!g_log && GetEnvironmentVariableA("SCORCHER_DDLOG", env, sizeof env)) {
        GetModuleFileNameA(0, path, MAX_PATH);
        p = strrchr(path, '\\'); if (p) strcpy(p + 1, "ddraw.log");
        g_log = fopen(path, "w");
    }
    QueryPerformanceFrequency(&g_qpf); QueryPerformanceCounter(&g_t0);
    timeBeginPeriod(1);
    g_dd = (DD *)calloc(1, sizeof *g_dd);
    g_dd->vt = dd_vt; g_dd->ref = 1; g_dd->w = 640; g_dd->h = 480; g_dd->bpp = 16;
    lg("DirectDrawCreate -> %p", g_dd);
    *o = g_dd;
    return DD_OK;
}

/* ---------------------------------------------------------------- CD audio from WAV files
   SCORWIN plays its music from the CD through MCI "cdaudio" (cd_Init 0x0043265d, cd_MciPlayTrack
   0x00432864). The exe's mciSendCommandA import is pointed here; a cdaudio device is faked and
   PLAY opens music\trackNN.wav (tools\ripaudio.py) with the real MCI waveaudio device, notifying
   the game's window when the track ends like the CD would (the game then loops it). */
#define FAKE_CD 0x4344
static MCIERROR (WINAPI *real_mci)(MCIDEVICEID, UINT, DWORD_PTR, DWORD_PTR);
static MCIDEVICEID g_wav;
static int g_cdtrack, g_cdplaying;

static void wav_path(int track, char *p)
{
    char *s;
    GetModuleFileNameA(0, p, MAX_PATH);
    s = strrchr(p, '\\'); if (!s) s = p - 1;
    sprintf(s + 1, "music\\track%02d.wav", track);
}
static int track_seconds(int track)
{
    /* WAV data length + the 2 s pregap = the length MCI reports for the CD track */
    char p[MAX_PATH]; WIN32_FILE_ATTRIBUTE_DATA a;
    wav_path(track, p);
    if (!GetFileAttributesExA(p, GetFileExInfoStandard, &a)) return 0;
    return (int)((a.nFileSizeLow - 44) / 176400) + 2;
}
static void wav_close(void)
{
    if (g_wav) { real_mci(g_wav, MCI_CLOSE, 0, 0); g_wav = 0; }
    g_cdplaying = 0;
}
static MCIERROR WINAPI fake_mci(MCIDEVICEID id, UINT msg, DWORD_PTR fl, DWORD_PTR prm)
{
    if (msg == MCI_OPEN && (fl & MCI_OPEN_TYPE) && !(fl & MCI_OPEN_TYPE_ID)) {
        MCI_OPEN_PARMSA *o = (MCI_OPEN_PARMSA *)prm;
        if (o->lpstrDeviceType && !lstrcmpiA(o->lpstrDeviceType, "cdaudio")) {
            o->wDeviceID = FAKE_CD; lg("cd open"); return 0;
        }
    }
    if (id != FAKE_CD) return real_mci(id, msg, fl, prm);
    switch (msg) {
    case MCI_STATUS: {
        MCI_STATUS_PARMS *s = (MCI_STATUS_PARMS *)prm;
        if (!(fl & MCI_STATUS_ITEM)) return MCIERR_MISSING_PARAMETER;
        switch (s->dwItem) {
        case MCI_STATUS_LENGTH: {
            int t = (fl & MCI_TRACK) ? (int)s->dwTrack : 2, sec = track_seconds(t);
            s->dwReturn = MCI_MAKE_MSF(sec / 60, sec % 60, 0);
            lg("cd length track %d = %d s", t, sec);
            return sec ? 0 : MCIERR_OUTOFRANGE;
        }
        case MCI_STATUS_NUMBER_OF_TRACKS: s->dwReturn = 9; return 0;
        case MCI_STATUS_MODE: s->dwReturn = g_cdplaying ? MCI_MODE_PLAY : MCI_MODE_STOP; return 0;
        case MCI_STATUS_READY: case MCI_STATUS_MEDIA_PRESENT: s->dwReturn = TRUE; return 0;
        case MCI_STATUS_CURRENT_TRACK: s->dwReturn = g_cdtrack; return 0;
        case MCI_STATUS_POSITION: s->dwReturn = g_cdtrack; return 0;
        }
        return 0;
    }
    case MCI_PLAY: {
        MCI_PLAY_PARMS *p = (MCI_PLAY_PARMS *)prm;
        MCI_OPEN_PARMSA o; MCI_PLAY_PARMS pp;
        char path[MAX_PATH];
        int t = (fl & MCI_FROM) ? (int)(p->dwFrom & 0xff) : g_cdtrack;
        wav_close();
        wav_path(t, path);
        memset(&o, 0, sizeof o); o.lpstrDeviceType = "waveaudio"; o.lpstrElementName = path;
        if (real_mci(0, MCI_OPEN, MCI_OPEN_TYPE | MCI_OPEN_ELEMENT, (DWORD_PTR)&o)) { lg("cd play %d: no %s", t, path); return 0; }
        g_wav = o.wDeviceID; g_cdtrack = t; g_cdplaying = 1;
        memset(&pp, 0, sizeof pp); pp.dwCallback = (fl & MCI_NOTIFY) ? p->dwCallback : 0;
        real_mci(g_wav, MCI_PLAY, (fl & MCI_NOTIFY) ? MCI_NOTIFY : 0, (DWORD_PTR)&pp);
        lg("cd play track %d", t);
        return 0;
    }
    case MCI_STOP: case MCI_PAUSE: if (g_wav) real_mci(g_wav, MCI_PAUSE, 0, 0); g_cdplaying = 0; return 0;
    case MCI_RESUME: if (g_wav) { real_mci(g_wav, MCI_RESUME, 0, 0); g_cdplaying = 1; } return 0;
    case MCI_CLOSE: wav_close(); return 0;
    }
    return 0;
}

/* Points the exe's WINMM.dll!mciSendCommandA import slot at fake_mci. */
static void hook_mci(void)
{
    BYTE *b = (BYTE *)GetModuleHandleA(0);
    IMAGE_NT_HEADERS *nt = (IMAGE_NT_HEADERS *)(b + ((IMAGE_DOS_HEADER *)b)->e_lfanew);
    IMAGE_DATA_DIRECTORY *dd = &nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
    IMAGE_IMPORT_DESCRIPTOR *imp;
    if (real_mci || !dd->VirtualAddress) return;
    real_mci = (void *)GetProcAddress(LoadLibraryA("winmm.dll"), "mciSendCommandA");
    for (imp = (IMAGE_IMPORT_DESCRIPTOR *)(b + dd->VirtualAddress); imp->Name; imp++) {
        IMAGE_THUNK_DATA *names = (IMAGE_THUNK_DATA *)(b + (imp->OriginalFirstThunk ? imp->OriginalFirstThunk : imp->FirstThunk));
        IMAGE_THUNK_DATA *slots = (IMAGE_THUNK_DATA *)(b + imp->FirstThunk);
        if (lstrcmpiA((char *)b + imp->Name, "winmm.dll")) continue;
        for (; names->u1.AddressOfData; names++, slots++) {
            DWORD old;
            if (IMAGE_SNAP_BY_ORDINAL(names->u1.Ordinal)) continue;
            if (strcmp((char *)((IMAGE_IMPORT_BY_NAME *)(b + names->u1.AddressOfData))->Name, "mciSendCommandA")
                && slots->u1.Function != (ULONG_PTR)real_mci) continue;
            VirtualProtect(&slots->u1.Function, 4, PAGE_READWRITE, &old);
            slots->u1.Function = (ULONG_PTR)fake_mci;
            VirtualProtect(&slots->u1.Function, 4, old, &old);
            lg("mciSendCommandA hooked at %p", &slots->u1.Function);
            return;
        }
    }
}

BOOL WINAPI DllMain(HINSTANCE h, DWORD why, LPVOID r)
{
    char env[8], path[MAX_PATH], *p;
    if (why != DLL_PROCESS_ATTACH) return TRUE;
    if (GetEnvironmentVariableA("SCORCHER_DDLOG", env, sizeof env)) {
        GetModuleFileNameA(0, path, MAX_PATH);
        p = strrchr(path, '\\'); if (p) strcpy(p + 1, "ddraw.log");
        g_log = fopen(path, "w");
    }
    hook_mci();
    return TRUE;
}

HRESULT WINAPI DirectDrawEnumerateA(LPDDENUMCALLBACKA cb, LPVOID ctx)
{
    cb(NULL, "Display", "display", ctx);
    return DD_OK;
}
