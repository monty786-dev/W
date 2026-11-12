#include "winshell.h"
#include <shlobj.h>

/* The setup.exe is a tiny self-contained Win32 executable that embeds
   winshell.exe and winshell-taskbar.exe as resources and deploys them into
   %LOCALAPPDATA%\Programs\WinShell, then writes the Wine registry keys that
   Winlator needs. That is all an installer has to do.

   Resource IDs:
     IDR_EXE_MAIN     — winshell.exe       (the full shell)
     IDR_EXE_LAUNCHER — winshell-start.exe (tiny: launches main after N sec)
     IDR_DOC_README   — README.txt
*/

static BOOL ExtractResource(int id, const wchar_t* dst);
static BOOL EnsureDirTree(const wchar_t* path);
static BOOL CopySelfToInstallDir(wchar_t* out_path);

static void Progress(const wchar_t* msg){
    /* simple owner-drawn install window; message box fallback for skeleton */
    (void)msg;
}

int WsInstallRun(void){
    wchar_t target[MAX_PATH];
    if (!CopySelfToInstallDir(target)){
        MessageBoxW(NULL,L"Could not copy installer files.",L"WinShell Setup",MB_ICONERROR);
        return 1;
    }

    /* 1. Extract winshell-taskbar.exe */
    wchar_t tb[MAX_PATH]; swprintf(tb, MAX_PATH, L"%s\\winshell-taskbar.exe", target);
    ExtractResource(101, tb);

    /* 2. Write the wine registry keys needed for Winlator stability/performance */
    /*    HKCU\Software\Wine\DirectSound  Emulation = "full"   (audio robustness)
     *    HKCU\Software\Wine\Direct3D     VideoMemorySize = "512"
     *    HKCU\Control Panel\Desktop      FontSmoothing = "2"
     *    HKCU\Software\Wine\WineDbg      ShowCrashDialog = "0"
     *    HKLM\Software\Microsoft\Windows NT\CurrentVersion\FontSubstitutes
     *        "Segoe UI" -> "Tahoma"
     *    HKCU\Software\Classes\Directory\shell\open\command -> "winshell-taskbar.exe --open %1"
     */
    HKEY k;
    if (RegCreateKeyExW(HKEY_CURRENT_USER,L"Software\\Wine\\DirectSound",0,NULL,0,KEY_SET_VALUE,NULL,&k,NULL)==0){
        const wchar_t* v = L"full";
        RegSetValueExW(k,L"Emulation",0,REG_SZ,(BYTE*)v,(DWORD)((wcslen(v)+1)*sizeof(wchar_t)));
        RegCloseKey(k);
    }
    if (RegCreateKeyExW(HKEY_CURRENT_USER,L"Software\\Wine\\WineDbg",0,NULL,0,KEY_SET_VALUE,NULL,&k,NULL)==0){
        DWORD zero=0; RegSetValueExW(k,L"ShowCrashDialog",0,REG_DWORD,(BYTE*)&zero,4); RegCloseKey(k);
    }
    if (RegCreateKeyExW(HKEY_LOCAL_MACHINE,L"Software\\Microsoft\\Windows NT\\CurrentVersion\\FontSubstitutes",
                        0,NULL,0,KEY_SET_VALUE,NULL,&k,NULL)==0){
        const wchar_t* v=L"Tahoma";
        RegSetValueExW(k,L"Segoe UI",0,REG_SZ,(BYTE*)v,(DWORD)((wcslen(v)+1)*sizeof(wchar_t)));
        RegCloseKey(k);
    }
    if (RegCreateKeyExW(HKEY_CURRENT_USER,L"Software\\Classes\\Directory\\shell\\open\\command",
                        0,NULL,0,KEY_SET_VALUE,NULL,&k,NULL)==0){
        wchar_t v[MAX_PATH+40]; swprintf(v,MAX_PATH+40,L"\"%s\" --open \"%%1\"",tb);
        RegSetValueExW(k,NULL,0,REG_SZ,(BYTE*)v,(DWORD)((wcslen(v)+1)*sizeof(wchar_t)));
        RegCloseKey(k);
    }

    /* 3. Autostart. We register the LAUNCHER (with N-second delay) in the
       Run key so it can start *after* wfm.exe has finished initializing. */
    wchar_t launcher[MAX_PATH]; swprintf(launcher,MAX_PATH,L"%s\\winshell-start.exe",target);
    ExtractResource(102, launcher);
    if (RegCreateKeyExW(HKEY_CURRENT_USER,
            L"Software\\Microsoft\\Windows\\CurrentVersion\\Run",0,NULL,0,KEY_SET_VALUE,NULL,&k,NULL)==0){
        wchar_t cmd[MAX_PATH+60];
        swprintf(cmd,MAX_PATH+60,L"\"%s\" --delay 3",launcher);
        RegSetValueExW(k,L"WinShell",0,REG_SZ,(BYTE*)cmd,(DWORD)((wcslen(cmd)+1)*sizeof(wchar_t)));
        RegCloseKey(k);
    }

    MessageBoxW(NULL,L"WinShell installed.\nRestart the container to see the new taskbar.",
                L"WinShell Setup",MB_ICONINFORMATION);
    return 0;
}

BOOL WsSetWineHideBar(BOOL hide){
    HKEY k;
    if (RegCreateKeyExW(HKEY_CURRENT_USER,L"Software\\Wine\\Explorer",0,NULL,0,KEY_SET_VALUE,NULL,&k,NULL)!=0)
        return FALSE;
    const wchar_t* v = hide?L"1":L"0";
    RegSetValueExW(k,L"EnableShell",0,REG_SZ,(BYTE*)v,(DWORD)((wcslen(v)+1)*sizeof(wchar_t)));
    RegCloseKey(k);
    return TRUE;
}

BOOL WsUninstall(void){
    HKEY k;
    if (RegOpenKeyExW(HKEY_CURRENT_USER,L"Software\\Microsoft\\Windows\\CurrentVersion\\Run",
                      0,KEY_SET_VALUE,&k)==0){
        RegDeleteValueW(k,L"WinShell"); RegCloseKey(k);
    }
    RegDeleteKeyW(HKEY_CURRENT_USER,L"Software\\Classes\\Directory\\shell\\open");
    return TRUE;
}

int WINAPI wWinMain(HINSTANCE hi, HINSTANCE hp, PWSTR cmd, int show){
    (void)hi;(void)hp;(void)cmd;(void)show;
    return WsInstallRun();
}
