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
#include <dsound.h>
#include <math.h>
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

/* ---------------------------------------------------------------- settings (scorcher.ini next to the exe) */
static struct { int master, music, sfx, borderless, res, manx; } g_set = { 100, 100, 100, 0, 0, 0 };   /* manx: Manx TT bike mode (manx_*) */
static const struct { int w, h; const char *aspect; } RES[] = {
    {0, 0, ""},
    {640, 480, "4:3"}, {800, 600, "4:3"}, {1024, 768, "4:3"}, {1152, 864, "4:3"}, {1280, 960, "4:3"},
    {1400, 1050, "4:3"}, {1440, 1080, "4:3"}, {1600, 1200, "4:3"}, {1920, 1440, "4:3"}, {2048, 1536, "4:3"},
    {1280, 800, "16:10"}, {1440, 900, "16:10"}, {1680, 1050, "16:10"}, {1920, 1200, "16:10"}, {2560, 1600, "16:10"},
    {1280, 720, "16:9"}, {1366, 768, "16:9"}, {1600, 900, "16:9"}, {1920, 1080, "16:9"}, {2560, 1440, "16:9"},
    {3840, 2160, "16:9"},
};
#define NRES ((int)(sizeof RES / sizeof RES[0]))

static void ini_path(char *p)
{
    char *s;
    GetModuleFileNameA(0, p, MAX_PATH);
    s = strrchr(p, '\\'); if (!s) s = p - 1;
    strcpy(s + 1, "scorcher.ini");
}
static int clampi(int v, int lo, int hi) { return v < lo ? lo : v > hi ? hi : v; }
#define MAXOPP_INI 5   /* the default; up to MAXOPP with the extra rivals (rivals_*) */
static int g_opponents = MAXOPP_INI;   /* Practice opponents, set on the stage select page */
static int g_camkey[2] = { VK_TAB, VK_SHIFT };   /* camera angle / in-car view keys (cam_apply) */
static int g_keyov = 1;   /* key display at the bottom left (ini [game] keys, F10 / options row) */
static void cam_apply(void);
static void write_code(DWORD at, const void *b, int n);
static void settings_load(void)
{
    char p[MAX_PATH];
    ini_path(p);
    g_set.master = clampi(GetPrivateProfileIntA("audio", "master", 100, p), 0, 100);
    g_set.music = clampi(GetPrivateProfileIntA("audio", "music", 100, p), 0, 100);
    g_set.sfx = clampi(GetPrivateProfileIntA("audio", "effects", 100, p), 0, 100);
    g_set.borderless = GetPrivateProfileIntA("video", "borderless", 0, p) != 0;
    g_set.res = clampi(GetPrivateProfileIntA("video", "resolution", 0, p), 0, NRES - 1);
    g_opponents = clampi(GetPrivateProfileIntA("game", "opponents", MAXOPP_INI, p), 0, 8);
    g_set.manx = GetPrivateProfileIntA("game", "manx", 0, p) != 0;
    g_keyov = GetPrivateProfileIntA("game", "keys", 1, p) != 0;
    g_camkey[0] = clampi(GetPrivateProfileIntA("controls", "camera_angle", VK_TAB, p), 1, 0x7f);
    g_camkey[1] = clampi(GetPrivateProfileIntA("controls", "incar_view", VK_SHIFT, p), 1, 0x7f);
}
static void settings_save(void)
{
    char p[MAX_PATH], v[16];
    ini_path(p);
    sprintf(v, "%d", g_set.master); WritePrivateProfileStringA("audio", "master", v, p);
    sprintf(v, "%d", g_set.music); WritePrivateProfileStringA("audio", "music", v, p);
    sprintf(v, "%d", g_set.sfx); WritePrivateProfileStringA("audio", "effects", v, p);
    sprintf(v, "%d", g_set.borderless); WritePrivateProfileStringA("video", "borderless", v, p);
    sprintf(v, "%d", g_set.res); WritePrivateProfileStringA("video", "resolution", v, p);
    sprintf(v, "%d", g_opponents); WritePrivateProfileStringA("game", "opponents", v, p);
    sprintf(v, "%d", g_set.manx); WritePrivateProfileStringA("game", "manx", v, p);
    sprintf(v, "%d", g_keyov); WritePrivateProfileStringA("game", "keys", v, p);
    sprintf(v, "%d", g_camkey[0]); WritePrivateProfileStringA("controls", "camera_angle", v, p);
    sprintf(v, "%d", g_camkey[1]); WritePrivateProfileStringA("controls", "incar_view", v, p);
}

/* audio side, further down */
static void sfx_apply_all(void);
static void sfx_pause(int on);

/* ---------------------------------------------------------------- options overlay (F1)
   Drawn with GDI over the scaled frame. While it is open the game thread is held inside
   present() (pumping messages), the clock the game reads (timeGetTime) stands still and the
   playing sound effects are stopped; the music keeps playing so its level can be heard. */
/* Row ids. Pages: 0 main, 1 controls, 2 audio, 3 graphics; menu_rows() lists the rows a page shows
   and g_row is an index into that list. */
enum { ROW_MASTER, ROW_MUSIC, ROW_SFX, ROW_DISPLAY, ROW_RES, ROW_DIFF, ROW_VIEW, ROW_BACKDROP, ROW_GROUND,
       ROW_CONTROLS, ROW_GAME, ROW_QUIT, ROW_CLOSE, ROW_KEY0, ROW_JOY = ROW_KEY0 + 8, ROW_DEFAULTS, ROW_BACK,
       ROW_AUDIO, ROW_GRAPHICS, ROW_CAM0, ROW_CAM1, ROW_MANX, ROW_KEYS };
enum { PG_MAIN, PG_CONTROLS, PG_AUDIO, PG_GRAPHICS };
#define MAXROWS 16
/* g_from_title: 1 = opened from the title Options button, 2 = from Esc in a race; only then is the row
   ROW_GAME (the game's own options page / in-race options level) shown, and Quit race only in a race.
   g_pass_once lets one switch to the title page through the hook; g_race_opts asks for the game's
   in-race options level, g_race_quit for the title. g_waitkey: a key row waits for the new key. */
static int g_row, g_from_title, g_pass_once, g_post_enter, g_race_opts, g_race_quit, g_page, g_waitkey;
/* Race modes and the Practice opponent count (patch_modes, enemy_stub; NOTES "run 8" / "Practice racer count"). */
#define MAXOPP 8   /* 9 cars: the HUD place is one digit (10th shows blank) */
#define STAGEPAGE 10
static volatile int g_encmp, g_practice;
static int g_curpage = -1;
/* The game's settings block (0x98 bytes at 0x43eadc, NOTES "SCORWIN game settings map"); the game
   saves it itself (temp:\scor95.bin) when it closes. */
#define GSET(i) (((volatile int *)0x43eadc)[i])
enum { GS_KEYS = 1, GS_DIFF = 13, GS_VIEW = 28, GS_BACKDROP = 29, GS_GROUND = 30, GS_JOY = 37 };
#define CAMREC 0x4570c4   /* camera record; +0xf4 = view mode 0 textured, 1 flat, 2 wireframe (cam_ApplyViewOption 0x429612) */
static const int KEY_DEFAULTS[8] = { VK_UP, VK_DOWN, VK_LEFT, VK_RIGHT, VK_LEFT, VK_RIGHT, VK_MENU, VK_CONTROL };
static const char *KEY_LABEL[8] = { "Speed", "Brake", "Left", "Right", "Hard Left", "Hard Right", "Boost", "Jump" };
/* Camera keys: car_CameraKeys 0x415960 reads the key state bytes [0x4571e0]+9 (Tab, camera angle) and +0x10 (Shift,
   in-car view) with `mov al, [eax+disp8]`; the disp8 bytes below are the keys (the table holds VK & 0x7f). */
static const DWORD CAMKEY_AT[2] = { 0x41597d, 0x4159ea };
static const int CAM_DEFAULTS[2] = { VK_TAB, VK_SHIFT };
static const char *CAM_LABEL[2] = { "Camera angle", "Camera view" };

static int menu_rows(int *r)
{
    int n = 0, i;
    if (g_page == PG_CONTROLS) {
        for (i = 0; i < 8; i++) r[n++] = ROW_KEY0 + i;
        r[n++] = ROW_JOY; r[n++] = ROW_DEFAULTS;
    } else if (g_page == PG_AUDIO) {
        r[n++] = ROW_MASTER; r[n++] = ROW_MUSIC; r[n++] = ROW_SFX;
    } else if (g_page == PG_GRAPHICS) {
        r[n++] = ROW_DISPLAY; r[n++] = ROW_RES; r[n++] = ROW_VIEW; r[n++] = ROW_BACKDROP; r[n++] = ROW_GROUND;
    }
    if (g_page) { r[n++] = ROW_BACK; return n; }
    r[n++] = ROW_AUDIO; r[n++] = ROW_GRAPHICS; r[n++] = ROW_CONTROLS; r[n++] = ROW_DIFF; r[n++] = ROW_KEYS;
    if (g_from_title) r[n++] = ROW_GAME;
    if (g_from_title == 2) r[n++] = ROW_QUIT;
    r[n++] = ROW_CLOSE;
    return n;
}
static int cur_row(void) { int r[MAXROWS], n = menu_rows(r); return r[g_row < n ? g_row : n - 1]; }
static int key_bound(int vk) { int i; for (i = 0; i < 8; i++) if (GSET(GS_KEYS + i) == vk) return 1; return 0; }
static void key_name(int vk, char *s, int n)
{
    LONG sc = MapVirtualKeyA(vk, 0) << 16;
    if ((vk >= VK_PRIOR && vk <= VK_DOWN) || vk == VK_INSERT || vk == VK_DELETE || vk == VK_DIVIDE) sc |= 1 << 24;
    if (vk == VK_LBUTTON || vk == VK_RBUTTON || vk == VK_MBUTTON) { strcpy(s, vk == VK_LBUTTON ? "Left mouse" : vk == VK_RBUTTON ? "Right mouse" : "Middle mouse"); return; }
    if (!sc || !GetKeyNameTextA(sc, s, n)) sprintf(s, "Key %d", vk);
}
static volatile int g_menu, g_dirty;
static int g_inpause;
static WPARAM g_eatup;
static DWORD g_pausedms, g_pause_t0, g_hint_until;
static CRITICAL_SECTION g_cs;
static HDC g_mdc, g_bdc;
static HBITMAP g_mbm;
static int g_mw, g_mh, g_fh;
static HFONT g_font, g_fontsm;
static void apply_window(void);
static WNDPROC g_oldproc;

static void get_monitor(MONITORINFO *mi)
{
    mi->cbSize = sizeof *mi;
    GetMonitorInfoA(MonitorFromWindow(g_dd ? g_dd->hwnd : 0, MONITOR_DEFAULTTOPRIMARY), mi);
}
/* Resolution i usable in the current display mode: borderless up to the monitor, windowed up to the work area. */
static int res_fits(int i)
{
    MONITORINFO mi; RECT r = {0, 0, 0, 0};
    if (!i) return 1;
    get_monitor(&mi);
    if (g_set.borderless)
        return RES[i].w <= mi.rcMonitor.right - mi.rcMonitor.left && RES[i].h <= mi.rcMonitor.bottom - mi.rcMonitor.top;
    r.right = RES[i].w; r.bottom = RES[i].h;
    AdjustWindowRect(&r, WS_OVERLAPPEDWINDOW, FALSE);
    return r.right - r.left <= mi.rcWork.right - mi.rcWork.left && r.bottom - r.top <= mi.rcWork.bottom - mi.rcWork.top;
}

static void text_at(HDC dc, int x, int y, const char *s, COLORREF c, UINT align)
{
    SetTextColor(dc, c); SetTextAlign(dc, align | TA_TOP);
    TextOutA(dc, x, y, s, (int)strlen(s));
}
static void fill(HDC dc, int x, int y, int w, int h, COLORREF c)
{
    RECT r; HBRUSH b = CreateSolidBrush(c);
    r.left = x; r.top = y; r.right = x + w; r.bottom = y + h;
    FillRect(dc, &r, b); DeleteObject(b);
}
static HFONT make_font(int px, int weight, int italic, const char *face, const char *alt)
{
    HFONT f = CreateFontA(-px, 0, 0, 0, weight, italic, 0, 0, ANSI_CHARSET, 0, 0, ANTIALIASED_QUALITY, 0, face);
    char got[64]; HDC dc = GetDC(0); HGDIOBJ o = SelectObject(dc, f);
    GetTextFaceA(dc, sizeof got, got); SelectObject(dc, o); ReleaseDC(0, dc);
    if (lstrcmpiA(got, face)) { DeleteObject(f); f = CreateFontA(-px, 0, 0, 0, weight, italic, 0, 0, ANSI_CHARSET, 0, 0, ANTIALIASED_QUALITY, 0, alt); }
    return f;
}
/* Fonts in the style of the game menus: a light condensed sans-serif, slanted. */
static HFONT g_fontbig;
static void make_fonts(int rh)
{
    if (g_font && g_fh == rh) return;
    if (g_font) { DeleteObject(g_font); DeleteObject(g_fontsm); DeleteObject(g_fontbig); }
    g_font = make_font(rh * 80 / 100, FW_NORMAL, 1, "Bahnschrift SemiLight Condensed", "Arial Narrow");
    g_fontbig = make_font(rh * 125 / 100, FW_NORMAL, 1, "Bahnschrift SemiLight Condensed", "Arial Narrow");
    g_fontsm = make_font(rh * 52 / 100, FW_NORMAL, 0, "Bahnschrift Light Condensed", "Arial Narrow");
    g_fh = rh;
}

/* 32-bit DIB layers the size of the 4:3 picture: the frame copy, the sharp text (ink) and the glow source. */
typedef struct { HDC dc; HBITMAP bm; DWORD *px; int w, h; } Layer;
static Layer g_lfr, g_link, g_lglow;
static DWORD *g_tmp;
static int g_tmpn;
static void layer_size(Layer *l, HDC ref, int w, int h)
{
    BITMAPINFO bi;
    if (l->dc && l->w == w && l->h == h) return;
    if (!l->dc) l->dc = CreateCompatibleDC(ref);
    if (l->bm) DeleteObject(l->bm);
    memset(&bi, 0, sizeof bi);
    bi.bmiHeader.biSize = sizeof bi.bmiHeader; bi.bmiHeader.biWidth = w; bi.bmiHeader.biHeight = -h;
    bi.bmiHeader.biPlanes = 1; bi.bmiHeader.biBitCount = 32;
    l->bm = CreateDIBSection(ref, &bi, DIB_RGB_COLORS, (void **)&l->px, 0, 0);
    SelectObject(l->dc, l->bm);
    l->w = w; l->h = h;
}
/* Box blur of the three colour channels along one line (n pixels, stride st), radius r. */
static void blur_line(DWORD *p, int n, int st, int r, DWORD *t)
{
    int i, sr = 0, sg = 0, sb = 0, d = 2 * r + 1;
    for (i = 0; i < n; i++) t[i] = p[i * st];
    for (i = -r; i < r; i++) { DWORD c = t[clampi(i, 0, n - 1)]; sr += c >> 16 & 255; sg += c >> 8 & 255; sb += c & 255; }
    for (i = 0; i < n; i++) {
        DWORD a = t[clampi(i + r, 0, n - 1)], b = t[clampi(i - r - 1, 0, n - 1)];
        sr += a >> 16 & 255; sg += a >> 8 & 255; sb += a & 255;
        p[i * st] = (sr / d) << 16 | (sg / d) << 8 | (sb / d);
        sr -= b >> 16 & 255; sg -= b >> 8 & 255; sb -= b & 255;
    }
}
static void blur(Layer *l, int r)
{
    int x, y;
    for (y = 0; y < l->h; y++) blur_line(l->px + y * l->w, l->w, 1, r, g_tmp);
    for (x = 0; x < l->w; x++) blur_line(l->px + x, l->h, l->w, r, g_tmp);
}
/* Text into both layers: sharp in colour c, and into the glow layer in glow colour gc. */
static void ftext(int x, int y, const char *s, COLORREF c, COLORREF gc, UINT align)
{
    text_at(g_link.dc, x, y, s, c, align);
    text_at(g_lglow.dc, x, y, s, gc, align);
}
static void fbar(int x, int y, int w, int h, COLORREF c, COLORREF gc)
{
    fill(g_link.dc, x, y, w, h, c);
    fill(g_lglow.dc, x, y, w, h, gc);
}
static int sat(int v) { return v > 255 ? 255 : v; }

/* The options overlay in the look of the game menus: slanted light condensed letters, a green
   bloom around everything (orange around the chosen row), the picture darkened behind the rows
   but kept behind the logo at the top, and faint scanlines. */
static void draw_menu(HDC dc, RECT *cr, RECT *fr)
{
    static const char *VIEW[3] = { "Textured", "Flat", "Wireframe" };
    const COLORREF selc = RGB(255, 236, 200), dimc = RGB(178, 200, 196), green = RGB(40, 150, 70), orange = RGB(230, 110, 10);
    int fw = fr->right - fr->left, fh = fr->bottom - fr->top, rh, i, x, y, big, rows[MAXROWS], n = menu_rows(rows);
    int cx = fw / 2, top = fh * 37 / 100, bw = fw * 24 / 100, gap;
    char v[80], kn[64];
    const char *hint;
    (void)cr;
    if (fw < 64 || fh < 48) return;
    rh = fh * 57 / 100 / n;
    if (rh > fh / 12) rh = fh / 12;
    gap = rh / 3;
    layer_size(&g_lfr, dc, fw, fh); layer_size(&g_link, dc, fw, fh); layer_size(&g_lglow, dc, fw, fh);
    big = fw > fh ? fw : fh;
    if (g_tmpn < big) { if (g_tmp) HeapFree(GetProcessHeap(), 0, g_tmp); g_tmp = HeapAlloc(GetProcessHeap(), 0, big * 4); g_tmpn = big; }
    BitBlt(g_lfr.dc, 0, 0, fw, fh, dc, fr->left, fr->top, SRCCOPY);
    memset(g_link.px, 0, fw * fh * 4); memset(g_lglow.px, 0, fw * fh * 4);
    make_fonts(rh);
    SetBkMode(g_link.dc, TRANSPARENT); SetBkMode(g_lglow.dc, TRANSPARENT);
    SelectObject(g_link.dc, g_fontbig); SelectObject(g_lglow.dc, g_fontbig);
    ftext(cx, top - rh * 16 / 10, g_page == PG_CONTROLS ? "Controls" : g_page == PG_AUDIO ? "Audio" : g_page == PG_GRAPHICS ? "Graphics" : "Options", RGB(255, 255, 255), orange, TA_CENTER);
    SelectObject(g_link.dc, g_font); SelectObject(g_lglow.dc, g_font);
    for (i = 0; i < n; i++) {
        int id = rows[i], sel = i == g_row, ty = top + i * rh;
        COLORREF c = sel ? selc : dimc, gc = sel ? orange : green;
        const char *lab = 0, *val = 0;
        if (id <= ROW_SFX) {
            static const char *VOL[3] = { "Master volume", "Music volume", "Effects volume" };
            int pct = id == ROW_MASTER ? g_set.master : id == ROW_MUSIC ? g_set.music : g_set.sfx;
            int bx = cx + gap, bh = rh / 7, by = ty + rh / 2 - bh / 2;
            ftext(cx - gap, ty, VOL[id], c, gc, TA_RIGHT);
            fbar(bx, by, bw, bh, RGB(40, 64, 52), RGB(10, 40, 20));
            fbar(bx, by, bw * pct / 100, bh, sel ? RGB(255, 190, 90) : RGB(120, 200, 150), gc);
            sprintf(v, "%d%%", pct);
            ftext(bx + bw + gap, ty, v, c, gc, TA_LEFT);
            continue;
        }
        switch (id) {
        case ROW_DISPLAY: lab = "Display"; val = g_set.borderless ? "Borderless" : "Windowed"; break;
        case ROW_RES:
            lab = "Resolution"; val = v;
            if (g_set.res) sprintf(v, "%d x %d  (%s)", RES[g_set.res].w, RES[g_set.res].h, RES[g_set.res].aspect);
            else if (g_set.borderless) {
                MONITORINFO mi; get_monitor(&mi);
                sprintf(v, "Full screen (%d x %d)", (int)(mi.rcMonitor.right - mi.rcMonitor.left), (int)(mi.rcMonitor.bottom - mi.rcMonitor.top));
            } else strcpy(v, "Auto");
            break;
        case ROW_DIFF: lab = "Difficulty"; val = GSET(GS_DIFF) ? "Hard" : "Normal"; break;
        case ROW_MANX: lab = "Bike"; val = g_set.manx ? "Manx TT (next race)" : "Scorcher"; break;
        case ROW_KEYS: lab = "Show keys"; val = g_keyov ? "On" : "Off"; break;
        case ROW_VIEW: lab = "Drawing"; val = VIEW[clampi(GSET(GS_VIEW), 0, 2)]; break;
        case ROW_BACKDROP: lab = "Backdrop"; val = GSET(GS_BACKDROP) ? "On" : "Off"; break;
        case ROW_GROUND: lab = "Ground"; val = GSET(GS_GROUND) ? "On" : "Off"; break;
        case ROW_JOY: lab = "Joystick"; val = GSET(GS_JOY) ? "On (keys off)" : "Off"; break;
        default:
            if (id == ROW_CAM0 || id == ROW_CAM1) {
                lab = CAM_LABEL[id - ROW_CAM0];
                if (sel && g_waitkey) val = "press a key...";
                else { key_name(g_camkey[id - ROW_CAM0], kn, sizeof kn); val = kn; }
            } else if (id >= ROW_KEY0 && id < ROW_KEY0 + 8) {
                lab = KEY_LABEL[id - ROW_KEY0];
                if (sel && g_waitkey) val = "press a key...";
                else { key_name(GSET(GS_KEYS + id - ROW_KEY0), kn, sizeof kn); val = kn; }
            }
        }
        if (lab) {
            ftext(cx - gap, ty, lab, c, gc, TA_RIGHT);
            ftext(cx + gap, ty, val, sel && g_waitkey ? RGB(255, 255, 255) : c, gc, TA_LEFT);
        } else
            ftext(cx, ty, id == ROW_CONTROLS ? "Controls..." : id == ROW_AUDIO ? "Audio..." : id == ROW_GRAPHICS ? "Graphics..." : id == ROW_GAME ? "Game options..." : id == ROW_QUIT ? "Quit race" :
                          id == ROW_DEFAULTS ? "Reset to defaults" : id == ROW_BACK ? "Back" : g_from_title == 1 ? "Back" : "Back to game",
                  c, gc, TA_CENTER);
    }
    SelectObject(g_link.dc, g_fontsm); SelectObject(g_lglow.dc, g_fontsm);
    hint = g_waitkey ? "Press the new key        Esc  cancel"
         : g_page == PG_CONTROLS ? "Up / Down  choose        Enter  change key        Esc  back"
         : "Up / Down  choose        Left / Right  change        Esc  back";
    ftext(cx, fh - fh * 5 / 100, hint, RGB(150, 175, 165), RGB(20, 70, 35), TA_CENTER);
    GdiFlush();
    /* two box blurs = about a gaussian bloom */
    blur(&g_lglow, rh / 5 + 1); blur(&g_lglow, rh / 5 + 1);
    for (y = 0; y < fh; y++) {
        int k = y < fh * 20 / 100 ? 210 : y < fh * 28 / 100 ? 210 - (y - fh * 20 / 100) * 180 / (fh * 8 / 100 + 1) : 30;
        int scan = (y & 1) ? 228 : 256;
        DWORD *f = g_lfr.px + y * fw, *in = g_link.px + y * fw, *gl = g_lglow.px + y * fw;
        for (x = 0; x < fw; x++) {
            DWORD a = f[x], t = in[x], g = gl[x];
            int r = (a >> 16 & 255) * k >> 8, gg = ((a >> 8 & 255) * k >> 8) + 4, b = (a & 255) * k >> 8;
            int tr = t >> 16 & 255, tg = t >> 8 & 255, tb = t & 255, ta = tr > tg ? (tr > tb ? tr : tb) : (tg > tb ? tg : tb);
            r = r * scan >> 8; gg = gg * scan >> 8; b = b * scan >> 8;
            r = sat(r + (g >> 16 & 255) * 2); gg = sat(gg + (g >> 8 & 255) * 2); b = sat(b + (g & 255) * 2);
            r = sat((r * (255 - ta) >> 8) + tr); gg = sat((gg * (255 - ta) >> 8) + tg); b = sat((b * (255 - ta) >> 8) + tb);
            f[x] = r << 16 | gg << 8 | b;
        }
    }
    BitBlt(dc, fr->left, fr->top, fw, fh, g_lfr.dc, 0, 0, SRCCOPY);
}


