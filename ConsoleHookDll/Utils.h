#ifndef UTILS_H
#define UTILS_H

#include <windows.h>

void GetFilePathInCurrentDir(char *buf, int len, const char *szFileName);
void DebugPrint(const char *format, ...);
void DumpCopyData(COPYDATASTRUCT *cds);

#endif