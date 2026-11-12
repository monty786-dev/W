#include "winshell.h"

typedef struct { HWND hwnd; wchar_t exe[MAX_PATH]; wchar_t title[128]; HICON icon; int active; } Entry;
static Entry s_entries[64]; static int s_entry_count;
static HWND  s_bar;
static int   s_hover = -1;
static int   s_revealed_until = 0;
static int   s_hidden = 0;
static HFONT s_font_ui, s_font_ui_big;

#define TIMER_TICK  1
#define TIMER_FAST  2

static void ScanEntries(void){
    /* Enumerate top-level app windows, group by process exe. */
    s_entry_count = 0;
    EnumWindows((WNDENUMPROC)[](HWND h, LPARAM lp){
        if (h==s_bar) return TRUE;
        if (!IsWindowVisible(h)) return TRUE;
        LONG ex = GetWindowLongW(h,GWL_EXSTYLE);
        if ((ex & WS_EX_TOOLWINDOW) && !(ex & WS_EX_APPWINDOW)) return TRUE;
        if (GetWindow(h,GW_OWNER) && !(ex & WS_EX_APPWINDOW)) return TRUE;
        wchar_t title[128]={0}; GetWindowTextW(h,title,128);
        if (!title[0]) return TRUE;
        wchar_t cls[64]={0}; GetClassNameW(h,cls,64);
        if (!wcscmp(cls,L"WinShellTaskbar")||!wcscmp(cls,L"WinShellStart")||
            !wcscmp(cls,L"WinShellSkin")||!wcscmp(cls,L"Progman")||!wcscmp(cls,L"Shell_TrayWnd"))
            return TRUE;
        wchar_t exe[MAX_PATH]={0}; DWORD pid=0;
        GetWindowThreadProcessId(h,&pid);
        HANDLE ph = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION,FALSE,pid);
        if (ph){ DWORD cch=MAX_PATH; QueryFullProcessImageNameW(ph,0,exe,&cch); CloseHandle(ph);}
        int found=-1;
        for (int i=0;i<s_entry_count;i++) if (!_wcsicmp(s_entries[i].exe,exe)){found=i;break;}
        if (found<0 && s_entry_count<64){ found=s_entry_count++; ZeroMemory(&s_entries[found],sizeof(Entry));
            wcscpy(s_entries[found].exe,exe); }
        if (found<0) return TRUE;
        Entry* e=&s_entries[found];
        if (!e->hwnd){ e->hwnd=h; e->icon=(HICON)SendMessageTimeoutW(h,WM_GETICON,ICON_SMALL2,0,SMTO_ABORTIFHUNG,80,NULL);
                       if(!e->icon) e->icon=(HICON)GetClassLongPtrW(h,GCLP_HICONSM);
                       if(!e->icon) e->icon=LoadIconW(NULL,IDI_APPLICATION); }
        wcscpy(e->title,title);
        HWND fg=GetForegroundWindow();
        if (fg==h||GetAncestor(fg,GA_ROOTOWNER)==h) e->active=1;
        return TRUE;
    },0);
    if (s_entry_count>0) s_entries[0].active=s_entries[0].active; /* keep */
}