static void draw_hint(HDC dc, RECT *fr)
{
    int rh = (fr->bottom - fr->top) / 13;
    make_fonts(rh);
    SetBkMode(dc, TRANSPARENT);
    SelectObject(dc, g_fontsm);
    text_at(dc, fr->left + rh / 3 + 2, fr->bottom - rh * 6 / 10 + 2, "F1: options", RGB(0, 0, 0), TA_LEFT);
    text_at(dc, fr->left + rh / 3, fr->bottom - rh * 6 / 10, "F1: options", RGB(255, 200, 60), TA_LEFT);
}

/* Practice opponents counter, top right of the stage select page (the game's "Select stage" title is top left). */
static int counter_on(void) { return g_practice && g_curpage == STAGEPAGE; }
static void draw_counter(HDC dc, RECT *fr)
{
    int rh = (fr->bottom - fr->top) / 13, x = fr->right - rh / 2, y = fr->top + rh / 3;
    char t[64];
    make_fonts(rh);
    SetBkMode(dc, TRANSPARENT);
    SelectObject(dc, g_font);
    sprintf(t, "Opponents  < %d >", g_opponents);
    text_at(dc, x + 2, y + 2, t, RGB(0, 0, 0), TA_RIGHT);
    text_at(dc, x, y, t, RGB(255, 200, 60), TA_RIGHT);
    SelectObject(dc, g_fontsm);
    text_at(dc, x + 1, y + rh * 11 / 10 + 1, "Left / Right", RGB(0, 0, 0), TA_RIGHT);
    text_at(dc, x, y + rh * 11 / 10, "Left / Right", RGB(150, 175, 165), TA_RIGHT);
}

/* Bike select before a race (user 10-13, "like the daytona usa selection"): Scorcher on the left, Manx TT on
   the right; the chosen bike turns on its pedestal under a spotlight, the other stands dimmed. bikesel_run opens it
   from rivals_open when a level's setup file opens after a menu page, before the bike world loads, and sets
   g_set.manx. Sprites: bikesel\bikesel.bin next to the exe ("BSL1", w, h, frames, bikes, then BGRA premultiplied;
   made by tools\bikesel_render.py from the ripped models, local only). */
#define BS_SECS 10
static volatile int g_bsel;
static int g_bscur, g_bsw, g_bsh, g_bsn, g_bsb;
static DWORD *g_bspx, g_bst0;
static int bikesel_load(void)
{
    char p[MAX_PATH], *s;
    FILE *f;
    int hd[5];
    size_t n;
    if (g_bspx) return 1;
    GetModuleFileNameA(0, p, MAX_PATH);
    s = strrchr(p, '\\');
    strcpy(s + 1, "bikesel\\bikesel.bin");
    if (!(f = fopen(p, "rb"))) { lg("bikesel: %s missing, no select screen", p); return 0; }
    if (fread(hd, 4, 5, f) != 5 || hd[0] != 0x314c5342 || hd[1] <= 0 || hd[2] <= 0 || hd[3] <= 0 || hd[4] < 2) { fclose(f); lg("bikesel: bad header"); return 0; }
    g_bsw = hd[1]; g_bsh = hd[2]; g_bsn = hd[3]; g_bsb = hd[4];
    n = (size_t)g_bsw * g_bsh * g_bsn * g_bsb;
    g_bspx = (DWORD *)malloc(n * 4);
    if (g_bspx && fread(g_bspx, 4, n, f) != n) { free(g_bspx); g_bspx = 0; }
    fclose(f);
    lg("bikesel: %dx%d, %d frames, %d bikes%s", g_bsw, g_bsh, g_bsn, g_bsb, g_bspx ? "" : " (read failed)");
    return g_bspx != 0;
}
/* One turntable frame scaled to size x size, bottom centre at (cx, by); bright 0..256, grey = desaturate,
   flip = upside down below by (the floor reflection, fading out). */
static void bs_blit(Layer *l, int bike, int frame, int cx, int by, int size, int bright, int grey, int flip)
{
    DWORD *src = g_bspx + ((size_t)bike * g_bsn + frame) * g_bsw * g_bsh;
    int x0 = cx - size / 2, y0 = flip ? by : by - size, x, y;
    int xa = clampi(x0, 0, l->w), xb = clampi(x0 + size, 0, l->w), ya = clampi(y0, 0, l->h), yb = clampi(y0 + size, 0, l->h);
    for (y = ya; y < yb; y++) {
        int sy = (y - y0) * g_bsh / size, br = bright;
        DWORD *d = l->px + y * l->w, *sr;
        if (flip) { br = bright * (size / 3 - (y - y0)) / (size / 3 + 1); if (br <= 0) break; sy = g_bsh - 1 - sy; }
        sr = src + sy * g_bsw;
        for (x = xa; x < xb; x++) {
            DWORD c = sr[(x - x0) * g_bsw / size], o;
            int a = c >> 24, r = c >> 16 & 255, g = c >> 8 & 255, b = c & 255, ia;
            if (!a) continue;
            if (grey) { int m = (r * 30 + g * 59 + b * 11) / 100; r = (r + m * 3) / 4; g = (g + m * 3) / 4; b = (b + m * 3) / 4; }
            a = a * (flip ? br : 256) >> 8;
            ia = 255 - a; o = d[x];
            r = (r * br >> 8) + ((o >> 16 & 255) * ia >> 8);
            g = (g * br >> 8) + ((o >> 8 & 255) * ia >> 8);
            b = (b * br >> 8) + ((o & 255) * ia >> 8);
            d[x] = sat(r) << 16 | sat(g) << 8 | sat(b);
        }
    }
}
static void draw_bikesel(HDC dc, RECT *fr)
{
    static const char *NAME[2] = { "SCORCHER", "MANX TT" }, *SUB[2] = { "The original", "Manx TT Super Bike handling" };
    static const int TR[2] = { 50, 255 }, TG[2] = { 230, 110 }, TB[2] = { 255, 30 };
    int fw = fr->right - fr->left, fh = fr->bottom - fr->top, rh, x, y, i, big, s = g_bscur;
    int pedy, rx, ry, pcx[2], size, pulse, left;
    DWORD el = GetTickCount() - g_bst0;
    char t[32];
    if (fw < 64 || fh < 48 || !g_bspx) return;
    rh = fh / 12;
    pedy = fh * 70 / 100; rx = fw * 16 / 100; ry = fh * 4 / 100; pcx[0] = fw * 27 / 100; pcx[1] = fw * 73 / 100;
    size = fh * 52 / 100;
    pulse = 200 + (int)(56 * sin(el * 0.004));
    layer_size(&g_lfr, dc, fw, fh); layer_size(&g_link, dc, fw, fh); layer_size(&g_lglow, dc, fw, fh);
    big = fw > fh ? fw : fh;
    if (g_tmpn < big) { if (g_tmp) HeapFree(GetProcessHeap(), 0, g_tmp); g_tmp = HeapAlloc(GetProcessHeap(), 0, big * 4); g_tmpn = big; }
    BitBlt(g_lfr.dc, 0, 0, fw, fh, dc, fr->left, fr->top, SRCCOPY);
    GdiFlush();
    /* the game picture darkened under a purple-blue gradient, the spotlight cone and the two pedestals */
    for (y = 0; y < fh; y++) {
        DWORD *f = g_lfr.px + y * fw;
        int hw = fw * 3 / 100 + fw * 13 / 100 * y / pedy, ey = y - pedy;
        for (x = 0; x < fw; x++) {
            DWORD a = f[x];
            int r = ((a >> 16 & 255) * 36 >> 8) + 14 + 10 * y / fh, g = ((a >> 8 & 255) * 36 >> 8) + 6, b = ((a & 255) * 36 >> 8) + 26 + 20 * y / fh;
            int d = x - pcx[s];
            if (d < 0) d = -d;
            if (y < pedy && d < hw) {
                int k = (hw - d) * 256 / hw;
                k = (k * k >> 8) * (60 + 140 * y / pedy) / 256 * pulse >> 8;
                r += TR[s] * k >> 8; g += TG[s] * k >> 8; b += TB[s] * k >> 8;
                r += 40 * k >> 8; g += 40 * k >> 8; b += 40 * k >> 8;
            }
            if (ey > -ry && ey < ry) {
                for (i = 0; i < 2; i++) {
                    double ex = (x - pcx[i]) / (double)rx, eyy = ey / (double)ry, e = ex * ex + eyy * eyy;
                    if (e < 1) {
                        int k = e > 0.82 ? 255 : (int)(70 * (1 - e));
                        k = i == s ? k * pulse >> 8 : k / 4;
                        r += TR[i] * k >> 8; g += TG[i] * k >> 8; b += TB[i] * k >> 8;
                    }
                }
            }
            f[x] = sat(r) << 16 | sat(g) << 8 | sat(b);
        }
    }
    for (i = 0; i < 2; i++) {
        int sel = i == s, fr0 = i ? g_bsn * 5 / 6 : g_bsn / 6;
        int fno = sel ? (int)(el * g_bsn / 4000 + fr0) % g_bsn : fr0, sz = sel ? size * 108 / 100 : size;
        bs_blit(&g_lfr, i, fno, pcx[i], pedy + ry / 3, sz, sel ? 90 : 30, !sel, 1);
        bs_blit(&g_lfr, i, fno, pcx[i], pedy + ry / 3, sz, sel ? 256 : 80, !sel, 0);
    }
    /* text: ink + glow layers as in draw_menu */
    memset(g_link.px, 0, fw * fh * 4); memset(g_lglow.px, 0, fw * fh * 4);
    make_fonts(rh);
    SetBkMode(g_link.dc, TRANSPARENT); SetBkMode(g_lglow.dc, TRANSPARENT);
    SelectObject(g_link.dc, g_fontbig); SelectObject(g_lglow.dc, g_fontbig);
    ftext(fw / 2, fh * 4 / 100, "Select your bike", RGB(255, 255, 255), RGB(220, 40, 200), TA_CENTER);
    left = BS_SECS - (int)(el / 1000);
    sprintf(t, "%d", left < 0 ? 0 : left);
    ftext(fw - rh, fh * 4 / 100, t, left <= 3 ? RGB(255, 90, 80) : RGB(255, 236, 200), left <= 3 ? RGB(200, 20, 20) : RGB(230, 110, 10), TA_RIGHT);
    for (i = 0; i < 2; i++) {
        int sel = i == s;
        COLORREF tc = RGB(TR[i], TG[i], TB[i]);
        SelectObject(g_link.dc, g_fontbig); SelectObject(g_lglow.dc, g_fontbig);
        if (sel && (el / 400) % 2) sprintf(t, "<  %s  >", NAME[i]); else strcpy(t, NAME[i]);
        ftext(pcx[i], pedy + ry + rh / 2, t, sel ? RGB(255, 255, 255) : RGB(110, 120, 130), sel ? tc : RGB(10, 20, 30), TA_CENTER);
        SelectObject(g_link.dc, g_fontsm); SelectObject(g_lglow.dc, g_fontsm);
        ftext(pcx[i], pedy + ry + rh * 2, SUB[i], sel ? RGB(220, 230, 235) : RGB(90, 100, 110), sel ? tc : RGB(0, 0, 0), TA_CENTER);
    }
    SelectObject(g_link.dc, g_fontsm); SelectObject(g_lglow.dc, g_fontsm);
    ftext(fw / 2, fh - fh * 6 / 100, "Left / Right  choose        Enter  race", RGB(150, 175, 165), RGB(20, 70, 35), TA_CENTER);
    GdiFlush();
    blur(&g_lglow, rh / 5 + 1); blur(&g_lglow, rh / 5 + 1);
    for (y = 0; y < fh; y++) {
        int scan = (y & 1) ? 232 : 256;
        DWORD *f = g_lfr.px + y * fw, *in = g_link.px + y * fw, *gl = g_lglow.px + y * fw;
        for (x = 0; x < fw; x++) {
            DWORD a = f[x], tt = in[x], g = gl[x];
            int r = (a >> 16 & 255) * scan >> 8, gg = (a >> 8 & 255) * scan >> 8, b = (a & 255) * scan >> 8;
            int tr = tt >> 16 & 255, tg = tt >> 8 & 255, tb = tt & 255, ta = tr > tg ? (tr > tb ? tr : tb) : (tg > tb ? tg : tb);
            r = sat(r + (g >> 16 & 255) * 2); gg = sat(gg + (g >> 8 & 255) * 2); b = sat(b + (g & 255) * 2);
            r = sat((r * (255 - ta) >> 8) + tr); gg = sat((gg * (255 - ta) >> 8) + tg); b = sat((b * (255 - ta) >> 8) + tb);
            f[x] = r << 16 | gg << 8 | b;
        }
    }
    BitBlt(dc, fr->left, fr->top, fw, fh, g_lfr.dc, 0, 0, SRCCOPY);
}

/* ---------------------------------------------------------------- presenting */
static void client_fit(int cw, int ch, RECT *r)
{
    int w = cw, h = cw * 3 / 4;
    if (h > ch) { h = ch; w = ch * 4 / 3; }
    r->left = (cw - w) / 2; r->top = (ch - h) / 2; r->right = r->left + w; r->bottom = r->top + h;
}

/* Draws the primary surface (and the overlay) into the window. The overlay frames are put
   together in a memory DC first so they do not flicker. */
/* Key display (user 10-13: "a string of buttons on bottom left showing the keyboard + mouse inputs"):
   an arrow cluster (arrows or W A S D, as typed) that lights while held, then a cap for each other key
   or mouse button held, kept lit 250 ms after release so taps show in recordings. */
