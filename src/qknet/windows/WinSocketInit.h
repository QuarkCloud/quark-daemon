
#ifndef QKDN_WIN_SOCKET_INIT_H
#define QKDN_WIN_SOCKET_INIT_H 1

#include <windows.h>
#include <windef.h>
#include <winsock2.h>
#include "qknet/Compile.h"

#ifdef __cplusplus
extern "C" {
#endif


QKNETAPI BOOL AcceptEx(
    SOCKET sListenSocket,
    SOCKET sAcceptSocket,
    PVOID lpOutputBuffer,
    DWORD dwReceiveDataLength,
    DWORD dwLocalAddressLength,
    DWORD dwRemoteAddressLength,
    LPDWORD lpdwBytesReceived,
    LPOVERLAPPED lpOverlapped
);
QKNETAPI VOID GetAcceptexSockAddrs(
    PVOID lpOutputBuffer,
    DWORD dwReceiveDataLength,
    DWORD dwLocalAddressLength,
    DWORD dwRemoteAddressLength,
    struct sockaddr** LocalSockaddr,
    LPINT LocalSockaddrLength,
    struct sockaddr** RemoteSockaddr,
    LPINT RemoteSockaddrLength
);

QKNETAPI BOOL ConnectEx(
    SOCKET s,
    const struct sockaddr* name,
    int namelen,
    PVOID lpSendBuffer,
    DWORD dwSendDataLength,
    LPDWORD lpdwBytesSent,
    LPOVERLAPPED lpOverlapped
);

QKNETAPI BOOL DisconnectEx(
    SOCKET s,
    LPOVERLAPPED lpOverlapped,
    DWORD  dwFlags,
    DWORD  dwReserved
);

#define SO_UPDATE_ACCEPT_CONTEXT    0x700B
#define SO_UPDATE_CONNECT_CONTEXT   0x7010

QKNETAPI bool WinSocketInitialize();
QKNETAPI void WinSocketFinalize();

#ifdef __cplusplus
}
#endif

#endif /**QKDN_WIN_SOCKET_INIT_H*/
