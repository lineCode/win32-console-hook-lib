#include <stdio.h>
#include "Utils.h"

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

void DebugPrint(const char *format, ...)
{
    char buf[1024];
    va_list argList;
    va_start(argList, format);

    vsprintf_s(buf, _countof(buf), format, argList);
    OutputDebugString(buf);

    va_end(argList);
}

void DumpCopyData(COPYDATASTRUCT *cds)
{
    char buf[256] = { 0 };
    char tmp[16];
    unsigned int i;
    unsigned char byte;

    DebugPrint("dwData = %d\n", cds->dwData);
    DebugPrint("cbData = %d\n", cds->cbData);

    for (i = 0; i < cds->cbData; i++)
    {
        if (i % 16 == 0)
        {
            DebugPrint("%s\n", buf);
            buf[0] = '\0';
        }

        byte = *((unsigned char *)cds->lpData + i);
        sprintf_s(tmp, 16, "%02x ", (int)byte);
        strcat_s(buf, 256, tmp);
    }
}
