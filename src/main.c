#include "winshell.h"
#include <shellapi.h>

WsConfig g_cfg;
HINSTANCE g_hinst;
UINT WM_WS_RELOAD = 0;

static HANDLE g_mutex = NULL;

static int WINAPI WndProcNotify(HWND h, UINT m, WPARAM w, LPARAM l);

static void PumpSecondInstance(const wchar_t* arg, int cmd) {
    WsSendCommand(cmd, arg);
}

int WINAPI wWinMain(HINSTANCE hi, HINSTANCE hp, PWSTR cmdline, int show) {
    g_hinst = hi;
    (void)hp; (void)show;

    /* 1) single instance */
    g_mutex = CreateMutexW(NULL, TRUE, WS_MUTEX_NAME);
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        /* forward argv to the running taskbar */
        int argc = 0;
        LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
        for (int i = 1; i < argc; i++) {
            if (!wcscmp(argv[i], L"--open") && i + 1 < argc)
                PumpSecondInstance(argv[++i], WS_CMD_OPEN);
            else if (!wcscmp(argv[i], L"--setup"))
                PumpSecondInstance(L"", WS_CMD_SETUP);
        }
        LocalFree(argv);
        if (g_mutex) CloseHandle(g_mutex);
        return 0;
    }

    /* 2) DPI + common controls + app mutex path */
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    INITCOMMONCONTROLSEX icc = { sizeof(icc), ICC_WIN95_CLASSES | ICC_BAR_CLASSES };
    InitCommonControlsEx(&icc);

    WM_WS_RELOAD = RegisterWindowMessageW(L"WinShell.Reload");

    /* 3) load config (creates it with defaults if missing) */
    WsConfigLoad();

    /* 4) build the shell */
    if (!WsTaskbarCreate()) {
        MessageBoxW(NULL, L"Failed to create taskbar.", WS_APP_NAME, MB_ICONERROR);
        return 1;
    }
    WsSkinInit();

    /* 5) honor --open / --setup on first launch */
    {
        int argc = 0;
        LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
        for (int i = 1; i < argc; i++) {
            if (!wcscmp(argv[i], L"--open") && i + 1 < argc)
                WsSendCommand(WS_CMD_OPEN, argv[++i]);
            else if (!wcscmp(argv[i], L"--setup"))
                WsSendCommand(WS_CMD_SETUP, L"");
        }
        LocalFree(argv);
    }

    /* 6) message pump. Filter in dispatcher is unnecessary; each window handles what it needs. */
    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0) > 0) {
        if (msg.message == WM_WS_RELOAD) { WsTaskbarApply(); continue; }
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    WsSkinShutdown();
    if (g_mutex) { ReleaseMutex(g_mutex); CloseHandle(g_mutex); }
    return (int)msg.wParam;
}
