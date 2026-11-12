#include "winshell.h"

static wchar_t s_path[MAX_PATH];
static int     s_mtime = 0;

void WsConfigPath(wchar_t* out, int cch) {
    wchar_t progdata[MAX_PATH] = {0};
    if (!GetEnvironmentVariableW(L"PROGRAMDATA", progdata, MAX_PATH))
        wcscpy(progdata, L"C:\\ProgramData");
    _snwprintf(out, cch, L"%s\\WinShell\\config.ini", progdata);
    out[cch-1] = 0;
}

static void EnsureDir(const wchar_t* file) {
    wchar_t buf[MAX_PATH];
    wcscpy(buf, file);
    wchar_t* p = wcsrchr(buf, L'\\');
    if (p) { *p = 0; CreateDirectoryW(buf, NULL); }
}

void WsConfigDefaults(void) {
    memset(&g_cfg, 0, sizeof(g_cfg));
    g_cfg.bar_position = 0; g_cfg.bar_alignment = 0;
    g_cfg.bar_height = 40; g_cfg.icon_size = 24;
    g_cfg.taskbar_always_front = 1;
    g_cfg.preview_enabled = 1; g_cfg.preview_live = 1;
    g_cfg.preview_w = 200; g_cfg.preview_h = 128; g_cfg.preview_delay_ms = 450;
    g_cfg.strip_enabled = 0; g_cfg.strip_seconds = 4;
    g_cfg.strip_pos = 1; g_cfg.strip_align = 0; g_cfg.strip_height = 6; g_cfg.strip_width_pct = 40;
    g_cfg.start_size = 0;
    g_cfg.skin_enabled = 0; g_cfg.skin_theme = 0;
    g_cfg.theme = 0; g_cfg.dark_mode = 1;
    g_cfg.glass = 1; g_cfg.glass_opacity = 88;
    g_cfg.glass_shine = 50; g_cfg.glass_gloss = 50;
    g_cfg.ui_scale = 1.0f; g_cfg.font_size = 14;
    wcscpy(g_cfg.font_name, L"Segoe UI");
    g_cfg.cursor_enabled = 0;
    g_cfg.kbd_auto = 0; g_cfg.kbd_height_pct = 38; g_cfg.kbd_opacity = 96;
    g_cfg.grid_size = 96; g_cfg.grid_snap = 1; g_cfg.desktop_icons = 1;
    g_cfg.desktop_color = RGB(30,58,95);
    g_cfg.tile_size = 96; g_cfg.tile_transparency = 15;
    wcscpy(g_cfg.user_name, L"User");
    /* default pins: file manager + notepad */
    wcscpy(g_cfg.pinned[0], L"explorer.exe");  g_cfg.pinned_count = 1;
}

