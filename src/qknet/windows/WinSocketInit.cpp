
#include <windows.h>
#include "WinSocketInit.h"
#include "qkrtl/Logger.h"


#ifndef WSAID_ACCEPTEX

typedef BOOL(PASCAL FAR* LPFN_ACCEPTEX)(
    IN SOCKET sListenSocket,
    IN SOCKET sAcceptSocket,
    IN PVOID lpOutputBuffer,
    IN DWORD dwReceiveDataLength,
    IN DWORD dwLocalAddressLength,
    IN DWORD dwRemoteAddressLength,
    OUT LPDWORD lpdwBytesReceived,
    IN LPOVERLAPPED lpOverlapped
    );

#define WSAID_ACCEPTEX                                                       \
    {0xb5367df1,0xcbac,0x11cf,{0x95,0xca,0x00,0x80,0x5f,0x48,0xa1,0x92}}

#endif


#ifndef WSAID_GETACCEPTEXSOCKADDRS

typedef VOID(PASCAL FAR* LPFN_GETACCEPTEXSOCKADDRS)(
    IN PVOID lpOutputBuffer,
    IN DWORD dwReceiveDataLength,
    IN DWORD dwLocalAddressLength,
    IN DWORD dwRemoteAddressLength,
    OUT struct sockaddr** LocalSockaddr,
    OUT LPINT LocalSockaddrLength,
    OUT struct sockaddr** RemoteSockaddr,
    OUT LPINT RemoteSockaddrLength
    );

#define WSAID_GETACCEPTEXSOCKADDRS                                           \
        {0xb5367df2,0xcbac,0x11cf,{0x95,0xca,0x00,0x80,0x5f,0x48,0xa1,0x92}}

#endif



#ifndef WSAID_CONNECTEX

typedef BOOL(PASCAL FAR* LPFN_CONNECTEX) (
    IN SOCKET s,
    IN const struct sockaddr FAR* name,
    IN int namelen,
    IN PVOID lpSendBuffer OPTIONAL,
    IN DWORD dwSendDataLength,
    OUT LPDWORD lpdwBytesSent,
    IN LPOVERLAPPED lpOverlapped
    );

#define WSAID_CONNECTEX \
    {0x25a207b9,0xddf3,0x4660,{0x8e,0xe9,0x76,0xe5,0x8c,0x74,0x06,0x3e}}

#endif


#ifndef WSAID_DISCONNECTEX

typedef BOOL(PASCAL FAR* LPFN_DISCONNECTEX) (
    IN SOCKET s,
    IN LPOVERLAPPED lpOverlapped,
    IN DWORD  dwFlags,
    IN DWORD  dwReserved
    );

#define WSAID_DISCONNECTEX                                                   \
    {0x7fda2e11,0x8630,0x436f,{0xa0,0x31,0xf5,0x36,0xa6,0xee,0xc1,0x57}}

#endif


static LPFN_ACCEPTEX              lpfn_AcceptEx = NULL;
static LPFN_GETACCEPTEXSOCKADDRS  lpfn_GetAcceptexSockAddrs = NULL;
static LPFN_CONNECTEX             lpfn_ConnectEx = NULL;
static LPFN_DISCONNECTEX          lpfn_DisconnectEx = NULL;

BOOL AcceptEx( SOCKET sListenSocket, SOCKET sAcceptSocket, PVOID lpOutputBuffer,
    DWORD dwReceiveDataLength,   DWORD dwLocalAddressLength,   DWORD dwRemoteAddressLength,
    LPDWORD lpdwBytesReceived,   LPOVERLAPPED lpOverlapped)
{
    if (lpfn_AcceptEx == NULL)
        return FALSE;

    return lpfn_AcceptEx(sListenSocket, sAcceptSocket, lpOutputBuffer,
        dwReceiveDataLength, dwLocalAddressLength, dwRemoteAddressLength,
        lpdwBytesReceived, lpOverlapped);
}
VOID GetAcceptexSockAddrs( PVOID lpOutputBuffer,  DWORD dwReceiveDataLength,
    DWORD dwLocalAddressLength,    DWORD dwRemoteAddressLength,
    struct sockaddr** LocalSockaddr,    LPINT LocalSockaddrLength,
    struct sockaddr** RemoteSockaddr,    LPINT RemoteSockaddrLength)
{
    if (lpfn_GetAcceptexSockAddrs == NULL)
        return;

