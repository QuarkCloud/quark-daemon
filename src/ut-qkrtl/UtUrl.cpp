
#include <string>
#include <iostream>

#include "qkrtl/TestKits.h"
#include "qkrtl/Url.h"

class UrlHandler : public qkrtl::UrlHandler {
public:
    UrlHandler() :port_(0) {}
    virtual ~UrlHandler() {}
    virtual bool onProtocol(const char* str, int size)
    {
        if(str != NULL && size > 0)
            protocol_.assign(str, size);
        return true;
    }
    virtual bool onHost(const char* str, int size)
    {
        if (str != NULL && size > 0)
            host_.assign(str, size);
        return true;
    }
    virtual bool onPort(int port)
    {
        port_ = port;
        return true;
    }
    virtual bool onPath(const char* str, int size)
    {
        if (str != NULL && size > 0)
            path_.assign(str, size);
        return true;
    }
    virtual bool onQuery(const char* str, int size)
    {
        if (str != NULL && size > 0)
            query_.assign(str, size);
        return true;
    }

    inline const std::string& protocol() const { return protocol_; }
    inline const std::string& host() const { return host_; }
    inline int port() const { return port_; }
    inline const std::string& path() const { return path_; }
    inline const std::string& query() const { return query_; }
private:
    std::string protocol_;
    std::string host_;
    int port_;
    std::string path_;
    std::string query_;
};

bool testUrl()
{
    /**
        http://www.abc.com:8080/dir/index.html?id=255&m=hello
        protocol = http
        host = www.abc.com
        port = 8080
        path = /dir/index.html
        query =?id=255&m=hello
    */
    std::string protocol = "http";
    std::string host = "www.abc.com";
    int port = 8080;
    std::string path = "/dir/index.html";
    std::string query = "?id=255&m=hello";

    char str[1024] = { '\0' };
    int slen = ::sprintf(str, "%s://%s:%d%s%s",
        protocol.c_str(), host.c_str(), port, path.c_str(), query.c_str());

    UrlHandler handler;
    qkrtl::UrlParser parser(&handler);
    
    bool result = parser.decode(str, slen);
    TESTCMP(IsEqual(result, true));
    
    TESTCMP(IsEqual(handler.protocol(), protocol));
    TESTCMP(IsEqual(handler.host(), host));
    TESTCMP(IsEqual(handler.port(), port));
    TESTCMP(IsEqual(handler.path(), path));
    TESTCMP(IsEqual(handler.query(), query));


    return true;
}

TESTCASE(testUrl);