static DWORD g_kdown[256], g_kup[256];   /* tick of the last press / release, 0 = never */
static BYTE g_korder[16]; static int g_nkorder;
static void key_note(WPARAM k, int down)
{
    int i;
    if (k >= 256) return;
    if (down) {
        if (!g_kdown[k] || g_kup[k] >= g_kdown[k]) {
            for (i = 0; i < g_nkorder && g_korder[i] != k; i++) ;
            if (i == g_nkorder && g_nkorder < 16) g_korder[g_nkorder++] = (BYTE)k;
        }
        g_kdown[k] = GetTickCount() | 1; g_kup[k] = 0;
    } else if (g_kdown[k]) g_kup[k] = GetTickCount() | 1;
}
static void key_clear(void) { int i; for (i = 0; i < 256; i++) if (g_kdown[i] && !g_kup[i]) g_kup[i] = GetTickCount() | 1; }
static int key_lit(int k)
{
    if (!g_kdown[k]) return 0;
    return !g_kup[k] || GetTickCount() - g_kup[k] < 250;
}
static const char *kd_name(int k, char *b)
{
    switch (k) {
    case VK_CONTROL: return "Ctrl"; case VK_MENU: return "Alt"; case VK_SHIFT: return "Shift";
    case VK_SPACE: return "Space"; case VK_TAB: return "Tab"; case VK_RETURN: return "Enter";
    case VK_ESCAPE: return "Esc"; case VK_BACK: return "Bksp"; case VK_LBUTTON: return "LMB";
    case VK_RBUTTON: return "RMB"; case VK_MBUTTON: return "MMB";
    }
    if ((k >= '0' && k <= '9') || (k >= 'A' && k <= 'Z')) { b[0] = (char)k; b[1] = 0; return b; }
    if (k >= VK_F1 && k <= VK_F24) { sprintf(b, "F%d", k - VK_F1 + 1); return b; }
    if (!GetKeyNameTextA((LONG)MapVirtualKeyA(k, 0) << 16, b, 16)) sprintf(b, "%02X", k);
    return b;
}
static void key_cap(HDC dc, int x, int y, int w, int h, int lit, COLORREF lc, const char *txt, int arrow)
{
    HBRUSH br = CreateSolidBrush(lit ? lc : RGB(18, 22, 30));
    HPEN pn = CreatePen(PS_SOLID, h / 14 + 1, lit ? RGB(255, 255, 255) : RGB(70, 84, 110));
    HGDIOBJ ob = SelectObject(dc, br), op = SelectObject(dc, pn);
    RoundRect(dc, x, y, x + w, y + h, h / 3, h / 3);
    if (arrow) {   /* 1 up 2 down 3 left 4 right */
        POINT p[3]; int cx = x + w / 2, cy = y + h / 2, a = h / 4;
        HBRUSH ab = CreateSolidBrush(lit ? RGB(0, 0, 0) : RGB(120, 135, 160));
        HGDIOBJ o2;
        if (arrow == 1) { p[0].x = cx; p[0].y = cy - a; p[1].x = cx - a; p[1].y = cy + a / 2; p[2].x = cx + a; p[2].y = cy + a / 2; }
        else if (arrow == 2) { p[0].x = cx; p[0].y = cy + a; p[1].x = cx - a; p[1].y = cy - a / 2; p[2].x = cx + a; p[2].y = cy - a / 2; }
        else if (arrow == 3) { p[0].x = cx - a; p[0].y = cy; p[1].x = cx + a / 2; p[1].y = cy - a; p[2].x = cx + a / 2; p[2].y = cy + a; }
        else { p[0].x = cx + a; p[0].y = cy; p[1].x = cx - a / 2; p[1].y = cy - a; p[2].x = cx - a / 2; p[2].y = cy + a; }
        SelectObject(dc, GetStockObject(NULL_PEN)); o2 = SelectObject(dc, ab);
        Polygon(dc, p, 3);
        SelectObject(dc, o2); DeleteObject(ab);
    } else if (txt) {
        RECT t = { x, y, x + w, y + h };
        SetBkMode(dc, TRANSPARENT); SetTextColor(dc, lit ? RGB(0, 0, 0) : RGB(150, 165, 190));
        DrawTextA(dc, txt, -1, &t, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
    SelectObject(dc, ob); SelectObject(dc, op); DeleteObject(br); DeleteObject(pn);
}
static HFONT make_font(int px, int weight, int italic, const char *face, const char *alt);
static int g_colov;   /* F2: draw the player's collision sphere (user 10-13) */
static void draw_collider(HDC dc, RECT *r, int sw, int sh);
static void draw_keys(HDC dc, RECT *r)
{
    static HFONT f; static int fpx;
    int u = (r->bottom - r->top) / 20, g = u / 6 + 1, x0 = r->left + u / 2, y1 = r->bottom - u / 2 - u, x, i, j;
    char b[24];
    HGDIOBJ of;
    COLORREF cy = RGB(0, 210, 255);
    if (u < 8) return;
    if (!f || fpx != u) { if (f) DeleteObject(f); fpx = u; f = make_font(u * 11 / 20, FW_BOLD, 0, "Bahnschrift", "Arial"); }
    of = SelectObject(dc, f);
    key_cap(dc, x0 + u + g, y1 - u - g, u, u, key_lit(VK_UP) || key_lit('W'), cy, 0, 1);
    key_cap(dc, x0, y1, u, u, key_lit(VK_LEFT) || key_lit('A'), cy, 0, 3);
    key_cap(dc, x0 + u + g, y1, u, u, key_lit(VK_DOWN) || key_lit('S'), cy, 0, 2);
    key_cap(dc, x0 + 2 * (u + g), y1, u, u, key_lit(VK_RIGHT) || key_lit('D'), cy, 0, 4);
    x = x0 + 3 * (u + g) + u / 2;
    for (i = j = 0; i < g_nkorder; i++) {
        int k = g_korder[i], w, m;
        if (!key_lit(k)) {   /* drop it from the order once it has faded */
            if (!g_kdown[k] || (g_kup[k] && GetTickCount() - g_kup[k] > 2000)) continue;
            g_korder[j++] = (BYTE)k; continue;
        }
        g_korder[j++] = (BYTE)k;
        if (k == VK_UP || k == VK_DOWN || k == VK_LEFT || k == VK_RIGHT || k == 'W' || k == 'A' || k == 'S' || k == 'D') continue;
        kd_name(k, b);
        { SIZE sz; GetTextExtentPoint32A(dc, b, (int)strlen(b), &sz); w = sz.cx + u / 2; if (w < u) w = u; }
        m = k == VK_LBUTTON || k == VK_RBUTTON || k == VK_MBUTTON;
        key_cap(dc, x, y1, w, u, 1, m ? RGB(80, 255, 120) : RGB(255, 170, 40), b, 0);
        x += w + g;
    }
    g_nkorder = j;
    SelectObject(dc, of);
}

static void show(void)
{
    Surf *s = g_primary;
    HWND hw = g_dd ? g_dd->hwnd : 0;
    struct { BITMAPINFOHEADER h; RGBQUAD c[256]; } bi;
    RECT cr, r;
    HDC dc, to;
    int i, over;
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
    over = g_menu || g_bsel || GetTickCount() < g_hint_until || counter_on() || g_keyov || g_colov;
    EnterCriticalSection(&g_cs);
    dc = GetDC(hw);
    to = dc;
    if (over) {
        if (!g_mdc) g_mdc = CreateCompatibleDC(dc);
        if (g_mw != cr.right || g_mh != cr.bottom) {
            HBITMAP b = CreateCompatibleBitmap(dc, cr.right, cr.bottom);
            SelectObject(g_mdc, b);
            if (g_mbm) DeleteObject(g_mbm);
            g_mbm = b; g_mw = cr.right; g_mh = cr.bottom;
        }
        to = g_mdc;
    }
    SetStretchBltMode(to, COLORONCOLOR);
    StretchDIBits(to, r.left, r.top, r.right - r.left, r.bottom - r.top, 0, 0, s->w, s->h,
                  s->bits, (BITMAPINFO *)&bi, DIB_RGB_COLORS, SRCCOPY);
    if (r.left > 0) { RECT a = {0, 0, r.left, cr.bottom}, b = {r.right, 0, cr.right, cr.bottom};
        FillRect(to, &a, GetStockObject(BLACK_BRUSH)); FillRect(to, &b, GetStockObject(BLACK_BRUSH)); }
    if (r.top > 0) { RECT a = {0, 0, cr.right, r.top}, b = {0, r.bottom, cr.right, cr.bottom};
        FillRect(to, &a, GetStockObject(BLACK_BRUSH)); FillRect(to, &b, GetStockObject(BLACK_BRUSH)); }
    if (over) {
        if (g_bsel) draw_bikesel(to, &r);
        else if (g_menu) draw_menu(to, &cr, &r);
        else { if (counter_on()) draw_counter(to, &r); if (GetTickCount() < g_hint_until) draw_hint(to, &r);
            if (g_keyov) draw_keys(to, &r);
            if (g_colov) draw_collider(to, &r, s->w, s->h); }
        BitBlt(dc, 0, 0, cr.right, cr.bottom, to, 0, 0, SRCCOPY);
    }
    ReleaseDC(hw, dc);
    LeaveCriticalSection(&g_cs);
}

/* Key-ups for the keys the game may have seen go down before the overlay opened. */
static void release_game_keys(void)
{
    static const WPARAM k[] = { VK_UP, VK_DOWN, VK_LEFT, VK_RIGHT, VK_RETURN, VK_ESCAPE, VK_CONTROL, VK_MENU, VK_SPACE };
    int i;
    for (i = 0; i < (int)(sizeof k / sizeof k[0]); i++)
        CallWindowProcA(g_oldproc, g_dd->hwnd, WM_KEYUP, k[i], 0xC0000001);
}

/* Holds the game thread while the overlay is open. */
static void pause_loop(void)
{
    MSG m;
    g_pause_t0 = timeGetTime();
    g_inpause = 1;
    sfx_pause(1);
    lg("options open");
    while (g_menu) {
        while (PeekMessageA(&m, 0, 0, 0, PM_REMOVE)) {
            if (m.message == WM_QUIT) { PostQuitMessage((int)m.wParam); g_menu = 0; break; }
            TranslateMessage(&m); DispatchMessageA(&m);
        }
        if (g_dirty) { g_dirty = 0; show(); }
        Sleep(10);
    }
    settings_save();
    sfx_pause(0);
    if (g_dd && g_dd->hwnd) release_game_keys();
    g_pausedms += timeGetTime() - g_pause_t0;
    g_inpause = 0;
    lg("options closed (paused %u ms in all)", g_pausedms);
    show();
}

/* Holds the game thread (inside the setup file open) while the bike select screen is up. */
static void bikesel_run(void)
{
    MSG m;
    if (!g_dd || !g_dd->hwnd || !bikesel_load()) return;
    g_bscur = g_set.manx ? 1 : 0; g_bst0 = GetTickCount(); g_bsel = 1;
    lg("bikesel open");
    while (g_bsel) {
        while (PeekMessageA(&m, 0, 0, 0, PM_REMOVE)) {
            if (m.message == WM_QUIT) { PostQuitMessage((int)m.wParam); g_bsel = 0; break; }
            TranslateMessage(&m); DispatchMessageA(&m);
        }
        if (GetTickCount() - g_bst0 >= BS_SECS * 1000 + 300) g_bsel = 0;
        show();
        Sleep(10);
    }
    g_set.manx = g_bscur;
    settings_save();
    release_game_keys();
    lg("bikesel: %s", g_set.manx ? "Manx TT" : "Scorcher");
    show();
}
static void bikesel_key(WPARAM k)
{
    if (k == VK_LEFT || k == 'A') g_bscur = 0;
    else if (k == VK_RIGHT || k == 'D') g_bscur = 1;
    else if (k == VK_RETURN || k == VK_SPACE || k == VK_ESCAPE || k == VK_CONTROL) { g_bsel = 0; g_eatup = k; }
}

static void sfx_log(void);
static void race_options(void);
static void race_quit(void);
static void present(void)
{
    { static int n; if (n++ % 300 == 0) lg("present #%d %p %dx%dx%d", n, g_primary, g_primary ? g_primary->w : 0, g_primary ? g_primary->h : 0, g_primary ? g_primary->bpp : 0); }
    { static int n; if (getenv("SCORCHER_SNDLOG") && n++ % 150 == 0) sfx_log(); }
    show();
    if (g_menu && !g_inpause) pause_loop();
    if (g_race_opts) { g_race_opts = 0; race_options(); }
    if (g_race_quit) { g_race_quit = 0; race_quit(); }
    /* "Game options...": press Enter for the game (down now, up a few frames later) on its Options
       button, which the cursor is still on; the hook lets that one switch through */
    if (g_post_enter && g_dd && g_dd->hwnd) {
        if (g_post_enter == 1 && g_eatup) return;   /* the real Enter is still down */
        if (g_post_enter == 1) { PostMessageA(g_dd->hwnd, WM_KEYDOWN, VK_RETURN, 0x001C0001); }
        if (++g_post_enter > 6) { PostMessageA(g_dd->hwnd, WM_KEYUP, VK_RETURN, 0xC01C0001); g_post_enter = 0; }
    }
}

static void menu_change(int dir)
{
    int id = cur_row();
    int *v = id == ROW_MASTER ? &g_set.master : id == ROW_MUSIC ? &g_set.music : id == ROW_SFX ? &g_set.sfx : 0;
    if (v) {
        *v = clampi(*v + dir * 5, 0, 100);
        sfx_apply_all();   /* the music picks its level up per buffer */
    } else if (id == ROW_DISPLAY) {
        g_set.borderless ^= 1;
        if (!res_fits(g_set.res)) g_set.res = 0;
        apply_window();
    } else if (id == ROW_RES) {
        int i = g_set.res;
        do i = (i + dir + NRES) % NRES; while (!res_fits(i));
        g_set.res = i;
        apply_window();
    } else if (id == ROW_DIFF) GSET(GS_DIFF) ^= 1;
    else if (id == ROW_MANX) g_set.manx ^= 1;
    else if (id == ROW_KEYS) g_keyov ^= 1;
    else if (id == ROW_VIEW) {
        GSET(GS_VIEW) = (clampi(GSET(GS_VIEW), 0, 2) + dir + 3) % 3;
        *(volatile int *)(CAMREC + 0xf4) = GSET(GS_VIEW);   /* what cam_ApplyViewOption does at race start */
    }
    else if (id == ROW_BACKDROP) GSET(GS_BACKDROP) ^= 1;
    else if (id == ROW_GROUND) GSET(GS_GROUND) ^= 1;
    else if (id == ROW_JOY) GSET(GS_JOY) ^= 1;
}
static void menu_page(int page, int row) { g_page = page; g_row = row; }
/* Back to the main page with the cursor on the row that opened this page. */
static void menu_up(void)
{
    int r[MAXROWS], n, i, id = g_page == PG_AUDIO ? ROW_AUDIO : g_page == PG_GRAPHICS ? ROW_GRAPHICS : ROW_CONTROLS;
    g_page = PG_MAIN; n = menu_rows(r); g_row = 0;
    for (i = 0; i < n; i++) if (r[i] == id) g_row = i;
}
static void menu_key(WPARAM k)
{
    int rows[MAXROWS], n = menu_rows(rows), id = cur_row(), i;
    if (g_waitkey) {
        /* a new key for the row; the game's key table 0x43eb8c holds -1 for keys it cannot use */
        if (k == VK_ESCAPE) g_waitkey = 0;
        else if (id == ROW_CAM0 || id == ROW_CAM1) {
            if (k != VK_RETURN && k != VK_BACK && k != VK_F1 && k > 0 && k < 0x80 && *(int *)(0x43eb8c + k * 12) != -1) {
                g_camkey[id - ROW_CAM0] = (int)k;
                cam_apply();
                settings_save();
                g_waitkey = 0;
            }
        }
        else if (k != VK_RETURN && k != VK_BACK && k != VK_F1 && k < 256 && *(int *)(0x43eb8c + k * 12) != -1) {
            GSET(GS_KEYS + id - ROW_KEY0) = (int)k;
            g_waitkey = 0;
        }
        g_eatup = k;
        g_dirty = 1;
        return;
    }
    switch (k) {
    case VK_UP: case 'W': g_row = (g_row + n - 1) % n; break;
    case VK_DOWN: case 'S': g_row = (g_row + 1) % n; break;
    case VK_LEFT: case 'A': menu_change(-1); break;
    case VK_RIGHT: case 'D': menu_change(1); break;
    case VK_RETURN: case VK_SPACE:
        if (id == ROW_CLOSE) { g_menu = 0; g_eatup = k; }
        else if (id == ROW_GAME || id == ROW_QUIT) {
            g_menu = 0; g_eatup = k;
            if (id == ROW_QUIT) g_race_quit = 1;
            else if (g_from_title == 2) g_race_opts = 1; else { g_pass_once = 1; g_post_enter = 1; }
        }
        else if (id == ROW_CONTROLS) menu_page(PG_CONTROLS, 0);
        else if (id == ROW_AUDIO) menu_page(PG_AUDIO, 0);
        else if (id == ROW_GRAPHICS) menu_page(PG_GRAPHICS, 0);
        else if (id == ROW_BACK) menu_up();
        else if (id == ROW_DEFAULTS) { for (i = 0; i < 8; i++) GSET(GS_KEYS + i) = KEY_DEFAULTS[i];
            g_camkey[0] = CAM_DEFAULTS[0]; g_camkey[1] = CAM_DEFAULTS[1]; cam_apply(); settings_save(); }
        else if (id == ROW_CAM0 || id == ROW_CAM1) g_waitkey = 1;
        else if (id >= ROW_KEY0 && id < ROW_KEY0 + 8) g_waitkey = 1;
        else if (id >= ROW_DISPLAY) menu_change(1);
        break;
    case VK_ESCAPE: case VK_F1:
        if (g_page && k == VK_ESCAPE) menu_up();
        else { g_menu = 0; g_eatup = k; }
        break;
    }
    g_dirty = 1;
}

/* The game's window procedure, wrapped: F1 opens the options overlay (which then takes every
   key), W A S D also act as the arrow keys (the game's key table DAT_004571e0 is indexed by VK),
   and WM_PAINT shows the last frame again. */
/* The WinMain loop (0x430d55) runs a frame only while the app-active flag 0x45e214 is set (WM_ACTIVATEAPP);
   a parked test window is never activated, so it stalled at the title. Parked runs keep it set. */
#define APPACTIVE 0x45e214
static int g_parked;
static LRESULT CALLBACK wrap_proc(HWND hw, UINT m, WPARAM wp, LPARAM lp)
{
    if (g_parked && m == WM_ACTIVATEAPP) wp = TRUE;
    if (getenv("SCORCHER_KEYLOG") && (m == WM_KEYDOWN || m == WM_KEYUP)) lg("key %s %x menu %d eat %x last %x", m == WM_KEYDOWN ? "down" : "up", (int)wp, g_menu, (int)g_eatup, *(int *)0x45d9cc);
    if (m == WM_KEYDOWN || m == WM_SYSKEYDOWN) key_note(wp, 1);
    else if (m == WM_KEYUP || m == WM_SYSKEYUP) key_note(wp, 0);
    else if (m == WM_LBUTTONDOWN || m == WM_LBUTTONUP) key_note(VK_LBUTTON, m == WM_LBUTTONDOWN);
    else if (m == WM_RBUTTONDOWN || m == WM_RBUTTONUP) key_note(VK_RBUTTON, m == WM_RBUTTONDOWN);
    else if (m == WM_MBUTTONDOWN || m == WM_MBUTTONUP) key_note(VK_MBUTTON, m == WM_MBUTTONDOWN);
    else if (m == WM_KILLFOCUS || (m == WM_ACTIVATEAPP && !wp)) key_clear();
    /* SCORWIN's own WM_CLOSE case (win_OnCloseFatal 0x430552) shows MessageBox "WM_CLOSE" / "Error" and
       exits with -1: save and quit quietly instead */
    if (m == WM_CLOSE) { settings_save(); lg("WM_CLOSE: exit"); if (g_log) fflush(g_log); ExitProcess(0); }
    if (m == WM_KEYUP && g_eatup && wp == g_eatup) { g_eatup = 0; return 0; }
    if ((m == WM_KEYDOWN || m == WM_SYSKEYDOWN) && wp == VK_F10 && !g_menu && !g_bsel) {
        if (!(lp & 0x40000000)) { g_keyov ^= 1; settings_save(); }
        return 0;
    }
    if ((m == WM_KEYUP || m == WM_SYSKEYUP) && wp == VK_F10) return 0;
    if (m == WM_KEYDOWN && wp == VK_F2 && !g_menu && !g_bsel) {
        if (!(lp & 0x40000000)) { g_colov ^= 1; lg("collider view %s", g_colov ? "on" : "off"); }
        return 0;
    }
    if (m == WM_KEYUP && wp == VK_F2) return 0;
    if (g_bsel) {
        if (m == WM_KEYDOWN || (m == WM_SYSKEYDOWN && wp != VK_F4)) { if (!(lp & 0x40000000)) bikesel_key(wp); return 0; }
        if (m == WM_KEYUP || m == WM_SYSKEYUP || m == WM_CHAR || m == WM_SYSCHAR) return 0;
    }
    if (g_menu) {
        if (m == WM_KEYDOWN || (m == WM_SYSKEYDOWN && wp != VK_F4)) { menu_key(wp); return 0; }
        if (m == WM_KEYUP || m == WM_SYSKEYUP || m == WM_CHAR || m == WM_SYSCHAR) return 0;
    } else if (m == WM_KEYDOWN && counter_on() && (wp == VK_LEFT || wp == VK_RIGHT || wp == 'A' || wp == 'D')) {
        g_opponents = clampi(g_opponents + (wp == VK_LEFT || wp == 'A' ? -1 : 1), 0, MAXOPP);
        settings_save();
        return 0;
    } else if (m == WM_KEYDOWN && wp == VK_F1) {
        if (!(lp & 0x40000000)) { g_row = 0; g_page = g_waitkey = 0; g_from_title = 0; g_menu = 1; g_dirty = 1; g_eatup = VK_F1; InvalidateRect(hw, 0, FALSE); }
        return 0;
    }
    if (m == WM_KEYDOWN || m == WM_KEYUP) {
        switch (wp) {
        case 'W': if (!key_bound('W')) wp = VK_UP; break;   /* unless a control is bound to the letter */
        case 'A': if (!key_bound('A')) wp = VK_LEFT; break;
        case 'S': if (!key_bound('S')) wp = VK_DOWN; break;
        case 'D': if (!key_bound('D')) wp = VK_RIGHT; break;
        }
    } else if (m == WM_PAINT) {
        PAINTSTRUCT ps;
        BeginPaint(hw, &ps); EndPaint(hw, &ps);
        show();
        return 0;
    } else if (m == WM_ERASEBKGND)
        return 1;
    else if (m == WM_GETMINMAXINFO || m == WM_WINDOWPOSCHANGING)
        return DefWindowProcA(hw, m, wp, lp);   /* the game clamps its window to 640x480 outside */
    else if (m == WM_DESTROY)
        g_menu = 0;
    return CallWindowProcA(g_oldproc, hw, m, wp, lp);
}

/* Window for the current settings:
   windowed + Auto = the old behaviour (captioned window, 4:3 client sized for the game mode, SCORCHER_SCALE);
   windowed + a resolution = captioned window with that client size;
   borderless = a popup window of that size (Auto = the whole monitor), centred on the monitor.
   The 4:3 picture is pillar-boxed inside 16:9 / 16:10 sizes. */
static void apply_window(void)
{
    HWND hw = g_dd ? g_dd->hwnd : 0;
    MONITORINFO mi;
    RECT wa, mr, r;
    int cw, ch, scale, w = g_dd ? g_dd->w : 640, x, y;
    DWORD st;
    char env[16];
    if (!hw) return;
    get_monitor(&mi);
    wa = mi.rcWork; mr = mi.rcMonitor;
    if (!res_fits(g_set.res)) g_set.res = 0;
    if (g_set.res) { cw = RES[g_set.res].w; ch = RES[g_set.res].h; }
    else if (g_set.borderless) { cw = mr.right - mr.left; ch = mr.bottom - mr.top; }
    else {
        scale = (w <= 320) ? 3 : 2;
        if (GetEnvironmentVariableA("SCORCHER_SCALE", env, sizeof env)) scale = atoi(env);
        cw = (w <= 320) ? 320 * scale : 640 * scale / 2;
        ch = cw * 3 / 4;
        while ((ch > (wa.bottom - wa.top) * 9 / 10 || cw > (wa.right - wa.left) * 9 / 10) && cw > 320) { cw -= 64; ch = cw * 3 / 4; }
    }
    r.left = 0; r.top = 0; r.right = cw; r.bottom = ch;
    if (g_set.borderless) {
        st = WS_POPUP | WS_VISIBLE;
        x = mr.left + ((mr.right - mr.left) - cw) / 2; y = mr.top + ((mr.bottom - mr.top) - ch) / 2;
    } else {
        st = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_THICKFRAME | WS_VISIBLE;
        AdjustWindowRect(&r, st, FALSE);
        x = wa.left + ((wa.right - wa.left) - (r.right - r.left)) / 2; y = wa.top + ((wa.bottom - wa.top) - (r.bottom - r.top)) / 2;
        if (x < wa.left) x = wa.left;
        if (y < wa.top) y = wa.top;
    }
    SetWindowLongA(hw, GWL_STYLE, st);
    if (GetEnvironmentVariableA("SCORCHER_PARK", env, sizeof env)) {
        /* test runs: off-screen (left of the virtual screen) and never activated */
        g_parked = 1;
        *(volatile int *)APPACTIVE = 1;
        SetWindowPos(hw, HWND_BOTTOM, GetSystemMetrics(SM_XVIRTUALSCREEN) - (r.right - r.left) - 200, 0,
                     r.right - r.left, r.bottom - r.top, SWP_FRAMECHANGED | SWP_NOACTIVATE);
    } else {
        /* activated: DirectSound mutes a window that does not have the focus */
        SetWindowPos(hw, g_set.borderless ? HWND_TOP : HWND_NOTOPMOST, x, y, r.right - r.left, r.bottom - r.top,
                     SWP_FRAMECHANGED);
        SetForegroundWindow(hw);
    }
    /* after a popup -> captioned switch Windows can keep the old frame size: correct the client size */
    GetClientRect(hw, &r);
    if (r.right != cw || r.bottom != ch) {
        RECT wr; GetWindowRect(hw, &wr);
        lg("client came out %dx%d, correcting", r.right, r.bottom);
        SetWindowPos(hw, 0, 0, 0, wr.right - wr.left + cw - r.right, wr.bottom - wr.top + ch - r.bottom,
                     SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
    }
    if (!g_hint_until) g_hint_until = GetTickCount() + 8000;
    InvalidateRect(hw, 0, FALSE);
    lg("window %dx%d client (%s, res %d) for mode %dx%d", cw, ch, g_set.borderless ? "borderless" : "windowed", g_set.res, w, g_dd->h);
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
    apply_window();
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

/* ---------------------------------------------------------------- title menu hook */
/* menu_SwitchPage 0x42485f (Watcom: EAX = new page, EDX = new row, EBX = &current page, ECX = &rows,
   two stack args, callee pops): the title's Options page opens our overlay instead. */
#define SWITCHPAGE 0x42485f
static int g_opt_page = 1;   /* the title Options page */
static int __cdecl switch_hook(int page, int row)
{
    lg("menu_SwitchPage page %d row %d (current %d)", page, row, *(int *)(*(BYTE **)0x45d854 + 0x28));
    g_curpage = page;
    if (page != g_opt_page) return 0;
    if (g_pass_once) { g_pass_once = 0; return 0; }
    g_row = 0; g_page = g_waitkey = 0; g_menu = 1; g_dirty = 1; g_from_title = 1;
    return 1;
}
static const DWORD sw_back = SWITCHPAGE + 10, sw_chk = 0x4338ef;
__declspec(naked) static void switch_stub(void)
{
    __asm {
        pushad
        push edx
        push eax
        call switch_hook
        add esp, 8
        test eax, eax
        popad
        jnz skip
        push 0x14
        push sw_back
        jmp dword ptr [sw_chk]
    skip:
        ret 8
    }
}
static void hook_code(DWORD at, void *to)
{
    DWORD old;
    BYTE *p = (BYTE *)at;
    VirtualProtect(p, 5, PAGE_EXECUTE_READWRITE, &old);
    p[0] = 0xe9; *(DWORD *)(p + 1) = (DWORD)to - (at + 5);
    VirtualProtect(p, 5, old, &old);
    FlushInstructionCache(GetCurrentProcess(), p, 5);
}

/* Esc in a race: scboard_Frmupdate 0x41f91a `mov eax, "options"; call menu_SetNext 0x408875` loads the
   in-race options level. The call at 0x41f91f is pointed at esc_stub, which opens our overlay instead;
   its "Game options..." row then calls menu_SetNext("options") itself (race_options). */
#define ESCCALL 0x41f91f
#define SETNEXT 0x408875
static const DWORD setnext = SETNEXT, opts_name = 0x43b53a;
static int __cdecl esc_hook(void)
{
    lg("race Esc: options overlay");
    g_row = 0; g_page = g_waitkey = 0; g_menu = 1; g_dirty = 1; g_from_title = 2;
    return 1;
}
__declspec(naked) static void esc_stub(void)
{
    __asm {
        pushad
        call esc_hook
        test eax, eax
        popad
        jz pass
        ret
    pass:
        jmp dword ptr [setnext]
    }
}
__declspec(naked) static void race_options(void)
{
    __asm {
        pushad
        mov eax, opts_name
        call dword ptr [setnext]
        popad
        ret
    }
}
/* Quit race: leave the race level (menu_Pop = DAT_00457078 + 1) and start the title level. */
static const DWORD title_name = 0x43bdc0;
__declspec(naked) static void race_quit(void)
{
    __asm {
        pushad
        inc dword ptr ds:[0x457078]
        mov eax, title_name
        call dword ptr [setnext]
        popad
        ret
    }
}
/* Race modes (NOTES "run 8"). The game's title rows: Championship = mode 1, Time Attack = mode 0 (with the
   enemy cars and lap times), Practice = mode 2 (alone, no lap times). We want Practice with the enemy
   cars and Time Attack alone, so both rows now set mode 0 and Time Attack sets g_encmp = 2, which the
   enemy check in enecon01_Init (`cmp [0x43eb30],2` @0x4154be, now a call to enemy_stub) reads to free the
   enemy cars. menu_ModeSelectHandler's tail 0x4230b1..0x4230c8 (EDX = row) becomes `call mode_stub`. */
#define MODETAIL 0x4230b1
#define ENEMYCMP 0x4154be
#define UNLOCKLD 0x42441c
/* Time Attack has no countdown (user 10-12): in scboard_Frmupdate the timer rec+0x64 is decremented by
   `sub ebx, eax` (29 c3) @0x41f879, so it never runs out (no TIME OUT); the two timer digits are drawn
   from 0x41f23a to 0x41f2da, which becomes `jmp 0x41f2da`. Patched when the mode row is picked. */
#define TATIMER 0x41f879
#define TADIGITS 0x41f23a
#define TADIGEND 0x41f2da
static void __cdecl ta_patch(void)
{
    static const BYTE sub[2] = { 0x29, 0xc3 }, dig[5] = { 0x8b, 0x75, 0xfc, 0x8b, 0x76 };
    static int ok = -1, last = -1;
    BYTE b[5];
    int off = g_encmp == 2 || g_practice;   /* Practice has no countdown either (user 10-13) */
    if (off == last) return;   /* the mode page calls this every frame */
    last = off;
    if (ok < 0) {
        ok = !memcmp((void *)TATIMER, sub, 2) && !memcmp((void *)TADIGITS, dig, 5);
        if (!ok) lg("time attack timer: bytes differ, skipped");
    }
    if (!ok) return;
    if (off) {
        b[0] = b[1] = 0x90; write_code(TATIMER, b, 2);
        b[0] = 0xe9; *(DWORD *)(b + 1) = TADIGEND - (TADIGITS + 5); write_code(TADIGITS, b, 5);
    } else {
        write_code(TATIMER, sub, 2);
        write_code(TADIGITS, dig, 5);
    }
    lg("race countdown %s (time attack %d practice %d)", off ? "off" : "on", g_encmp == 2, g_practice);
}
__declspec(naked) static void mode_stub(void)
{
    __asm {
        mov g_practice, 0
        mov g_encmp, 0
        cmp edx, 1
        jne not_ta
        mov dword ptr ds:[0x43eb30], 0
        mov g_encmp, 2
        jmp done
    not_ta:
        cmp edx, 2
        jne done
        mov dword ptr ds:[0x43eb30], 0
        mov g_practice, 1
    done:
        pushad
        call ta_patch
        popad
        ret
    }
}
/* Practice opponents (NOTES "Practice racer count"): chosen with Left/Right on the stage select page
   (menu page 10), shown at the top of the screen. The enemy check in enecon01_Init (`cmp [0x43eb30],2`,
   7 bytes at 0x4154be, then jne 0x4154d1) becomes `call enemy_stub` + nop; the stub returns ZF=1
   to free the car (world_QueueFree) and ZF=0 to keep it. EBP = the car's wram, +0x174 = its grid slot
   10..14 (from its set1.bin VALS). */
static BYTE *g_mxriv[16];   /* kept rival cars (enemy_keep), for the Manx catch-up pull */
static DWORD g_mxrivt;
static BYTE *g_ghost;   /* the Time Attack ghost car's world header, 0 when none (see ghost_tick) */
static void ghost_start(BYTE *wh);
static BYTE *g_grec;   /* test: SCORCHER_GHOSTTEST=1 records Practice enemy slot 0 as the ghost lap */
static void ghost_rec_start(BYTE *wh);
static int __cdecl enemy_keep(BYTE *wram, BYTE *wh)
{
    int slot = *(int *)(wram + 0x174) - 10;
    int keep = g_encmp == 2 ? slot == 0 : !g_practice || slot < g_opponents;
    if (g_encmp != 2) g_ghost = 0;
    else if (slot == 0) ghost_start(wh);   /* Time Attack: slot 0 becomes the ghost */
    if (g_encmp != 2 && g_practice && slot == 0 && GetEnvironmentVariableA("SCORCHER_GHOSTTEST", 0, 0)) ghost_rec_start(wh);
    lg("enecon01_Init slot %d -> %s (wh %p bits %02x)", slot, keep ? (g_encmp == 2 ? "ghost" : "keep") : "free", wh, wh[0]);
    if (GetTickCount() - g_mxrivt > 3000) memset(g_mxriv, 0, sizeof g_mxriv);   /* a new race: forget the last one's cars */
    g_mxrivt = GetTickCount();
    if (slot >= 0 && slot < 16) g_mxriv[slot] = keep && g_encmp != 2 ? wh : 0;
    g_curpage = -1;   /* a race is loading: the stage select page is gone */
    return keep;
}
__declspec(naked) static void enemy_stub(void)
{
    __asm {
        pushad
        push esi
        push ebp
        call enemy_keep
        add esp, 8
        test eax, eax
        popad
        ret
    }
}
/* Time Attack ghost (user 10-12, NOTES "run 10"). The game has none: we keep enemy car slot 0 alive in
   Time Attack and move it along the best lap so far. The player is recorded every RateLoop tick
   (33 ms) through the bilstyring routine slot; the ghost car's enecon01 RateLoop is replaced by
   ghost_tick, which writes the recorded position and matrix. A lap ends when the player's max lap
   wram+0xe4 goes up; ticks while input is off (0x458574, the start countdown) are not counted, so
   the ghost and the player share one lap clock. The best lap per stage is kept in ghosts\stageN.gho. */
#define RT_BILRATE 0x441214   /* routine table: bilstyring RateLoop slot */
#define RT_ENERATE 0x441248   /* routine table: enecon01 RateLoop slot */
#define GH_MAX 9000           /* ticks per lap (5 min) */
typedef struct { int pos[3], mat[9], speed; } GSAMPLE;
static GSAMPLE g_gcur[GH_MAX], g_gbest[GH_MAX];
static int g_gcur_n, g_gbest_n, g_glap, g_gstage = -1;
static DWORD g_bil_orig, g_ene_orig;
static BYTE *g_gplayer;   /* test log: the player's world header */
static void ghost_file(char *p, int stage)
{
    char *s;
    GetModuleFileNameA(0, p, MAX_PATH);
    s = strrchr(p, '\\');
    sprintf(s + 1, "ghosts");
    CreateDirectoryA(p, 0);
    sprintf(s + 1, "ghosts\\stage%d.gho", stage);
}
static void ghost_load(int stage)
{
    char p[MAX_PATH];
    FILE *f;
    int n = 0;
    g_gbest_n = 0;
    ghost_file(p, stage);
    if (!(f = fopen(p, "rb"))) return;
    if (fread(&n, 4, 1, f) == 1 && n > 0 && n <= GH_MAX && fread(g_gbest, sizeof(GSAMPLE), n, f) == (size_t)n) g_gbest_n = n;
    fclose(f);
    lg("ghost: stage %d best lap %d ticks", stage, g_gbest_n);
}
static void ghost_save(int stage)
{
    char p[MAX_PATH];
    FILE *f;
    ghost_file(p, stage);
    if (!(f = fopen(p, "wb"))) return;
    fwrite(&g_gbest_n, 4, 1, f);
    fwrite(g_gbest, sizeof(GSAMPLE), g_gbest_n, f);
    fclose(f);
}
static void ghost_start(BYTE *wh)
{
    int stage = *(int *)0x43eb38;   /* the stage picked on the stage select page */
    g_ghost = wh;
    g_glap = 0;
    g_gcur_n = 0;
    if (stage != g_gstage) { g_gstage = stage; ghost_load(stage); }
}
static void ghost_rec_start(BYTE *wh)
{
    g_grec = wh;
    g_gstage = *(int *)0x43eb38;
    g_glap = g_gcur_n = g_gbest_n = 0;
    lg("ghost test: recording enemy slot 0 on stage %d", g_gstage);
}
/* Called after each player tick (wh = the player's world header), or the test enemy's. */
static void __cdecl ghost_player_tick(BYTE *wh)
{
    BYTE *wram = *(BYTE **)(wh + 0x9c);
    int lap;
    if (wh != g_grec) g_gplayer = wh;
    if (*(int *)0x458574) return;   /* input off */
    if (wh == g_grec && !(g_gcur_n % 300)) lg("ghost test: tick %d lap e4 %d e8 %d", g_gcur_n, *(int *)(wram + 0xe4), *(int *)(wram + 0xe8));
    lap = *(int *)(wram + 0xe4);
    if (lap > g_glap) {   /* a lap is done */
        if (g_glap > 0 || g_gcur_n > 0) {
            lg("ghost: lap %d in %d ticks (best %d)", lap, g_gcur_n, g_gbest_n);
            if (g_gcur_n > 0 && (!g_gbest_n || g_gcur_n < g_gbest_n)) {
                g_gbest_n = g_gcur_n;
                memcpy(g_gbest, g_gcur, g_gcur_n * sizeof(GSAMPLE));
                ghost_save(g_gstage);
            }
        }
        g_glap = lap;
        g_gcur_n = 0;
    }
    if (g_gcur_n < GH_MAX) {
        GSAMPLE *s = &g_gcur[g_gcur_n++];
        memcpy(s->pos, wh + 0x20, 12);
        memcpy(s->mat, wh + 0x5c, 36);
        s->speed = *(int *)(wram + 0x20);
    }
}
__declspec(naked) static void bil_rate_hook(void)
{
    __asm {
        push eax
        call dword ptr [g_bil_orig]
        xchg eax, [esp]
        pushad
        cmp g_ghost, 0
        je skip
        push eax
        call ghost_player_tick
        add esp, 4
    skip:
        popad
        pop eax
        ret
    }
}
/* See-through ghost: rend_BuildPolyList 0x40b4de ends every object at 0x40b557 (`mov ebx,[esp+0x38];
   mov eax,[esp]`, 7 bytes); then the object's polygon records are the pointers in [ebx, eax) and the
   draw-list entry [esp+0x18] holds the world. Textured records (+2 & 2) of the ghost get flag 8, the
   engine's own 50% blend filler (16 bpp 0x4b5430). */
#define POLYTAIL 0x40b557
static int g_gtag;   /* ghost objects tagged (test log) */
__declspec(naked) static void poly_tag_hook(void)
{
    __asm {
        mov ebx, [esp + 0x38]
        mov eax, [esp]
        push ecx
        push edx
        mov edx, [esp + 0x20]
        mov edx, [edx]
        test edx, edx
        jz done
        cmp edx, g_ghost
        jne done
        inc g_gtag
        mov ecx, ebx
    next:
        cmp ecx, eax
        jae done
        mov edx, [ecx]
        test word ptr [edx + 2], 2
        jz skip
        or word ptr [edx + 2], 8
    skip:
        add ecx, 4
        jmp next
    done:
        pop edx
        pop ecx
        push 0x40b55e
        ret
    }
}
/* The ghost car's tick, in place of enecon01_RateLoop (no physics, smoke or sound). wh[0] bit 1 hides a
   world from the renderer and from col_Trace's car loop. col_Trace also skips the ghost after col_hook
   (the other cars drive through it). The minimap dot reads wram+0/+8; wram+0xe8 = -1 keeps the ghost
   behind the player in the place count. */
#define COLSKIP 0x417c8c   /* col_Trace car loop: `test byte [edi],1; jne 0x417c67` (5 bytes) */
__declspec(naked) static void col_hook(void)
{
    __asm {
        test byte ptr [edi], 1
        jnz skip
        cmp edi, g_ghost
        je skip
        push 0x417c91
        ret
    skip:
        push 0x417c67
        ret
    }
}
static void __cdecl ghost_tick(BYTE *wh)
{
    BYTE *wram = *(BYTE **)(wh + 0x9c), *com = *(BYTE **)(wh + 0x98);
    GSAMPLE *s;
    int i = g_gcur_n;
    if (!g_gbest_n || *(int *)0x458574) {   /* nothing recorded yet, or the countdown: hide it */
        if (!g_gbest_n) { wh[0] |= 1; *(int *)(wram + 0) = *(int *)(wram + 8) = 0x7fff0000; return; }
        i = 0;
    }
    if (!(g_gcur_n % 150)) lg("ghost: tick %d of %d, tagged %d, bits %02x, ghost %d %d player %d %d", g_gcur_n, g_gbest_n, g_gtag, wh[0], *(int *)(wh + 0x20) >> 16, *(int *)(wh + 0x28) >> 16, g_gplayer ? *(int *)(g_gplayer + 0x20) >> 16 : 0, g_gplayer ? *(int *)(g_gplayer + 0x28) >> 16 : 0);
    if (i >= g_gbest_n) i = g_gbest_n - 1;   /* slower than the best: it waits at the line */
    s = &g_gbest[i];
    wh[0] &= ~1;
    memcpy(wh + 0x2c, wh + 0x20, 12);
    memcpy(wh + 0x38, wh + 0x5c, 36);
    memcpy(wh + 0x20, s->pos, 12);
    memcpy(wh + 0x5c, s->mat, 36);
    if (g_gplayer && GetEnvironmentVariableA("SCORCHER_GHOSTNEAR", 0, 0)) {   /* test: park it in front of the player */
        int k, *pp = (int *)(g_gplayer + 0x20), *pm = (int *)(g_gplayer + 0x5c);
        char nv[16] = "6"; double d;
        GetEnvironmentVariableA("SCORCHER_GHOSTNEAR", nv, sizeof nv); d = atof(nv);
        double len = sqrt((double)pm[6] * pm[6] + (double)pm[7] * pm[7] + (double)pm[8] * pm[8]) + 1;
        for (k = 0; k < 3; k++) ((int *)(wh + 0x20))[k] = pp[k] + (int)(pm[6 + k] * (d * 65536 / len));
        memcpy(wh + 0x5c, pm, 36);
        memcpy(s->pos, wh + 0x20, 12);   /* g_gbest is a test copy here */
        if (!(g_gcur_n % 150)) lg("ghost near: row2 %d %d %d", pm[6], pm[7], pm[8]);
    }
    memcpy(wram, s->pos, 12);
    *(int *)(wram + 0x20) = s->speed;
    *(int *)(wram + 0xe8) = -1;
    if (com) { memcpy(com + 4, s->pos, 12); memset(com + 0x18, 0, 12); }
}
/* Race start (user 10-13: "all racers start at the same time and at speed zero"). Stock enecon01_RateLoop
   (0x41b54d) staggers the rivals: slot n = state+0x174 - 10 rolls from game tick n*15+10 (ctx 0x457064 +0x130
   game ms / 33), i.e. during the 3-2-1 intro (input off 0x458574 until tick 75), with a fixed launch path
   speed for 40 ticks and then the full recorded path speed. Now: frozen (the call sees game ms 0, so its own
   freeze flag 0x45d484 path runs) until the player gets control, then the stagger is skipped (0x43eb34 = 1
   for the call) and the path speed state+0xf8 (16.16 points per tick, added to +0xfc by path_Advance) is
   scaled by a ramp from 0 to 1 over ENE_RAMP ms of race time. */
#define ENE_RAMP 3500
static int g_ene_go = -1, g_ene_mode, g_ene_s43, g_ene_sms, g_ene_sf8;
static void __cdecl ene_pre(BYTE *wh)
{
    BYTE *ctx = *(BYTE **)0x457064, *st = *(BYTE **)(wh + 0x9c);
    int t = *(int *)(ctx + 0x130);
    g_ene_mode = 0;
    if (*(int *)0x43eb34 == 1 || !st) return;   /* stock no-stagger mode (end sequence) */
    if (t / 33 < 10) g_ene_go = -1;   /* a new race */
    if (g_ene_go < 0) {
        if (t / 33 < 75 || *(int *)0x458574) {
            g_ene_mode = 1; g_ene_sms = t; *(int *)(ctx + 0x130) = 0;
            return;
        }
        g_ene_go = t;
        lg("rivals go at %d ms (path speed %d)", t, *(int *)(st + 0xf8));
    }
    g_ene_mode = 2; g_ene_s43 = *(int *)0x43eb34; *(int *)0x43eb34 = 1;
    g_ene_sf8 = *(int *)(st + 0xf8);
    if (t - g_ene_go < ENE_RAMP) {
        double k = (double)(t - g_ene_go) / ENE_RAMP;
        *(int *)(st + 0xf8) = (int)(g_ene_sf8 * k);
    }
}
static void __cdecl ene_post(BYTE *wh)
{
    BYTE *st = *(BYTE **)(wh + 0x9c);
    if (g_ene_mode == 1) *(int *)(*(BYTE **)0x457064 + 0x130) = g_ene_sms;
    else if (g_ene_mode == 2) { *(int *)0x43eb34 = g_ene_s43; if (st) *(int *)(st + 0xf8) = g_ene_sf8; }
    g_ene_mode = 0;
}
__declspec(naked) static void ene_run(void)
{
    __asm {
        push eax
        pushad
        push eax
        call ene_pre
        add esp, 4
        popad
        call dword ptr [g_ene_orig]
        pushad
        push dword ptr [esp + 32]
        call ene_post
        add esp, 4
        popad
        add esp, 4
        ret
    }
}
__declspec(naked) static void ene_rate_hook(void)
{
    __asm {
        cmp eax, g_ghost
        je ghost
        cmp eax, g_grec
        je rec
        jmp ene_run
    rec:
        push eax
        call dword ptr [g_ene_orig]
        xchg eax, [esp]
        pushad
        push eax
        call ghost_player_tick
        add esp, 4
        popad
        pop eax
        ret
    ghost:
        pushad
        push eax
        call ghost_tick
        add esp, 4
        popad
        ret
    }
}
static void write_code(DWORD at, const void *b, int n)
{
    DWORD old;
    VirtualProtect((void *)at, n, PAGE_EXECUTE_READWRITE, &old);
    memcpy((void *)at, b, n);
    VirtualProtect((void *)at, n, old, &old);
    FlushInstructionCache(GetCurrentProcess(), (void *)at, n);
}
static void cam_apply(void)
{
    static const BYTE orig[2][3] = { { 0x8a, 0x40, 0x09 }, { 0x8a, 0x40, 0x10 } };
    static int ok = -1;
    int i;
    if (ok < 0) {   /* first call: check the original instructions */
        ok = !memcmp((void *)(CAMKEY_AT[0] - 2), orig[0], 3) && !memcmp((void *)(CAMKEY_AT[1] - 2), orig[1], 3);
        if (!ok) lg("camera keys: bytes differ, not rebindable");
    }
    if (!ok) return;
    /* user 10-13 "remove the zoom" / "just remove in-car camera": both camera key tests read key slot 0, which no
       key sets, so the FOV stays 0x4000 and the view stays the chase camera (g_camkey kept in the ini, unused) */
    for (i = 0; i < 2; i++) { BYTE b = 0; write_code(CAMKEY_AT[i], &b, 1); }
    lg("camera keys %02x %02x", g_camkey[0], g_camkey[1]);
}
/* More rivals in Practice (user 10-12, NOTES "r4 rivals"). Stages have 3 (stage 1) or 5 enemy cars,
   worlds fjende1..5. When the Practice opponent count is higher, the levN.txt setup gets extra
   `Wfjende<k> <id> <inst>` lines; a world name loaded twice reuses its .wl3, so each extra car only needs
   worlds\fjende<k>\set<inst>.bin, which we serve as a copy of set1.bin with a new grid position (rows
   behind the last car; one row = pos(fjende3) - pos(fjende1) on every stage) and a lower path speed.
   The copy keeps fjende<k>'s grid slot (wram+0x174), so the minimap dot colour and the start stagger
   match its model. Both hooks sit on file_Load's calls to file_Open and file_ReadInto. */
#define FOPEN_AT 0x42e0bb   /* file_Load: call file_Open 0x42d4bd (EAX name -> size) */
#define FREAD_AT 0x42e0f2   /* file_Load: call file_ReadInto 0x42d3f1 (EAX buffer, EDX name) */
#define PLACESW 0x41f52a    /* scboard place switch: `cmp eax, 5; ja sys_Fatal` (place 7+ has no case) */
static void hook_call(DWORD at, void *to);
typedef struct { int k, inst, pos[3], speed; } XRIV;
static XRIV g_xr[MAXOPP];
static int g_xn, g_xtxt_len, g_xtxt_add;
static char g_xtxt[MAX_PATH], g_xadd[MAXOPP * 24];
/* Calls a Watcom register-convention function fn(EAX = a, EDX = d). */
static int __declspec(naked) watcall2(DWORD fn, DWORD a, DWORD d)
{
    __asm {
        push ebx
        push ecx
        push esi
        push edi
        push ebp
        mov eax, [esp + 28]
        mov edx, [esp + 32]
        call dword ptr [esp + 24]
        pop ebp
        pop edi
        pop esi
        pop ecx
        pop ebx
        ret
    }
}
static int game_open(const char *n) { return watcall2(0x42d4bd, (DWORD)n, 0); }
static int game_exists(const char *n) { return watcall2(0x42d5a4, (DWORD)n, 0); }   /* file_Exists; file_Open is fatal on a missing file */
static void game_read(void *b, const char *n) { watcall2(0x42d3f1, (DWORD)b, (DWORD)n); }
static int be32(const BYTE *p) { return p[0] << 24 | p[1] << 16 | p[2] << 8 | p[3]; }
static void put_be32(BYTE *p, int v) { p[0] = (BYTE)(v >> 24); p[1] = (BYTE)(v >> 16); p[2] = (BYTE)(v >> 8); p[3] = (BYTE)v; }
/* Reads <dir>worlds\<w>\set1.bin: the SMAT position (16.16 -> units) and the VALS path speed. */
static int read_set(const char *dir, const char *w, double *pos, int *speed)
{
    char n[MAX_PATH];
    int len, i, ok = 0;
    BYTE *b;
    sprintf(n, "%sworlds\\%s\\set1.bin", dir, w);
    if (!game_exists(n) || (len = game_open(n)) <= 0 || !(b = (BYTE *)calloc(len + 16, 1))) return 0;
    game_read(b, n);
    for (i = 0; i + 0x18 <= len; i += 4)
        if (!memcmp(b + i, "SMAT", 4)) {
            pos[0] = be32(b + i + 0xc) / 65536.0; pos[1] = be32(b + i + 0x10) / 65536.0; pos[2] = be32(b + i + 0x14) / 65536.0;
            ok = 1;
            break;
        }
    if (speed) *speed = len >= 0x60 ? be32(b + 0x5c) : 0;
    free(b);
    return ok;
}
/* A levN.txt is being opened: count its enemy worlds and plan the extra cars. */
static void rivals_plan(const char *txt)
{
    char dir[MAX_PATH], w[16], *s;
    double fp[5][3], tp[3], u[3], step, dmax, d;
    int fs[5], m = 0, want, e, i, j;
    g_xn = 0;
    if (!g_practice || g_encmp == 2) return;   /* Time Attack keeps slot 0 only (the ghost) */
    strcpy(dir, txt);
    if (!(s = strrchr(dir, '\\'))) return;
    s[1] = 0;
    while (m < 5) {
        sprintf(w, "fjende%d", m + 1);
        if (!read_set(dir, w, fp[m], &fs[m])) break;
        m++;
    }
    want = g_opponents - m;
    if (m < 3 || want <= 0 || !read_set(dir, "ting", tp, 0)) return;
    for (i = 0; i < 3; i++) u[i] = fp[2][i] - fp[0][i];
    step = sqrt(u[0] * u[0] + u[2] * u[2]);
    if (step < 1) return;
    u[0] /= step; u[1] = 0; u[2] /= step;
    dmax = (tp[0] - fp[0][0]) * u[0] + (tp[2] - fp[0][2]) * u[2];
    for (j = 0; j < m; j++) {
        d = (fp[j][0] - fp[0][0]) * u[0] + (fp[j][2] - fp[0][2]) * u[2];
        if (d > dmax) dmax = d;
    }
    for (e = 0; e < want; e++) {
        XRIV *x = &g_xr[g_xn++];
        int b = e & 1, row = e / 2 + 1;   /* two columns, behind fjende1 and fjende2 */
        double db = (fp[b][0] - fp[0][0]) * u[0] + (fp[b][2] - fp[0][2]) * u[2], off = dmax - db + step * row;
        x->k = e % m + 1;
        x->inst = e / m + 2;
        for (i = 0; i < 3; i++) x->pos[i] = (int)((fp[b][i] + u[i] * off) * 65536.0);
        x->speed = fs[x->k - 1] - 0x300 * row;
    }
    lg("rivals: %s has %d enemies, %d extra (row step %.2f)", txt, m, g_xn, step);
}
/* The name of an extra car's set file, or -1. */
static int rivals_find(const char *n)
{
    int i;
    for (i = 0; i < g_xn; i++) {
        char want[32];
        int l = sprintf(want, "fjende%d\\set%d.bin", g_xr[i].k, g_xr[i].inst), ln = (int)strlen(n);
        if (ln > l && !_stricmp(n + ln - l, want)) return i;
    }
    return -1;
}
/* Manx TT bike (user 10-13, option "Bike"): the player world ting (worlds\ting\ting.wl3 and bigs\ting.0) is
   served from manx\ting.wl3 / manx\ting.0 next to the exe (made by tools\manx2ting.py --pc; ripped data, local
   only). file_Load opens then reads the same name, so the file is loaded at open and copied at read. */
static BYTE *g_mxbuf, *g_mxwl3;   /* g_mxwl3: the game's ting.wl3 buffer (rider poses + wheel spin write its vertices) */
static int g_mxlen, g_mxposeok = -1;
static char g_mxname[MAX_PATH];
static int manx_open(const char *n)
{
    static const char *const want[2] = { "worlds\\ting\\ting.wl3", "bigs\\ting.0" }, *const file[2] = { "ting.wl3", "ting.0" };
    char p[MAX_PATH], *s;
    int i, ln = (int)strlen(n), l;
    FILE *f;
    g_mxname[0] = 0;
    if (!g_set.manx) return -1;
    for (i = 0; i < 2; i++) {
        l = (int)strlen(want[i]);
        if (ln >= l && !_stricmp(n + ln - l, want[i])) break;
    }
    if (i == 2) return -1;
    GetModuleFileNameA(0, p, MAX_PATH);
    s = strrchr(p, '\\');
    sprintf(s + 1, "manx\\%s", file[i]);
    if (!(f = fopen(p, "rb"))) { lg("manx: %s missing, stock bike", p); return -1; }
    fseek(f, 0, SEEK_END); l = ftell(f); fseek(f, 0, SEEK_SET);
    free(g_mxbuf);
    g_mxbuf = (BYTE *)malloc(l);
    g_mxlen = g_mxbuf ? (int)fread(g_mxbuf, 1, l, f) : 0;
    fclose(f);
    if (g_mxlen != l) return -1;
    strcpy(g_mxname, n);
    lg("manx: serving %s for %s (%d bytes)", p, n, l);
    return l;
}
static int __cdecl rivals_open(const char *n)
{
    const char *f = strrchr(n, '\\');
    char tmp[MAX_PATH];
    int len, i;
    f = f ? f + 1 : n;
    if (getenv("SCORCHER_FILELOG")) lg("open %s", n);
    if ((len = manx_open(n)) > 0) return len;
    if (!_strnicmp(f, "lev", 3) && strlen(f) > 4 && !_stricmp(f + strlen(f) - 4, ".txt") && strstr(n, "setups")) {
        if (g_curpage > 0) { g_curpage = -1; bikesel_run(); }   /* chosen from a menu, not the title demo */
        rivals_plan(n);
        len = game_open(n);
        g_xtxt[0] = 0;
        if (len > 0 && g_xn) {
            char *p = g_xadd;
            for (i = 0; i < g_xn; i++) {
                p += sprintf(p, "Wfjende%d", g_xr[i].k) + 1;
                p += sprintf(p, "%d", 20 + g_xr[i].k) + 1;
                p += sprintf(p, "%d", g_xr[i].inst) + 1;
                p += sprintf(p, "0") + 1;
                p += sprintf(p, "0") + 1;
            }
            strcpy(g_xtxt, n);
            g_xtxt_len = len;
            g_xtxt_add = (int)(p - g_xadd);
            len += g_xtxt_add;
        }
        return len;
    }
    if (rivals_find(n) >= 0) {
        strcpy(tmp, n);
        strcpy(tmp + strlen(tmp) - 8, "set1.bin");
        return game_open(tmp) + 16;   /* +16: zero padding */
    }
    return game_open(n);
}
/* User 10-13 "drop from the racers the wireframe greenthing": in a rival bike world (worlds\fjendeN\fjendeN.wl3,
   raw big-endian before wl3_PrepareFile) the 30-vertex object at the origin is the wireframe sphere; its vertices
   collapse to the origin so it draws nothing, records and faces unchanged (as tools\manx2ting.py does for ting). */
static void nosphere(BYTE *b, const char *n)
{
    const char *f = strrchr(n, '\\');
    int t, cnt, i, o, nv, vo, z = 0;
    f = f ? f + 1 : n;
    if (_strnicmp(f, "fjende", 6) || _stricmp(f + strlen(f) - 4, ".wl3")) return;
    t = be32(b); cnt = be32(b + 0x10);
    if (t <= 0 || t > 0x100000 || cnt <= 0 || cnt > 64) return;
    for (i = 0; i < cnt; i++) {
        o = t + be32(b + t + 4 * i);
        nv = be32(b + o + 0x18);
        if (nv != 30 || be32(b + o + 0x2c) || be32(b + o + 0x30) || be32(b + o + 0x34)) continue;
        vo = o + be32(b + o + 4);
        if (vo <= 0 || vo > 0x100000) continue;
        memset(b + vo, 0, 6 * nv);
        z++;
    }
    lg("nosphere: %s, %d sphere%s hidden", f, z, z == 1 ? "" : "s");
}
static void __cdecl rivals_read(BYTE *b, const char *n)
{
    char tmp[MAX_PATH];
    int i;
    if (g_mxname[0] && !strcmp(n, g_mxname)) {
        memcpy(b, g_mxbuf, g_mxlen);
        if (strstr(n, ".wl3")) { g_mxwl3 = b; g_mxposeok = -1; }
        g_mxname[0] = 0;
        return;
    }
    if (g_xtxt[0] && !strcmp(n, g_xtxt)) {
        /* insert the extra lines after the last Wfjende entry (5 strings each) */
        char *p = (char *)b, *end = p + g_xtxt_len, *at = end, *q;
        game_read(b, n);
        for (q = p; q < end; q += strlen(q) + 1)
            if (!strncmp(q, "Wfjende", 7)) {
                int t;
                at = q;
                for (t = 0; t < 5 && at < end; t++) at += strlen(at) + 1;
            }
        memmove(at + g_xtxt_add, at, end - at);
        memcpy(at, g_xadd, g_xtxt_add);
        g_xtxt[0] = 0;
        return;
    }
    if ((i = rivals_find(n)) >= 0) {
        XRIV *x = &g_xr[i];
        strcpy(tmp, n);
        strcpy(tmp + strlen(tmp) - 8, "set1.bin");
        game_read(b, tmp);
        put_be32(b + 0x5c, x->speed);
        put_be32(b + 0x88, x->pos[0]); put_be32(b + 0x8c, x->pos[1]); put_be32(b + 0x90, x->pos[2]);
        lg("rivals: %s = fjende%d copy at %.1f %.1f, speed %x", n, x->k, x->pos[0] / 65536.0, x->pos[2] / 65536.0, x->speed);
        return;
    }
    game_read(b, n);
    nosphere(b, n);
}
__declspec(naked) static void fopen_hook(void)
{
    __asm {
        pushad
        push eax
        call rivals_open
        add esp, 4
        mov [esp + 28], eax
        popad
        ret
    }
}
__declspec(naked) static void fread_hook(void)
{
    __asm {
        pushad
        push edx
        push eax
        call rivals_read
        add esp, 8
        popad
        ret
    }
}
__declspec(naked) static void place_hook(void)
{
    __asm {
        cmp eax, 5
        jbe ok
        mov eax, 5   /* 7th and lower show as 6th */
    ok:
        jmp dword ptr [eax * 4 + 0x41ea70]
    }
}
/* Manx TT physics (user 10-13, option "Bike" = Manx TT). The player's car_ApplyControls 0x415f2a call in
   car_ReadInputApply (0x41afa0: EAX wh, EDX wram, EBX ctrl[8]) goes through manx_ctl_hook. Before the stock
   call the speed/brake/steer keys are cleared and the wheel speed wram+0x130/+0x134 set to the current speed
   (then the stock coast branch adds nothing); after it, the forward speed is set from the Manx bike model
   (port of native\game\manx_bike.h: torque curve 0x0603AF7C, 6 gears, drag, 500 kgf brake, slope; lean
   spring-damper turns into the yaw command wram+0x16c that car_ApplySteerYaw uses). Boost and jump stay stock.
   Forward = the shorts at wram+0x4c/+0x4e/+0x50 (as car_ApplyControls reads them), y is down.
   Scale: g_mxscale world units per tick per m/s (ini [game] manx_scale; 5200 = ~17% over the stock
   maximum 0x56b80). Collisions slow the bike: the model speed is re-read from the real forward speed each tick. */
#define MXCTL_AT 0x41afa0
#define MXCTL_FN 0x415f2a
static const double MX_TORQUE[151] = {
    3.43, 3.56, 3.7, 3.83, 3.96, 4.09, 4.22, 4.35, 4.48, 4.6, 4.73, 4.85, 4.98, 5.1, 5.21, 5.33, 5.44, 5.56, 5.67, 5.77, 5.88,
    5.98, 6.08, 6.17, 6.26, 6.35, 6.44, 6.52, 6.6, 6.67, 6.74, 6.81, 6.87, 6.93, 6.98, 7.03, 7.08, 7.12, 7.16, 7.19, 7.22, 7.25,
    7.27, 7.28, 7.3, 7.3, 7.3, 7.33, 7.36, 7.38, 7.41, 7.43, 7.46, 7.48, 7.51, 7.53, 7.55, 7.57, 7.59, 7.61, 7.63, 7.64, 7.65,
    7.67, 7.68, 7.69, 7.7, 7.7, 7.71, 7.71, 7.71, 7.7, 7.69, 7.66, 7.63, 7.59, 7.54, 7.49, 7.44, 7.38, 7.32, 7.27, 7.22, 7.17,
    7.13, 7.09, 7.07, 7.06, 7.05, 7.06, 7.08, 7.11, 7.15, 7.21, 7.27, 7.34, 7.42, 7.5, 7.58, 7.66, 7.74, 7.82, 7.89, 7.95, 8.01,
    8.05, 8.08, 8.1, 8.11, 8.11, 8.1, 8.08, 8.06, 8.04, 8.01, 7.97, 7.93, 7.89, 7.84, 7.79, 7.73, 7.67, 7.61, 7.54, 7.48, 7.41,
    7.34, 7.27, 7.19, 7.1, 7.0, 6.91, 6.81, 6.72, 6.62, 6.53, 6.43, 6.34, 6.24, 6.15, 6.05, 5.96, 5.86, 5.77, 5.67, 5.58, 5.48,
    5.39, 5.29, 5.2};
static const double MX_RATIO[7] = { 0, 2.4, 1.941, 1.631, 1.434, 1.291, 1.192 };
static const double MX_EFF[7] = { 1.0, 0.92, 0.9, 0.88, 0.86, 0.83, 0.8 };
#define MX_MASS 297.0
#define MX_DT 0.033   /* Scorcher RateLoop tick (Manx PAL tick is 0.04) */
static struct { double speed, rpm, steer, lean, lean_v, roll, ptarget, ax, slip, wheel, vy, imp, sq, idlet, catchf, r, vb; int gear, shift, air, init, wasground, crash, idle; } g_mx;
/* chase camera state (per tick from manx_post, used per frame by manx_cam_post) */
static struct { double pos[3], yaw, pitch, cyaw, cpitch, speed; int have; DWORD t; } g_mxc;
static int g_mxsteersign = 1, g_mxcam = 1, g_mxcamk = 2500, g_mxtilt = 10, g_mxgrip = 50, g_mxairg = 50;
static BYTE *g_mxwh, *g_mxwram;
static int *g_mxctl, g_mxkeys[8], g_mxlive;
static int g_mxrev = 4, g_mxsquat = 100, g_mxcatch = 1;
static int g_mxscale = 4400, g_mxleanvis = 100, g_mxaccel = 200, g_mxlogt, g_mxkick = 0, g_mxtyre = 0, g_mxmu = 110, g_mxfull = 1, g_mxrider = 1, g_mxspin = 1, g_mxcrash = 1;
static int g_mxcolr, g_mxcolflip, g_mxcolset;
static int g_mxwheel = 1, g_mxwheelr = 1500, g_mxwheelon;   /* ini manx_wheelcol / manx_wheelr: one small sphere per wheel */
static double g_mxcolt[3];   /* ini manx_colr: 0 = stock 64000, else the Manx bike sphere radius */   /* player collision sphere radius, *(*(wh + 0x98) + 0x10) as col_Trace reads it */
static int g_mxst = 1, g_mxyawi = 150, g_mxus = 105;   /* single-track model (ini manx_model 1), yaw inertia %, understeer % */
static double g_mxp0[3], g_mxv0[3];
static double mxclamp(double v, double lo, double hi) { return v < lo ? lo : v > hi ? hi : v; }
static void mx_fwd(BYTE *wram, double f[3])
{
    double l;
    f[0] = *(short *)(wram + 0x4c); f[1] = *(short *)(wram + 0x4e); f[2] = *(short *)(wram + 0x50);
    l = sqrt(f[0] * f[0] + f[1] * f[1] + f[2] * f[2]);
    if (l < 1) { f[0] = f[1] = 0; f[2] = 1; return; }
    f[0] /= l; f[1] /= l; f[2] /= l;
}
static void __cdecl manx_pre(BYTE *wh, BYTE *wram, int *ctl)
{
    int i;
    g_mxlive = g_set.manx;
    if (!g_mxlive) return;
    if (wh != g_mxwh) { memset(&g_mx, 0, sizeof g_mx); memset(&g_mxc, 0, sizeof g_mxc); g_mx.rpm = 1300; g_mx.gear = 1; g_mxwh = wh; lg("manx: physics on, scale %d", g_mxscale); }
    g_mxwram = wram; g_mxctl = ctl;
    for (i = 0; i < 8; i++) g_mxkeys[i] = ctl[i];
    ctl[0] = ctl[1] = ctl[2] = ctl[3] = ctl[4] = ctl[5] = 0;
    *(int *)(wram + 0x130) = *(int *)(wram + 0x134) = *(int *)(wram + 0x20);
    *(int *)(wram + 0x16c) = 0;
}
static double mx_dot(const double a[3], const double b[3]) { return a[0] * b[0] + a[1] * b[1] + a[2] * b[2]; }
static void mx_rot(double v[3], const double k[3], double a)   /* Rodrigues, k unit */
{
    double c = cos(a), sn = sin(a), d = mx_dot(k, v), x[3];
    int i;
    x[0] = k[1] * v[2] - k[2] * v[1]; x[1] = k[2] * v[0] - k[0] * v[2]; x[2] = k[0] * v[1] - k[1] * v[0];
    for (i = 0; i < 3; i++) v[i] = v[i] * c + x[i] * sn + k[i] * d * (1 - c);
}
/* rotates the car's orientation bases (+0x40/+0x4c/+0x58: f and s as 2.14 shorts at +0 and +6) and the body
   matrix +0x88 (3 rows of 2.30 ints) by a about k, as native rotate_car (scorcher_layer.cpp) */
static void mx_rotate_car(BYTE *wram, const double k[3], double a)
{
    static const int base[3] = { 0x40, 0x4c, 0x58 };
    int b, o, i;
    double v[3];
    for (b = 0; b < 3; b++)
        for (o = 0; o <= 6; o += 6) {
            short *q = (short *)(wram + base[b] + o);
            for (i = 0; i < 3; i++) v[i] = q[i];
            mx_rot(v, k, a);
            for (i = 0; i < 3; i++) q[i] = (short)floor(mxclamp(v[i], -32767, 32767) + 0.5);
        }
    for (b = 0; b < 3; b++) {
        int *q = (int *)(wram + 0x88 + 12 * b);
        for (i = 0; i < 3; i++) v[i] = q[i] / 1073741824.0;
        mx_rot(v, k, a);
        for (i = 0; i < 3; i++) q[i] = (int)floor(mxclamp(v[i], -1.99, 1.99) * 1073741824.0 * 0.999999 + 0.5);
    }
}
/* bike frame from base 0: nose f (+0x40), side s (+0x46), up u = f x s (unsigned, as native) */
static int mx_frame(BYTE *wram, double f[3], double sd[3], double u[3])
{
    double fl, sl, ul;
    int i;
    for (i = 0; i < 3; i++) { f[i] = *(short *)(wram + 0x40 + 2 * i); sd[i] = *(short *)(wram + 0x46 + 2 * i); }
    u[0] = f[1] * sd[2] - f[2] * sd[1]; u[1] = f[2] * sd[0] - f[0] * sd[2]; u[2] = f[0] * sd[1] - f[1] * sd[0];
    fl = sqrt(mx_dot(f, f)); sl = sqrt(mx_dot(sd, sd)); ul = sqrt(mx_dot(u, u));
    if (fl < 1 || sl < 1 || ul < 1) return 0;
    for (i = 0; i < 3; i++) { f[i] /= fl; sd[i] /= sl; u[i] /= ul; }
    return 1;
}
#define MX_DEG (3.14159265358979 / 180)
/* Manx catch-up (car_ComputeCatchupBonus 0x060330e0, added to the forward force above 5 m/s): toward the racer one
   rank ahead when within 600 m and 0xC3/4096 of a turn (17.1 deg) of dead ahead: (600 - dist) * (0xC3 - angle) * 100
   / 65536, capped at 80. Scorcher's ranks are not read here: the nearest kept rival inside that cone stands in for the
   racer one rank ahead (the leader with nobody in front gets none). ini manx_catchup. */
#define MX_SQM 1660.0
static double mx_catchup(const double *t)
{
    int i, *pp = (int *)(g_mxwh + 0x20);
    double upm = g_mxscale / MX_DT, best = -1, th = sqrt(t[0] * t[0] + t[2] * t[2]);
    if (th < 1e-6) return 0;
    for (i = 0; i < 16; i++) {
        int *rp;
        double dx, dz, d, a;
        if (!g_mxriv[i] || g_mxriv[i] == g_mxwh) continue;
        rp = (int *)(g_mxriv[i] + 0x20);
        dx = (rp[0] - (double)pp[0]) / upm; dz = (rp[2] - (double)pp[2]) / upm;
        d = sqrt(dx * dx + dz * dz);
        if (d < 1 || d >= 600) continue;
        a = acos(mxclamp((dx * t[0] + dz * t[2]) / (d * th), -1, 1)) * 4096 / (2 * 3.14159265358979);
        if (a > 0xc3) continue;
        if (best < 0 || d < best) { best = d; g_mx.catchf = (600 - d) * (0xc3 - a) * 100 / 65536; }
    }
    return best < 0 ? 0 : mxclamp(g_mx.catchf, 0, 80);
}
static void __cdecl manx_post(void)
{
    BYTE *wram = g_mxwram;
    int *k = g_mxkeys, i, *vel;
    double f[3], sd[3], u[3], t[3], vf, vs, ratio, target, drag, F, x, gain, sp, rate, pitch, s0;
    int ground;
    if (!g_mxlive) return;
    for (i = 0; i < 8; i++) g_mxctl[i] = k[i];
    if (!mx_frame(wram, f, sd, u)) return;
    { BYTE *shr = g_mxwh ? *(BYTE **)(g_mxwh + 0x98) : 0; if (shr && g_mxcolset > 0) *(int *)(shr + 0x10) = g_mxcolset; g_mxcolr = shr ? *(int *)(shr + 0x10) : 0; }
    mx_fwd(wram, t);
    vel = (int *)(wram + 0xc);
    vf = (vel[0] * t[0] + vel[1] * t[1] + vel[2] * t[2]) / g_mxscale;   /* m/s along the thrust axis */
    vs = vel[0] * sd[0] + vel[1] * sd[1] + vel[2] * sd[2];
    ground = *(int *)(wram + 0x3c) > 0;
    /* the model speed persists across ticks (stock damping no longer eats it); a crash (measured well
       below the model) pulls it down */
    if (!g_mx.init) { g_mx.init = 1; g_mx.speed = vf > 0 ? vf : 0; g_mx.wasground = ground; g_mx.r = 0; }
    if (g_mx.speed > 0 && vf < g_mx.speed * 0.85 - 0.5) g_mx.speed = vf > 0 ? vf : 0;
    s0 = g_mx.speed;
    /* rpm and gears */
    ratio = MX_RATIO[g_mx.gear];
    target = mxclamp(g_mx.speed * 4.5605 * 30.715 * ratio, 1300, 15000);
    if (fabs(target - g_mx.rpm) < 200) g_mx.rpm = target;
    else g_mx.rpm += (target - g_mx.rpm) * (target > g_mx.rpm ? 0.67 : 0.54);
    if (g_mx.shift > 0) g_mx.shift--;
    else if (g_mx.rpm > 13000 && g_mx.gear < 6) { g_mx.gear++; g_mx.shift = 26; }
    else if (g_mx.rpm < 10000 && g_mx.gear > 1) { g_mx.gear--; g_mx.shift = 26; }
    /* forces */
    g_mx.air = ground ? 0 : g_mx.air + 1;
    drag = 0.0334 * 0.502 * g_mx.speed * g_mx.speed;
    if (g_mx.air >= 5) F = -drag;
    else {
        double thr = k[0] && !g_mx.crash ? 1 : 0, eng;
        eng = MX_TORQUE[(int)mxclamp(g_mx.rpm / 100, 0, 150)] * thr * 14.67 * ratio * MX_EFF[g_mx.gear];
        eng -= g_mx.rpm * 0.01 * (1 - thr) * ratio;
        F = eng + MX_MASS * t[1] - 0.012 * MX_MASS - drag;   /* y down: nose up (t[1] < 0) slows; rolling resistance */
        if (k[1]) F -= 500;
    }
    if (F > 0) F *= g_mxaccel / 100.0;   /* arcade pull (ini manx_accel %); braking and drag stay Manx */
    g_mx.catchf = g_mxcatch && g_mx.speed > 5 && g_mx.air < 5 ? mx_catchup(t) : 0;
    F += g_mx.catchf;
    g_mx.ax = F * 9.81 / (MX_MASS + 12);   /* m/s^2 along the bike, shares the tyre friction circle */
    g_mx.speed += g_mx.ax * MX_DT;
    if (GetTickCount() - g_mxlogt > 1000) { g_mxlogt = GetTickCount(); lg("manx spd model %.1f measured %.1f gear %d rpm %d ground %d slip %.1f deg lean %.1f deg catch %.0f squat %.2f st %d r %.2f", g_mx.speed, vf, g_mx.gear, (int)g_mx.rpm, ground, g_mx.slip / MX_DEG, g_mx.roll / MX_DEG, g_mx.catchf, g_mx.sq, g_mxst, g_mx.r); }
    if (g_mx.crash > 0) {
        /* Manx crash state (car_UpdateFallState 0x060321bc): 3 ticks, speed x0.98 on the first, x0.91 after, no drive */
        g_mx.speed *= g_mx.crash == 3 ? 0.98 : 0.91;
        g_mx.crash--;
    }
    /* reverse (user 10-13 "theres no backup"; Manx has none): Down on a stopped bike backs up to manx_reverse m/s,
       throttle or letting go brings it back to 0 */
    if (g_mxrev && (s0 < 0 || (g_mx.speed <= 0 && k[1] && !k[0] && ground))) {
        double r = s0 < 0 ? s0 : 0;
        if (k[1] && !k[0] && ground) r = mxclamp(r - 3.0 * MX_DT, -g_mxrev, 0);
        else if (k[0]) r += 8.0 * MX_DT;
        else r = r + 4.0 * MX_DT > 0 ? 0 : r + 4.0 * MX_DT;
        g_mx.speed = r;
    } else if (g_mx.speed < 0) g_mx.speed = 0;
    if (k[6] && *(int *)(wram + 0xdc)) g_mx.speed += 0.6;   /* boost: the stock branch still burns the meter */
    /* steering: keys -> shaped steer -> lean spring-damper -> turn rate (Manx), applied by turning the bike */
    x = ((k[3] || k[5]) ? 1 : 0) - ((k[2] || k[4]) ? 1 : 0);
    if (x != 0) g_mx.steer = mxclamp(g_mx.steer + (x + x * fabs(x)) * 0.5 * 0.3, -1, 1);
    else g_mx.steer -= mxclamp(g_mx.steer, -0.0085 * 8, 0.0085 * 8);
    sp = g_mx.speed > 0.01 ? g_mx.speed : 0.01;
    gain = sp > 25 && !g_mxtyre && !g_mxst ? 25 / sp : 1;
    g_mx.lean_v = (g_mx.lean_v + 300 * MX_DT * (g_mx.steer * gain * 0.5 - g_mx.lean)) * 0.51;
    g_mx.lean += g_mx.lean_v * MX_DT;
    if (g_mxst) {
        /* single-track model (user 10-13 "lets try with the successor ideas first"; Meccanica 2022, s11012-022-01603-8:
           a tuned understeering single-track car model with high yaw inertia, driven through a lean-to-steer rider law).
           Keys set the lean (up to atan(0.95 mu)); the rider law picks the bar angle for it, d = atan(L g tan(lean) / u^2)
           (max 35 deg); yaw rate r and side speed vb then follow m (vb' + u r) = Fyf cos d + Fyr, Iz r' = a Fyf cos d
           - b Fyr, each tyre a magic formula on its slip angle inside its friction circle (load moves back under drive,
           forward under brakes; brakes mostly on the front). vb is read back from the game each tick so walls and
           bumps count. Below 3 m/s plain kinematics. Turn-positive frame: r > 0 turns the way rate > 0 does. */
        const double L = 1.41, a = 0.70, b = 0.71, h = 0.6, m = MX_MASS + 75, g = 9.81;
        double uu = g_mx.speed, au = fabs(uu), mu = g_mxmu / 100.0, ph, dl, Iz = m * a * b * g_mxyawi / 100.0;
        double f2[3] = { f[0], f[1], f[2] }, sgs;
        int n;
        mx_rot(f2, u, g_mxsteersign * 0.01);
        sgs = (f2[0] - f[0]) * sd[0] + (f2[1] - f[1]) * sd[1] + (f2[2] - f[2]) * sd[2] > 0 ? 1 : -1;
        ph = mxclamp(g_mx.lean / 0.5, -1, 1) * atan(0.95 * mu);
        dl = atan(L * g * tan(ph) / (au > 3 ? au * au : 9.0));
        dl = mxclamp(dl, -35 * MX_DEG, 35 * MX_DEG);
        if (!ground) g_mx.r *= 0.97;
        else if (au < 3) { g_mx.r = uu * tan(dl) / L; g_mx.vb = 0; }
        else {
            double bf = 8.0 * 100.0 / g_mxus, br = 8.0, ax = g_mx.ax;
            double fzf = mxclamp(m * (g * b - h * ax) / L, 0.05 * m * g, 0.95 * m * g), fzr = m * g - fzf;
            double fxf = ax < 0 ? 0.7 * m * ax : 0, fxr = m * ax - fxf;
            double df = sqrt(mxclamp(mu * mu * fzf * fzf - fxf * fxf, 0, 1e12)), dr = sqrt(mxclamp(mu * mu * fzr * fzr - fxr * fxr, 0, 1e12));
            g_mx.vb = sgs * vs / g_mxscale;
            for (n = 0; n < 8; n++) {
                double alf = dl - (g_mx.vb + a * g_mx.r) / uu, alr = -(g_mx.vb - b * g_mx.r) / uu, xf = bf * alf, xr = br * alr, fyf, fyr;
                fyf = df * sin(1.6 * atan(xf - 0.95 * (xf - atan(xf))));
                fyr = dr * sin(1.6 * atan(xr - 0.95 * (xr - atan(xr))));
                g_mx.vb += MX_DT / 8 * ((fyf * cos(dl) + fyr) / m - uu * g_mx.r);
                g_mx.r += MX_DT / 8 * (a * fyf * cos(dl) - b * fyr) / Iz;
            }
            g_mx.slip = atan2(fabs(g_mx.vb), au);
        }
        g_mx.roll = -mxclamp(atan(uu * g_mx.r / g), -60 * MX_DEG, 60 * MX_DEG);
        rate = g_mx.r * MX_DT;
        g_mx.vb *= sgs;   /* world side speed along sd, m/s */
    } else if (g_mxtyre) {
        /* physical lean (user 10-13 "implement the leaning fully"): the shaped steer asks for a lateral accel up to
           0.95 mu g; turn rate w = a / v (capped by 30 deg of bar at walking pace), lean = atan(v w / g), so a fast
           bike leans far and turns wide instead of snapping round; the tyre model below decides if it grips */
        double v = fabs(g_mx.speed), alat = mxclamp(g_mx.lean / 0.5, -1, 1) * 0.95 * g_mxmu / 100.0 * 9.81, w;
        w = v > 0.5 ? alat / v : 0;
        w = mxclamp(w, -v * 0.577 / 1.41, v * 0.577 / 1.41) * mxclamp(v / 2, 0, 1);
        g_mx.roll = -atan(v * w / 9.81);
        rate = w * MX_DT;
    } else {
    g_mx.roll = -atan2(g_mx.lean, 1.41);
    /* rad per tick; below 8 m/s it turns as at 8 m/s, fading out under 2 m/s (no spinning in place) */
    sp = fabs(g_mx.speed);
    rate = (sp < 8 ? 8 * mxclamp(sp / 2, 0, 1) : sp) * tan(-g_mx.roll) / 14.1 * MX_DT * (g_mx.speed < 0 ? -1 : 1);
    }
    if (!ground) rate *= 0.5;
    if (fabs(rate) > 1e-6) mx_rotate_car(wram, u, g_mxsteersign * rate);
    /* Manx air pitch: on takeoff above 20000 (hypot of forward and side speed) aim 20 deg nose up, then ease
       towards it at 0.3 x error (max 10 deg) about the side axis */
    pitch = asin(mxclamp(-f[1], -1, 1));
    if (!ground) {
        double err;
        if (g_mx.wasground) {
            double vfr = vel[0] * f[0] + vel[1] * f[1] + vel[2] * f[2];
            g_mx.ptarget = pitch + (g_mxkick && sqrt(vfr * vfr + vs * vs) > 20000 ? g_mxkick * MX_DEG : 0);   /* ini manx_airkick deg (0: every bump launched the bike) */
        }
        err = mxclamp(g_mx.ptarget - pitch, -10 * MX_DEG, 10 * MX_DEG);
        if (fabs(err) > 1e-4) {
            double f2[3] = { f[0], f[1], f[2] };
            mx_rot(f2, sd, 0.01);
            mx_rotate_car(wram, sd, (asin(mxclamp(-f2[1], -1, 1)) > pitch ? 1 : -1) * 0.3 * err);
        }
        vel[1] -= (int)((100 - g_mxairg) / 100.0 * 0x1200);   /* lighter gravity in the air (y down) */
    }
    /* Manx landing squat (car_UpdateLeanBalance 0x06031f14, visual only): the first ground tick sets an impulse =
       fall speed per 0.04 s tick * mass, which decays by mass * 0.0392 per ground tick and drives the compression
       sq += (imp * 0.001 - 0.1065 * sq) * 9.29 (the return branch is damped; here x0.2), clamped 0..12.5; drawn as
       body + rider moved down sq * 0.4 Manx units. MX_SQM stands in for the per-slot mass (not found). */
    if (!ground) g_mx.vy = vel[1] / (double)g_mxscale;   /* m/s, y down */
    else {
        if (!g_mx.wasground && g_mx.vy > 0) g_mx.imp = g_mx.vy * 0.04 * MX_SQM;
        g_mx.imp = g_mx.imp > MX_SQM * 0.0392 ? g_mx.imp - MX_SQM * 0.0392 : 0;
    }
    {
        double r7 = (g_mx.imp * 0.001 - 0.1065 * g_mx.sq) * 9.29;
        g_mx.sq = mxclamp(g_mx.sq + (r7 >= 0 ? r7 : r7 * 0.2), 0, 12.5);
    }
    g_mx.wasground = ground;
    /* forward speed (and side grip) on the ground; no thrust in the air */
    if (ground) {
        double dv = (g_mx.speed - vf) * g_mxscale, ls;
        mx_fwd(wram, t);
        vs = vel[0] * sd[0] + vel[1] * sd[1] + vel[2] * sd[2];
        if (g_mxst) ls = vs - (fabs(g_mx.speed) < 3 ? 0 : g_mx.vb * g_mxscale);   /* single-track model side speed */
        else if (g_mxtyre) {
            /* tyre model (user 10-13 "realistic bike tire simulation", replaces the snap-to-heading grip): slip angle
               a = atan(side / forward speed); lateral accel = Pacejka magic formula (B 8, C 1.6, E 0.95, peak ~11 deg)
               x what the friction circle mu*g leaves after the drive/brake accel; it can only cancel side speed, never
               push past zero. Past the peak the bike slides. */
            double vfa = fabs(vel[0] * t[0] + vel[1] * t[1] + vel[2] * t[2]) / g_mxscale, vsm = vs / g_mxscale;
            double mug = g_mxmu / 100.0 * 9.81, bx, mf, amax, dvs;
            g_mx.slip = atan2(fabs(vsm), vfa > 0.5 ? vfa : 0.5);
            bx = 8 * g_mx.slip;
            mf = sin(1.6 * atan(bx - 0.95 * (bx - atan(bx))));
            amax = sqrt(mxclamp(mug * mug - g_mx.ax * g_mx.ax, 0, 1e9));
            dvs = mf * amax * MX_DT;
            ls = (vsm > 0 ? 1 : -1) * (fabs(vsm) < dvs ? fabs(vsm) : dvs) * g_mxscale;
        } else if (g_mxfull) ls = vs;   /* Manx: no tyre model, the bike goes where it points */
        else ls = vs * g_mxgrip / 100.0;
        for (i = 0; i < 3; i++) vel[i] += (int)(t[i] * dv - sd[i] * ls);
    }
    /* camera tracking, once per tick (as native mx_track) */
    if (mx_frame(wram, f, sd, u)) {
        double e;
        for (i = 0; i < 3; i++) g_mxc.pos[i] = *(int *)(g_mxwh + 0x20 + 4 * i);
        g_mxc.speed = vel[0] * f[0] + vel[1] * f[1] + vel[2] * f[2];
        g_mxc.yaw = atan2(f[2], f[0]);
        g_mxc.pitch = asin(mxclamp(-f[1], -1, 1));
        if (!g_mxc.have) { g_mxc.have = 1; g_mxc.cyaw = g_mxc.yaw; g_mxc.cpitch = g_mxc.pitch; }
        e = g_mxc.cyaw - g_mxc.yaw;
        e -= 2 * 3.14159265358979 * floor(e / (2 * 3.14159265358979) + 0.5);
        g_mxc.cyaw -= e / 8;
        g_mxc.cpitch += (g_mxc.pitch - g_mxc.cpitch) / 18.048;
        g_mxc.t = GetTickCount();
    }
}
__declspec(naked) static void manx_ctl_hook(void)
{
    __asm {
        pushad
        push ebx
        push edx
        push eax
        call manx_pre
        add esp, 12
        popad
        mov ecx, MXCTL_FN
        call ecx
        pushad
        call manx_post
        popad
        ret
    }
}
/* Manx lean, visual only (user 10-13 "the turnings werent leaning"): the player path of bilstyring_RateLoop calls
   car_BuildWorldMatrix 0x4180d3 at 0x41b175 (EAX = ESI = wh, EDX = EDI = wram), which rebuilds the render matrix
   wh+0x5c..+0x7c (9 ints, 2.30) from the orientation every tick. After it the matrix is rolled about the bike's
   forward axis by the Manx lean angle (g_mx.roll) x manx_leanvis %, so the bike banks into the turn; physics never
   sees it and it cannot build up. Rows or columns: whichever holds the forward vector is rotated. */
#define MXWM_AT 0x41b175
#define MXWM_FN 0x4180d3
/* Manx rider lean animation + wheel spin (user 10-13 "lets keep going until the import is fully done"). rider.bin
   (tools\manx2ting.py --pc) holds RIDER_M clip 1 = 15 lean keys of the ting rider (key 7 upright, 8.025 keys per rad of
   roll as Manx rider_UpdateAnimPathDraw), the tyre vertex ranges of the bike object and their axles. Each tick the
   rider vertex stream inside the game's loaded ting.wl3 (g_mxwl3) gets the blend of the two keys around the roll, and
   the tyre vertices are turned about their axles (x). First use checks the buffer holds key 7 (little- or big-endian)
   and logs "manx pose: ..."; on a mismatch nothing is written. ini manx_rider (1, -1 = other side, 0 = off), manx_spin. */
static struct { int magic, nk, nv, voff, bnv, boff, f0, fn, r0, rn, fy, fz, ry, rz, side; } *g_mxr;
static short *g_mxrv, *g_mxtyre0;
static int g_mxnid;   /* RIDER_M clip 2 (stopped) keys after the lean keys, MXR2 only */
static int g_mxlast[4], g_mxhave;   /* what the last tick wrote (guard against freed or reused memory) */
static void mx_pose_load(void)
{
    char p[MAX_PATH], *sl;
    FILE *f;
    long l;
    int ok;
    static int tried;
    if (tried) return;
    tried = 1;
    GetModuleFileNameA(0, p, MAX_PATH);
    sl = strrchr(p, '\\');
    strcpy(sl + 1, "manx\\rider.bin");
    if (!(f = fopen(p, "rb"))) { lg("manx pose: %s missing, no rider lean / wheel spin", p); return; }
    fseek(f, 0, SEEK_END); l = ftell(f); fseek(f, 0, SEEK_SET);
    g_mxr = malloc(l);
    ok = g_mxr && (long)fread(g_mxr, 1, l, f) == l && l >= 60;
    if (ok && l >= 64 && g_mxr->magic == 0x3252584d) g_mxnid = ((int *)g_mxr)[15];
    if (!ok || (g_mxr->magic != 0x3152584d && g_mxr->magic != 0x3252584d) || g_mxnid < 0
        || l != (g_mxnid ? 64 : 60) + (g_mxr->nk + g_mxnid) * g_mxr->nv * 6) {
        lg("manx pose: %s bad", p); free(g_mxr); g_mxr = 0; g_mxnid = 0;
    } else { g_mxrv = (short *)((BYTE *)g_mxr + (g_mxr->magic == 0x3252584d ? 64 : 60)); lg("manx pose: %d keys + %d idle, rider %d verts, tyres %d + %d", g_mxr->nk, g_mxnid, g_mxr->nv, g_mxr->fn, g_mxr->rn); }
    fclose(f);
}
static short mx_s16(const BYTE *q, int be) { return be ? (short)(q[0] << 8 | q[1]) : *(const short *)q; }
static void mx_w16(BYTE *q, int v, int be)
{
    v = v < -32768 ? -32768 : v > 32767 ? 32767 : v;
    if (be) { q[0] = (BYTE)(v >> 8); q[1] = (BYTE)v; } else *(short *)q = (short)v;
}
static void mx_pose(BYTE *wh, BYTE *wram)
{
    BYTE *rv, *bv;
    int i, j, be, k0, side, sq;
    double kf, w, m[9], mx[3], f[3], sd[3], u[3], c[3];
    int *q = (int *)(wh + 0x5c);
    const short *a, *b;
    mx_pose_load();
    if (!g_mxr || !g_mxwl3 || (!g_mxrider && !g_mxspin)) return;
    rv = g_mxwl3 + g_mxr->voff;
    bv = g_mxwl3 + g_mxr->boff;
    if (g_mxposeok < 0) {
        const short *k7 = g_mxrv + 7 * g_mxr->nv * 3;
        int le = 1, bg = 1;
        for (i = 0; i < g_mxr->nv * 3; i++) { if (mx_s16(rv + 2 * i, 0) != k7[i]) le = 0; if (mx_s16(rv + 2 * i, 1) != k7[i]) bg = 0; }
        g_mxposeok = le ? 1 : bg ? 2 : 0;
        g_mxhave = 0;
        lg("manx pose: buffer %p rider verts %s (first %d %d %d, want %d %d %d)", g_mxwl3, le ? "match (little-endian)" : bg ? "match (big-endian)" : "DIFFER, poses off",
           mx_s16(rv, 0), mx_s16(rv + 2, 0), mx_s16(rv + 4, 0), k7[0], k7[1], k7[2]);
        if (g_mxposeok) {
            free(g_mxtyre0);
            g_mxtyre0 = malloc(g_mxr->bnv * 6);
            if (g_mxtyre0) for (i = 0; i < g_mxr->bnv * 3; i++) g_mxtyre0[i] = mx_s16(bv + 2 * i, g_mxposeok == 2);
        }
    }
    if (g_mxposeok <= 0) return;
    be = g_mxposeok == 2;
    if (g_mxhave && (mx_s16(rv, be) != g_mxlast[0] || mx_s16(rv + 2, be) != g_mxlast[1] || mx_s16(bv + 6 * g_mxr->r0 + 2, be) != g_mxlast[2]
        || mx_s16(bv + 6 * g_mxr->r0 + 4, be) != g_mxlast[3])) { g_mxposeok = 0; lg("manx pose: buffer changed under us, poses off"); return; }
    if (g_mxrider) {
        /* which model side is the inside of the turn: the turn centre (as manx_lean) against the matrix x axis */
        for (i = 0; i < 9; i++) m[i] = q[i] / 1073741824.0;
        side = 1;
        if (mx_frame(wram, f, sd, u)) {
            double br = 0, bc = 0, d;
            for (i = 0; i < 3; i++) {
                d = fabs(m[3 * i] * f[0] + m[3 * i + 1] * f[1] + m[3 * i + 2] * f[2]); if (d > br) br = d;
                d = fabs(m[i] * f[0] + m[i + 3] * f[1] + m[i + 6] * f[2]); if (d > bc) bc = d;
            }
            for (i = 0; i < 3; i++) mx[i] = br >= bc ? m[i] : m[3 * i];
            c[0] = u[1] * f[2] - u[2] * f[1]; c[1] = u[2] * f[0] - u[0] * f[2]; c[2] = u[0] * f[1] - u[1] * f[0];
            side = mx_dot(mx, c) * g_mxsteersign * (g_mx.roll < 0 ? 1 : -1) > 0 ? 1 : -1;
        }
        kf = 7 + side * g_mxr->side * g_mxrider * fabs(g_mx.roll) * 8.025;
        kf = mxclamp(kf, 0, g_mxr->nk - 1.001);
        k0 = (int)kf; w = kf - k0;
        a = g_mxrv + k0 * g_mxr->nv * 3;
        /* Manx rider state 2 (clip 2 STOPPED) below speed 0x4ccb = 0.3: foot down towards key 4.0; moving again plays
           it back to 0, then the lean clip resumes at key 7 (step per tick not read: 0.35 here) */
        if (g_mxnid > 1) {
            if (g_mx.speed < 0.3) { g_mx.idle = 1; g_mx.idlet = g_mx.idlet + 0.35 < 4 ? g_mx.idlet + 0.35 : 4; }
            else if (g_mx.idle && (g_mx.idlet -= 0.35) <= 0) { g_mx.idle = 0; g_mx.idlet = 0; }
            if (g_mx.idle) {
                kf = mxclamp(g_mx.idlet, 0, g_mxnid - 1.001);
                k0 = (int)kf; w = kf - k0;
                a = g_mxrv + (g_mxr->nk + k0) * g_mxr->nv * 3;
            }
        }
        b = a + g_mxr->nv * 3;
        sq = (int)mxclamp(g_mx.sq * 0.4 * 724 * g_mxsquat / 100.0, 0, 1200);   /* 724 v16 per Manx unit (14.1 wheelbase = 10215) */
        for (i = 0; i < g_mxr->nv * 3; i++)
            mx_w16(rv + 2 * i, (int)floor(a[i] + (b[i] - a[i]) * w + 0.5) + (i % 3 == 1 ? sq : 0), be);
    }
    if (g_mxspin && g_mxtyre0) {
        /* rad per tick, kept under ~1 so the 30 Hz picture still reads as forward spin */
        double cs, sn;
        g_mx.wheel += mxclamp(g_mx.speed * 0.02, -1.0, 1.0);
        g_mx.wheel = fmod(g_mx.wheel, 2 * 3.14159265358979);
        cs = cos(g_mx.wheel); sn = sin(g_mx.wheel);
        for (j = 0; j < 2; j++) {
            int first = j ? g_mxr->r0 : g_mxr->f0, n = j ? g_mxr->rn : g_mxr->fn;
            double ay = j ? g_mxr->ry : g_mxr->fy, az = j ? g_mxr->rz : g_mxr->fz;
            for (i = first; i < first + n && i < g_mxr->bnv; i++) {
                double y = g_mxtyre0[3 * i + 1] - ay, z = g_mxtyre0[3 * i + 2] - az;
                mx_w16(bv + 6 * i + 2, (int)floor(ay + y * cs + z * sn + 0.5), be);
                mx_w16(bv + 6 * i + 4, (int)floor(az - y * sn + z * cs + 0.5), be);
            }
        }
    }
    if (g_mxtyre0 && g_mxsquat) {
        /* landing squat on the body (tyres stay on the ground) */
        sq = (int)mxclamp(g_mx.sq * 0.4 * 724 * g_mxsquat / 100.0, 0, 1200);
        for (i = 0; i < g_mxr->bnv; i++)
            if (!(i >= g_mxr->f0 && i < g_mxr->f0 + g_mxr->fn) && !(i >= g_mxr->r0 && i < g_mxr->r0 + g_mxr->rn))
                mx_w16(bv + 6 * i + 2, g_mxtyre0[3 * i + 1] + sq, be);
    }
    g_mxlast[0] = mx_s16(rv, be); g_mxlast[1] = mx_s16(rv + 2, be);
    g_mxlast[2] = mx_s16(bv + 6 * g_mxr->r0 + 2, be); g_mxlast[3] = mx_s16(bv + 6 * g_mxr->r0 + 4, be);
    g_mxhave = 1;
}
static void __cdecl manx_lean(BYTE *wh, BYTE *wram)
{
    static int said;
    double f[3], sd[3], u[3], up[3], c[3], fu[3], m[9], ax[3], a, br = 0, bc = 0, d;
    int *q = (int *)(wh + 0x5c), i, j, rows, bi = 0, ci = 0;
    if (g_mxlive && wh == g_mxwh) mx_pose(wh, wram);
    if (!g_mxlive || wh != g_mxwh || !g_mxleanvis || fabs(g_mx.roll) < 1e-4) return;
    if (!mx_frame(wram, f, sd, u)) return;
    for (i = 0; i < 9; i++) m[i] = q[i] / 1073741824.0;
    for (i = 0; i < 3; i++) {
        d = fabs(m[3 * i] * f[0] + m[3 * i + 1] * f[1] + m[3 * i + 2] * f[2]); if (d > br) { br = d; bi = i; }
        d = fabs(m[i] * f[0] + m[i + 3] * f[1] + m[i + 6] * f[2]); if (d > bc) { bc = d; ci = i; }
    }
    rows = br >= bc;
    for (i = 0; i < 3; i++) ax[i] = rows ? m[3 * bi + i] : m[ci + 3 * i];
    d = sqrt(mx_dot(ax, ax));
    if (d < 0.5) return;
    for (i = 0; i < 3; i++) ax[i] /= d;
    if (mx_dot(ax, f) < 0) for (i = 0; i < 3; i++) ax[i] = -ax[i];
    if (!said) { said = 1; lg("manx lean: matrix %s, forward axis %d (|dot| %.3f), leanvis %d%%", rows ? "rows" : "columns", rows ? bi : ci, rows ? br : bc, g_mxleanvis); }
    /* the turn centre side: the steering turns by g_mxsteersign * sign(-roll) about u, i.e. towards that x (u x f);
       true up has y < 0 (y down); roll so that up moves towards the centre */
    for (i = 0; i < 3; i++) up[i] = u[1] < 0 ? u[i] : -u[i];
    c[0] = u[1] * f[2] - u[2] * f[1]; c[1] = u[2] * f[0] - u[0] * f[2]; c[2] = u[0] * f[1] - u[1] * f[0];
    d = g_mxsteersign * (g_mx.roll < 0 ? 1 : -1);
    fu[0] = ax[1] * up[2] - ax[2] * up[1]; fu[1] = ax[2] * up[0] - ax[0] * up[2]; fu[2] = ax[0] * up[1] - ax[1] * up[0];
    a = fabs(g_mx.roll) * g_mxleanvis / 100.0;
    if (mx_dot(fu, c) * d < 0) a = -a;
    for (j = 0; j < 3; j++) {
        double v[3];
        for (i = 0; i < 3; i++) v[i] = rows ? m[3 * j + i] : m[j + 3 * i];
        mx_rot(v, ax, a);
        for (i = 0; i < 3; i++) {
            int x = (int)floor(mxclamp(v[i], -1.99, 1.99) * 1073741824.0 * 0.999999 + 0.5);
            if (rows) q[3 * j + i] = x; else q[j + 3 * i] = x;
        }
    }
}
/* Full Manx player (user 10-13 "lets start the player mechanics from scratch, lets fully import the manx tt
   gameplay"; ini manx_full, default 1). The Scorcher player tick (bilstyring_RateLoop 0x41b05c, ESI wh, EDI wram)
   treats the vehicle as a ball: car_DampVelocity, car_CollideStep bounces, car_GroundResponse bounce + slope roll.
   For the Manx bike: DampVelocity (call 0x41b0f5) is skipped; CollideStep (0x41b131, push 6) still sweeps the
   collision sphere, but the Manx wall rule replaces the bounce: speed keeps only the fraction of the forward move
   that got through (Manx: a blocked substep zeroes speed) and the velocity is put back along the heading;
   GroundResponse (0x41b15a) still counts ground ticks (+0x3c) but its velocity change is undone. */
#define MXDMP_AT 0x41b0f5
#define MXDMP_FN 0x4157d1
#define MXCOL_AT 0x41b131
#define MXCOL_FN 0x417d5d
#define MXGR_AT 0x41b15a
#define MXGR_FN 0x416be8
static int __cdecl mx_full_on(void) { return g_mxfull && g_set.manx && g_mxlive; }
static void __cdecl mx_col_pre(BYTE *wh, BYTE *wram)
{
    int i;
    for (i = 0; i < 3; i++) { g_mxv0[i] = *(int *)(wram + 0xc + 4 * i); g_mxp0[i] = *(int *)(wh + 0x20 + 4 * i) - g_mxv0[i]; }
}
static void __cdecl mx_col_post(BYTE *wh, BYTE *wram)
{
    double t[3], want, got, r;
    int *vel = (int *)(wram + 0xc), i;
    if (!mx_full_on()) return;
    mx_fwd(wram, t);
    want = g_mxv0[0] * t[0] + g_mxv0[2] * t[2];
    got = (*(int *)(wh + 0x20) - g_mxp0[0]) * t[0] + (*(int *)(wh + 0x28) - g_mxp0[2]) * t[2];
    if (fabs(want) > 2000) {
        r = got / want;
        if (r < 0.9) {
            double was = g_mx.speed;
            g_mx.speed *= mxclamp(r, 0, 1);
            if (g_mxcrash && r < 0.5 && was > 8 && !g_mx.crash) g_mx.crash = 3;   /* hard hit: Manx crash state */
            lg("manx wall: %.0f%% of the move got through, speed %.1f -> %.1f%s", r * 100, was, g_mx.speed, g_mx.crash == 3 ? " CRASH" : "");
        }
    }
    for (i = 0; i < 3; i += 2) vel[i] = (int)(t[i] * g_mx.speed * g_mxscale);
}
static DWORD g_mxdmpfn = MXDMP_FN, g_mxcolfn = MXCOL_FN, g_mxgrfn = MXGR_FN;
static BYTE *g_mxcwh, *g_mxcwram;
static int g_mxgrv[3];
static void __cdecl mx_col_post0(void) { mx_col_post(g_mxcwh, g_mxcwram); }
static void __cdecl mx_gr_pre(BYTE *wram) { int i; for (i = 0; i < 3; i++) g_mxgrv[i] = *(int *)(wram + 0xc + 4 * i); }
static void __cdecl mx_gr_post(void)
{
    int i;
    if (mx_full_on()) for (i = 0; i < 3; i++) *(int *)(g_mxcwram + 0xc + 4 * i) = g_mxgrv[i];
}
__declspec(naked) static void manx_dmp_hook(void)
{
    __asm {
        pushad
        call mx_full_on
        test eax, eax
        popad
        jnz skip
        jmp dword ptr [g_mxdmpfn]
    skip:
        ret
    }
}
/* Wheel colliders (user 10-13 "can we have that we just have a collider for each of the wheels. like 2 balls there?"):
   car_CollideStep sweeps the one sphere shared+0x10 from the old pos wram+0..8 to the new pos wh+0x20. With
   manx_wheelcol 1 it runs twice, each time with both points moved to a wheel (+-0.7 m along the heading) and the
   radius set to manx_wheelr; the two push-outs are merged per axis (same sign: the larger, else both). */
static int g_mxcret;
__declspec(naked) static int __cdecl mx_col_raw(BYTE *wh, BYTE *wram, int arg)
{
    __asm {
        push ebx
        push esi
        push edi
        push ebp
        mov eax, [esp + 20]
        mov edx, [esp + 24]
        push dword ptr [esp + 28]
        call dword ptr [g_mxcolfn]
        pop ebp
        pop edi
        pop esi
        pop ebx
        ret
    }
}
static void __cdecl mx_col_do(BYTE *wh, BYTE *wram, int arg)
{
    static int snap[6];   /* old pos + velocity only: the rest of wram holds list links (restoring them crashed list_Begin) */
    BYTE *shr = *(BYTE **)(wh + 0x98);
    int *pos = (int *)(wh + 0x20), *old = (int *)wram, npos[3], o[3], d[2][3], r0, w, i, copies = 0;
    double off;
    mx_col_pre(wh, wram);
    g_mxwheelon = g_mxwheel && mx_full_on() && shr;
    if (!g_mxwheelon) { g_mxcret = mx_col_raw(wh, wram, arg); mx_col_post(wh, wram); return; }
    mx_fwd(wram, g_mxcolt);
    off = 0.7 * g_mxscale;
    memcpy(snap, wram, sizeof snap);   /* wram +0..+0x14 */
    for (i = 0; i < 3; i++) npos[i] = pos[i];
    r0 = *(int *)(shr + 0x10);
    for (w = 0; w < 2; w++) {
        if (w) { memcpy(wram, snap, sizeof snap); for (i = 0; i < 3; i++) pos[i] = npos[i]; }
        o[0] = (int)(g_mxcolt[0] * off * (w ? -1 : 1)); o[1] = 0; o[2] = (int)(g_mxcolt[2] * off * (w ? -1 : 1));
        for (i = 0; i < 3; i++) { old[i] += o[i]; pos[i] += o[i]; }
        *(int *)(shr + 0x10) = g_mxwheelr;
        g_mxcret = mx_col_raw(wh, wram, arg);
        copies = old[0] == pos[0] && old[2] == pos[2];
        for (i = 0; i < 3; i++) { d[w][i] = pos[i] - o[i] - npos[i]; old[i] -= o[i]; }
    }
    for (i = 0; i < 3; i++) {
        int a = d[0][i], b = d[1][i];
        pos[i] = npos[i] + ((a >= 0) == (b >= 0) ? (abs(a) > abs(b) ? a : b) : a + b);
        if (copies) old[i] = pos[i];
    }
    *(int *)(shr + 0x10) = r0;
    mx_col_post(wh, wram);
}
__declspec(naked) static void manx_col_hook(void)
{
    __asm {
        mov g_mxcwh, eax
        mov g_mxcwram, edx
        pushad
        push dword ptr [esp + 36]
        push edx
        push eax
        call mx_col_do
        add esp, 12
        popad
        mov eax, g_mxcret
        ret 4
    }
}
__declspec(naked) static void manx_gr_hook(void)
{
    __asm {
        mov g_mxcwram, edx
        pushad
        push edx
        call mx_gr_pre
        add esp, 4
        popad
        call dword ptr [g_mxgrfn]
        pushad
        call mx_gr_post
        popad
        ret
    }
}
__declspec(naked) static void manx_lean_hook(void)
{
    __asm {
        mov ecx, MXWM_FN
        call ecx
        pushad
        push edi
        push esi
        call manx_lean
        add esp, 8
        popad
        ret
    }
}
/* Manx engine sound: car_EngineSoundMix 0x415e2f (EAX wh, EDX wram, ECX sample count 0x18, 2 stack args, ret 8)
   picks one of the looping engine samples 0x14e+i from |vel| (wram+0xc) * count / 0x56b80 and crossfades by volume;
   stock clamps |vel| > 0x2b5c0 to the top sample (cmp immediate at 0x415e5f). For the player's call at 0x41b0ec
   the velocity is swapped for a fake one whose length follows the Manx rpm (1300..14000 over the whole ladder) and
   the clamp is lifted for that call only, so the pitch climbs through each gear and drops on every upshift. */
#define MXSND_AT 0x41b0ec
#define MXSND_FN 0x415e2f
#define MXSND_CLAMP 0x415e5f
static int g_mxsv[3], g_mxsnd;
static DWORD g_mxsndfn = MXSND_FN;
static void __cdecl manx_snd_pre(BYTE *wh, BYTE *wram)
{
    double fr;
    g_mxsnd = g_set.manx && wh == g_mxwh;
    if (!g_mxsnd) return;
    memcpy(g_mxsv, wram + 0xc, 12);
    fr = mxclamp((g_mx.rpm - 1300) / (14000 - 1300), 0, 1);
    *(int *)(wram + 0xc) = (int)(fr * 0x56b80); *(int *)(wram + 0x10) = *(int *)(wram + 0x14) = 0;
    *(DWORD *)MXSND_CLAMP = 0x56b80;
    g_mxwram = wram;
}
static void __cdecl manx_snd_post(void)
{
    if (!g_mxsnd) return;
    memcpy(g_mxwram + 0xc, g_mxsv, 12);
    *(DWORD *)MXSND_CLAMP = 0x2b5c0;
}
__declspec(naked) static void manx_snd_hook(void)
{
    __asm {
        pushad
        push edx
        push eax
        call manx_snd_pre
        add esp, 8
        popad
        push dword ptr [esp + 8]
        push dword ptr [esp + 8]
        call dword ptr [g_mxsndfn]
        pushad
        call manx_snd_post
        popad
        ret 8
    }
}
/* Manx chase camera: moincam_PublishView 0x429bd1 (one caller, 0x429e10; ret, keeps ebx..edi) copies the camera
   eye and matrix into the view record *(DWORD *)0x440bd8: +4 eye (3 ints), +0x1c matrix (9 ints, 2.30, rows =
   view direction, screen down, down x dir as on Saturn). After it the view is rewritten with the port of native
   mx_camera: pivot = bike + (0,-12,+7) in the bike yaw/pitch frame, eye = pivot - speed/K/8 along the camera
   heading + (0,-4,-26) in the lagged camera frame (yaw lag 1/8, pitch 1/18.048 per tick), aim tilted down
   g_mxtilt deg; K = g_mxcamk Scorcher units per Manx unit. */
#define MXCAM_AT 0x429e10
#define MXCAM_FN 0x429bd1
static void mx_ypframe(double yaw, double pitch, double f[3], double d[3])
{
    double cp = cos(pitch), sp = sin(pitch), cy = cos(yaw), sy = sin(yaw);
    f[0] = cp * cy; f[1] = -sp; f[2] = cp * sy;
    d[0] = sp * cy; d[1] = cp; d[2] = sp * sy;
}
static void __cdecl manx_cam_post(void)
{
    int *view = *(int **)0x440bd8, i, r;
    double f[3], d[3], fc[3], dc[3], sc[3], e[3], hz[3], pull, K = g_mxcamk;
    const double *rows[3];
    static int logged;
    if (!g_set.manx || !g_mxcam || !g_mxc.have || !view || GetTickCount() - g_mxc.t > 250) return;
    if (!logged) {
        logged = 1;
        lg("manx cam: stock eye %d %d %d rows %d %d %d / %d %d %d / %d %d %d (>>20); bike %.0f %.0f %.0f yaw %.2f pitch %.2f",
           view[1], view[2], view[3], view[7] >> 20, view[8] >> 20, view[9] >> 20, view[10] >> 20, view[11] >> 20,
           view[12] >> 20, view[13] >> 20, view[14] >> 20, view[15] >> 20, g_mxc.pos[0], g_mxc.pos[1], g_mxc.pos[2],
           g_mxc.yaw, g_mxc.pitch);
    }
    mx_ypframe(g_mxc.yaw, g_mxc.pitch, f, d);
    mx_ypframe(g_mxc.cyaw, g_mxc.cpitch, fc, dc);
    pull = (g_mxc.speed > 0 ? g_mxc.speed : 0) / K / 8;
    hz[0] = cos(g_mxc.cyaw); hz[1] = 0; hz[2] = sin(g_mxc.cyaw);
    for (i = 0; i < 3; i++)
        e[i] = g_mxc.pos[i] + K * (-12.0 * d[i] + 7.0 * f[i]) - K * pull * hz[i] + K * (-4.0 * dc[i] - 26.0 * fc[i]);
    if (g_mxtilt) mx_ypframe(g_mxc.cyaw, g_mxc.cpitch - g_mxtilt * MX_DEG, fc, dc);
    sc[0] = dc[1] * fc[2] - dc[2] * fc[1]; sc[1] = dc[2] * fc[0] - dc[0] * fc[2]; sc[2] = dc[0] * fc[1] - dc[1] * fc[0];
    rows[0] = fc; rows[1] = dc; rows[2] = sc;
    for (i = 0; i < 3; i++) view[1 + i] = (int)e[i];
    for (r = 0; r < 3; r++)
        for (i = 0; i < 3; i++) view[7 + 3 * r + i] = (int)floor(rows[r][i] * 1073741824.0 * 0.999999 + 0.5);
}
/* F2 collider view (user 10-13 "make so f1 renders the player colliuder"; F1 is the options menu): car_CollideStep
   sweeps ONE sphere per car through col_Trace, centre = the car position wh+0x20, radius = *(*(wh + 0x98) + 0x10).
   Drawn as its outline plus three great circles, projected through the view record *(DWORD *)0x440bd8 (+4 eye,
   +0x1c rows view dir / screen down / down x dir, 2.30) and the pixel focal length camera+0x2c (16.16,
   cam_ComputeFovScale 0x40d4f5; camera record *(DWORD *)0x43e998). ini manx_colflip 1 mirrors x if it draws on the
   wrong side. */
static int mx_proj(const int *view, double fo, int sw, int sh, RECT *r, const double P[3], POINT *o)
{
    double d[3], v[3];
    int i, k;
    for (i = 0; i < 3; i++) d[i] = P[i] - view[1 + i];
    for (k = 0; k < 3; k++) { v[k] = 0; for (i = 0; i < 3; i++) v[k] += view[7 + 3 * k + i] / 1073741824.0 * d[i]; }
    if (v[0] < 50) return 0;
    if (g_mxcolflip) v[2] = -v[2];
    o->x = r->left + (LONG)((sw / 2.0 + v[2] * fo / v[0]) * (r->right - r->left) / sw);
    o->y = r->top + (LONG)((sh / 2.0 + v[1] * fo / v[0]) * (r->bottom - r->top) / sh);
    return 1;
}
static void draw_collider(HDC dc, RECT *r, int sw, int sh)
{
    int *view = *(int **)0x440bd8, i, j, k, ns, ok, live;
    BYTE *cam = *(BYTE **)0x43e998;
    double fo, c[3], R, P[3];
    char b[112];
    HPEN pn, py, op;
    POINT q, q0;
    ns = g_mxwheelon ? 2 : 1;
    R = g_mxwheelon ? g_mxwheelr : g_mxcolr;
    live = view && g_set.manx && g_mxc.have && GetTickCount() - g_mxc.t < 250 && R > 0;
    if (live) {
        fo = cam && *(int *)(cam + 0x2c) > 0 ? *(int *)(cam + 0x2c) / 65536.0 : sw / 2.0 / tan(35 * MX_DEG);
        pn = CreatePen(PS_SOLID, 2, RGB(40, 255, 80)); py = CreatePen(PS_SOLID, 1, RGB(255, 230, 40));
        op = SelectObject(dc, pn);
        SelectObject(dc, GetStockObject(NULL_BRUSH));
        for (k = 0; k < ns; k++) {
            for (i = 0; i < 3; i++) c[i] = g_mxc.pos[i] + (ns == 2 ? g_mxcolt[i] * 0.7 * g_mxscale * (k ? -1 : 1) : 0);
            SelectObject(dc, pn);
            for (j = 0; j < 3; j++) {   /* great circles in the world XY, YZ, ZX planes */
                ok = 0;
                for (i = 0; i <= 32; i++) {
                    double a = i * 2 * 3.14159265358979 / 32, ca = cos(a) * R, sa = sin(a) * R;
                    P[0] = c[0]; P[1] = c[1]; P[2] = c[2];
                    P[j] += ca; P[(j + 1) % 3] += sa;
                    if (mx_proj(view, fo, sw, sh, r, P, &q)) { if (ok) LineTo(dc, q.x, q.y); else MoveToEx(dc, q.x, q.y, 0); ok = 1; }
                    else ok = 0;
                }
            }
            if (mx_proj(view, fo, sw, sh, r, c, &q0)) {   /* centre cross + outline (screen radius R * focal / depth) */
                double d[3], z = 0; int rr;
                for (i = 0; i < 3; i++) d[i] = c[i] - view[1 + i];
                for (i = 0; i < 3; i++) z += view[7 + i] / 1073741824.0 * d[i];
                rr = (int)(R * fo / z * (r->right - r->left) / sw);
                MoveToEx(dc, q0.x - 6, q0.y, 0); LineTo(dc, q0.x + 7, q0.y);
                MoveToEx(dc, q0.x, q0.y - 6, 0); LineTo(dc, q0.x, q0.y + 7);
                SelectObject(dc, py);
                Ellipse(dc, q0.x - rr, q0.y - rr, q0.x + rr, q0.y + rr);
            }
        }
        SelectObject(dc, op); DeleteObject(pn); DeleteObject(py);
        if (ns == 2) sprintf(b, "collider: 2 wheel spheres, r %d (%.2f m), stock %d", g_mxwheelr, R / g_mxscale, g_mxcolr);
        else sprintf(b, "collider: 1 sphere, r %d (%.2f m)", g_mxcolr, R / g_mxscale);
    } else sprintf(b, "collider: no Manx bike in play");
    SetBkMode(dc, TRANSPARENT);
    text_at(dc, r->left + 8, r->top + (r->bottom - r->top) / 8, b, RGB(40, 255, 80), TA_LEFT);
}
__declspec(naked) static void manx_cam_hook(void)
{
    __asm {
        mov eax, MXCAM_FN
        call eax
        pushad
        call manx_cam_post
        popad
        ret
    }
}
static void patch_manx(void)
{
    char p[MAX_PATH];
    ini_path(p);
    g_mxscale = clampi(GetPrivateProfileIntA("game", "manx_scale", 4400, p), 500, 40000);   /* 5200 until 10-13 23:10 (user: "bike is also too fast") */
    g_mxsteersign = GetPrivateProfileIntA("game", "manx_steersign", 1, p) < 0 ? -1 : 1;
    g_mxcam = GetPrivateProfileIntA("game", "manx_cam", 1, p) != 0;
    g_mxcamk = clampi(GetPrivateProfileIntA("game", "manx_camscale", 2500, p), 300, 20000);
    g_mxtilt = clampi(GetPrivateProfileIntA("game", "manx_camtilt", 10, p), -30, 45);
    g_mxgrip = clampi(GetPrivateProfileIntA("game", "manx_grip", 50, p), 0, 100);
    g_mxairg = clampi(GetPrivateProfileIntA("game", "manx_airgrav", 50, p), 0, 100);
    g_mxkick = clampi(GetPrivateProfileIntA("game", "manx_airkick", 0, p), 0, 30);
    g_mxtyre = GetPrivateProfileIntA("game", "manx_tyre", 0, p) != 0;
    g_mxcolflip = GetPrivateProfileIntA("game", "manx_colflip", 0, p) != 0;
    g_mxcolset = GetPrivateProfileIntA("game", "manx_colr", 0, p);
    g_mxwheel = GetPrivateProfileIntA("game", "manx_wheelcol", 1, p) != 0;
    g_mxwheelr = clampi(GetPrivateProfileIntA("game", "manx_wheelr", 1500, p), 300, 20000);
    if (g_mxcolset) g_mxcolset = clampi(g_mxcolset, 2000, 64000);
    g_mxst = GetPrivateProfileIntA("game", "manx_model", 1, p) == 1;   /* 1 = single-track successor model, 0 = Manx */
    g_mxyawi = clampi(GetPrivateProfileIntA("game", "manx_yawi", 150, p), 30, 600);
    g_mxus = clampi(GetPrivateProfileIntA("game", "manx_us", 105, p), 50, 300);
    g_mxfull = GetPrivateProfileIntA("game", "manx_full", 1, p) != 0;
    g_mxrider = clampi(GetPrivateProfileIntA("game", "manx_rider", 1, p), -1, 1);   /* -1 = lean the rider the other way */
    g_mxspin = GetPrivateProfileIntA("game", "manx_spin", 1, p) != 0;
    g_mxcrash = GetPrivateProfileIntA("game", "manx_crash", 1, p) != 0;
    g_mxrev = clampi(GetPrivateProfileIntA("game", "manx_reverse", 4, p), 0, 15);   /* m/s, 0 = no reverse */
    g_mxsquat = clampi(GetPrivateProfileIntA("game", "manx_squat", 100, p), 0, 300);
    g_mxcatch = GetPrivateProfileIntA("game", "manx_catchup", 1, p) != 0;
    g_mxmu = clampi(GetPrivateProfileIntA("game", "manx_mu", 110, p), 20, 300);
    g_mxaccel = clampi(GetPrivateProfileIntA("game", "manx_accel", 200, p), 50, 600);
    g_mxleanvis = clampi(GetPrivateProfileIntA("game", "manx_leanvis", 100, p), 0, 400);
    if (*(BYTE *)MXCTL_AT != 0xe8 || *(DWORD *)(MXCTL_AT + 1) != MXCTL_FN - (MXCTL_AT + 5)) { lg("manx: control call differs, no Manx physics"); return; }
    hook_call(MXCTL_AT, manx_ctl_hook);
    if (*(BYTE *)MXSND_AT == 0xe8 && *(DWORD *)(MXSND_AT + 1) == MXSND_FN - (MXSND_AT + 5) && *(DWORD *)MXSND_CLAMP == 0x2b5c0) {
        DWORD old;
        VirtualProtect((void *)MXSND_CLAMP, 4, PAGE_EXECUTE_READWRITE, &old);   /* rewritten per call by manx_snd_pre/post */
        hook_call(MXSND_AT, manx_snd_hook);
    } else lg("manx: sound call differs, stock engine sound");
    if (*(BYTE *)MXCAM_AT == 0xe8 && *(DWORD *)(MXCAM_AT + 1) == MXCAM_FN - (MXCAM_AT + 5)) hook_call(MXCAM_AT, manx_cam_hook);
    else lg("manx: view publish call differs, stock camera");
    if (*(BYTE *)MXWM_AT == 0xe8 && *(DWORD *)(MXWM_AT + 1) == MXWM_FN - (MXWM_AT + 5)) hook_call(MXWM_AT, manx_lean_hook);
    else lg("manx: world matrix call differs, no lean");
    if (*(BYTE *)MXDMP_AT == 0xe8 && *(DWORD *)(MXDMP_AT + 1) == MXDMP_FN - (MXDMP_AT + 5) && *(BYTE *)MXCOL_AT == 0xe8 && *(DWORD *)(MXCOL_AT + 1) == MXCOL_FN - (MXCOL_AT + 5)
        && *(BYTE *)MXGR_AT == 0xe8 && *(DWORD *)(MXGR_AT + 1) == MXGR_FN - (MXGR_AT + 5)) {
        hook_call(MXDMP_AT, manx_dmp_hook); hook_call(MXCOL_AT, manx_col_hook); hook_call(MXGR_AT, manx_gr_hook);
        lg("manx: full Manx player %s", g_mxfull ? "on" : "off (ini manx_full 0)");
    } else lg("manx: player tick calls differ, no full Manx");
}
static void patch_rivals(void)
{
    static const BYTE sw[5] = { 0x83, 0xf8, 0x05, 0x77, 0xc1 };
    BYTE b[5];
    if (*(BYTE *)FOPEN_AT != 0xe8 || *(BYTE *)FREAD_AT != 0xe8 || memcmp((void *)PLACESW, sw, 5)) {
        lg("rivals: file_Load or place switch differs, no extra rivals");
        return;
    }
    hook_call(FOPEN_AT, fopen_hook);
    hook_call(FREAD_AT, fread_hook);
    b[0] = 0xe9; *(DWORD *)(b + 1) = (DWORD)place_hook - (PLACESW + 5);
    write_code(PLACESW, b, 5);
}
static void patch_modes(void)
{
    static const BYTE tail[24] = { 0x83,0xfa,0x01,0x75,0x08,0x31,0xc9,0x89,0x0d,0x30,0xeb,0x43,0x00,
                                   0x83,0xfa,0x02,0x75,0x06,0x89,0x15,0x30,0xeb,0x43,0x00 };
    static const BYTE cmp[7] = { 0x83,0x3d,0x30,0xeb,0x43,0x00,0x02 };
    static const BYTE ld[6] = { 0x8b,0x15,0x58,0xeb,0x43,0x00 };
    BYTE b[24];
    if (memcmp((void *)MODETAIL, tail, 24) || memcmp((void *)ENEMYCMP, cmp, 7) || memcmp((void *)UNLOCKLD, ld, 6)) {
        lg("race mode patch: bytes differ, skipped");
        return;
    }
    memset(b, 0x90, sizeof b);
    b[0] = 0xe8; *(DWORD *)(b + 1) = (DWORD)mode_stub - (MODETAIL + 5);
    write_code(MODETAIL, b, 24);
    b[0] = 0xe8; *(DWORD *)(b + 1) = (DWORD)enemy_stub - (ENEMYCMP + 5);
    b[5] = 0x66; b[6] = 0x90;   /* 2-byte nop; the jne at 0x4154c5 is kept */
    write_code(ENEMYCMP, b, 7);
    /* Stage select 0x42427e: the unlocked count (0x43eb58) is loaded into EDX; championship never shows
       this page, so every stage is unlocked for Time Attack and Practice: `mov edx, 0x40; nop`. */
    b[0] = 0xba; *(DWORD *)(b + 1) = 0x40; b[5] = 0x90;
    write_code(UNLOCKLD, b, 6);
    /* Ghost hooks: the routine table slots of the player and enemy RateLoops. */
    if (*(DWORD *)RT_BILRATE == 0x41b05c && *(DWORD *)RT_ENERATE == 0x41b54d) {
        DWORD v;
        g_bil_orig = 0x41b05c; g_ene_orig = 0x41b54d;
        v = (DWORD)bil_rate_hook; write_code(RT_BILRATE, &v, 4);
        v = (DWORD)ene_rate_hook; write_code(RT_ENERATE, &v, 4);
    } else lg("ghost: routine table differs, no ghost");
    {
        static const BYTE tail[7] = { 0x8b, 0x5c, 0x24, 0x38, 0x8b, 0x04, 0x24 };
        if (GetEnvironmentVariableA("SCORCHER_GHOSTSOLID", 0, 0)) lg("ghost: solid (test)");
        else if (!memcmp((void *)POLYTAIL, tail, 7)) {
            b[0] = 0xe9; *(DWORD *)(b + 1) = (DWORD)poly_tag_hook - (POLYTAIL + 5); b[5] = b[6] = 0x90;
            write_code(POLYTAIL, b, 7);
        } else lg("ghost: polygon tail differs, ghost not see-through");
        if (GetEnvironmentVariableA("SCORCHER_GHOSTCOL", 0, 0)) lg("ghost: collides (test)");
        else if (!memcmp((void *)COLSKIP, "�u�", 5)) {
            b[0] = 0xe9; *(DWORD *)(b + 1) = (DWORD)col_hook - (COLSKIP + 5); write_code(COLSKIP, b, 5);
        }
        else lg("ghost: col_Trace differs, the ghost is solid");
    }
}
static void hook_call(DWORD at, void *to)
{
    DWORD old;
    BYTE *p = (BYTE *)at;
    VirtualProtect(p, 5, PAGE_EXECUTE_READWRITE, &old);
    *(DWORD *)(p + 1) = (DWORD)to - (at + 5);
    VirtualProtect(p, 5, old, &old);
    FlushInstructionCache(GetCurrentProcess(), p, 5);
}

/* ---------------------------------------------------------------- import hooks */
/* Points the exe's import slot for dll!name at fn; returns the original function. */
static void *hook_import(const char *dll, const char *name, void *fn)
{
    BYTE *b = (BYTE *)GetModuleHandleA(0);
    IMAGE_NT_HEADERS *nt = (IMAGE_NT_HEADERS *)(b + ((IMAGE_DOS_HEADER *)b)->e_lfanew);
    IMAGE_DATA_DIRECTORY *dd = &nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
    IMAGE_IMPORT_DESCRIPTOR *imp;
    void *real = (void *)GetProcAddress(LoadLibraryA(dll), name);
    if (!dd->VirtualAddress) return real;
    for (imp = (IMAGE_IMPORT_DESCRIPTOR *)(b + dd->VirtualAddress); imp->Name; imp++) {
        IMAGE_THUNK_DATA *names = imp->OriginalFirstThunk ? (IMAGE_THUNK_DATA *)(b + imp->OriginalFirstThunk) : 0;
        IMAGE_THUNK_DATA *slots = (IMAGE_THUNK_DATA *)(b + imp->FirstThunk);
        if (lstrcmpiA((char *)b + imp->Name, dll)) continue;
        for (; slots->u1.Function; slots++, names = names ? names + 1 : 0) {
            DWORD old;
            int hit = slots->u1.Function == (ULONG_PTR)real;
            if (!hit && names && !IMAGE_SNAP_BY_ORDINAL(names->u1.Ordinal))
                hit = !strcmp((char *)((IMAGE_IMPORT_BY_NAME *)(b + names->u1.AddressOfData))->Name, name);
            if (!hit) continue;
            real = (void *)slots->u1.Function;
            VirtualProtect(&slots->u1.Function, 4, PAGE_READWRITE, &old);
            slots->u1.Function = (ULONG_PTR)fn;
            VirtualProtect(&slots->u1.Function, 4, old, &old);
            lg("%s hooked at %p", name, &slots->u1.Function);
            return real;
        }
    }
    lg("%s: no import slot", name);
    return real;
}

/* ---------------------------------------------------------------- volumes */
static float g_wave_game = 1.0f, g_aux_game = 1.0f;   /* the game's own waveOutSetVolume / auxSetVolume levels */
static float vol_gain(int pct) { float f = pct / 100.0f; return f * f; }
static float music_gain(void) { return vol_gain(g_set.master) * vol_gain(g_set.music) * g_aux_game; }
static float sfx_gain(void) { return vol_gain(g_set.master) * vol_gain(g_set.sfx) * g_wave_game; }
static float level_of(DWORD v) { DWORD l = v & 0xffff, r = v >> 16; return (l > r ? l : r) / 65535.0f; }

static MMRESULT WINAPI fake_waveOutSetVolume(HWAVEOUT h, DWORD v)
{
    lg("game waveOutSetVolume %08x", v);
    g_wave_game = level_of(v); sfx_apply_all();
    return MMSYSERR_NOERROR;
}
static MMRESULT WINAPI fake_auxSetVolume(UINT id, DWORD v)
{
    lg("game auxSetVolume %u %08x", id, v);
    g_aux_game = level_of(v);
    return MMSYSERR_NOERROR;
}
/* The game's clock stands still while the options overlay is open. */
static DWORD WINAPI fake_timeGetTime(void)
{
    { static int n; if (!n++) lg("game timeGetTime first call"); }
    return (g_inpause ? g_pause_t0 : timeGetTime()) - g_pausedms;
}

/* ---------------------------------------------------------------- sound effects (DirectSound)
   Every secondary buffer is created with DSBCAPS_CTRLVOLUME; the game's own SetVolume value is
   kept per buffer and the effects level is added on top (in 1/100 dB). */
typedef struct {
    IDirectSoundBufferVtbl *vt;
    HRESULT (STDMETHODCALLTYPE *setvol)(IDirectSoundBuffer *, LONG);
    HRESULT (STDMETHODCALLTYPE *getvol)(IDirectSoundBuffer *, LPLONG);
    ULONG (STDMETHODCALLTYPE *release)(IDirectSoundBuffer *);
} BufVt;
typedef struct { IDirectSoundBuffer *b; LONG vol; int paused, loop; } Buf;
static BufVt g_bvt[4];
static int g_nbvt, g_nbufs;
static Buf g_bufs[512];
static CRITICAL_SECTION g_scs;
static HRESULT (STDMETHODCALLTYPE *real_csb)(IDirectSound *, LPCDSBUFFERDESC, LPDIRECTSOUNDBUFFER *, LPUNKNOWN);
static HRESULT (STDMETHODCALLTYPE *real_dup)(IDirectSound *, LPDIRECTSOUNDBUFFER, LPDIRECTSOUNDBUFFER *);
static HRESULT (WINAPI *real_dsc)(LPCGUID, LPDIRECTSOUND *, LPUNKNOWN);

static BufVt *bvt_of(IDirectSoundBuffer *b)
{
    int i;
    for (i = 0; i < g_nbvt; i++) if (g_bvt[i].vt == b->lpVtbl) return &g_bvt[i];
    return 0;
}
static Buf *buf_find(IDirectSoundBuffer *b)
{
    int i;
    for (i = 0; i < g_nbufs; i++) if (g_bufs[i].b == b) return &g_bufs[i];
    return 0;
}
static LONG atten(float g) { return g < 0.0001f ? DSBVOLUME_MIN : (LONG)(2000.0 * log10(g)); }
static void buf_apply(Buf *e)
{
    BufVt *v = bvt_of(e->b);
    LONG x = e->vol + atten(sfx_gain());
    if (x < DSBVOLUME_MIN) x = DSBVOLUME_MIN;
    if (v) v->setvol(e->b, x);
}
static void sfx_apply_all(void)
{
    int i;
    if (!g_nbvt) return;
    EnterCriticalSection(&g_scs);
    for (i = 0; i < g_nbufs; i++) buf_apply(&g_bufs[i]);
    LeaveCriticalSection(&g_scs);
}
static void sfx_pause(int on)
{
    int i;
    EnterCriticalSection(&g_scs);
    for (i = 0; i < g_nbufs; i++) {
        Buf *e = &g_bufs[i];
        if (on) {
            DWORD st = 0;
            IDirectSoundBuffer_GetStatus(e->b, &st);
            e->paused = (st & DSBSTATUS_PLAYING) != 0; e->loop = (st & DSBSTATUS_LOOPING) != 0;
            if (e->paused) IDirectSoundBuffer_Stop(e->b);
        } else if (e->paused) {
            IDirectSoundBuffer_Play(e->b, 0, 0, e->loop ? DSBPLAY_LOOPING : 0);
            e->paused = 0;
        }
    }
    LeaveCriticalSection(&g_scs);
}

/* SCORCHER_SNDLOG=1: every few seconds the playing buffers with the game volume and the real one. */
static void sfx_log(void)
{
    int i, n = 0; char t[600]; int k = 0;
    EnterCriticalSection(&g_scs);
    for (i = 0; i < g_nbufs && k < 560; i++) {
        DWORD st = 0; LONG rv = 0; BufVt *v = bvt_of(g_bufs[i].b);
        IDirectSoundBuffer_GetStatus(g_bufs[i].b, &st);
        if (!(st & DSBSTATUS_PLAYING)) continue;
        n++; if (v) v->getvol(g_bufs[i].b, &rv);
        k += sprintf(t + k, " %ld/%ld", g_bufs[i].vol, rv);
    }
    t[k] = 0;
    LeaveCriticalSection(&g_scs);
    lg("snd: %d bufs, %d playing (game/real):%s", g_nbufs, n, t);
}
static HRESULT STDMETHODCALLTYPE hook_SetVolume(IDirectSoundBuffer *b, LONG v)
{
    Buf *e;
    EnterCriticalSection(&g_scs);
    e = buf_find(b);
    if (e) { e->vol = v; buf_apply(e); }
    LeaveCriticalSection(&g_scs);
    return e ? DS_OK : bvt_of(b)->setvol(b, v);
}
static HRESULT STDMETHODCALLTYPE hook_GetVolume(IDirectSoundBuffer *b, LPLONG v)
{
    Buf *e;
    EnterCriticalSection(&g_scs);
    e = buf_find(b);
    if (e && v) *v = e->vol;
    LeaveCriticalSection(&g_scs);
    return e ? DS_OK : bvt_of(b)->getvol(b, v);
}
static ULONG STDMETHODCALLTYPE hook_Release(IDirectSoundBuffer *b)
{
    BufVt *v = bvt_of(b);
    ULONG r = v->release(b);
    if (!r) {
        Buf *e;
        EnterCriticalSection(&g_scs);
        e = buf_find(b);
        if (e) *e = g_bufs[--g_nbufs];
        LeaveCriticalSection(&g_scs);
    }
    return r;
}
static void buf_add(IDirectSoundBuffer *b, LONG vol)
{
    IDirectSoundBufferVtbl *vt = b->lpVtbl;
    EnterCriticalSection(&g_scs);
    if (!bvt_of(b) && g_nbvt < 4) {
        BufVt *e = &g_bvt[g_nbvt++];
        DWORD old;
        e->vt = vt; e->setvol = vt->SetVolume; e->getvol = vt->GetVolume; e->release = vt->Release;
        VirtualProtect(vt, sizeof *vt, PAGE_READWRITE, &old);
        vt->SetVolume = hook_SetVolume; vt->GetVolume = hook_GetVolume; vt->Release = hook_Release;
        VirtualProtect(vt, sizeof *vt, old, &old);
        lg("dsound buffer vtable %p hooked", vt);
    }
    if (bvt_of(b) && g_nbufs < 512) {
        Buf *e = &g_bufs[g_nbufs++];
        e->b = b; e->vol = vol; e->paused = 0; e->loop = 0;
        buf_apply(e);
    }
    LeaveCriticalSection(&g_scs);
}
static HRESULT STDMETHODCALLTYPE hook_csb(IDirectSound *ds, LPCDSBUFFERDESC d, LPDIRECTSOUNDBUFFER *o, LPUNKNOWN u)
{
    DSBUFFERDESC c;
    HRESULT hr;
    int sec = d && !(d->dwFlags & DSBCAPS_PRIMARYBUFFER);
    if (sec) {
        memset(&c, 0, sizeof c);
        memcpy(&c, d, d->dwSize < sizeof c ? d->dwSize : sizeof c);
        c.dwFlags |= DSBCAPS_CTRLVOLUME;
        d = &c;
    }
    hr = real_csb(ds, d, o, u);
    { static int n; if (n++ < 40) lg("CreateSoundBuffer flags %x bytes %u -> %08x", d ? d->dwFlags : 0, d ? d->dwBufferBytes : 0, hr); }
    if (SUCCEEDED(hr) && sec) buf_add(*o, 0);
    return hr;
}
static HRESULT STDMETHODCALLTYPE hook_dup(IDirectSound *ds, LPDIRECTSOUNDBUFFER s, LPDIRECTSOUNDBUFFER *o)
{
    HRESULT hr = real_dup(ds, s, o);
    if (SUCCEEDED(hr)) {
        Buf *e; LONG v = 0;
        EnterCriticalSection(&g_scs);
        e = buf_find(s); if (e) v = e->vol;
        LeaveCriticalSection(&g_scs);
        buf_add(*o, v);
    }
    return hr;
}
static HRESULT WINAPI fake_DirectSoundCreate(LPCGUID g, LPDIRECTSOUND *o, LPUNKNOWN u)
{
    HRESULT hr = real_dsc(g, o, u);
    lg("DirectSoundCreate -> %08x", hr);
    if (SUCCEEDED(hr) && !real_csb) {
        IDirectSoundVtbl *vt = (*o)->lpVtbl;
        DWORD old;
        real_csb = vt->CreateSoundBuffer; real_dup = vt->DuplicateSoundBuffer;
        VirtualProtect(vt, sizeof *vt, PAGE_READWRITE, &old);
        vt->CreateSoundBuffer = hook_csb; vt->DuplicateSoundBuffer = hook_dup;
        VirtualProtect(vt, sizeof *vt, old, &old);
    }
    return hr;
}

/* ---------------------------------------------------------------- CD audio from WAV files
   SCORWIN plays its music from the CD through MCI "cdaudio" (cd_Init 0x0043265d, cd_MciPlayTrack
   0x00432864). The exe's mciSendCommandA import is pointed here; a cdaudio device is faked and
   PLAY streams music\trackNN.wav (tools\ripaudio.py) through waveOut with the music level applied
   per buffer, posting MM_MCINOTIFY to the game's window when the track ends like the CD would
   (the game then loops it). */
#define FAKE_CD 0x4344
#define MBUFS 8
#define MFRAMES 2205   /* 50 ms at 44.1 kHz */
static MCIERROR (WINAPI *real_mci)(MCIDEVICEID, UINT, DWORD_PTR, DWORD_PTR);
static int g_cdtrack, g_cdplaying;
static HWAVEOUT g_wo;
static HANDLE g_mfile = INVALID_HANDLE_VALUE, g_mthr, g_mev;
static volatile int g_mstop;
static DWORD g_mleft;
static HWND g_mnotify;
static WAVEHDR g_wh[MBUFS];
static short g_mb[MBUFS][MFRAMES * 2];

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

static DWORD WINAPI music_thread(LPVOID arg)
{
    int busy[MBUFS] = {0}, eof = 0, i, n, k;
    while (!g_mstop) {
        for (i = n = 0; i < MBUFS; i++) {
            if (busy[i] && (g_wh[i].dwFlags & WHDR_DONE)) busy[i] = 0;
            if (!busy[i] && !eof) {
                DWORD want = sizeof g_mb[i] < g_mleft ? sizeof g_mb[i] : g_mleft, got = 0;
                float g = music_gain();
                ReadFile(g_mfile, g_mb[i], want, &got, 0);
                got &= ~3u; g_mleft -= got;
                if (!got) eof = 1;
                else {
                    if (g < 0.999f) for (k = 0; k < (int)got / 2; k++) g_mb[i][k] = (short)(g_mb[i][k] * g);
                    g_wh[i].dwBufferLength = got;
                    waveOutWrite(g_wo, &g_wh[i], sizeof g_wh[i]);
                    busy[i] = 1;
                }
            }
            n += busy[i];
        }
        if (eof && !n) {
            lg("cd track %d ended", g_cdtrack);
            g_cdplaying = 0;
            if (g_mnotify) PostMessageA(g_mnotify, MM_MCINOTIFY, MCI_NOTIFY_SUCCESSFUL, FAKE_CD);
            break;
        }
        WaitForSingleObject(g_mev, 100);
    }
    return 0;
}
static void music_stop(void)
{
    int i;
    if (g_mthr) { g_mstop = 1; SetEvent(g_mev); WaitForSingleObject(g_mthr, 3000); CloseHandle(g_mthr); g_mthr = 0; }
    if (g_wo) {
        waveOutReset(g_wo);
        for (i = 0; i < MBUFS; i++) waveOutUnprepareHeader(g_wo, &g_wh[i], sizeof g_wh[i]);
        waveOutClose(g_wo); g_wo = 0;
    }
    if (g_mfile != INVALID_HANDLE_VALUE) { CloseHandle(g_mfile); g_mfile = INVALID_HANDLE_VALUE; }
    g_cdplaying = 0;
}
static int music_play(int t, HWND notify)
{
    char path[MAX_PATH];
    DWORD got, id[2], riff[3];
    WAVEFORMATEX wf;
    int i, havefmt = 0;
    music_stop();
    wav_path(t, path);
    g_mfile = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if (g_mfile == INVALID_HANDLE_VALUE) { lg("cd play %d: no %s", t, path); return 0; }
    memset(&wf, 0, sizeof wf);
    if (!ReadFile(g_mfile, riff, 12, &got, 0) || got != 12 || riff[0] != 0x46464952 || riff[2] != 0x45564157) goto bad;
    for (;;) {   /* chunks up to "data" */
        if (!ReadFile(g_mfile, id, 8, &got, 0) || got != 8) goto bad;
        if (id[0] == 0x61746164) break;
        if (id[0] == 0x20746d66) {
            if (!ReadFile(g_mfile, &wf, 16, &got, 0) || got != 16) goto bad;
            havefmt = 1;
            SetFilePointer(g_mfile, ((id[1] + 1) & ~1u) - 16, 0, FILE_CURRENT);
        } else SetFilePointer(g_mfile, (id[1] + 1) & ~1u, 0, FILE_CURRENT);
    }
    if (!havefmt || wf.wFormatTag != WAVE_FORMAT_PCM || wf.wBitsPerSample != 16) goto bad;
    g_mleft = id[1];
    if (!g_mev) g_mev = CreateEventA(0, FALSE, FALSE, 0);
    if (waveOutOpen(&g_wo, WAVE_MAPPER, &wf, (DWORD_PTR)g_mev, 0, CALLBACK_EVENT)) { g_wo = 0; goto bad; }
    for (i = 0; i < MBUFS; i++) {
        memset(&g_wh[i], 0, sizeof g_wh[i]);
        g_wh[i].lpData = (LPSTR)g_mb[i]; g_wh[i].dwBufferLength = sizeof g_mb[i];
        waveOutPrepareHeader(g_wo, &g_wh[i], sizeof g_wh[i]);
    }
    g_cdtrack = t; g_cdplaying = 1; g_mnotify = notify; g_mstop = 0;
    g_mthr = CreateThread(0, 0, music_thread, 0, 0, 0);
    lg("cd play track %d", t);
    return 1;
bad:
    lg("cd play %d: bad wav %s", t, path);
    music_stop();
    return 0;
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
        int t = (fl & MCI_FROM) ? (int)(p->dwFrom & 0xff) : g_cdtrack;
        music_play(t, (fl & MCI_NOTIFY) ? (HWND)p->dwCallback : 0);
        return 0;
    }
    case MCI_STOP: case MCI_PAUSE: if (g_wo) waveOutPause(g_wo); g_cdplaying = 0; return 0;
    case MCI_RESUME: if (g_wo) { waveOutRestart(g_wo); g_cdplaying = 1; } return 0;
    case MCI_CLOSE: music_stop(); return 0;
    }
    return 0;
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
    InitializeCriticalSection(&g_cs);
    InitializeCriticalSection(&g_scs);
    settings_load();
    { int off = GetEnvironmentVariableA("SCORCHER_NOHOOK", env, sizeof env) ? atoi(env) : 0;   /* debug: bit mask of hooks to skip */
    real_mci = hook_import("winmm.dll", "mciSendCommandA", fake_mci);
    if (!(off & 1)) hook_import("winmm.dll", "timeGetTime", fake_timeGetTime);
    if (!(off & 2)) hook_import("winmm.dll", "waveOutSetVolume", fake_waveOutSetVolume);
    if (!(off & 4)) hook_import("winmm.dll", "auxSetVolume", fake_auxSetVolume);
    if (!(off & 16) && *(BYTE *)SWITCHPAGE == 0x68) hook_code(SWITCHPAGE, switch_stub);
    if (!(off & 32) && *(BYTE *)ESCCALL == 0xe8 && *(DWORD *)(ESCCALL + 1) == SETNEXT - (ESCCALL + 5)) hook_call(ESCCALL, esc_stub);
    if (!(off & 64)) patch_modes();
    if (!(off & 128)) cam_apply();
    if (!(off & 256)) patch_rivals();
    if (!(off & 256)) patch_manx();
    if (!(off & 8)) real_dsc = hook_import("dsound.dll", "DirectSoundCreate", fake_DirectSoundCreate); }
    return TRUE;
}

HRESULT WINAPI DirectDrawEnumerateA(LPDDENUMCALLBACKA cb, LPVOID ctx)
{
    cb(NULL, "Display", "display", ctx);
    return DD_OK;
}
