
#include "qknet/Socket.h"
#include "qkrtl/Logger.h"

namespace qknet {

Socket::Socket():socket_(INVALID_SOCKET) , type_(kTypeNone)
{
    //
}
Socket::Socket(const Socket& s):socket_(s.socket_) , type_(s.type_)
{
    //
}
Socket::~Socket()
{
    //
}
Socket& Socket::operator=(const Socket& s)
{
    socket_ = s.socket_;
    type_ = s.type_;
    return (*this);
}
bool Socket::init(int type)
{
    SOCKET s = INVALID_SOCKET;
    if (type == kTypeTcp || type == kTypeNone)
    {
        s = ::WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
        type = kTypeTcp;
    }
    else
    {
        s = ::WSASocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP, NULL, 0, WSA_FLAG_OVERLAPPED);
    }
    if (s == INVALID_SOCKET)
    {
        LOGERR("Socket[%p] failed to init socket , errCode[%d]" , this , ::WSAGetLastError());
        return false;
    }
    socket_ = s;
    type_ = type;
    LOGCRIT("Socket[%p] handle[%p] created" , this , getHandle());
    return true;
}
bool Socket::bind(const SocketAddr& addr)
{
    int retval = ::bind(socket_, addr.getSockAddr(), addr.getSocklen());
    if (retval == SOCKET_ERROR)
    {
        LOGERR("Socket[%p] failed to bind [%s] , errCode[%d]" , 
            this , addr.toString().c_str() , ::WSAGetLastError());
        return false;
    }
    return true;
}
void Socket::final()
{
    HANDLE handle = getHandle();
    SOCKET s = socket_;
    socket_ = INVALID_SOCKET;
    if (s != INVALID_SOCKET)
    {
        LOGCRIT("Socket[%p] handle[%p] will be closed" , this , handle);
        ::closesocket(s);
    }
}
void Socket::reset()
{
    socket_ = INVALID_SOCKET;
    type_ = kTypeNone;
}
bool Socket::swap(SOCKET& s, int& type)
{
    SOCKET oldSocket = socket_;
    socket_ = s;
    s = oldSocket;

    int oldType = type_;
    type_ = type;
    type = oldType;

    return true;
}
bool Socket::swap(Socket& s)
{
    return swap(s.socket_, s.type_);
}
bool Socket::nonblocking(bool value)
{
    u_long arg = 0;
    if (value == true)
        arg = 1;
    if (::ioctlsocket(socket_, FIONBIO, &arg) == SOCKET_ERROR)
    {
        LOGERR("failed to set socket[%d] nonblocking[%s] , errCode[%d]" , (int)socket_ , (value?"TRUE":"FALSE") , ::WSAGetLastError());
        return false;
    }
    return true;
}
bool Socket::reuse(bool value)
{
    int optvalue = (value ? 1 : 0);
    if (::setsockopt(socket_, SOL_SOCKET, SO_REUSEADDR, (const char*)&optvalue, sizeof(optvalue)) 
        == SOCKET_ERROR)
    {
        LOGERR("failed to set socket[%d] , SO_REUSEADDR[%s] , errCode[%d]",
            (int)socket_, (value ? "TRUE" : "FALSE"), ::WSAGetLastError());
        return false;
    }
    return true;
}
bool Socket::reuse() const
{
    int optvalue = 0;
    int optlen = sizeof(optvalue);

    if (::getsockopt(socket_, SOL_SOCKET, SO_REUSEADDR, (char*)&optvalue, &optlen) == SOCKET_ERROR)
    {
        LOGERR("failed to get socket[%d] , SO_REUSEADDR , errCode[%d]",
            (int)socket_, ::WSAGetLastError());
        return false;
    }
    return (optvalue != 0);
}
bool Socket::keepalive(bool value)
{
    int optvalue = (value ? 1 : 0);
    if (::setsockopt(socket_, SOL_SOCKET, SO_KEEPALIVE, (const char *)&optvalue, sizeof(optvalue)) == SOCKET_ERROR)
    {
        LOGERR("failed to set socket[%d] , SO_KEEPALIVE[%s] , errCode[%d]", 
            (int)socket_, (value?"TRUE":"FALSE") , ::WSAGetLastError());
        return false;
    }
    return true;
}
bool Socket::keepalive() const
{
    int optvalue = 0;
    int optlen = sizeof(optvalue);

    if (::getsockopt(socket_, SOL_SOCKET, SO_KEEPALIVE, (char *)&optvalue, &optlen) == SOCKET_ERROR)
    {
        LOGERR("failed to get socket[%d] , SO_KEEPALIVE , errCode[%d]" ,
            (int)socket_ , ::WSAGetLastError());
        return false;
    }
    return (optvalue != 0);
}

int Socket::avaibleSize() const
{
    HANDLE handle = getHandle();
    if (valid() == false)
    {
        LOGERR("SOCKET[%p] invalid , failed to get avaibleSize", handle);
        return -1;
    }

    u_long value = 0;
    if (::ioctlsocket(socket_, FIONREAD, &value) == SOCKET_ERROR)
    {
        LOGERR("failed to get socket[%p] FIONREAD , errCode[%d]", handle, ::WSAGetLastError());
        return -1;
    }
    return (int)value;
}

