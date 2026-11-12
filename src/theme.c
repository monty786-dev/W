#include "winshell.h"
#include <math.h>

typedef struct { const wchar_t* n; COLORREF c[7]; BOOL dark; int style; int corner; } Raw;
#define C7(b,a,bar,s,rail,acc,txt) {{b,a,bar,s,rail,acc,txt}}
static const Raw kRaw[] = {
 {L"Windows 98",        C7(RGB(0xC0,0xC0,0xC0),RGB(0xD4,0xD0,0xC8),RGB(0xC0,0xC0,0xC0),RGB(0xC0,0xC0,0xC0),RGB(0x80,0x80,0x80),RGB(0x0A,0x24,0x6A),RGB(0,0,0)), FALSE, 3, 0},
 {L"Windows 7",         C7(RGB(0xF0,0xF0,0xF0),RGB(0xE6,0xEB,0xF2),RGB(0xA4,0xC6,0xE8),RGB(0xE8,0xF0,0xF8),RGB(0xD6,0xE2,0xEE),RGB(0x1E,0x77,0xC4),RGB(0,0,0)), FALSE, 1, 6},
 {L"Windows 8",         C7(RGB(0x1C,0x1C,0x1C),RGB(0x2A,0x2A,0x2A),RGB(0x10,0x10,0x10),RGB(0x1A,0x1A,0x1A),RGB(0x08,0x08,0x08),RGB(0x00,0x99,0xBC),RGB(255,255,255)), TRUE, 0, 0},
 {L"Windows 10",        C7(RGB(0x20,0x20,0x20),RGB(0x2B,0x2B,0x2B),RGB(0x17,0x17,0x17),RGB(0x1F,0x1F,0x1F),RGB(0x14,0x14,0x14),RGB(0x00,0x78,0xD7),RGB(255,255,255)), TRUE, 0, 0},
 {L"Ubuntu (Yaru)",     C7(RGB(0x2C,0x2C,0x2C),RGB(0x3A,0x3A,0x3A),RGB(0x1C,0x1C,0x1C),RGB(0x2C,0x2C,0x2C),RGB(0x14,0x14,0x14),RGB(0xE9,0x54,0x20),RGB(255,255,255)), TRUE, 4, 6},
 {L"Arch Linux",        C7(RGB(0x1E,0x1E,0x28),RGB(0x28,0x28,0x36),RGB(0x10,0x10,0x18),RGB(0x1E,0x1E,0x28),RGB(0x0A,0x0A,0x10),RGB(0x17,0x92,0xDD),RGB(255,255,255)), TRUE, 4, 6},
 {L"Linux Mint",        C7(RGB(0xDD,0xDD,0xDD),RGB(0xEC,0xEC,0xEC),RGB(0xC8,0xC8,0xC8),RGB(0xDD,0xDD,0xDD),RGB(0xBB,0xBB,0xBB),RGB(0x6C,0xB7,0x3F),RGB(0,0,0)), FALSE, 4, 6},
 {L"Xfce (Greybird)",   C7(RGB(0xE1,0xE2,0xE6),RGB(0xEE,0xEE,0xF0),RGB(0xD0,0xD2,0xD8),RGB(0xE1,0xE2,0xE6),RGB(0xC4,0xC6,0xCC),RGB(0x36,0x9B,0xD3),RGB(0,0,0)), FALSE, 4, 6},
 {L"KDE (Breeze)",      C7(RGB(0xE3,0xE5,0xE7),RGB(0xEF,0xF0,0xF1),RGB(0xD6,0xD8,0xDA),RGB(0xE3,0xE5,0xE7),RGB(0xC4,0xC6,0xC8),RGB(0x3D,0xAE,0xE9),RGB(0,0,0)), FALSE, 5, 8},
 {L"Cinnamon",          C7(RGB(0x23,0x27,0x2E),RGB(0x2F,0x34,0x3D),RGB(0x1A,0x1D,0x22),RGB(0x23,0x27,0x2E),RGB(0x14,0x16,0x1A),RGB(0x3A,0xB1,0xF4),RGB(255,255,255)), TRUE, 4, 8},
 {L"macOS (Aqua)",      C7(RGB(0xEC,0xEC,0xEC),RGB(0xF4,0xF4,0xF4),RGB(0xE8,0xE8,0xE8),RGB(0xEC,0xEC,0xEC),RGB(0xD8,0xD8,0xD8),RGB(0x0A,0x84,0xFF),RGB(0,0,0)), FALSE, 6, 12},
 {L"AMOLED",            C7(RGB(0,0,0),RGB(0x10,0x10,0x10),RGB(0,0,0),RGB(0x08,0x08,0x08),RGB(0,0,0),RGB(0x00,0x99,0xBB),RGB(255,255,255)), TRUE, 0, 0},
};
#undef C7

int WsThemeCount(void){ return (int)(sizeof(kRaw)/sizeof(kRaw[0])); }
const WsTheme* WsThemeAt(int i){
    static WsTheme out; if (i<0||i>=WsThemeCount()) i=3;
    const Raw* r=&kRaw[i];
    out.name=r->n; out.bg=r->c[0]; out.alt=r->c[1]; out.bar=r->c[2];
    out.start_bg=r->c[3]; out.rail=r->c[4]; out.accent=r->c[5]; out.text=r->c[6];
    out.dark=r->dark; out.taskbar_style=r->style; out.corner=r->corner;
    return &out;
}

