
#include "qknet/Address.h"
#include "qkrtl/Logger.h"
#include <ws2tcpip.h>

namespace qknet{

Address::Address() :sockAddr_{0}
{
    clear();
}
Address::Address(const Address& addr):sockAddr_{0}
{
    assign(addr);
}
Address::Address(const std::string& host, uint16_t port)
{
    assign(host, port);
}
Address::~Address()
{
    //
}
void Address::clear()
{
    ::memset(&sockAddr_, 0, sizeof(AddrHolder));
}
bool Address::assign(const Address& addr)
{
    ::memcpy(&sockAddr_, &addr.sockAddr_, sizeof(AddrHolder));
    return true;
}
bool Address::assign(const std::string& addr)
{
    std::string host;
    uint16_t port = 0;
    if (Address::parseAddr(addr, host, port) == false)
        return false;
    return assign(host, port);
}
bool Address::assign(const std::string& host, uint16_t port)
{
    clear();
    uint32_t addr = 0;
    if (InetAddr(host, addr) == false)
        return false;

    sockAddr_.sockAddrV4.sin_family = AF_INET;
    sockAddr_.sockAddrV4.sin_addr.S_un.S_addr = addr;
    sockAddr_.sockAddrV4.sin_port = ::htons(port);
    return true;
}
bool Address::assign(const struct sockaddr & addr)
{
    clear();
    ::memcpy(&sockAddr_.sockAddr, &addr, sizeof(struct sockaddr));
    return true;
}
bool Address::assign(const struct sockaddr_in & addr)
{
    clear();
    ::memcpy(&sockAddr_.sockAddrV4, &addr, sizeof(struct sockaddr_in));
    return true;
}

uint32_t Address::getInetAddr() const
{
    return sockAddr_.sockAddrV4.sin_addr.S_un.S_addr;
}
uint16_t Address::getPort() const
{
    uint16_t port = sockAddr_.sockAddrV4.sin_port;
    return ::ntohs(port);
}
std::string Address::toString() const
{
    uint32_t addr = getInetAddr();
    uint16_t port = getPort();

    char str[256] = { '\0' };
    ::snprintf(str, sizeof(str), "%s:%hu", InetNtoa(addr).c_str(), port);
    return str;
}
const struct sockaddr * Address::getSockAddr() const
{
    return &sockAddr_.sockAddr;
}
struct sockaddr* Address::getSockAddr()
{
    return &sockAddr_.sockAddr;
}
int Address::getSocklen() const
{
    return sizeof(sockAddr_);
}
bool Address::parseAddr(const std::string& addr, std::string& host, uint16_t& port)
{
    host.clear();
    port = 0;
    const size_t kMaxBufferSize = 256;
    char buffer[kMaxBufferSize] = { '\0' };
    size_t buflen = 0 , offset = 0;
    if (addr.size() >= kMaxBufferSize)
        return false;
    for (; offset < addr.size(); ++offset)
    {
        char ch = addr[offset];
        if (ch == '.' || (ch >= '0' && ch <= '9'))
        {
            buffer[buflen++] = ch;
        }
        else if (ch == ':')
        {
            ++offset;
            break;
        }
        else
        {
            return false;
        }
    }

    if (buflen == 0)
        return false;
    buffer[buflen] = '\0';
    host = buffer;
    buflen = 0;

    for (; offset < addr.size(); ++offset)
    {
        char ch = addr[offset];
        if (ch >= '0' && ch <= '9')
        {
            buffer[buflen++] = ch;
        }
        else
            return false;
    }
    //û�ж˿ں�
    if (buflen == 0)
        return true;

    int value = ::atoi(buffer);
    if (value < 0 || value >= 256)
        return false;
    port = (uint16_t)value;
    return true;
}
bool GetPeerAddrInfo(const std::string& host, std::vector<Address>& addrs)
{
    struct addrinfo hints;
    ::memset(&hints, 0, sizeof(hints));
    hints.ai_flags = AI_ADDRCONFIG;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    struct addrinfo * result = NULL;

    INT retValue = ::getaddrinfo(host.c_str(), NULL, &hints, &result);
    if (retValue != 0)
    {
        LOGERR("failed to get [%s]'s addr info , errcode[%d]" , host.c_str() , retValue);
        return false;
    }

    struct addrinfo * cur = NULL;
    for (cur = result; cur != NULL; cur = cur->ai_next)
    {
        int family = cur->ai_family;
        int socktype = cur->ai_socktype;
        if (family != AF_INET || socktype != SOCK_STREAM)
            continue;

        size_t addrlen = cur->ai_addrlen;
        const struct sockaddr * addr = cur->ai_addr;
        if (addrlen < sizeof(struct sockaddr))
            continue;

        Address sockaddr;
        if (sockaddr.assign(*addr) == false)
            continue;

        addrs.push_back(sockaddr);
    }

    ::freeaddrinfo(result);
    return true;
}
bool InetAddr(const std::string& host, uint32_t& addr)
{
    if (host.empty() == true)
    {
        addr = 0;
        return true;
    }

    const char * str = host.c_str();
    int buflen = 0;
    int value = 0;
    uint8_t * bytes = (uint8_t *)&addr;
    size_t offset = 0;
    for (size_t idx = 0; idx < host.size(); ++idx)
    {
        char ch = str[idx];
        if (ch == '.')
        {
            if (buflen == 0)
                return false;
            buflen = 0;
            if (value < 0 || value >= 256)
                return false;
            uint8_t u8 = (uint8_t)value;
            bytes[offset++] = u8;
            value = 0;
            if (offset >= 4)
                return false;
        }
        else if (ch >= '0' && ch <= '9')
        {
            value = value * 10 + (ch - '0');
            buflen++;
        }
        else
        {
            return false;
        }
    }

    if (buflen > 0)
    {
        if (value < 0 || value >= 256)
            return false;
        bytes[offset++] = (uint8_t)value;
    }
    if (offset != 4)
        return false;
    return true;
}
std::string InetNtoa(const uint32_t addr)
{
    const uint8_t * bytes = (const uint8_t *)&addr;
    char str[256] = { '\0' };
    ::snprintf(str, sizeof(str), "%hhu.%hhu.%hhu.%hhu",
        bytes[0], bytes[1], bytes[2], bytes[3]);
    return str;
}
}
