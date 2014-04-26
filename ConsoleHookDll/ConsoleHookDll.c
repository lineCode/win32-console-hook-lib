#include <stdio.h>
#include "ConsoleHookDll.h"
#include "Packet.h"
#include "Utils.h"
#include "detours.h"

enum LibraryMode enumLibraryMode = LM_CLIENT;
enum TransmissionMode enumTransMode = TM_WM_COPYDATA;
HWND hClient;

// Callbacks
AllocConsoleCallback cbAllocConsole = NULL;
WriteConsoleCallback cbWriteConsole = NULL;

// Detours Hook ///////////////////////////////////////////////////////////////////////////////////
BOOL WINAPI CopyAllocConsole();
BOOL WINAPI CopyWriteConsole(
    HANDLE hConsoleOutput, 
    const VOID *lpBuffer, 
    DWORD nNumberOfCharsToWrite, 
    LPDWORD lpNumberOfCharsWritten, 
    LPVOID lpReserved
);

DETOUR_TRAMPOLINE(
    BOOL WINAPI CopyAllocConsole(), 
    AllocConsole
);
DETOUR_TRAMPOLINE(
    BOOL WINAPI CopyWriteConsole(
        HANDLE hConsoleOutput, 
        const VOID *lpBuffer, 
        DWORD nNumberOfCharsToWrite, 
        LPDWORD lpNumberOfCharsWritten, 
        LPVOID lpReserved), 
    WriteConsole
);

BOOL WINAPI MyAllocConsole()
{
    BOOL ret = CopyAllocConsole();
    char *buf = malloc(sizeof(int));
    int len = sizeof(int);
    PrepareAllocConsolePacket(buf, &len);

    if (enumTransMode ==TM_WM_COPYDATA)
    {
        COPYDATASTRUCT cds;
        cds.dwData = 0;
        cds.lpData = buf;
        cds.cbData = len;
        SendMessage(hClient, WM_COPYDATA, 0, (LPARAM)&cds);
    }
    free(buf);
    return ret;
}

BOOL WINAPI MyWriteConsole(HANDLE hConsoleOutput, const VOID *lpBuffer, DWORD nNumberOfCharsToWrite, LPDWORD lpNumberOfCharsWritten, LPVOID lpReserved)
{
    BOOL ret = CopyWriteConsole(
        hConsoleOutput, lpBuffer, nNumberOfCharsToWrite, lpNumberOfCharsWritten, lpReserved);
    char *buf = malloc(1024);
    int len = 1024;
    if (PrepareWriteConsolePacket(buf, &len, lpBuffer, nNumberOfCharsToWrite) == FALSE)
    {
        buf = realloc(buf, len);
        PrepareWriteConsolePacket(buf, &len, lpBuffer, nNumberOfCharsToWrite);
    }

    if (enumTransMode == TM_WM_COPYDATA)
    {
        COPYDATASTRUCT cds;
        cds.dwData = 0;
        cds.lpData = buf;
        cds.cbData = len;
        SendMessage(hClient, WM_COPYDATA, 0, (LPARAM)&cds);
    }
    free(buf);
    return ret;
}

// API ////////////////////////////////////////////////////////////////////////////////////////////
void SetMode(enum LibraryMode libraryMode, enum TransmissionMode transMode)
{
    enumLibraryMode = libraryMode;
    enumTransMode = transMode;
}

void RegisterAllocConsoleCallback(AllocConsoleCallback cb)
{
    cbAllocConsole = cb;
}
void RegisterWriteConsoleCallback(WriteConsoleCallback cb)
{
    cbWriteConsole = cb;
}

