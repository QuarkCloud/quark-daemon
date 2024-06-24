
#ifndef QKNET_SOCKET_H
#define QKNET_SOCKET_H 1

#include <WinSock2.h>
#include <string>
#include <memory>
#include "qknet/Compile.h"
#include "qknet/SocketAddr.h"

namespace qknet{

/**
    去掉Socket的基类IoHandle
*/
class QKNETAPI Socket{
public:
    Socket();
    Socket(const Socket& s);
    virtual ~Socket();      //特别注意，没有关闭socket

    Socket& operator=(const Socket& s);
    bool init(int type = kTypeTcp);
    void final();
    /**
        特别注意，没有关闭socket，只是重置为原始状态。
        需要在其他地方释放掉SOCKET，否则该SOCKET会处于非法状态
    */
    void reset();   

    bool bind(const SocketAddr& addr);
    bool swap(SOCKET& s , int& type);
    bool swap(Socket& s);

    bool nonblocking(bool value);

    bool reuse(bool value);
    bool reuse() const;

    bool keepalive(bool value);
    bool keepalive() const;

    int avaibleSize() const;

    bool sendBufsize(int size);
    int sendBufsize() const;
    bool recvBufsize(int size);
    int recvBufsize() const;
    int sockType() const;

    bool localAddr(SocketAddr& addr) const;
    bool remoteAddr(SocketAddr& addr) const; 
    std::string getFullAddress() const;

    virtual bool valid() const;
    virtual const HANDLE getHandle() const;

    inline operator SOCKET () { return getSocket(); }
    inline operator const SOCKET () const { return getSocket(); }
    inline operator HANDLE () { return getHandle(); }
    inline operator const HANDLE() const { return getHandle(); }

    inline SOCKET getSocket() const { return socket_; }
    inline int getType() const { return type_; }

    static const int kTypeNone = 0;
    static const int kTypeTcp = 1;
    static const int kTypeUdp = 2;
protected:
    mutable SOCKET socket_;
    int type_;
};

class QKNETAPI TcpSocket : public Socket{
public:
    TcpSocket();
    TcpSocket(const Socket& s);
    virtual ~TcpSocket();
    bool listen(int backlog);

    bool nodelay(bool value);
    bool nodelay() const;
};
}
#endif /**QKNET_SOCKET_H*/
