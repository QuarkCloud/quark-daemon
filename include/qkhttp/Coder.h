
#ifndef QKHTTP_CODER_H
#define QKHTTP_CODER_H 1

#include <stdint.h>
#include <string.h>
#include <string>
#include <vector>
#include "qkrtl/Coder.h"
#include "qkhttp/Compile.h"
#include "qkhttp/String.h"
#include "qkhttp/Header.h"

namespace qkhttp {

/**
*   2024-06-11
    HTTP的编解码器对性能影响不小，后续再优化
*/
class QKHTTPAPI Decoder : public qkrtl::Decoder {
public:
    Decoder();
    virtual ~Decoder();
    virtual int decode(const qkrtl::Buffer& buffer);
    virtual bool begin();
    virtual bool end();
    virtual int decode(const char* str, int slen);

    virtual bool handleRequest(const String& method, const String& url, const String& protocol);
    virtual bool handleResponse(const String& protocol, const String& status, const String& message);
    virtual bool handleField(const String& key, const String& value);

private:
    //第一行，为请求行或者应答行。
    int parseFirstLine(const char* str, int slen);
    int parseField(const char* str, int slen , bool& completed);
    //判断是否为协议版本，格式为HTTP/1.1
    bool isProtocol(const String& field) const;
};

class Encoder : public qkrtl::Encoder {
public:
    QKHTTPAPI Encoder();
    QKHTTPAPI virtual ~Encoder();

    QKHTTPAPI virtual int encode(qkrtl::Buffer& buffer);
    QKHTTPAPI virtual bool begin();
    QKHTTPAPI virtual bool end();

    QKHTTPAPI int encode(char * buffer , int buflen) ;
    QKHTTPAPI int calcSize() const;

    QKHTTPAPI bool request(const std::string& method, const std::string& url, const std::string& protocol);
    QKHTTPAPI bool response(const std::string& protocol, const std::string& status, const std::string& message);
    QKHTTPAPI bool pushField(const std::string& key, const std::string& value);

private:
    std::string firstLine_;
    std::vector<std::string> fields_;
};

class RequestDecoder : public Decoder {
public:
    QKHTTPAPI RequestDecoder();
    QKHTTPAPI virtual ~RequestDecoder();

    QKHTTPAPI virtual bool handleRequest(const String& method, const String& url, const String& protocol);
    QKHTTPAPI virtual bool handleField(const String& key, const String& value);

    inline const Request& request() const { return request_; }
    inline Request& request() { return request_; }
private:
    Request request_;
};

class ResponseDecoder : public Decoder {
public:
    QKHTTPAPI ResponseDecoder();
    QKHTTPAPI virtual ~ResponseDecoder();

    QKHTTPAPI virtual bool handleResponse(const String& protocol, const String& status, const String& message);
    QKHTTPAPI virtual bool handleField(const String& key, const String& value);
    QKHTTPAPI virtual bool handleContent(const String& content);

    inline const Response& response() const { return response_; }
    inline Response& response() { return response_; }
private:
    Response response_;
};

class RequestEncoder : public qkrtl::Encoder {
public:
    QKHTTPAPI RequestEncoder(const Request& request);
    QKHTTPAPI virtual ~RequestEncoder();

    QKHTTPAPI virtual int encode(qkrtl::Buffer& buffer);
    QKHTTPAPI int encode(char* buffer, int buflen);
    QKHTTPAPI virtual int calcSize() const;
    QKHTTPAPI virtual bool begin();
    QKHTTPAPI virtual bool end();
private:
    const Request& request_;
};


class ResponseEncoder : public qkrtl::Encoder {
public:
    QKHTTPAPI ResponseEncoder(Response& response);
    QKHTTPAPI virtual ~ResponseEncoder();

    QKHTTPAPI virtual int encode(qkrtl::Buffer& buffer);
    QKHTTPAPI int encode(char* buffer, int buflen);
    QKHTTPAPI virtual int calcSize() const;
    QKHTTPAPI virtual bool begin();
    QKHTTPAPI virtual bool end();
private:
    Response& response_;
};

}


#endif /**QKHTTP_CODER_H*/
