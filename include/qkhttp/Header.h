
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
    HTTP����ĸ�ʽΪ
    ��һ�У����󷽷� + �ո� + URL + �ո� + Э��汾 + \r\n
        POST / HTTP/1.1\r\n
    �ڶ��У��ֶ��� + ":" + ֵ + \r\n
    �����У�\r\n�����н���
    ��������

    HTTPӦ��ĸ�ʽΪ
    ��һ�У��汾Э�� + �ո� + ״̬�� + �ո� + ״̬�ı� + \r\n
        HTTP/1.1 200 OK\r\n
    �ڶ��У��ֶ��� + ":" + ֵ + \r\n
    �����У�\r\n�����н���

    HTTP֮״̬��
    1. 1**����Ϣ���������յ�������Ҫ�����߼���ִ�в���
    2. 2**���ɹ����������ɹ����ղ�����
    3. 3**���ض�����Ҫ��һ���Ĳ������������
    4. 4**���ͻ��˴�����������﷨������޷��������
    5. 5**�����������󣬷������ڴ�������Ĺ����з����˴���
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
    ����������Ӧ�����ܻ���Ҫ�Ż���
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
