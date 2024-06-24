
#ifndef QKRTL_BUFFER_H
#define QKRTL_BUFFER_H 1

#include "qkrtl/Compile.h"
#include <string.h>

namespace qkrtl {
/**
    需要特别注意，Buffer类不参与内存的分配和释放。
    Buffer不支持虚函数，所以可以等价于struct，可以用malloc/free
*/
class QKRTLAPI Buffer {
public:
    Buffer();
    Buffer(const Buffer& buffer);
    Buffer(char* buffer, int buflen);
    virtual ~Buffer();

    bool extend(int length);    //dataSize += length ;
    bool shrink(int length);    //dataSize -= length ; dataStart += length ;
    //将数据拷贝到头部，确保dataStart = 0 ;
    void squish();
    //废弃所有数据，重置头部
    void discard();
    bool refer(char * buffer , int buflen , int start = 0, int size = 0);
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
private:
    mutable char* cache_;
    int capacity_;
    int dataStart_;
    int dataSize_;
};


}

#ifdef __cplusplus
extern "C" {
#endif 

QKRTLAPI char* AllocPage(int size , int * exactSize = NULL);
QKRTLAPI void FreePage(char * addr);

#ifdef __cplusplus
}
#endif 

#endif /**QKRTL_BUFFER_H*/
