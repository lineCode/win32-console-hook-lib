#include <stdio.h>
#include <windows.h>
#include "../ConsoleHookDll/ConsoleHookDll.h"

PROCESS_INFORMATION pi;

void OnAllocConsole()
{
    OutputDebugString("OnAllocConsole\n");
}

void OnWriteConsole(char *buf, int len)
{
    OutputDebugString("OnWriteConsole\n");

    char info[1024];
    sprintf_s(info, 1024, "len = %d, data = %s", len, buf);
    OutputDebugString(info);
}

INT_PTR CALLBACK ProcDlgMain(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == WM_INITDIALOG)
    {
        // Register callbacks and start target app
        SetMode(LM_CLIENT, TM_WM_COPYDATA);
        RegisterAllocConsoleCallback(OnAllocConsole);
        RegisterWriteConsoleCallback(OnWriteConsole);
        StartTargetApp("TargetConsoleApp.exe", &pi, hWnd);

        // - Now the handle of the process is available at pi.hProcess.
        // - If you want to wait until the process exits, 
        //   do it in another thread or the WM_COPYDATA messages would be blocked.
        // - The handle of the created process and thread should be released
        //   in order to avoid memory leaks.

        // Display window
        ShowWindow(hWnd, SW_SHOW);
        return TRUE;
    }
    else if (uMsg == WM_COPYDATA)
    {
        // Should be called on each WM_COPYDATA message.
        // When callbacks have been registered, 
        // it would jump into the corresponding callbacks
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

///----------------------------------------------------------------------------------------------//
///                                    WinMain Entry                                             //
///----------------------------------------------------------------------------------------------//
int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR lpCmdLine, int nCmdShow)
{
    MSG stMsg;
    CreateDialogParam(hInstance, TEXT("DLG_MAIN"), NULL, ProcDlgMain, 0);

    while (GetMessage(&stMsg, NULL, 0, 0) != 0)
    {
        TranslateMessage(&stMsg);
        DispatchMessage(&stMsg);
    }

    return 0;
}