static void BarPaint(HWND h, HDC hdc){
    RECT rc; GetClientRect(h,&rc);
    const WsTheme* th = WsThemeAt(g_cfg.theme);
    COLORREF base = (g_cfg.taskbar_style_color&0xFFFFFF) ? (COLORREF)g_cfg.taskbar_style_color : th->bar;

    if (g_cfg.glass){
        WsGlassBitmap(h, hdc, rc.right, rc.bottom, 0, 0, base, TRUE);
    } else {
        HBRUSH b = CreateSolidBrush(base); FillRect(hdc,&rc,b); DeleteObject(b);
    }

    HFONT old = SelectObject(hdc, s_font_ui);
    SetBkMode(hdc, TRANSPARENT);

    /* Start button: draw the 4-tile logo */
    int H = rc.bottom;
    int start_w = H + (H/5);
    RECT rs = {0,0,start_w,H};
    if (s_hover==-100){ HBRUSH b=CreateSolidBrush(WsMix(base,th->text,0.16f));
                        FillRect(hdc,&rs,b); DeleteObject(b);}
    COLORREF lc = (g_cfg.start_icon_color&0xFFFFFF) ? (COLORREF)g_cfg.start_icon_color : th->text;
    int tile = H/4, gap = H/24;
    int bx = (start_w - 2*tile - gap)/2, by = (H - 2*tile - gap)/2;
    HBRUSH tb = CreateSolidBrush(lc);
    RECT t1={bx,by,bx+tile,by+tile};
    RECT t2={bx+tile+gap,by,bx+2*tile+gap,by+tile};
    RECT t3={bx,by+tile+gap,bx+tile,by+2*tile+gap};
    RECT t4={bx+tile+gap,by+tile+gap,bx+2*tile+gap,by+2*tile+gap};
    FillRect(hdc,&t1,tb); FillRect(hdc,&t2,tb); FillRect(hdc,&t3,tb); FillRect(hdc,&t4,tb);
    DeleteObject(tb);

    /* Entries */
    int n = s_entry_count;
    int isz = (int)(g_cfg.icon_size * g_cfg.ui_scale);
    int btn_w = g_cfg.show_titles ? (isz + 160) : (isz + 16);
    int align_x = start_w + 8;
    if (g_cfg.bar_alignment==1) align_x = (rc.right - n*btn_w)/2;
    else if (g_cfg.bar_alignment==2) align_x = rc.right - n*btn_w - 8;
    for (int i=0;i<n;i++){
        RECT rr = { align_x+i*btn_w, 2, align_x+(i+1)*btn_w-2, H-2 };
        if (s_hover==i || s_entries[i].active){
            HBRUSH b=CreateSolidBrush(WsMix(base, th->text, s_entries[i].active?0.30f:0.16f));
            FillRect(hdc,&rr,b); DeleteObject(b);
        }
        int ix = rr.left + (g_cfg.show_titles ? 6 : (btn_w-isz)/2);
        int iy = (H-isz)/2;
        if (s_entries[i].icon) DrawIconEx(hdc,ix,iy,s_entries[i].icon,isz,isz,0,NULL,DI_NORMAL);
        if (g_cfg.show_titles){
            RECT tr = { rr.left+isz+12, 0, rr.right-4, H };
            SetTextColor(hdc, th->text);
            DrawTextW(hdc,s_entries[i].title,-1,&tr,DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS|DT_NOPREFIX);
        }
        /* underline */
        if (g_cfg.underline_height>0){
            int ux1 = rr.left + btn_w/4, ux2 = rr.right - btn_w/4;
            int uy0 = H - g_cfg.underline_height;
            COLORREF uc = (g_cfg.underline_from_icon)?WsAccent():WsAccent();
            HBRUSH ub = CreateSolidBrush(uc);
            RECT ur={ux1,uy0,ux2,H};
            FillRect(hdc,&ur,ub); DeleteObject(ub);
        }
    }

    /* Clock */
    wchar_t clock[64];
    time_t t = time(NULL); struct tm tm; localtime_s(&tm, &t);
    SYSTEMTIME st; GetLocalTime(&st);
    swprintf(clock, 64, L"%d:%02d %s\n%02d-%02d-%04d",
             st.wHour%12?st.wHour%12:12, st.wMinute, st.wHour<12?L"AM":L"PM",
             st.wDay, st.wMonth, st.wYear);
    RECT cr = { rc.right - 110, 0, rc.right - 8, H };
    SetTextColor(hdc, th->text);
    DrawTextW(hdc, clock, -1, &cr, DT_RIGHT|DT_VCENTER|DT_NOPREFIX);

    SelectObject(hdc, old);
}

