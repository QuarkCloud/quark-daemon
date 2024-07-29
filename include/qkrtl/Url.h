
#ifndef QKRTL_URL_H
#define QKRTL_URL_H 1

#include <string.h>
#include "qkrtl/Compile.h"
#include "qkrtl/String.h"

namespace qkrtl {

/**
    ��ʽ��protocol://host[:port]/path/[:parameters][?query]#fragment
    ������http://www.abc.com:8080/dir/index.html?id=255&m=hello#top

    protocol = http 
    host = abc.com
    port = 8080
    path = /dir/index.html
    query=?id=255&m=hello

    url��һ��ͨ�ø�ʽ����������������protocol����������
    file:///c:/users/abc.txt �������ʽ�������ʽ��ͬ����û��host:port
    protocol��׺������ "://"����

    2024-07-16
    Ŀǰֻ֧�������ʽ�����ظ�ʽ��Ϊ�Ͳ���ϵͳ�йأ���ʱ�Ⱥ��ԡ�����file+pipe
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
    //ֻ����protocol://host:port������������url�������Ҫע��
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

//û��ǰ�ÿո���Ч�ַ�Ϊ�ַ������֣�_����://����������ֵ����3���ַ��ĺ�׺
QKRTLAPI int ParseProtocol(const char* str, int size);

}

#endif /**QKRTL_URL_H*/