bool Socket::sendBufsize(int size)
{ 
    if (::setsockopt(socket_, SOL_SOCKET, SO_SNDBUF, (const char *)&size, sizeof(size)) == SOCKET_ERROR)
    {
        LOGERR("failed to set socket[%d] SO_SNDBUF[%d] , errCode[%d]" , 
            (int)socket_ , size , ::WSAGetLastError());
        return false;
    }
    return true;
}
int Socket::sendBufsize() const
{
    int optvalue = 0;
    int optlen = sizeof(optvalue);
    if (::getsockopt(socket_, SOL_SOCKET, SO_SNDBUF, (char *)&optvalue, &optlen) == SOCKET_ERROR)
    {
        LOGERR("failed to get socket[%d] SO_SNDBUF , errCode[%d]" , (int)socket_ , ::WSAGetLastError());
        return -1;
    }
    return optvalue;
}
bool Socket::recvBufsize(int size)
{
    if (::setsockopt(socket_, SOL_SOCKET, SO_RCVBUF, (const char *)&size, sizeof(size)) == SOCKET_ERROR)
    {
        LOGERR("failed to set socket[%d] SO_RCVBUF[%d] , errCode[%d]",
            (int)socket_, size, ::WSAGetLastError());
        return false;
    }
    return true;
}
int Socket::recvBufsize() const
{
    int optvalue = 0;
    int optlen = sizeof(optvalue);
    if (::getsockopt(socket_, SOL_SOCKET, SO_RCVBUF, (char *)&optvalue, &optlen) == SOCKET_ERROR)
    {
        LOGERR("failed to get socket[%d] SO_RCVBUF , errCode[%d]", (int)socket_, ::WSAGetLastError());
        return -1;
    }
    return optvalue;
}
int Socket::sockType() const
{
    int optvalue = 0;
    int optlen = sizeof(optvalue);
    if (::getsockopt(socket_, SOL_SOCKET, SO_TYPE, (char*)&optvalue, &optlen) == SOCKET_ERROR)
    {
        LOGERR("failed to get socket[%d] SO_TYPE , errCode[%d]", (int)socket_, ::WSAGetLastError());
        return kTypeNone;
    }

    if (optvalue == SOCK_DGRAM)
        return kTypeUdp;
    else if (optvalue == SOCK_STREAM)
        return kTypeTcp;
    else
        return kTypeNone;
}
bool Socket::localAddr(SocketAddr& addr) const
{
    struct sockaddr name;
    int socklen = sizeof(name);
    ::memset(&name, 0, sizeof(name));
    if (::getsockname(socket_, &name, &socklen) == SOCKET_ERROR)
    {
        LOGERR("failed to get socket[%d]'s local addr , errCode[%d]" ,
            (int)socket_ , ::WSAGetLastError());
        return false;
    }
    return addr.assign(name);
}
bool Socket::remoteAddr(SocketAddr& addr) const
{
    struct sockaddr name;
    int socklen = sizeof(name);
    ::memset(&name, 0, sizeof(name));
    if (::getpeername(socket_, &name, &socklen) == SOCKET_ERROR)
    {
        int errCode = ::WSAGetLastError();
        if (errCode != WSAENOTCONN)
        {
            LOGERR("Socket[%p] failed to get socket[%p]'s remote addr , errCode[%d]",
                this, getHandle(), errCode);
        }
        return false;
    }
    return addr.assign(name);
}
std::string Socket::getFullAddress() const
{
    char str[256] = { '\0' };
    if (valid() == true)
    {
        int slen = 0;
        SocketAddr local;
        if (localAddr(local) == true)
        {
            slen = ::sprintf(str, "%s", local.toString().c_str());
        }

        SocketAddr remote;
        if (remoteAddr(remote) == true)
        {
            ::sprintf(str + slen, "-->%s", remote.toString().c_str());
        }
    }
    return str;
}
bool Socket::valid() const
{
    return (socket_ != INVALID_SOCKET);
}
const HANDLE Socket::getHandle() const
{
    return (HANDLE)socket_;
}


TcpSocket::TcpSocket()
{
    socket_ = INVALID_SOCKET;
    type_ = Socket::kTypeTcp;
}
TcpSocket::TcpSocket(const Socket& s):Socket(s)
{
    //
}
TcpSocket::~TcpSocket()
{
    //
}

bool TcpSocket::listen(int backlog)
{
    if (::listen(socket_, backlog) == SOCKET_ERROR)
    {
        LOGERR("socket[%p] faild to listen , errCode[%d]",
            getHandle(), ::WSAGetLastError());
        return false;
    }
    return true;
}

bool TcpSocket::nodelay(bool value)
{
    int optvalue = 0;
    if (value == true)
        optvalue = 1;
    if (::setsockopt(socket_, IPPROTO_TCP, TCP_NODELAY, (const char*)&optvalue, sizeof(optvalue)) == SOCKET_ERROR)
    {
        LOGERR("failed to set socket[%d] TCP_NODELAY [%s] , errCode[%d]",
            (int)socket_, (value ? "TRUE" : "FALSE"), ::WSAGetLastError());
        return false;
    }
    return true;
}
bool TcpSocket::nodelay() const
{
    int optvalue = 0;
    int optlen = sizeof(optvalue);
    if (::getsockopt(socket_, IPPROTO_TCP, TCP_NODELAY, (char*)&optvalue, &optlen) == SOCKET_ERROR)
    {
        LOGERR("failed to get socket[%d] , TCP_NODELAY , errCode[%d]", (int)socket_, ::WSAGetLastError());
        return false;
    }

    return (optvalue != 0);
}



}
