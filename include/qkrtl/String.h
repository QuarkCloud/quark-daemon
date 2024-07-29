
#ifndef QKRTL_STRING_H
#define QKRTL_STRING_H 1

#include <stdint.h>
#include <string.h>
#include <string>
#include "qkrtl/Compile.h"
#include "qkrtl/Buffer.h"

namespace qkrtl {

/**
    字符串默认不以\0结尾，必须注意。
*/
class QKRTLAPI String : public Buffer{
public:
    String();
    String(const String& str);
    virtual ~String();

    bool refer(const char* str, int slen);
    //拷贝内容，强制分配内存
    bool copy(const char* str, int slen = -1);
    void clear();

    void get(const char*& str, int& slen) const;
    int compare(const char* str, int slen = -1) const;

    inline const char* buffer() const { return head(); }
    inline int size() const { return dataSize(); }
};

QKRTLAPI bool IsAlphabet(const char ch);
QKRTLAPI bool IsNumber(const char ch);
QKRTLAPI bool IsHex(const char ch);
QKRTLAPI bool HexToInt(const char ch, uint8_t& value);

}


#endif /**QKRTL_STRING_H*/
