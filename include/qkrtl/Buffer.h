
#ifndef QKRTL_BUFFER_H
#define QKRTL_BUFFER_H 1

#include "qkrtl/Compile.h"
#include <string.h>

namespace qkrtl {
/**
    �麯����ռ8���ֽڣ�����������ֽ���������32���ֽڣ�
    8 + 8 + 4 + 4 + 4 + 4
*/
class QKRTLAPI Buffer {
public:
    Buffer();
    Buffer(const Buffer& buffer);
    Buffer(char* buffer, int buflen);
    virtual ~Buffer();

    bool extend(int length);    //dataSize += length ;
    bool shrink(int length);    //dataSize -= length ; dataStart += length ;
    //�����ݿ�����ͷ����ȷ��dataStart = 0 ;
    void squish();
    //�����������ݣ�����ͷ��
    void discard();
    bool refer(const char * buffer , int buflen , int start = 0, int size = 0);
    bool refer(const Buffer& buffer);
    bool move(Buffer& buffer);
    void clear();
    int append(const char* buffer, int size);
    void swap(Buffer& buffer);

    const char* head() const;
    char* head();
    const char* tail() const;
    char* tail();

    inline char* cache() { return cache_; }
    inline const char* cache() const { return cache_; }
    inline int capacity() const { return capacity_; }
    inline int dataStart() const { return dataStart_; }
    inline int dataSize() const { return dataSize_; }

    inline int avaibleSize() const { return (capacity_ - (dataStart_ + dataSize_)); }
    inline bool empty() const { return (dataSize_ == 0); }
    inline bool full() const { return (dataSize_ == capacity_); }
    inline bool assigned() const { return (cache_ != NULL && capacity_ != 0); }

    bool malloc(int size);
    void free();

    static const int kStatusNone = 0;
    static const int kStatusOwned = 1;

    inline int status() const { return status_; }
private:
    mutable char* cache_;
    int capacity_;
    int dataStart_;
    int dataSize_;
    int status_;    //Ĭ��Ϊ0���������ڴ棬Ҳ���Զ��ͷ��ڴ棬������Էǳ���Ҫ��
};

class Allocator {
public:
    QKRTLAPI Allocator();
    QKRTLAPI virtual ~Allocator();

    QKRTLAPI bool alloc(Buffer& buffer, int size);
    QKRTLAPI void free(Buffer& buffer);
private:
    //
};

}

#endif /**QKRTL_BUFFER_H*/