    lpfn_GetAcceptexSockAddrs(lpOutputBuffer, dwReceiveDataLength, 
        dwLocalAddressLength, dwRemoteAddressLength, 
        LocalSockaddr, LocalSockaddrLength,
        RemoteSockaddr, RemoteSockaddrLength);
}
BOOL ConnectEx( SOCKET s,  const struct sockaddr* name,  int namelen,
    PVOID lpSendBuffer,   DWORD dwSendDataLength,    
    LPDWORD lpdwBytesSent,   LPOVERLAPPED lpOverlapped)
{
    if (lpfn_ConnectEx == NULL)
        return FALSE;
    return lpfn_ConnectEx(s, name, namelen,
        lpSendBuffer, dwSendDataLength, lpdwBytesSent, lpOverlapped);
}
BOOL DisconnectEx( SOCKET s,  LPOVERLAPPED lpOverlapped,  DWORD  dwFlags,  DWORD  dwReserved)
{
    if (lpfn_DisconnectEx == NULL)
        return FALSE;
    return lpfn_DisconnectEx(s, lpOverlapped, dwFlags, dwReserved);
}

bool WinSocketInitialize()
{
    WSADATA wsd;
    if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0) 
    {
        LOGERR("failed to WSAStartup , errCode[%d] " , ::WSAGetLastError());
        return false;
    }

    SOCKET s = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s == INVALID_SOCKET) {
        LOGERR("failed to create a socket2 , errCode[%d] ", ::WSAGetLastError());
        return false;
    }

    DWORD bytes = 0;
    GUID ACCEPTEX_GUID = WSAID_ACCEPTEX;
    if (WSAIoctl(s, SIO_GET_EXTENSION_FUNCTION_POINTER, &ACCEPTEX_GUID, sizeof(GUID),
        &lpfn_AcceptEx, sizeof(LPFN_ACCEPTEX), &bytes, NULL, NULL)
        == SOCKET_ERROR)
    {
        LOGERR("WSAIoctl(SIO_GET_EXTENSION_FUNCTION_POINTER, "
            "WSAID_ACCEPTEX) failed , errCode[%d]" , ::WSAGetLastError());
        return false;
    }

    GUID GETACCEPTEXSOCKADDRS_GUID = WSAID_GETACCEPTEXSOCKADDRS;
    if (WSAIoctl(s, SIO_GET_EXTENSION_FUNCTION_POINTER, &GETACCEPTEXSOCKADDRS_GUID, sizeof(GUID),
        &lpfn_GetAcceptexSockAddrs, sizeof(LPFN_GETACCEPTEXSOCKADDRS),
        &bytes, NULL, NULL)
        == SOCKET_ERROR)
    {
        LOGERR("WSAIoctl(SIO_GET_EXTENSION_FUNCTION_POINTER, "
            "WSAID_GETACCEPTEXSOCKADDRS) failed , errCode[%d]" , ::WSAGetLastError());
        return false;
    }

    GUID CONNECTEX_GUID = WSAID_CONNECTEX;
    if (WSAIoctl(s, SIO_GET_EXTENSION_FUNCTION_POINTER, &CONNECTEX_GUID, sizeof(GUID),
        &lpfn_ConnectEx, sizeof(LPFN_CONNECTEX), &bytes,
        NULL, NULL)
        == SOCKET_ERROR)
    {
        LOGERR("WSAIoctl(SIO_GET_EXTENSION_FUNCTION_POINTER, "
            "WSAID_CONNECTEX) failed , errCode[%d]" , ::WSAGetLastError());
        return false;
    }

    GUID DISCONNECTEX_GUID = WSAID_DISCONNECTEX;
    if (WSAIoctl(s, SIO_GET_EXTENSION_FUNCTION_POINTER, &DISCONNECTEX_GUID, sizeof(GUID),
        &lpfn_DisconnectEx, sizeof(LPFN_DISCONNECTEX), &bytes,
        NULL, NULL)
        == SOCKET_ERROR)
    {
        LOGERR( "WSAIoctl(SIO_GET_EXTENSION_FUNCTION_POINTER, "
            "WSAID_DISCONNECTEX) failed , errCode[%d]" , ::WSAGetLastError());
        return false;
    }

    ::closesocket(s);
    return true;
}
void WinSocketFinalize()
{
    WSACleanup();
}
