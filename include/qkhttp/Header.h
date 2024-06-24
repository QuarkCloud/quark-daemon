
#ifndef QKHTTP_HEADER_H
#define QKHTTP_HEADER_H 1

#include <stdint.h>
#include <string.h>
#include <string>
#include <map>
#include <vector>
#include "qkhttp/Compile.h"

namespace qkhttp {

/**
    HTTP请求的格式为
    第一行：请求方法 + 空格 + URL + 空格 + 协议版本 + \r\n
        POST / HTTP/1.1\r\n
    第二行：字段名 + ":" + 值 + \r\n
    第三行：\r\n，空行结束
    请求数据

    HTTP应答的格式为
    第一行：版本协议 + 空格 + 状态码 + 空格 + 状态文本 + \r\n
        HTTP/1.1 200 OK\r\n
    第二行：字段名 + ":" + 值 + \r\n
    第三行：\r\n，空行结束

    HTTP之状态码
    1. 1**：信息，服务器收到请求，需要请求者继续执行操作
    2. 2**：成功，操作被成功接收并处理
    3. 3**：重定向，需要进一步的操作以完成请求
    4. 4**：客户端错误，请求包含语法错误或无法完成请求
    5. 5**：服务器错误，服务器在处理请求的过程中发生了错误
*/
class Field {
public:
    QKHTTPAPI Field();
    QKHTTPAPI Field(const Field& f);
    QKHTTPAPI Field(const std::string& k , const std::string& v);
    QKHTTPAPI virtual ~Field();

    QKHTTPAPI bool valid() const;

    QKHTTPAPI static const Field kEmpty;
    QKHTTPAPI static const std::string kEmptyValue;

    std::string key;
    std::string value;    
};
/**
    这里的请求和应答性能还需要优化。
*/
class HeaderFields {
public:
    QKHTTPAPI HeaderFields();
    QKHTTPAPI virtual ~HeaderFields();

    QKHTTPAPI bool push(const std::string& key, const std::string& value);
    QKHTTPAPI const std::string& find(const std::string& key) const;
    QKHTTPAPI int size() const;
    QKHTTPAPI const Field& get(int index) const;

private:
    std::vector<Field> fields_;
    std::map<std::string, int> indexes_;
};

class Request : public HeaderFields {
public:
    QKHTTPAPI Request();
    QKHTTPAPI virtual ~Request();

    inline const std::string& method() const { return method_; }
    inline std::string& method() { return method_; }

    inline const std::string& url() const { return url_; }
    inline std::string& url() { return url_; }

    inline const std::string& protocol() const { return protocol_; }
    inline std::string& protocol() { return protocol_; }

private:
    std::string method_;
    std::string url_;
    std::string protocol_;

    std::vector<Field> fields_;
    std::map<std::string, int> indexes_;
};

class Response : public HeaderFields {
public:
    QKHTTPAPI Response();
    QKHTTPAPI virtual ~Response();

    inline const std::string& protocol() const { return protocol_; }
    inline std::string& protocol() { return protocol_; }

    inline const std::string& status() const { return status_; }
    inline std::string& status() { return status_; }

    inline const std::string& message() const { return message_; }
    inline std::string& message() { return message_; }

    inline const std::string& content() const { return content_; }
    inline std::string& content() { return content_; }
private:
    std::string protocol_;
    std::string status_;
    std::string message_;
    std::string content_;
};

}


#endif /**QKHTTP_HEADER_H*/
