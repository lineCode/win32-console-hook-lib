#include "Packet.h"

// +----------+
// |  OPCODE  | 4 Bytes
// +----------+
// |  PKTLEN  | 4 Bytes
// +----------+
BOOL PrepareAllocConsolePacket(char *pktBuf, int *pktLen)
{
    if (*pktLen < sizeof(AllocConsoleHeader))
    {
        *pktLen = sizeof(AllocConsoleHeader);
        return FALSE;
    }
    ((AllocConsoleHeader *)pktBuf)->code = OP_ALLOC_CONSOLE;
    ((AllocConsoleHeader *)pktBuf)->pktLen = sizeof(AllocConsoleHeader);
    *pktLen = sizeof(AllocConsoleHeader);
    return TRUE;
}

// +----------+
// |  OPCODE  | 4 bytes
// +----------+
// |  PKTLEN  | 4 Bytes
// +----------+
// |  DATLEN  | 4 bytes
// +----------+
// |          |
// |   DATA   | DATLEN bytes (last one is \NUL)
// |          |
// +----------+
BOOL PrepareWriteConsolePacket(char *pktBuf, int *pktLen, const char *dataBuf, int dataLen)
{
    if (*pktLen < sizeof(WriteConsoleHeader) + dataLen + 1)
    {
        *pktLen = sizeof(WriteConsoleHeader) + dataLen + 1;
        return FALSE;
    }
    ((WriteConsoleHeader *)pktBuf)->code = OP_WRITE_CONSOLE;
    ((WriteConsoleHeader *)pktBuf)->pktLen = sizeof(WriteConsoleHeader) + dataLen + 1;
    ((WriteConsoleHeader *)pktBuf)->datLen = dataLen + 1;
    strncpy_s(pktBuf + sizeof(WriteConsoleHeader), dataLen + 1, dataBuf, dataLen);
    *pktLen = sizeof(WriteConsoleHeader) + dataLen + 1;
    return TRUE;
}

BOOL ParseOpCode(const char *pkt, int *op)
{
    *op = ((CommonHeader *)pkt)->code;
    return TRUE;
}

BOOL ParsePacketLength(const char *pkt, int *pktLen)
{
    *pktLen = ((CommonHeader *)pkt)->code;
    return TRUE;
}

BOOL ParseAllocConsolePacket(const char *pkt)
{
    return TRUE;
}

BOOL ParseWriteConsolePacket(const char *pkt, char *dataBuf, int *dataLen)
{
    int len = ((WriteConsoleHeader *)pkt)->datLen;
    if (*dataLen < len)
    {
        return FALSE;
    }
    strcpy_s(dataBuf, *dataLen, pkt + sizeof(WriteConsoleHeader));
    *dataLen = len - 1;
    return TRUE;
}
