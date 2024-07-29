
#include "qkrtl/Url.h"

namespace qkrtl {

Url::Url() :port_(0)
{
    //
}
Url::~Url()
{
    //
}
bool Url::decode(const char* str, int size)
{
    UrlParser parser;
    return parser.decode(str, size, *this);
}
int Url::encode(char* str, int size) const
{
    return ::snprintf(str, size, "%s://%s:%hu%s%s",
        protocol_.c_str(), host_.c_str(), port_,
        path_.c_str(), query_.c_str());
}
bool Url::fromString(const char* str, int size)
{
    return decode(str, size);
}
bool Url::fromString(const std::string& str)
{
    return fromString(str.c_str(), (int)str.size());
}
bool Url::toString(std::string& str) const
{
    const int kBufferMaxSize = 1024;
    char buffer[kBufferMaxSize] = { '\0' };
    int size = encode(buffer, kBufferMaxSize);
    if (size <= 0)
        return false;

    buffer[size] = '\0';
    str = buffer;
    return true;
}
bool Url::toAddress(std::string& addr) const
{
    const int kBufferMaxSize = 1024;
    char buffer[kBufferMaxSize] = { '\0' };
    int size = ::snprintf(buffer, kBufferMaxSize, "%s://%s:%hu",
        protocol_.c_str(), host_.c_str(), port_);
    if (size <= 0)
        return false;

    buffer[size] = '\0';
    addr = buffer;
    return true;
}
bool Url::valid(bool hasPort) const
{
    if (protocol_.empty() == true)
        return false;

    if (host_.empty() == false)
    {
        if (hasPort == true && port_ == 0)
            return false;
        return true;
    }
    else
    {
        //���û��host����ô������path
        if (path_.empty() == false)
            return true;
    }
    return false;
}
UrlHandler::UrlHandler()
{
    //   
}
UrlHandler::~UrlHandler()
{
    //
}
bool UrlHandler::onProtocol(const char* str, int size)
{
    return false;
}
bool UrlHandler::onHost(const char* str, int size)
{
    return false;
}
bool UrlHandler::onPort(int port)
{
    return false;
}
bool UrlHandler::onPath(const char* str, int size)
{
    return false;
}
bool UrlHandler::onQuery(const char* str, int size)
{
    return false;
}
UrlReader::UrlReader(Url& url):url_(url)
{
    //
}
UrlReader::~UrlReader()
{
    //
}
bool UrlReader::onProtocol(const char* str, int size)
{
    if (str != NULL && size > 0)
        url_.protocol().assign(str, size);
    return true;
}
bool UrlReader::onHost(const char* str, int size)
{
    if (str != NULL && size > 0)
        url_.host().assign(str, size);
    return true;
}
bool UrlReader::onPort(int port)
{
    url_.port() = port;
    return true;
}
bool UrlReader::onPath(const char* str, int size)
{
    if (str != NULL && size > 0)
        url_.path().assign(str, size);
    return true;
}
bool UrlReader::onQuery(const char* str, int size)
{
    if (str != NULL && size > 0)
        url_.query().assign(str, size);
    return true;
}

UrlParser::UrlParser(UrlHandler* handler): handler_(handler)
{
    //
}
UrlParser::~UrlParser()
{
    //
}
bool UrlParser::decode(const char* str, int size)
{
    if (str == NULL || size <= 0)
        return false;

    String url;
    if (url.refer(str, size) == false)
        return false;

    int dataSize = 0;

    /**
        protocol�����е�,����Ϊ�Ƿ�
    */
    dataSize = readProtocol(url.head() , url.dataSize());
    if (dataSize <= 3)
        return false;
    if (handler_ != NULL  && handler_->onProtocol(url.head(), dataSize - 3) == false)
        return false;
    url.shrink(dataSize);

    //file://��pipe://�ȱ���Э�飬û��host
    dataSize = readHost(url.head(), url.dataSize());
    if (dataSize > 0)
    {
        if (handler_ != NULL && handler_->onHost(url.head(), dataSize) == false)
            return false;
        url.shrink(dataSize);
    }

    dataSize = readPort(url.head(), url.dataSize());
    if (dataSize > 0)
    {
        //�˿ڲ��Ǳ����
        uint16_t port = 0;
        if (CalcPort(url.head() + 1, dataSize - 1, port) == false)
            return false;
        if (handler_ != NULL && handler_->onPort(port) == false)
            return false;
        url.shrink(dataSize);
    }

    dataSize = readPath(url.head(), url.dataSize());
    if (dataSize > 0)
    {
        if (handler_ != NULL && handler_->onPath(url.head() , dataSize) == false)
            return false;
        url.shrink(dataSize);
    }

    dataSize = readQuery(url.head(), url.dataSize());
    if (dataSize > 0)
    {
        if (handler_ != NULL && handler_->onQuery(url.head(), dataSize) == false)
            return false;
        url.shrink(dataSize);
    }

    return true;
}
bool UrlParser::decode(const char* str, int size, Url& url)
{
    UrlReader reader(url);
    handler_ = &reader;
    return decode(str, size);
}
int UrlParser::readProtocol(const char* str, int size)
{
    return ParseProtocol(str, size);
}
int UrlParser::readHost(const char* str, int size)
{
    //�Ϸ��ַ��������ַ������֣�'.'��'_','-'������":"����"/"����
    int idx = 0;
    while (idx < size)
    {
        char ch = str[idx];
        if (IsAlphabet(ch) == true || IsNumber(ch) == true || 
            ch == '_' || ch == '-' || ch == '.')
        {
            ++idx;
            continue;
        }

        if (ch == ':' || ch == '/')
        {
            return idx;
        }

        break;
    }

    if (idx == size)
    {
        //û�н������ţ���Ȼ���Ա���Ϊ�ǺϷ�
        return idx;
    }

    return -1;
}
int UrlParser::readPort(const char* str, int size)
{
    //�Ϸ��ַ���ֻ�������֣�������":"��ʼ����"/"����
    if (size <= 0)
        return -1;
    if (str[0] != ':')
        return -1;

    int idx = 1;
    while (idx < size)
    {
        char ch = str[idx];
        if (IsNumber(ch) == true)
        {
            ++idx;
            continue;
        }

        if (ch == '/')
        {
            return idx;
        }

        //�����ַ���������Ϊ��ʧ��
        return -1;
    }

    if (idx == size)
    {
        //û�н������ţ���Ȼ���Ա���Ϊ�ǺϷ�
        return idx;
    }

    return -1;
}
int UrlParser::readPath(const char* str, int size)
{
    /**
        �Ϸ��ַ��������ַ������֣�'.'��'_', '-'��':'�������'%'���������2��HEX�ַ���
        ����"/"��ʼ����"?#"������':'��Ҫ����windows��·��
    */
    if (size <= 0)
        return -1;
    if (str[0] != '/')
        return -1;

    int idx = 1;
    while (idx < size)
    {
        char ch = str[idx];
        if (IsAlphabet(ch) == true || IsNumber(ch) == true ||
            ch == '_' || ch == '-' || ch == '.' || ch == '/' || ch == ':')
        {
            ++idx;
            continue;
        }

        if (ch == '%')
        {
            int left = size - idx - 1;
            if (left < 2)
                return -1;
            if (IsHex(str[idx + 1]) == false || IsHex(str[idx + 2]) == false)
                return -1;
            idx += 3;
            continue;
        }

        if (ch == '#' || ch == '?')
        {
            return idx;
        }

        //�����ַ���������Ϊ��ʧ��
        return -1;
    }

    if (idx == size)
    {
        //û�н������ţ���Ȼ���Ա���Ϊ�ǺϷ�
        return idx;
    }

    return -1;
}
int UrlParser::readQuery(const char* str, int size)
{
    //�Ϸ��ַ��������ַ������֣�'.'��'_','-','=','&'���������'%'���������2��HEX�ַ���
    // ����"?"��ʼ����"#"����
    if (size <= 0)
        return -1;
    if (str[0] != '?')
        return -1;

    int idx = 1;
    while (idx < size)
    {
        char ch = str[idx];
        if (IsAlphabet(ch) == true || IsNumber(ch) == true ||
            ch == '_' || ch == '-' || ch == '.' || ch == '=' || ch == '&')
        {
            ++idx;
            continue;
        }

        if (ch == '%')
        {
            int left = size - idx - 1;
            if (left < 2)
                return -1;
            if (IsHex(str[idx + 1]) == false || IsHex(str[idx + 2]) == false)
                return -1;
            idx += 3;
            continue;
        }

        if (ch == '#')
        {
            return idx;
        }

        //�����ַ���������Ϊ��ʧ��
        return -1;
    }

    if (idx == size)
    {
        //û�н������ţ���Ȼ���Ա���Ϊ�ǺϷ�
        return idx;
    }

    return -1;
}

int EscapeString(const char* str, int size, unsigned char& value)
{
    if (str == NULL || size <= 2)
        return -1;
    if (str[0] != '%')
        return -1;

    unsigned char high = 0 , low = 0;
    if ((HexToInt(str[1], high) == false) || (HexToInt(str[2], low) == false))
        return -1;

    value = (high << 4) + low;
    return 3;
}
bool CalcPort(const char* str, int size, uint16_t& port)
{
    if (str == NULL || size <= 0 || size > 5)
        return false;
        
    int value = 0;
    for (int idx = 0; idx < size; ++idx)
    {
        char ch = str[idx];
        if (IsNumber(ch) == false)
            return false;

        value = value * 10 + (ch - '0');
    }

    if (value <= 0 || value >= 65536)
        return false;

    port = (uint16_t)value;
    return true;
}

int ParseProtocol(const char* str, int size)
{
    //��"://"����
    int idx = 0;
    while (idx < size)
    {
        char ch = str[idx];
        if (IsAlphabet(ch) == true || IsNumber(ch) == true || ch == '_')
        {
            ++idx;
            continue;
        }

        if (ch == ':')
        {
            //�������������"//"
            ++idx;
            int left = size - idx;
            if (left <= 2)
                break;

            if ((str[idx] == '/') && (str[idx + 1] == '/'))
            {
                //�Ϸ�����
                return (idx + 2);
            }
        }

        break;
    }

    return -1;
}
}

