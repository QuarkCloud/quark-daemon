
#include "qkrtl/String.h"

namespace qkrtl {

String::String()
{
    //
}
String::String(const String& str) 
{
    Buffer::refer(str);
}
String::~String()
{
    clear();
}
bool String::refer(const char* str, int slen)
{
    return Buffer::refer(str, slen , 0 , slen);
}
bool String::copy(const char* str, int slen)
{
    bool enough = false;
    if (status() == kStatusOwned && capacity() >= slen)
    {
        enough = true;
    }
    if (enough == false)
    {
        free();
        if (malloc(slen) == false)
            return false;
    }

    if (str == NULL || slen == 0)
        return true;

    char* sbuf = cache();
    if (sbuf == NULL)
        return false;
    ::memcpy(sbuf, str, slen);
    return extend(slen);
}
void String::clear()
{
    free();
}
void String::get(const char*& str, int& slen) const
{
    str = head();
    slen = dataSize();
}
int String::compare(const char* str, int slen) const
{
    const char* buffer = head();
    int size = dataSize();
    if (str == NULL)
    {
        if (buffer == NULL)
            return 0;
        return 1;
    }
    else
    {
        if (buffer == NULL)
            return -1;
    }

    if (slen < 0)
        slen = (int)::strlen(str);

    int dstSize = slen;
    if (dstSize > size)
        dstSize = size;

    for (int idx = 0; idx < dstSize; ++idx)
    {
        char diff = buffer[idx] - str[idx];
        if (diff == 0)
            continue;
        return diff;
    }

    return (size - slen);
}

bool IsAlphabet(const char ch)
{
    return ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z'));
}
bool IsNumber(const char ch)
{
    return (ch >= '0' && ch <= '9');
}
bool IsHex(const char ch)
{
    return ((ch >= 'A' && ch <= 'F') || (ch >= 'a' && ch <= 'f') || (ch >= '0' && ch <= '9'));
}
bool HexToInt(const char ch, uint8_t& value)
{
    if (ch >= '0' && ch <= '9')
    {
        value = (uint8_t)(ch - '0');
    }
    else if (ch >= 'A' && ch <= 'F')
    {
        value = (uint8_t)(ch - 'A') + 10;
    }
    else if (ch >= 'a' && ch <= 'f')
    {
        value = (uint8_t)(ch - 'a') + 10;
    }
    else
        return false;

    return true;
}
}

