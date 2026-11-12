/* winshell.h — shared definitions */
#pragma once
#define UNICODE
#define _UNICODE
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>
#include <shlobj.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define WS_APP_NAME       L"WinShell"
#define WS_APP_VERSION    L"1.0.0"
#define WS_MUTEX_NAME     L"WinShell.Taskbar.Mutex.v1"
#define WS_BAR_CLASS      L"WinShellTaskbar"
#define WS_START_CLASS    L"WinShellStart"
#define WS_SKIN_CLASS     L"WinShellSkin"
#define WS_NOTIFY_CLASS   L"WinShellNotify"
#define WS_REVEAL_CLASS   L"WinShellReveal"
#define WS_SHOT_CLASS     L"WinShellShot"
#define WS_FLOAT_CLASS    L"WinShellFloat"

/* WM_COPYDATA command codes for setup.exe / second instance */
#define WS_CMD_OPEN        1
#define WS_CMD_SETUP       2
#define WS_CMD_RELOAD      3
#define WS_CMD_QUIT        4

/* Registered shell hook message (used by Start menu focus-out) */
extern UINT WM_WS_RELOAD;

/* ---------- config ---------- */
typedef struct {
    /* geometry */
    int  bar_position;        /* 0=bottom,1=top,2=left,3=right */
    int  bar_alignment;       /* 0=left,1=center,2=right */
    int  bar_height;          /* px at 96dpi */
    int  icon_size;
    int  bar_round;
    int  underline_height;
    int  show_titles;
    int  underline_from_icon;
    int  taskbar_always_front;
    /* visibility */
    int  hide_wine_bar;
    int  fs_mode;             /* 0=hide,1=show,2=resize,3=float,4=nothing */
    int  max_mode;
    int  autostart;
    int  startup_delay_s;
    /* preview */
    int  preview_enabled;
    int  preview_live;
    int  preview_w, preview_h;
    int  preview_delay_ms;
    /* reveal bar */
    int  strip_enabled;
    int  strip_pos;           /* 0=left,1=center,2=right */
    int  strip_align;         /* 0=bottom,1=top */
    int  strip_height;
    int  strip_width_pct;
    int  strip_opacity;
    int  strip_corner;
    int  strip_color;         /* 0=AUTO, else 0xRRGGBB */
    int  strip_seconds;
    int  strip_anim;          /* 0=pop,1=slide,2=blink */
    /* start menu */
    int  start_size;          /* 0=normal,1=mid,2=large,3=full */
    int  start_round;
    int  start_icon_color;    /* 0=AUTO, else 0xRRGGBB */
    int  start_show_recent;
    int  start_show_pinned;
    int  tile_size;
    int  tile_color_mode;     /* 0=multi,1=icon,2=accent */
    int  tile_transparency;
    int  tile_anim;
    /* window skin */
    int  skin_enabled;
    int  skin_theme;          /* index into theme table */
    int  skin_color_mode;
    int  skin_custom_color;
    int  skin_border;
    int  skin_scrollbars;
    int  skin_syscolors;
    int  skin_owned;
    int  skin_btn_pin, skin_btn_snap, skin_btn_shade;
    int  skin_opacity;
    /* personalization */
    int  theme;               /* index into theme table */
    int  dark_mode;
    int  glass;
    int  glass_blur;
    int  glass_opacity;
    int  glass_shine;
    int  glass_gloss;
    int  shadow;
    int  round_corners;
    int  accent;              /* 0xAUTO or 0xRRGGBB */
    int  icon_preset;         /* 0..14 alternate icon presets */
    int  icon_tint;           /* tint app icons to theme */
    int  icon_fill;
    wchar_t font_name[64];
    int  font_size;
    float ui_scale;
    /* cursor */
    int  cursor_enabled;
    wchar_t cursor_arrow[260], cursor_wait[260], cursor_ibeam[260];
    wchar_t cursor_link[260], cursor_grab[260], cursor_load[260];
    /* keyboard */
    int  kbd_auto;
    int  kbd_height_pct;
    int  kbd_opacity;
    int  kbd_pan;
    /* desktop */
    int  desktop_mode;
    int  grid_size;
    int  grid_snap;
    int  desktop_icons;
    int  desktop_lock;
    wchar_t wallpaper[260];
    int  wallpaper_fit;       /* 0=fill,1=fit,2=stretch,3=center,4=tile */
    int  desktop_color;
    /* user */
    wchar_t user_name[64];
    wchar_t user_photo[260];
    wchar_t pinned[64][260];
    int  pinned_count;
    wchar_t start_pins[64][260];
    int  start_pins_count;
    wchar_t tray_apps[16][260];
    int  tray_apps_count;
    /* effects matrix */
    int  glass_bar, glass_start, glass_preview, glass_title, glass_install;
} WsConfig;

extern WsConfig g_cfg;
extern HINSTANCE g_hinst;

/* ---------- config.c ---------- */
BOOL WsConfigLoad(void);
BOOL WsConfigSave(void);
void WsConfigDefaults(void);
int  WsConfigFileMtime(void);
BOOL WsConfigChangedOnDisk(void);
void WsConfigPath(wchar_t* out, int cch);

/* ---------- theme.c ---------- */
typedef struct {
    const wchar_t* name;
    COLORREF bg, alt, bar, start_bg, rail, accent, text;
    BOOL     dark;
    int      taskbar_style;   /* 0=flat,1=aero,2=luna,3=classic,4=yaru,5=breeze,... */
    int      corner;          /* 0..24 */
} WsTheme;

int        WsThemeCount(void);
const WsTheme* WsThemeAt(int i);
COLORREF   WsAccent(void);
BOOL       WsDarkMode(void);
COLORREF   WsShade(COLORREF c, float f);
COLORREF   WsMix(COLORREF a, COLORREF b, float t);
COLORREF   WsContrastText(COLORREF bg);
HFONT      WsFont(int bump, int weight);
void       WsInvalidateFonts(void);
void       WsGlassBitmap(HWND hwnd, HDC hdc, int w, int h, int ox, int oy,
                         COLORREF base, BOOL allow_blur);

/* ---------- taskbar.c ---------- */
BOOL WsTaskbarCreate(void);
void WsTaskbarApply(void);
void WsTaskbarInvalidate(void);
RECT WsTaskbarRect(void);
RECT WsWorkRect(void);
HWND WsTaskbarHwnd(void);
void WsTaskbarReveal(int seconds);
void WsTaskbarForceFront(BOOL hard);

/* ---------- startmenu.c ---------- */
void WsStartToggle(void);
void WsStartClose(void);
BOOL WsStartVisible(void);

/* ---------- skin.c ---------- */
void WsSkinInit(void);
void WsSkinTick(void);
void WsSkinShutdown(void);
void WsSkinInvalidate(void);

/* ---------- ipc.c ---------- */
BOOL WsSendCommand(int cmd, const wchar_t* arg);
void WsHandleCopyData(HWND h, COPYDATASTRUCT* cds);

/* ---------- installer.c ---------- */
int  WsInstallRun(void);         /* called by setup.exe */
BOOL WsRegisterAutostart(BOOL on, DWORD delay_s);
BOOL WsSetWineHideBar(BOOL hide);
BOOL WsUninstall(void);

/* ---------- helpers ---------- */
void WsSetLayered(HWND h, BOOL on, int opacity_pct);
void WsFillRectC(HDC hdc, RECT* r, COLORREF c);
void WsFrameC(HDC hdc, RECT* r, COLORREF c, int t);
void WsTextC(HDC hdc, const wchar_t* s, RECT* r, UINT flags, COLORREF c, HFONT f);