static LRESULT CALLBACK BarProc(HWND h, UINT m, WPARAM w, LPARAM l){
    switch (m){
    case WM_PAINT: {
        PAINTSTRUCT ps; HDC hdc=BeginPaint(h,&ps);
        HDC mdc=CreateCompatibleDC(hdc);
        RECT rc; GetClientRect(h,&rc);
        HBITMAP bmp=CreateCompatibleBitmap(hdc,rc.right,rc.bottom);
        HBITMAP old=SelectObject(mdc,bmp);
        BarPaint(h,mdc);
        BitBlt(hdc,0,0,rc.right,rc.bottom,mdc,0,0,SRCCOPY);
        SelectObject(mdc,old); DeleteObject(bmp); DeleteDC(mdc);
        EndPaint(h,&ps); return 0;
    }
    case WM_ERASEBKGND: return 1;
    case WM_MOUSEACTIVATE: return MA_NOACTIVATE;
    case WM_TIMER:
        if (w==TIMER_TICK){ ScanEntries(); InvalidateRect(h,NULL,FALSE); }
        else if (w==TIMER_FAST){
            WsSkinTick();
            if (g_cfg.taskbar_always_front && !s_hidden) WsTaskbarForceFront(FALSE);
        }
        return 0;
    case WM_MOUSEMOVE:{
        int x=GET_X_LPARAM(l), y=GET_Y_LPARAM(l);
        int H = 40; /* recompute */
        RECT rc; GetClientRect(h,&rc);
        int start_w = rc.bottom + rc.bottom/5;
        int isz=(int)(g_cfg.icon_size*g_cfg.ui_scale);
        int btn_w=g_cfg.show_titles?(isz+160):(isz+16);
        int align_x = start_w+8;
        if (g_cfg.bar_alignment==1) align_x=(rc.right-s_entry_count*btn_w)/2;
        else if (g_cfg.bar_alignment==2) align_x=rc.right-s_entry_count*btn_w-8;
        int hv=-1;
        if (x<start_w) hv=-100;
        else for (int i=0;i<s_entry_count;i++){
            if (x>=align_x+i*btn_w && x<align_x+(i+1)*btn_w){ hv=i; break; }
        }
        if (hv!=s_hover){ s_hover=hv; InvalidateRect(h,NULL,FALSE);} return 0;
    }
    case WM_LBUTTONUP:{
        int x=GET_X_LPARAM(l), y=GET_Y_LPARAM(l);
        RECT rc; GetClientRect(h,&rc);
        int start_w=rc.bottom+rc.bottom/5;
        if (x<start_w){ WsStartToggle(); return 0;}
        int isz=(int)(g_cfg.icon_size*g_cfg.ui_scale);
        int btn_w=g_cfg.show_titles?(isz+160):(isz+16);
        int align_x=start_w+8;
        if (g_cfg.bar_alignment==1) align_x=(rc.right-s_entry_count*btn_w)/2;
        else if (g_cfg.bar_alignment==2) align_x=rc.right-s_entry_count*btn_w-8;
        for (int i=0;i<s_entry_count;i++){
            if (x>=align_x+i*btn_w && x<align_x+(i+1)*btn_w){
                HWND wh = s_entries[i].hwnd;
                if (IsIconic(wh)) { ShowWindow(wh,SW_RESTORE); SetForegroundWindow(wh);}
                else if (GetForegroundWindow()==wh) ShowWindow(wh,SW_MINIMIZE);
                else SetForegroundWindow(wh);
                return 0;
            }
        }
        return 0;
    }
    case WM_RBUTTONUP:{
        HMENU menu = CreatePopupMenu();
        AppendMenuW(menu,MF_STRING,1,L"Settings");
        AppendMenuW(menu,MF_STRING,2,L"Reload settings");
        AppendMenuW(menu,MF_STRING,3,L"Show desktop");
        AppendMenuW(menu,MF_SEPARATOR,0,NULL);
        AppendMenuW(menu,MF_STRING,4,L"Exit taskbar");
        POINT pt; GetCursorPos(&pt);
        int cmd = (int)TrackPopupMenu(menu,TPM_RETURNCMD|TPM_RIGHTBUTTON,pt.x,pt.y,0,h,NULL);
        DestroyMenu(menu);
        if (cmd==1) ShellExecuteW(NULL,L"open",L"winshell-setup.exe",L"--settings",NULL,SW_SHOWNORMAL);
        else if (cmd==2){ WsConfigLoad(); WsTaskbarApply(); }
        else if (cmd==3){ /* show desktop */ HWND d=GetDesktopWindow(); PostMessageW(d,WM_COMMAND,0x7402,0);} /* Win+D */
        else if (cmd==4){ PostMessageW(h,WM_CLOSE,0,0); }
        return 0;
    }
    case WM_COPYDATA:
        WsHandleCopyData(h,(COPYDATASTRUCT*)l); return 1;
    case WM_DESTROY:
        KillTimer(h,TIMER_TICK); KillTimer(h,TIMER_FAST);
        PostQuitMessage(0); return 0;
    }
    return DefWindowProcW(h,m,w,l);
}