COLORREF WsAccent(void){
    if (g_cfg.accent) return (COLORREF)g_cfg.accent;
    return WsThemeAt(g_cfg.theme)->accent;
}
BOOL WsDarkMode(void){ return g_cfg.dark_mode || WsThemeAt(g_cfg.theme)->dark; }

COLORREF WsShade(COLORREF c, float f){
    int r=(int)(GetRValue(c)*f), g=(int)(GetGValue(c)*f), b=(int)(GetBValue(c)*f);
    if(r>255)r=255; if(g>255)g=255; if(b>255)b=255;
    return RGB(r,g,b);
}
COLORREF WsMix(COLORREF a, COLORREF b, float t){
    return RGB(
        (int)(GetRValue(a)*(1-t) + GetRValue(b)*t),
        (int)(GetGValue(a)*(1-t) + GetGValue(b)*t),
        (int)(GetBValue(a)*(1-t) + GetBValue(b)*t));
}
COLORREF WsContrastText(COLORREF bg){
    int y = (GetRValue(bg)*299 + GetGValue(bg)*587 + GetBValue(bg)*114)/1000;
    return y < 140 ? RGB(255,255,255) : RGB(0,0,0);
}

/* ---- GDI+ glass, cached per HWND + size + scroll of wallpaper ---- */
static ULONG_PTR s_gdip_token;
static int       s_gdip_ok = -1;
typedef struct { HWND h; int w,h,ox,oy; COLORREF base; HBITMAP bmp; } GlassEnt;
static GlassEnt s_glass[8];

typedef int (WINAPI *PGdipStartup)(ULONG_PTR*, void*, void*);
typedef int (WINAPI *PGdipFromHDC)(HDC, void**);
typedef int (WINAPI *PGdipSetMode)(void*, int);
typedef int (WINAPI *PGdipDrawImg)(void*, void*, int,int,int,int);
typedef int (WINAPI *PGdipDelG)(void*);
typedef int (WINAPI *PGdipBmpFromFile)(const wchar_t*, void**);
typedef int (WINAPI *PGdipBmpToHBmp)(void*, HBITMAP*, COLORREF);
typedef int (WINAPI *PGdipDispose)(void*);

static struct {
    HMODULE dll;
    PGdipStartup Startup; PGdipFromHDC FromHDC; PGdipSetMode SetMode;
    PGdipDrawImg DrawImg; PGdipDelG DelG;
    PGdipBmpFromFile BmpFromFile; PGdipBmpToHBmp BmpToHBmp; PGdipDispose Dispose;
} G;