BOOL WsConfigLoad(void) {
    WsConfigDefaults();
    WsConfigPath(s_path, MAX_PATH);
    WIN32_FILE_ATTRIBUTE_DATA fa;
    if (!GetFileAttributesExW(s_path, GetFileExInfoStandard, &fa)) {
        WsConfigSave();
        return FALSE;
    }
    s_mtime = (int)fa.ftLastWriteTime.dwLowDateTime;

    #define G(s,k) GetPrivateProfileIntW(L"s", L"k", g_cfg.v, s_path)
    #define GT(s,k,v) GetPrivateProfileIntW(L"s", L"k", (v), s_path)
    #define GF(s,k)   (g_cfg.k = (float)_wtof(_GetStr(s,k)))
    #define GS(s,k,f) GetPrivateProfileStringW(L"s", L"k", f, g_cfg.k, ARRAYSIZE(g_cfg.k), s_path)

    g_cfg.bar_position      = GT(L"Taskbar", L"Position", 0);
    g_cfg.bar_alignment     = GT(L"Taskbar", L"Alignment", 0);
    g_cfg.bar_height        = GT(L"Taskbar", L"Height", 40);
    g_cfg.icon_size         = GT(L"Taskbar", L"IconSize", 24);
    g_cfg.bar_round         = GT(L"Taskbar", L"CornerRound", 0);
    g_cfg.show_titles       = GT(L"Taskbar", L"ShowTitles", 0);
    g_cfg.underline_from_icon = GT(L"Taskbar", L"UnderlineFromIcon", 1);
    g_cfg.underline_height  = GT(L"Taskbar", L"UnderlineHeight", 3);
    g_cfg.taskbar_always_front = GT(L"Taskbar", L"AlwaysFront", 1);
    g_cfg.hide_wine_bar     = GT(L"Taskbar", L"HideWineBar", 0);
    g_cfg.fs_mode           = GT(L"Taskbar", L"FsMode", 0);
    g_cfg.max_mode          = GT(L"Taskbar", L"MaxMode", 1);
    g_cfg.autostart         = GT(L"Taskbar", L"Autostart", 0);
    g_cfg.startup_delay_s   = GT(L"Taskbar", L"StartupDelay", 3);
    g_cfg.preview_enabled   = GT(L"Taskbar", L"PreviewEnabled", 1);
    g_cfg.preview_live      = GT(L"Taskbar", L"PreviewLive", 1);
    g_cfg.preview_w         = GT(L"Taskbar", L"PreviewW", 200);
    g_cfg.preview_h         = GT(L"Taskbar", L"PreviewH", 128);
    g_cfg.preview_delay_ms  = GT(L"Taskbar", L"PreviewDelayMs", 450);
    g_cfg.strip_enabled     = GT(L"Reveal",  L"Enabled", 0);
    g_cfg.strip_pos         = GT(L"Reveal",  L"Pos", 1);
    g_cfg.strip_align       = GT(L"Reveal",  L"Align", 0);
    g_cfg.strip_height      = GT(L"Reveal",  L"Height", 6);
    g_cfg.strip_width_pct   = GT(L"Reveal",  L"WidthPct", 40);
    g_cfg.strip_opacity     = GT(L"Reveal",  L"Opacity", 80);
    g_cfg.strip_corner      = GT(L"Reveal",  L"Corner", 1);
    g_cfg.strip_seconds     = GT(L"Reveal",  L"Seconds", 4);
    g_cfg.strip_anim        = GT(L"Reveal",  L"Anim", 0);
    g_cfg.start_size        = GT(L"Start",   L"Size", 0);
    g_cfg.start_round       = GT(L"Start",   L"Round", 0);
    g_cfg.start_show_recent = GT(L"Start",   L"ShowRecent", 1);
    g_cfg.start_show_pinned = GT(L"Start",   L"ShowPinned", 1);
    g_cfg.tile_size         = GT(L"Start",   L"TileSize", 96);
    g_cfg.tile_color_mode   = GT(L"Start",   L"TileColorMode", 0);
    g_cfg.tile_transparency = GT(L"Start",   L"TileTransparency", 15);
    g_cfg.skin_enabled      = GT(L"Skin",    L"Enabled", 0);
    g_cfg.skin_theme        = GT(L"Skin",    L"Theme", 0);
    g_cfg.skin_color_mode   = GT(L"Skin",    L"ColorMode", 0);
    g_cfg.skin_border       = GT(L"Skin",    L"Border", 1);
    g_cfg.skin_scrollbars   = GT(L"Skin",    L"Scrollbars", 1);
    g_cfg.skin_syscolors    = GT(L"Skin",    L"SysColors", 0);
    g_cfg.skin_owned        = GT(L"Skin",    L"Owned", 1);
    g_cfg.skin_opacity      = GT(L"Skin",    L"Opacity", 100);
    g_cfg.theme             = GT(L"Personalization", L"Theme", 0);
    g_cfg.dark_mode         = GT(L"Personalization", L"Dark", 1);
    g_cfg.glass             = GT(L"Personalization", L"Glass", 1);
    g_cfg.glass_blur        = GT(L"Personalization", L"Blur", 1);
    g_cfg.glass_opacity     = GT(L"Personalization", L"Opacity", 88);
    g_cfg.glass_shine       = GT(L"Personalization", L"Shine", 50);
    g_cfg.glass_gloss       = GT(L"Personalization", L"Gloss", 50);
    g_cfg.shadow            = GT(L"Personalization", L"Shadow", 0);
    g_cfg.round_corners     = GT(L"Personalization", L"RoundCorners", 0);
    g_cfg.accent            = GT(L"Personalization", L"Accent", 0);
    g_cfg.icon_preset       = GT(L"Personalization", L"IconPreset", 0);
    g_cfg.icon_tint         = GT(L"Personalization", L"IconTint", 0);
    g_cfg.icon_fill         = GT(L"Personalization", L"IconFill", 0);
    g_cfg.font_size         = GT(L"Personalization", L"FontSize", 14);
    g_cfg.glass_bar         = GT(L"Personalization", L"GlassBar", 1);
    g_cfg.glass_start       = GT(L"Personalization", L"GlassStart", 1);
    g_cfg.glass_preview     = GT(L"Personalization", L"GlassPreview", 1);
    g_cfg.glass_title       = GT(L"Personalization", L"GlassTitle", 1);
    g_cfg.cursor_enabled    = GT(L"Cursor",  L"Enabled", 0);
    g_cfg.kbd_auto          = GT(L"Keyboard", L"Auto", 0);
    g_cfg.kbd_height_pct    = GT(L"Keyboard", L"HeightPct", 38);
    g_cfg.kbd_opacity       = GT(L"Keyboard", L"Opacity", 96);
    g_cfg.kbd_pan           = GT(L"Keyboard", L"Pan", 1);
    g_cfg.desktop_mode      = GT(L"Desktop", L"Mode", 0);
    g_cfg.grid_size         = GT(L"Desktop", L"GridSize", 96);
    g_cfg.grid_snap         = GT(L"Desktop", L"Snap", 1);
    g_cfg.desktop_icons     = GT(L"Desktop", L"ShowIcons", 1);
    g_cfg.desktop_lock      = GT(L"Desktop", L"Lock", 0);
    g_cfg.wallpaper_fit     = GT(L"Desktop", L"Fit", 0);
    g_cfg.desktop_color     = GT(L"Desktop", L"Color", (int)RGB(30,58,95));

    GetPrivateProfileStringW(L"Personalization", L"Font", L"Segoe UI",
                             g_cfg.font_name, 64, s_path);
    g_cfg.ui_scale = (float)_wtof(L"1.0");
    {
        wchar_t tmp[32] = L"1.0";
        GetPrivateProfileStringW(L"Personalization", L"Scale", L"1.0", tmp, 32, s_path);
        g_cfg.ui_scale = (float)_wtof(tmp);
    }
    GetPrivateProfileStringW(L"User", L"Name", L"User", g_cfg.user_name, 64, s_path);
    GetPrivateProfileStringW(L"User", L"Photo", L"", g_cfg.user_photo, 260, s_path);
    GetPrivateProfileStringW(L"Cursor", L"Arrow", L"", g_cfg.cursor_arrow, 260, s_path);
    GetPrivateProfileStringW(L"Cursor", L"Wait", L"", g_cfg.cursor_wait, 260, s_path);
    GetPrivateProfileStringW(L"Cursor", L"Ibeam", L"", g_cfg.cursor_ibeam, 260, s_path);
    GetPrivateProfileStringW(L"Cursor", L"Link", L"", g_cfg.cursor_link, 260, s_path);
    GetPrivateProfileStringW(L"Cursor", L"Grab", L"", g_cfg.cursor_grab, 260, s_path);
    GetPrivateProfileStringW(L"Cursor", L"Load", L"", g_cfg.cursor_load, 260, s_path);
    GetPrivateProfileStringW(L"Desktop", L"Wallpaper", L"", g_cfg.wallpaper, 260, s_path);

    return TRUE;
}

