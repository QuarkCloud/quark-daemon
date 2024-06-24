
#ifndef QKNET_SOCKET_ADDR_H
#define QKNET_SOCKET_ADDR_H 1

#include "qknet/Compile.h"
#include <WinSock2.h>
#include <string>
#include <vector>

namespace qknet{

typedef union {
    struct sockaddr sockAddr;
    struct sockaddr_in sockAddrV4;
    char Padding[16];
} SockAddrHolder;

//host只支持IP地址，不支持域名，如果是域名，需要用GetPeerAddrInfo，先做域名解析
class QKNETAPI SocketAddr {
public:
    SocketAddr();
    SocketAddr(const SocketAddr& addr);
    SocketAddr(const std::string& host, uint16_t port);
    virtual ~SocketAddr();

    void clear();
    bool assign(const SocketAddr& addr);
    bool assign(const std::string& addr);
    bool assign(const std::string& host, uint16_t port);
    bool assign(const struct sockaddr & addr);
    bool assign(const struct sockaddr_in & addr);

    //IPV4地址
    uint32_t getInetAddr() const;
    uint16_t getPort() const;
    std::string toString() const;
    const struct sockaddr * getSockAddr() const;
    int getSocklen() const;

    static bool parseAddr(const std::string& addr, std::string& host, uint16_t& port);
private:
    SockAddrHolder sockAddr_;
};

QKNETAPI bool GetPeerAddrInfo(const std::string& host, std::vector<SocketAddr>& addrs);
QKNETAPI bool InetAddr(const std::string& host, uint32_t& addr);
QKNETAPI std::string InetNtoa(const uint32_t addr);
}


#endif /**QKNET_SOCKET_ADDR_H*/