static void GdipInit(void){
    if (s_gdip_ok != -1) return;
    s_gdip_ok = 0;
    G.dll = LoadLibraryW(L"gdiplus.dll");
    if (!G.dll) return;
    G.Startup    = (PGdipStartup)   GetProcAddress(G.dll,"GdiplusStartup");
    G.FromHDC    = (PGdipFromHDC)   GetProcAddress(G.dll,"GdipCreateFromHDC");
    G.SetMode    = (PGdipSetMode)   GetProcAddress(G.dll,"GdipSetInterpolationMode");
    G.DrawImg    = (PGdipDrawImg)   GetProcAddress(G.dll,"GdipDrawImageRectI");
    G.DelG       = (PGdipDelG)      GetProcAddress(G.dll,"GdipDeleteGraphics");
    G.BmpFromFile= (PGdipBmpFromFile)GetProcAddress(G.dll,"GdipCreateBitmapFromFile");
    G.BmpToHBmp  = (PGdipBmpToHBmp) GetProcAddress(G.dll,"GdipCreateHBITMAPFromBitmap");
    G.Dispose    = (PGdipDispose)   GetProcAddress(G.dll,"GdipDisposeImage");
    if (!G.Startup||!G.FromHDC) { FreeLibrary(G.dll); G.dll=NULL; return; }
    /* 1,4 bytes; NULL callback; 0,0 */
    unsigned char in[16]={1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    if (G.Startup(&s_gdip_token,in,NULL)!=0){ FreeLibrary(G.dll); G.dll=NULL; return; }
    s_gdip_ok = 1;
}

static HBITMAP LoadWallpaperToBitmap(const wchar_t* path, int* w, int* h){
    *w=*h=0;
    if (!path||!path[0]) return NULL;
    GdipInit();
    if (!s_gdip_ok) return (HBITMAP)LoadImageW(NULL,path,IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
    void* bmp=NULL;
    if (G.BmpFromFile(path,&bmp)!=0) return NULL;
    HBITMAP hb=NULL;
    if (G.BmpToHBmp(bmp,&hb,0xFF000000)==0){
        BITMAP bm; GetObjectW(hb,sizeof(bm),&bm);
        *w=bm.bmWidth; *h=bm.bmHeight;
    }
    G.Dispose(bmp);
    return hb;
}

void WsGlassBitmap(HWND hwnd, HDC hdc, int w, int h, int ox, int oy, COLORREF base, BOOL allow_blur){
    if (w<=0||h<=0) return;
    if (!g_cfg.glass_bar && hwnd==NULL) { /* caller decides */ }

    /* cache lookup */
    for (int i=0;i<8;i++){
        GlassEnt* e=&s_glass[i];
        if (e->h==hwnd && e->w==w && e->h && e->bmp && e->base==base && e->ox==ox && e->oy==oy){
            HDC m=CreateCompatibleDC(hdc); HBITMAP old=SelectObject(m,e->bmp);
            BitBlt(hdc,0,0,w,h,m,0,0,SRCCOPY);
            SelectObject(m,old); DeleteDC(m); return;
        }
    }

    HDC screen = GetDC(NULL);
    HDC mdc = CreateCompatibleDC(screen);
    HBITMAP bmp = CreateCompatibleBitmap(screen, w, h);
    HBITMAP old = SelectObject(mdc, bmp);

    /* base fill / blur */
    HBRUSH br = CreateSolidBrush(base);
    RECT rr = {0,0,w,h}; FillRect(mdc,&rr,br); DeleteObject(br);

    if (allow_blur && g_cfg.glass_blur && g_cfg.wallpaper[0]){
        int ww=0, wh=0;
        HBITMAP wall = LoadWallpaperToBitmap(g_cfg.wallpaper, &ww, &wh);
        if (wall){
            /* shrink-stretch fast blur (ARM friendly: no GDI+ bitmap writes needed) */
            const int SM=8;
            HDC m1=CreateCompatibleDC(screen);
            HBITMAP small=CreateCompatibleBitmap(screen, w/SM+1, h/SM+1);
            HBITMAP so=SelectObject(m1, small);
            HDC wdc=CreateCompatibleDC(screen); HBITMAP wo=SelectObject(wdc, wall);
            SetStretchBltMode(m1, HALFTONE);
            StretchBlt(m1,0,0,w/SM+1,h/SM+1,wdc, ox*(ww>0?1:1), oy, w*SM, h*SM, SRCCOPY);
            SetStretchBltMode(mdc, HALFTONE);
            StretchBlt(mdc,0,0,w,h,m1,0,0,w/SM+1,h/SM+1,SRCCOPY);
            SelectObject(wdc,wo); DeleteDC(wdc);
            SelectObject(m1,so); DeleteObject(small); DeleteDC(m1);
            SelectObject(wall?0:0,NULL); DeleteObject(wall);
        }
        /* translucent tint over blur */
        int a = 255 - (int)((g_cfg.glass_opacity/100.0f)*120.0f);
        HBRUSH tint = CreateSolidBrush(base);
        BLENDFUNCTION bf = { AC_SRC_OVER, 0, (BYTE)a, 0 };
        /* manual alpha: fallback to plain bitblt if msimg32 missing */
        HMODULE msi=LoadLibraryW(L"msimg32.dll");
        if (msi){
            typedef BOOL (WINAPI *PA)(HDC,int,int,int,int,HDC,int,int,int,int,BLENDFUNCTION);
            PA p=(PA)GetProcAddress(msi,"AlphaBlend");
            if (p){ p(mdc,0,0,w,h,mdc,0,0,w,h,bf); }
            FreeLibrary(msi);
        }
        DeleteObject(tint);
    }

    /* specular sheen from top */
    int sh = (int)(h * (g_cfg.glass_shine/100.0f) * 0.5f);
    for (int y=0;y<sh;y++){
        int alpha = (int)(60 * (1 - y/(float)sh) * (g_cfg.glass_gloss/100.0f));
        if (alpha<=0) break;
        HBRUSH wbr = CreateSolidBrush(RGB(255,255,255));
        BLENDFUNCTION bf = { AC_SRC_OVER, 0, (BYTE)alpha, 0 };
        HMODULE msi=LoadLibraryW(L"msimg32.dll");
        if (msi){
            typedef BOOL (WINAPI *PA)(HDC,int,int,int,int,HDC,int,int,int,int,BLENDFUNCTION);
            PA p=(PA)GetProcAddress(msi,"AlphaBlend");
            if (p){ RECT hpr={0,y,w,y+1}; HBRUSH b=CreateSolidBrush(RGB(255,255,255));
                    FillRect(mdc,&hpr,b); DeleteObject(b); }
            FreeLibrary(msi);
        }
        DeleteObject(wbr);
    }

    SelectObject(mdc, old);
    BitBlt(hdc,0,0,w,h,mdc,0,0,SRCCOPY);

    /* store */
    GlassEnt* slot=&s_glass[0];
    for(int i=0;i<8;i++) if(s_glass[i].h==hwnd){ slot=&s_glass[i]; break; }
    if (slot->bmp) DeleteObject(slot->bmp);
    slot->h=hwnd; slot->w=w; slot->h=h; slot->ox=ox; slot->oy=oy; slot->base=base; slot->bmp=bmp;

    SelectObject(mdc, old); DeleteDC(mdc); ReleaseDC(NULL,screen);
}
