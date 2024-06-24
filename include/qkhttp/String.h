
#ifndef QKHTTP_STRING_H
#define QKHTTP_STRING_H 1

#include <stdint.h>
#include <string.h>
#include <string>
#include "qkhttp/Compile.h"

namespace qkhttp {

class QKHTTPAPI String {
public:
    String();
    String(const String& str);
    virtual ~String();

    bool refer(const char* str, int slen);
    bool copy(const char* str, int slen = -1);
    void clear();

    void get(const char*& str, int& slen) const;
    int compare(const char* str, int slen = -1) const;
    bool empty() const;

    inline const char* buffer() const { return buffer_; }
    inline int size() const { return size_; }
protected:
    bool owned_;    //owned为true，那么就需要释放
    char* buffer_;  //默认不以\0结尾，必须注意。
    int size_;
};

}

#ifdef __cplusplus
extern "C" {
#endif

/**
    这个逻辑在HTTP中核心，因为HTTP协议头的每一行都是以\r\n结束。
    整个HTTP协议头以两个\r\n终止。
    返回是\r\n末尾的字节数，包含了\r\n本身
*/
QKHTTPAPI int LineParse(const char* str, int slen = -1);

#ifdef __cplusplus
}
#endif

#endif /**QKHTTP_STRING_H*/
