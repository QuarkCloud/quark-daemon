
#ifndef QKRTL_URL_H
#define QKRTL_URL_H 1

#include <string.h>
#include "qkrtl/Compile.h"
#include "qkrtl/String.h"

namespace qkrtl {

/**
    格式：protocol://host[:port]/path/[:parameters][?query]#fragment
    案例：http://www.abc.com:8080/dir/index.html?id=255&m=hello#top

    protocol = http 
    host = abc.com
    port = 8080
    path = /dir/index.html
    query=?id=255&m=hello

    url是一个通用格式，后续解析规则由protocol决定，比如
    file:///c:/users/abc.txt ，这个格式和网络格式不同，他没有host:port
    protocol后缀必须以 "://"结束

    2024-07-16
    目前只支持网络格式，本地格式因为和操作系统有关，暂时先忽略。比如file+pipe
*/

class Url {
public:
    QKRTLAPI Url();
    QKRTLAPI virtual ~Url();

    QKRTLAPI bool decode(const char* str, int size);
    QKRTLAPI int encode(char* str, int size) const;

    QKRTLAPI bool fromString(const char* str, int size);
    QKRTLAPI bool fromString(const std::string& str);
    QKRTLAPI bool toString(std::string& str) const;
    //只包括protocol://host:port，不是完整的url，这个需要注意
    QKRTLAPI bool toAddress(std::string& addr) const;

    QKRTLAPI bool valid(bool hasPort=false) const;

    inline const std::string& protocol() const { return protocol_; }
    inline const std::string& host() const { return host_; }
    inline uint16_t port() const { return port_; }
    inline const std::string& path() const { return path_; }
    inline const std::string& query() const { return query_; }

    inline std::string& protocol() { return protocol_; }
    inline std::string& host() { return host_; }
    inline uint16_t& port() { return port_; }
    inline std::string& path() { return path_; }
    inline std::string& query() { return query_; }

    inline void protocol(const std::string& value) { protocol_ = value; }
    inline void host(const std::string& value) { host_ = value; }
    inline void port(uint16_t value) { port_ = value; }
    inline void path(const std::string& value) { path_ = value; }
    inline void query(const std::string& value) { query_ = value; }
private:
    std::string protocol_;
    std::string host_;
    uint16_t port_;
    std::string path_;
    std::string query_;
};
class QKRTLAPI UrlHandler {
public:
    UrlHandler();
    virtual ~UrlHandler();
    virtual bool onProtocol(const char * str , int size);
    virtual bool onHost(const char* str, int size);
    virtual bool onPort(int port);
    virtual bool onPath(const char* str, int size);
    virtual bool onQuery(const char* str, int size);
};
class UrlReader : public UrlHandler {
public:
    QKRTLAPI UrlReader(Url& url);
    QKRTLAPI virtual ~UrlReader();
    QKRTLAPI virtual bool onProtocol(const char* str, int size);
    QKRTLAPI virtual bool onHost(const char* str, int size);
    QKRTLAPI virtual bool onPort(int port);
    QKRTLAPI virtual bool onPath(const char* str, int size);
    QKRTLAPI virtual bool onQuery(const char* str, int size);
private:
    Url& url_;
};

class QKRTLAPI UrlParser{
public:
    UrlParser(UrlHandler* handler = NULL);
    virtual ~UrlParser();
    bool decode(const char* str, int size);
    bool decode(const char* str, int size , Url& url);
private:
    UrlHandler* handler_;
    int readProtocol(const char* str, int size);
    int readHost(const char* str, int size);
    int readPort(const char* str, int size);
    int readPath(const char* str, int size);
    int readQuery(const char* str, int size);
};

QKRTLAPI int EscapeString(const char * str , int size , unsigned char& value);
QKRTLAPI bool CalcPort(const char* str, int size, uint16_t& port);

//没有前置空格，有效字符为字符，数字，_，以://结束。返回值包括3个字符的后缀
QKRTLAPI int ParseProtocol(const char* str, int size);

}

#endif /**QKRTL_URL_H*/