BOOL WsConfigSave(void) {
    WsConfigPath(s_path, MAX_PATH);
    EnsureDir(s_path);
    #define P(s,k,v) { wchar_t _b[32]; _itow(v,_b,10); WritePrivateProfileStringW(L"s",L"k",_b,s_path); }
    #define PS(s,k,v) WritePrivateProfileStringW(L"s",L"k",v,s_path)

    P(L"Taskbar",L"Position",g_cfg.bar_position); P(L"Taskbar",L"Alignment",g_cfg.bar_alignment);
    P(L"Taskbar",L"Height",g_cfg.bar_height);     P(L"Taskbar",L"IconSize",g_cfg.icon_size);
    P(L"Taskbar",L"CornerRound",g_cfg.bar_round); P(L"Taskbar",L"ShowTitles",g_cfg.show_titles);
    P(L"Taskbar",L"UnderlineFromIcon",g_cfg.underline_from_icon);
    P(L"Taskbar",L"UnderlineHeight",g_cfg.underline_height);
    P(L"Taskbar",L"AlwaysFront",g_cfg.taskbar_always_front);
    P(L"Taskbar",L"HideWineBar",g_cfg.hide_wine_bar);
    P(L"Taskbar",L"FsMode",g_cfg.fs_mode);        P(L"Taskbar",L"MaxMode",g_cfg.max_mode);
    P(L"Taskbar",L"Autostart",g_cfg.autostart);   P(L"Taskbar",L"StartupDelay",g_cfg.startup_delay_s);
    P(L"Taskbar",L"PreviewEnabled",g_cfg.preview_enabled);
    P(L"Taskbar",L"PreviewLive",g_cfg.preview_live);
    P(L"Taskbar",L"PreviewW",g_cfg.preview_w);    P(L"Taskbar",L"PreviewH",g_cfg.preview_h);
    P(L"Taskbar",L"PreviewDelayMs",g_cfg.preview_delay_ms);
    P(L"Reveal",L"Enabled",g_cfg.strip_enabled);  P(L"Reveal",L"Pos",g_cfg.strip_pos);
    P(L"Reveal",L"Align",g_cfg.strip_align);      P(L"Reveal",L"Height",g_cfg.strip_height);
    P(L"Reveal",L"WidthPct",g_cfg.strip_width_pct); P(L"Reveal",L"Opacity",g_cfg.strip_opacity);
    P(L"Reveal",L"Corner",g_cfg.strip_corner);    P(L"Reveal",L"Seconds",g_cfg.strip_seconds);
    P(L"Reveal",L"Anim",g_cfg.strip_anim);
    P(L"Start",L"Size",g_cfg.start_size);         P(L"Start",L"Round",g_cfg.start_round);
    P(L"Start",L"ShowRecent",g_cfg.start_show_recent);
    P(L"Start",L"ShowPinned",g_cfg.start_show_pinned);
    P(L"Start",L"TileSize",g_cfg.tile_size);
    P(L"Start",L"TileColorMode",g_cfg.tile_color_mode);
    P(L"Start",L"TileTransparency",g_cfg.tile_transparency);
    P(L"Skin",L"Enabled",g_cfg.skin_enabled);     P(L"Skin",L"Theme",g_cfg.skin_theme);
    P(L"Skin",L"ColorMode",g_cfg.skin_color_mode);
    P(L"Skin",L"Border",g_cfg.skin_border);       P(L"Skin",L"Scrollbars",g_cfg.skin_scrollbars);
    P(L"Skin",L"SysColors",g_cfg.skin_syscolors); P(L"Skin",L"Owned",g_cfg.skin_owned);
    P(L"Skin",L"Opacity",g_cfg.skin_opacity);
    P(L"Personalization",L"Theme",g_cfg.theme);   P(L"Personalization",L"Dark",g_cfg.dark_mode);
    P(L"Personalization",L"Glass",g_cfg.glass);   P(L"Personalization",L"Blur",g_cfg.glass_blur);
    P(L"Personalization",L"Opacity",g_cfg.glass_opacity);
    P(L"Personalization",L"Shine",g_cfg.glass_shine);
    P(L"Personalization",L"Gloss",g_cfg.glass_gloss);
    P(L"Personalization",L"Shadow",g_cfg.shadow);
    P(L"Personalization",L"RoundCorners",g_cfg.round_corners);
    P(L"Personalization",L"Accent",g_cfg.accent);
    P(L"Personalization",L"IconPreset",g_cfg.icon_preset);
    P(L"Personalization",L"IconTint",g_cfg.icon_tint);
    P(L"Personalization",L"IconFill",g_cfg.icon_fill);
    P(L"Personalization",L"FontSize",g_cfg.font_size);
    P(L"Personalization",L"GlassBar",g_cfg.glass_bar);
    P(L"Personalization",L"GlassStart",g_cfg.glass_start);
    P(L"Personalization",L"GlassPreview",g_cfg.glass_preview);
    P(L"Personalization",L"GlassTitle",g_cfg.glass_title);
    PS(L"Personalization",L"Font",g_cfg.font_name);
    { wchar_t b[32]; swprintf(b, 32, L"%.2f", g_cfg.ui_scale); PS(L"Personalization",L"Scale",b); }
    P(L"Cursor",L"Enabled",g_cfg.cursor_enabled);
    PS(L"Cursor",L"Arrow",g_cfg.cursor_arrow);    PS(L"Cursor",L"Wait",g_cfg.cursor_wait);
    PS(L"Cursor",L"Ibeam",g_cfg.cursor_ibeam);    PS(L"Cursor",L"Link",g_cfg.cursor_link);
    PS(L"Cursor",L"Grab",g_cfg.cursor_grab);      PS(L"Cursor",L"Load",g_cfg.cursor_load);
    P(L"Keyboard",L"Auto",g_cfg.kbd_auto);        P(L"Keyboard",L"HeightPct",g_cfg.kbd_height_pct);
    P(L"Keyboard",L"Opacity",g_cfg.kbd_opacity);  P(L"Keyboard",L"Pan",g_cfg.kbd_pan);
    P(L"Desktop",L"Mode",g_cfg.desktop_mode);     P(L"Desktop",L"GridSize",g_cfg.grid_size);
    P(L"Desktop",L"Snap",g_cfg.grid_snap);        P(L"Desktop",L"ShowIcons",g_cfg.desktop_icons);
    P(L"Desktop",L"Lock",g_cfg.desktop_lock);     P(L"Desktop",L"Fit",g_cfg.wallpaper_fit);
    P(L"Desktop",L"Color",g_cfg.desktop_color);
    PS(L"Desktop",L"Wallpaper",g_cfg.wallpaper);
    PS(L"User",L"Name",g_cfg.user_name);          PS(L"User",L"Photo",g_cfg.user_photo);

    WIN32_FILE_ATTRIBUTE_DATA fa;
    if (GetFileAttributesExW(s_path, GetFileExInfoStandard, &fa))
        s_mtime = (int)fa.ftLastWriteTime.dwLowDateTime;
    return TRUE;
}

BOOL WsConfigChangedOnDisk(void) {
    WIN32_FILE_ATTRIBUTE_DATA fa;
    if (!GetFileAttributesExW(s_path, GetFileExInfoStandard, &fa)) return FALSE;
    return (int)fa.ftLastWriteTime.dwLowDateTime != s_mtime;
}
