
#include "qkhttp/String.h"

namespace qkhttp {

String::String():owned_(false) , buffer_(NULL) , size_(0)
{
    //
}
String::String(const String& str) 
    :owned_(false), buffer_(NULL), size_(0)
{
    if (str.owned_ == true)
        return;

    owned_ = str.owned_;
    buffer_ = str.buffer_;
    size_ = str.size_;
}
String::~String()
{
    clear();
}
bool String::refer(const char* str, int slen)
{
    clear();
    buffer_ = (char *)str;
    size_ = slen;
    owned_ = false;
    return true;
}
bool String::copy(const char* str, int slen)
{
    clear();

    if (str == NULL || slen == 0)
        return true;

    if (slen < 0)
        slen = (int)::strlen(str);

    char* sbuf = (char*)::malloc(slen);
    if (sbuf == NULL)
        return false;

    ::memcpy(sbuf, str, slen);
    buffer_ = sbuf;
    size_ = slen;
    owned_ = true;
    return true;
}
void String::clear()
{
    if (owned_ == true && buffer_ != NULL)
    {
        ::free(buffer_);
    }
    buffer_ = NULL;
    size_ = 0;
}
void String::get(const char*& str, int& slen) const
{
    str = buffer_;
    slen = size_;
}
int String::compare(const char* str, int slen) const
{
    if (str == NULL)
    {
        if (buffer_ == NULL)
            return 0;
        return 1;
    }
    else
    {
        if (buffer_ == NULL)
            return -1;
    }

    if (slen < 0)
        slen = (int)::strlen(str);

    int dstSize = slen;
    if (dstSize > size_)
        dstSize = size_;

    for (int idx = 0; idx < dstSize; ++idx)
    {
        char diff = buffer_[idx] - str[idx];
        if (diff == 0)
            continue;
        return diff;
    }

    return (size_ - slen);
}
bool String::empty() const
{
    if (buffer_ == NULL || size_ == 0)
        return true;
    else
        return false;
}

}

int LineParse(const char* str, int slen)
{
    if (str == NULL || slen == 0)
        return 0;

    if (slen < 0)
        slen = INT_MAX;

    int offset = 0 , endSize = 0;
    while (offset < slen)
    {
        char ch = str[offset];
        if (ch == '\0')
        {
            break;
        }
        else if (ch == '\n')
        {
            if (endSize != 1)
                return -1;  //Ð­Òé´íÎó

            ++endSize;
            ++offset;
            break;
        }
        
        ++offset;
        if (ch == '\r')
            endSize = 1;
    }

    return offset;
}

