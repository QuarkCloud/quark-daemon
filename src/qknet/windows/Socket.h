
#ifndef QKNET_WINDOWS_SOCKET_H
#define QKNET_WINDOWS_SOCKET_H 1

#include <WinSock2.h>
#include <string>
#include <memory>
#include "qknet/Compile.h"
#include "qknet/Address.h"
#include "qkrtl/IoHandle.h"

namespace qknet{
namespace windows {
/**
    去掉Socket的基类IoHandle
*/
class Socket{
public:
    Socket();
    Socket(const Socket& s);
    virtual ~Socket();      //特别注意，没有关闭socket

    Socket& operator=(const Socket& s);
    bool init(int type = kTypeTcp);
    void final();
    void close();
    /**
        特别注意，没有关闭socket，只是重置为原始状态。
        需要在其他地方释放掉SOCKET，否则该SOCKET会处于非法状态
    */
    void reset();   

    bool swap(SOCKET& s , int& type);
    bool swap(Socket& s);

    bool bind(const Address& addr);
    bool listen(int backlog);
    bool nodelay(bool value);
    bool nodelay() const;


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

    bool localAddr(Address& addr) const;
    bool remoteAddr(Address& addr) const; 
    std::string getFullAddress() const;

    bool valid() const;

    const OsHandle getOsHandle() const;

    inline operator SOCKET () { return getSocket(); }
    inline operator const SOCKET () const { return getSocket(); }
    inline operator HANDLE () { return getOsHandle(); }
    inline operator const HANDLE() const { return getOsHandle(); }

    inline SOCKET getSocket() const { return socket_; }
    inline int getType() const { return type_; }

    static const int kTypeNone = 0;
    static const int kTypeTcp = 1;
    static const int kTypeUdp = 2;
protected:
    mutable SOCKET socket_;
    int type_;
};

}
}
#endif /**QKNET_WINDOWS_SOCKET_H*/
