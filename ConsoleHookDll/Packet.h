#ifndef PACKET_H
#define PACKET_H

#include <windows.h>

#define OP_ALLOC_CONSOLE 0
#define OP_WRITE_CONSOLE 1

// +----------+
// |  OPCODE  | 4 Bytes
// +----------+
// |  PKTLEN  | 4 Bytes
// +----------+
typedef struct AllocConsoleHeader
{
    int code;
    int pktLen;
} AllocConsoleHeader, CommonHeader;
BOOL PrepareAllocConsolePacket(char *pktBuf, int *pktLen);

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
typedef struct WriteConsoleHeader
{
    int code;
    int pktLen;
    int datLen;
} WriteConsoleHeader;
BOOL PrepareWriteConsolePacket(char *pktBuf, int *pktLen, const char *dataBuf, int dataLen);

BOOL ParseOpCode(const char *pkt, int *op);
BOOL ParsePacketLength(const char *pkt, int *pktLen);
BOOL ParseAllocConsolePacket(const char *pkt);
BOOL ParseWriteConsolePacket(const char *pkt, char *dataBuf, int *dataLen);

#endif
