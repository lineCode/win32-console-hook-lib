#include <stdio.h>
#include "../ConsoleHookDll/ConsoleHookDll.h"

HWND hWindow;
char szCommandline[MAX_PATH];
PROCESS_INFORMATION pi;
DWORD dwLastMessage = 0;
bool connected = false;

void GetFilePathInCurrentDir(char *buf, int len, const char *szFileName)
{
    char szCurrentExe[MAX_PATH];
    char szCurrentDir[MAX_PATH];
    char *pFilePart;

    // Get full path name of CoinTracker.exe
    GetModuleFileName(0, szCurrentExe, MAX_PATH);

    // Get base directory
    GetFullPathName(szCurrentExe, MAX_PATH, szCurrentDir, &pFilePart);
    *pFilePart = '\0';

    // Get full path name of OKCOIN.exe
    strcpy_s(buf, len, szCurrentDir);
    strcat_s(buf, len, szFileName);
}

void OnWriteConsole(char *buf, int len)
{
    // Plink uses fputc to generate output (to stderr)
    static char pattern[6];
    pattern[0] = pattern[1];
    pattern[1] = pattern[2];
    pattern[2] = pattern[3];
    pattern[3] = pattern[4];
    pattern[4] = *buf;
    pattern[5] = '\0';

    if (strcmp(pattern, "Using") == 0)
    {
        connected = true;
        OutputDebugString("Connection established\n");
    }
}

INT_PTR CALLBACK ProcDlgMain(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == WM_INITDIALOG)
    {
        // Register callbacks and start target app
        SetMode(LM_CLIENT, TM_WM_COPYDATA);
        RegisterWriteConsoleCallback(OnWriteConsole);
        StartTargetApp(szCommandline, &pi, hWnd);

        // Set timer
        SetTimer(hWnd, 0, 1000, NULL);
        dwLastMessage = GetTickCount();

        return TRUE;
    }
    else if (uMsg == WM_TIMER)
    {
        if (connected)
        {
            DWORD dwStatus;
            GetExitCodeProcess(pi.hProcess, &dwStatus);
            if (dwStatus != STILL_ACTIVE)
            {
                connected = false;
                OutputDebugString("Connection lost\n");

                // Start again
                SendMessage(hWindow, WM_INITDIALOG, 0, 0);
            }
        }
        else
        {
            OutputDebugString("Tick\n");
            if (GetTickCount() > dwLastMessage + 3000)
            {
                OutputDebugString("Restart\n");
                TerminateProcess(pi.hProcess, 0);

                // Start again
                SendMessage(hWindow, WM_INITDIALOG, 0, 0);
            }
        }
        return TRUE;
    }
    else if (uMsg == WM_COPYDATA)
    {
        dwLastMessage = GetTickCount();
        ProcessMessage(wParam, lParam);
        return TRUE;
    }
    else if (uMsg == WM_CLOSE)
    {
        DestroyWindow(hWnd);
        PostQuitMessage(0);
    }
    
    return FALSE;
}

int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR lpCmdLine, int nCmdShow)
{
    MSG stMsg;

    // Create commandline
    char szPlinkPath[MAX_PATH];
    GetFilePathInCurrentDir(szPlinkPath, MAX_PATH, "plink.exe");
    sprintf_s(szCommandline, MAX_PATH, "\"%s\" %s", szPlinkPath, lpCmdLine);

    // Create dialog
    hWindow = CreateDialogParam(hInstance, TEXT("DLG_MAIN"), NULL, ProcDlgMain, 0);

    while (GetMessage(&stMsg, NULL, 0, 0) != 0)
    {
        TranslateMessage(&stMsg);
        DispatchMessage(&stMsg);
    }

    return 0;
}