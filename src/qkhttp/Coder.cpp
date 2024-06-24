
#include "qkhttp/Coder.h"
#include "qkhttp/String.h"
#include <vector>

namespace qkhttp {

Decoder::Decoder()
{
    //
}
Decoder::~Decoder()
{
    //
}
int Decoder::decode(const qkrtl::Buffer& buffer)
{
    return decode(buffer.head(), buffer.dataSize());
}
bool Decoder::begin()
{
    return true;
}
bool Decoder::end()
{
    return true;
}
int Decoder::decode(const char* str, int slen)
{
    if (str == NULL || slen <= 0)
        return -1;

    if (begin() == false)
        return -1;

    int offset = parseFirstLine(str, slen);
    if (offset <= 0)
        return -1;

    //处理Head部分
    while (offset < slen)
    {
        const char* head = str + offset;
        int left = slen - offset;
        bool completed = false;

        int wsize = parseField(head, left, completed);
        if (wsize <= 0)
            return -1;

        offset += wsize;
        if (completed == true)
        {
            end();
            break;
        }
    }

    return offset;
}
bool Decoder::handleRequest(const String& method, const String& url, const String& protocol)
{
    return true;
}
bool Decoder::handleResponse(const String& protocol, const String& status, const String& message)
{
    return true;
}
bool Decoder::handleField(const String& key, const String& value)
{
    return true;
}
int Decoder::parseFirstLine(const char* str, int slen)
{
    if (str == NULL || slen <= 0)
        return -1;

    std::vector<String> fields;
    int offset = 0 , begin = 0 , end = 0;
    while (offset < slen)
    {
        char ch = str[offset];
        if (ch == ' ' || ch == '\r')
        {
            end = offset;
            String field;
            field.refer(str + begin, (end - begin));
            fields.push_back(field);
        }
        ++offset;
        if (ch == ' ')
        {
            begin = offset;
            continue;
        }

        if (ch == '\r')
        {
            if (offset >= slen)
                return -1;
            ch = str[offset];
            if (ch != '\n')
                return -1;
            ++offset;
        }
        else if (ch == '\n')
            return -1;
    }

    if (fields.size() != 3)
        return -1;

    if (isProtocol(fields[0]) == false)
    {
        handleRequest(fields[0], fields[1], fields[2]);
    }
    else
    {
        handleResponse(fields[0], fields[1], fields[2]);
    }

    return offset;
}
bool Decoder::isProtocol(const String& field) const
{
    const char* str = NULL;
    int slen = 0;
    field.get(str, slen);
    if (str == NULL || slen == 0)
        return false;

    const int kBufferMaxSize = 256;
    char buffer[kBufferMaxSize] = { '\0' };
    int buflen = 0;

    for (int idx = 0; idx < slen; ++idx)
    {
        char ch = str[idx];
        if (ch >= 'a' && ch <= 'z')
        {
            ch = 'A' + (ch - 'a');
            buffer[buflen++] = ch;
        }
        else if(ch >= 'A' && ch <= 'Z')
        { 
            buffer[buflen++] = ch;
        }
        else if (ch == '.' || (ch >= '0' && ch <= '9'))
        {
            buffer[buflen++] = ch;
        }
        else if (ch == '/')
        {
            buffer[buflen] = '\0';
            buflen = 0;

            if (::strcmp(buffer, "HTTP") == 0)
                return true;
        }
    }

    return false;
}
int Decoder::parseField(const char* str, int slen, bool& completed)
{
    if (str == NULL || slen <= 0)
        return -1;

    String key, value;
    int offset = 0, begin = 0, end = 0;
    while (offset < slen)
    {
        char ch = str[offset];
        if (ch != ' ')
            break;
        ++offset;
    }
    begin = offset;

    while (offset < slen)
    {
        char ch = str[offset];
        if (ch == ' ' || ch == ':')
            break;
        ++offset;
    }
    end = offset;
    key.refer(str + begin, (end - begin));

    //跳过    ":"
    while (offset < slen)
    {
        char ch = str[offset];
        if (ch == ':' || ch == ' ')
        {
            ++offset;
            if(ch == ':')
                break;
        }
        break;
    }

    while (offset < slen)
    {
        char ch = str[offset];
        if (ch != ' ')
            break;
        ++offset;
    }
    begin = offset;
    while (offset < slen)
    {
        char ch = str[offset];
        if (ch == ' ' || ch == '\r')
            break;
        ++offset;
    }
    end = offset;
    value.refer(str + begin, (end - begin));

    while (offset < slen)
    {
        char ch = str[offset];
        if (ch == ' ')
            ++offset;
        else
            break;
    }

    if ((offset + 2) > slen)
        return -1;

    char ch1 = str[offset++];
    char ch2 = str[offset++];

    if (ch1 != '\r' || ch2 != '\n')
        return -1;

    if (key.empty() == true || value.empty() == true)
    {
        completed = true;
    }
    else
    {
        handleField(key, value);
    }

    return offset;
}

Encoder::Encoder()
{
    //
}
Encoder::~Encoder()
{
    //
}
int Encoder::encode(qkrtl::Buffer& buffer) 
{
    return encode(buffer.tail(), buffer.avaibleSize());
}
bool Encoder::begin()
{
    return true;
}
bool Encoder::end()
{
    return true;
}
int Encoder::encode(char* buffer, int buflen) 
{
    int totalSize = calcSize();
    if (buffer == NULL || buflen <= totalSize)
        return -1;

    begin();

    int offset = ::sprintf(buffer, "%s\r\n", firstLine_.c_str());
    for (size_t fidx = 0; fidx < fields_.size(); ++fidx)
    {
        const std::string& field = fields_[fidx];
        if (field.empty() == true)
            continue;

        int fsize = ::sprintf(buffer + offset, "%s\r\n", field.c_str());
        offset += fsize;
    }

    offset += ::sprintf(buffer + offset, "\r\n");

    end();

    return (offset > totalSize);
}
int Encoder::calcSize() const
{
    if (firstLine_.empty() == true || fields_.empty() == true)
        return 0;

    int totalSize = 0;
    totalSize += (int)(firstLine_.size() + 2);
    for (size_t fidx = 0; fidx < fields_.size(); ++fidx)
    {
        const std::string& field = fields_[fidx];
        if (field.empty() == true)
            continue;
        totalSize += (int)(field.size() + 2);
    }

    totalSize += 2;
    return totalSize;
}
bool Encoder::request(const std::string& method, const std::string& url, const std::string& protocol)
{
    if (method.empty() == true || url.empty() == true || protocol.empty() == true)
        return false;

    firstLine_ = method + " " + url + " " + protocol;
    return true;
}
bool Encoder::response(const std::string& protocol, const std::string& status, const std::string& message)
{
    if (protocol.empty() == true || status.empty() == true || message.empty() == true)
        return false;

    firstLine_ = protocol + " " + status + " " + message;
    return true;
}
bool Encoder::pushField(const std::string& key, const std::string& value)
{
    if (key.empty() == true || value.empty() == true)
        return false;
    fields_.push_back(key + ":" + value);
    return true;
}


RequestDecoder::RequestDecoder()
{
    //
}
RequestDecoder::~RequestDecoder()
{
    //
}
bool RequestDecoder::handleRequest(const String& method, const String& url, const String& protocol)
{
    request_.method().assign(method.buffer() , method.size());
    request_.url().assign(url.buffer(), url.size());
    request_.protocol().assign(protocol.buffer(), protocol.size());
    return true;
}
bool RequestDecoder::handleField(const String& key, const String& value)
{
    return request_.push(std::string(key.buffer() , key.size()), 
        std::string(value.buffer() , value.size()));
}


ResponseDecoder::ResponseDecoder()
{
    //
}
ResponseDecoder::~ResponseDecoder()
{
    //
}
bool ResponseDecoder::handleResponse(const String& protocol, const String& status, const String& message)
{
    response_.protocol().assign(protocol.buffer(), protocol.size());
    response_.status().assign(status.buffer(), status.size());
    response_.message().assign(message.buffer(), message.size());
    return true;
}
bool ResponseDecoder::handleField(const String& key, const String& value)
{
    return response_.push(std::string(key.buffer(), key.size()),
        std::string(value.buffer(), value.size()));
}
bool ResponseDecoder::handleContent(const String& content)
{
    response_.content().assign(content.buffer(), content.size());
    return true;
}

RequestEncoder::RequestEncoder(const Request& request):request_(request)
{
    //
}
RequestEncoder::~RequestEncoder()
{
    //
}
int RequestEncoder::encode(qkrtl::Buffer& buffer)
{
    return encode(buffer.tail(), buffer.avaibleSize());
}
int RequestEncoder::encode(char* buffer, int buflen)
{
    if (buffer == NULL || buflen <= 0)
        return -1;

    if (begin() == false)
        return -1;

    char* str = buffer;
    int offset = 0;
    int slen =::snprintf(str + offset, buflen - offset, "%s %s %s\r\n",
        request_.method().c_str(), request_.url().c_str(), request_.protocol().c_str());
    if (slen <= 0)
        return -1;
    offset += slen;

    int fieldCount = request_.size();
    for (int fidx = 0; fidx < fieldCount; ++fidx)
    {
        const Field& field = request_.get(fidx);
        size_t fieldSize = field.key.size() + field.value.size();

        int flen = ::snprintf(str + offset, buflen - offset, "%s:%s\r\n",
            field.key.c_str(), field.value.c_str());
        if (flen <= 0)
            return -1;
        offset += flen;
    }

    if (offset + 2 > buflen)
        return -1;
    str[offset++] = '\r';
    str[offset++] = '\n';
    
    end();
    return offset;
}
int RequestEncoder::calcSize() const
{
    size_t firstLineSize = request_.method().size() + request_.url().size() + request_.protocol().size();
    size_t fieldTotalSize = 0;

    int fieldCount = request_.size();
    for (int fidx = 0; fidx < fieldCount; ++fidx)
    {
        const Field& field = request_.get(fidx);
        size_t fieldSize = field.key.size() + field.value.size();

        fieldTotalSize += fieldSize + 1 + 2; //附加':' + '\r\n'
    }
    if (fieldCount > 0)
        fieldTotalSize += 2; //'\r\n'

    return (int)(firstLineSize + 2 + fieldTotalSize);
}
bool RequestEncoder::begin()
{
    return true;
}
bool RequestEncoder::end()
{
    return true;
}



ResponseEncoder::ResponseEncoder(Response& response) :response_(response)
{
    //
}
ResponseEncoder::~ResponseEncoder()
{
    //
}
int ResponseEncoder::encode(qkrtl::Buffer& buffer)
{
    return encode(buffer.tail(), buffer.avaibleSize());
}
int ResponseEncoder::encode(char* buffer, int buflen)
{
    if (buffer == NULL || buflen <= 0)
        return -1;

    if (begin() == false)
        return -1;

    char* str = buffer;
    int offset = 0;
    int slen = ::snprintf(str + offset, buflen - offset, "%s %s %s\r\n",
        response_.protocol().c_str(), response_.status().c_str(), response_.message().c_str());
    if (slen <= 0)
        return -1;
    offset += slen;

    int fieldCount = response_.size();
    for (int fidx = 0; fidx < fieldCount; ++fidx)
    {
        const Field& field = response_.get(fidx);
        size_t fieldSize = field.key.size() + field.value.size();

        int flen = ::snprintf(str + offset, buflen - offset, "%s:%s\r\n",
            field.key.c_str(), field.value.c_str());
        if (flen <= 0)
            return -1;
        offset += flen;
    }

    if (offset + 2 > buflen)
        return -1;
    str[offset++] = '\r';
    str[offset++] = '\n';

    const std::string& content = response_.content();

    int contentSize = (int)content.size();

    if (offset + contentSize > buflen)
        return -1;

    if (contentSize > 0 || content.empty() == false)
    {
        ::memcpy(str + offset, response_.content().c_str(), contentSize);
        offset += contentSize;
    }

    end();
    return offset;
}
int ResponseEncoder::calcSize() const
{
    size_t firstLineSize = response_.protocol().size() + 
        response_.status().size() + response_.message().size();
    size_t fieldTotalSize = 0;

    int fieldCount = response_.size();
    for (int fidx = 0; fidx < fieldCount; ++fidx)
    {
        const Field& field = response_.get(fidx);
        size_t fieldSize = field.key.size() + field.value.size();

        fieldTotalSize += fieldSize + 1 + 2; //附加':' + '\r\n'
    }
    fieldTotalSize += 2; //'\r\n'

    fieldTotalSize += response_.content().size();

    return (int)(firstLineSize + 2 + fieldTotalSize);
}
bool ResponseEncoder::begin()
{
    return true;
}
bool ResponseEncoder::end()
{
    return true;
}
}