BOOL WsTaskbarCreate(void){
    WNDCLASSEXW wc = { sizeof(wc) };
    wc.style = CS_DBLCLKS;
    wc.lpfnWndProc = BarProc;
    wc.hInstance = g_hinst;
    wc.hCursor = LoadCursorW(NULL,IDC_ARROW);
    wc.lpszClassName = WS_BAR_CLASS;
    RegisterClassExW(&wc);
    WsInvalidateFonts();

    RECT r = WsTaskbarRect();
    s_bar = CreateWindowExW(WS_EX_TOPMOST|WS_EX_TOOLWINDOW|WS_EX_NOACTIVATE,
        WS_BAR_CLASS, L"WinShell", WS_POPUP|WS_VISIBLE,
        r.left, r.top, r.right-r.left, r.bottom-r.top, NULL,NULL,g_hinst,NULL);
    if (!s_bar) return FALSE;

    WsSetLayered(s_bar, g_cfg.glass, g_cfg.glass_opacity);
    SetTimer(s_bar, TIMER_TICK, 1000, NULL);
    SetTimer(s_bar, TIMER_FAST, 40, NULL);
    WsTaskbarApply();
    return TRUE;
}

void WsTaskbarApply(void){
    if (!s_bar) return;
    RECT r = WsTaskbarRect();
    SetWindowPos(s_bar, HWND_TOPMOST, r.left,r.top,r.right-r.left,r.bottom-r.top,
                 SWP_NOACTIVATE);
    WsSetLayered(s_bar, g_cfg.glass, g_cfg.glass_opacity);
    /* reserve work area */
    RECT wa = WsWorkRect();
    SystemParametersInfoW(SPI_SETWORKAREA,0,&wa,SPIF_SENDCHANGE);
    /* hide Wine's own taskbar */
    HWND wt = FindWindowW(L"Shell_TrayWnd",NULL);
    if (wt) ShowWindow(wt, g_cfg.hide_wine_bar?SW_HIDE:SW_SHOW);
    WsInvalidateFonts();
    InvalidateRect(s_bar,NULL,TRUE);
    WsSkinInvalidate();
}

RECT WsTaskbarRect(void){
    int sw = GetSystemMetrics(SM_CXSCREEN), sh = GetSystemMetrics(SM_CYSCREEN);
    int h = (int)(g_cfg.bar_height*g_cfg.ui_scale);
    RECT r;
    switch (g_cfg.bar_position){
    case 1: r.left=0; r.top=0; r.right=sw; r.bottom=h; break;
    case 2: r.left=0; r.top=0; r.right=h; r.bottom=sh; break;
    case 3: r.left=sw-h; r.top=0; r.right=sw; r.bottom=sh; break;
    default: r.left=0; r.top=sh-h; r.right=sw; r.bottom=sh; break;
    }
    return r;
}

RECT WsWorkRect(void){
    int sw = GetSystemMetrics(SM_CXSCREEN), sh = GetSystemMetrics(SM_CYSCREEN);
    int h = (int)(g_cfg.bar_height*g_cfg.ui_scale);
    RECT r = {0,0,sw,sh};
    switch (g_cfg.bar_position){
    case 1: r.top = h; break;
    case 2: r.left = h; break;
    case 3: r.right = sw-h; break;
    default: r.bottom = sh-h; break;
    }
    return r;
}

HWND WsTaskbarHwnd(void){ return s_bar; }
void WsTaskbarInvalidate(void){ if(s_bar) InvalidateRect(s_bar,NULL,FALSE); }
void WsTaskbarReveal(int s){ s_revealed_until = GetTickCount()+s*1000; if(s_hidden){s_hidden=0;ShowWindow(s_bar,SW_SHOWNOACTIVATE);} WsTaskbarForceFront(TRUE);}
void WsTaskbarForceFront(BOOL hard){
    if (!s_bar||s_hidden) return;
    UINT fl = SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE;
    if (hard) SetWindowPos(s_bar,HWND_NOTOPMOST,0,0,0,0,fl);
    SetWindowPos(s_bar,HWND_TOPMOST,0,0,0,0,fl);
}

void WsSetLayered(HWND h, BOOL on, int opacity){
    LONG ex = GetWindowLongW(h,GWL_EXSTYLE);
    if (on){ SetWindowLongW(h,GWL_EXSTYLE,ex|WS_EX_LAYERED);
             SetLayeredWindowAttributes(h,0,(BYTE)(255*opacity/100),LWA_ALPHA); }
    else if (ex&WS_EX_LAYERED) SetWindowLongW(h,GWL_EXSTYLE,ex&~WS_EX_LAYERED);
}

HFONT WsFont(int bump, int weight){
    int sz = (int)((g_cfg.font_size+bump)*g_cfg.ui_scale*96/72*-1);
    return CreateFontW(sz,0,0,0,weight,0,0,0,DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,0,g_cfg.font_name);
}
void WsInvalidateFonts(void){ /* fonts are created per-paint in the skeleton; production caches them */ }
