
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
    bool owned_;    //ownedΪtrue����ô����Ҫ�ͷ�
    char* buffer_;  //Ĭ�ϲ���\0��β������ע�⡣
    int size_;
};

}

#ifdef __cplusplus
extern "C" {
#endif

/**
    ����߼���HTTP�к��ģ���ΪHTTPЭ��ͷ��ÿһ�ж�����\r\n������
    ����HTTPЭ��ͷ������\r\n��ֹ��
    ������\r\nĩβ���ֽ�����������\r\n����
*/
QKHTTPAPI int LineParse(const char* str, int slen = -1);

#ifdef __cplusplus
}
#endif

#endif /**QKHTTP_STRING_H*/
