#ifndef CONSOLE_HOOK_DLL_H
#define CONSOLE_HOOK_DLL_H

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

// The target app and the client app share a same dll
// +--------------------+               +------------------------+
// | Target Console App |               | Client Console/GUI App |
// |  +--------------+  |  WM_COPYDATA  |     +------------+     |
// |  | Injected Dll |  | ------------> |     | Loaded Dll |     |
// |  +--------------+  |               |     +------------+     |
// +--------------------+               +------------------------+
//        Server                                   Client
typedef struct ConsoleBuffer
{
    int rows;
    int cols;
    char *data;
} ConsoleBuffer;

enum LibraryMode
{
    LM_CLIENT, LM_SERVER
};

enum TransmissionMode
{
    TM_WM_COPYDATA
};

typedef void (*AllocConsoleCallback)();
typedef void (*WriteConsoleCallback)(char *buf, int len);

void SetMode(enum LibraryMode libraryMode, enum TransmissionMode transMode);
void RegisterAllocConsoleCallback(AllocConsoleCallback cb);
void RegisterWriteConsoleCallback(WriteConsoleCallback cb);
void GetConsoleBuffer(ConsoleBuffer *buf);
void ReleaseConsoleBuffer(ConsoleBuffer *buf);

BOOL StartTargetApp(char *commandline, PROCESS_INFORMATION *ppi, HWND hClient);
BOOL ProcessMessage(WPARAM wParam, LPARAM lParam);
BOOL WaitEvent(BOOL *timedout);

#ifdef __cplusplus
}
#endif
#endif
