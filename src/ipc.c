#include "winshell.h"

BOOL WsSendCommand(int cmd, const wchar_t* arg){
    HWND h = FindWindowW(WS_BAR_CLASS, NULL);
    if (!h) return FALSE;
    wchar_t buf[1024] = {0};
    if (arg) wcsncpy(buf, arg, 1023);
    COPYDATASTRUCT cds;
    cds.dwData = (ULONG_PTR)cmd;
    cds.cbData = (DWORD)((wcslen(buf)+1)*sizeof(wchar_t));
    cds.lpData = buf;
    SendMessageW(h, WM_COPYDATA, 0, (LPARAM)&cds);
    return TRUE;
}

void WsHandleCopyData(HWND h, COPYDATASTRUCT* cds){
    (void)h;
    if (!cds || !cds->lpData) return;
    wchar_t buf[1024] = {0};
    int n = cds->cbData/sizeof(wchar_t);
    if (n>1024) n=1024;
    memcpy(buf, cds->lpData, n*sizeof(wchar_t));
    switch ((int)cds->dwData){
    case WS_CMD_OPEN:  if (buf[0]) ShellExecuteW(NULL,L"open",buf,NULL,NULL,SW_SHOWNORMAL); break;
    case WS_CMD_SETUP: ShellExecuteW(NULL,L"open",L"winshell-setup.exe",NULL,NULL,SW_SHOWNORMAL); break;
    case WS_CMD_RELOAD: WsConfigLoad(); WsTaskbarApply(); break;
    case WS_CMD_QUIT: PostMessageW(WsTaskbarHwnd(), WM_CLOSE,0,0); break;
    }
}