void GetConsoleBuffer(ConsoleBuffer *buf)
{
    // GetStdHandle, GetConsoleScreenBufferInfo & ReadConsoleOutput are used in this function
    // Be sure they're not hooked!
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO info;
    CHAR_INFO *buffer;
    COORD bufferSize;
    COORD bufferCoord = { 0, 0 };
    SMALL_RECT region;
    int i;

    // Get buffer info
    GetConsoleScreenBufferInfo(hConsole, &info);
    buf->rows = info.dwSize.Y;
    buf->cols = info.dwSize.X;
    buf->data = (char *)malloc(buf->rows * buf->cols);

    // Read buffer
    buffer = (CHAR_INFO *)malloc(buf->rows * buf->cols * sizeof(CHAR_INFO));
    bufferSize.X = buf->cols;
    bufferSize.Y = buf->rows;

    region.Left = 0;
    region.Top = 0;
    region.Right = buf->cols - 1;
    region.Bottom = buf->rows - 1;

    ReadConsoleOutput(hConsole, buffer, bufferSize, bufferCoord, &region);

    // Copy data to the ConsoleBuffer object
    for (i = 0; i < buf->rows * buf->cols; i++)
    {
        buf->data[i] = buffer[i].Char.AsciiChar;
    }

    // Cleanup
    free(buffer);
}

void ReleaseConsoleBuffer(ConsoleBuffer *buf)
{
    if (buf->data != NULL)
        free(buf->data);
}

BOOL StartTargetApp(char *commandline, PROCESS_INFORMATION *ppi, HWND hClient)
{
    // Prepare structures
    STARTUPINFO si = { 0 };
    PROCESS_INFORMATION pi;
    char dllPath[MAX_PATH];
    char clientHandle[16];
    char *environmentStrings;
    BOOL success;

	si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;

    // Prepare DLL path
    GetFilePathInCurrentDir(dllPath, MAX_PATH, "ConsoleHookDll.dll");

    // Prepare environment variable
    sprintf_s(clientHandle, 16, "%d", (int)hClient);
    SetEnvironmentVariable("CH_CLIENT_HWND", clientHandle);
    environmentStrings = GetEnvironmentStrings();

	// Start app
    success = DetourCreateProcessWithDll(
        NULL, commandline, NULL, NULL, TRUE, CREATE_DEFAULT_ERROR_MODE, 
        environmentStrings, NULL, &si, &pi, dllPath, NULL);
    if (ppi != NULL)
    {
        *ppi = pi;
    }
    return success;
}

BOOL ParsePacket(const char *pkt)
{
    int code = -1;
    ParseOpCode(pkt, &code);

    if (code == OP_ALLOC_CONSOLE)
    {
        if (cbAllocConsole != NULL)
            cbAllocConsole();
    }
    else if (code == OP_WRITE_CONSOLE)
    {
        char *buf = (char *)malloc(1024);
        int len = 1024;
        if (ParseWriteConsolePacket(pkt, buf, &len) == FALSE)
        {
            buf = realloc(buf, len);
            ParseWriteConsolePacket(pkt, buf, &len);
        }

        if (cbWriteConsole != NULL)
            cbWriteConsole(buf, len);
    }
    else
    {
        return FALSE;
    }
    return TRUE;
}

BOOL ProcessMessage(WPARAM wParam, LPARAM lParam)
{
    COPYDATASTRUCT *cds = (COPYDATASTRUCT *)lParam;
    // DumpCopyData(cds);
    return ParsePacket(cds->lpData);
}

// The Dll Main Entry /////////////////////////////////////////////////////////////////////////////
BOOL WINAPI DllMain(HINSTANCE hInst, DWORD dwReason, LPVOID reserved)
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        // Read environment variables
        char clientHandle[16];
        if (GetEnvironmentVariable("CH_CLIENT_HWND", clientHandle, 16) > 0)
        {
            enumLibraryMode = LM_SERVER;
            enumTransMode = TM_WM_COPYDATA;
            hClient = (HWND)atoi(clientHandle);
        }

        DetourFunctionWithTrampoline((PBYTE)CopyAllocConsole, (PBYTE)MyAllocConsole);
        DetourFunctionWithTrampoline((PBYTE)CopyWriteConsole, (PBYTE)MyWriteConsole);
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
        DetourRemove((PBYTE)CopyAllocConsole, (PBYTE)MyAllocConsole);
        DetourRemove((PBYTE)CopyWriteConsole, (PBYTE)MyWriteConsole);
    }
    return TRUE;
}
