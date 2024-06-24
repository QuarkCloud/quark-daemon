
#ifndef QKRTL_STREAM_H
#define QKRTL_STREAM_H 1

#include <string.h>
#include <deque>
#include <mutex>

#include "qkrtl/Compile.h"
#include "qkrtl/Buffer.h"

namespace qkrtl {
/**
    stream主要应用于单读写场景，所以对锁的要求并不严格。
    需要注意的是，需要手工释放Buffer中内存,Buffer本身也不负责释放内存。
    Stream并没有等待函数，如果需要时间驱动，需要继续优化。
    因为时间的关系，先放弃优化性能。
*/ 
class Stream {
public:
    QKRTLAPI Stream();
    QKRTLAPI virtual ~Stream();
    QKRTLAPI void close();
    QKRTLAPI bool push(const Buffer& buffer);
    QKRTLAPI bool pop(Buffer& buffer);
    QKRTLAPI bool pop(std::deque<Buffer>& buffers);
    //这个函数没有释放内存，包括整个类都没有释放内存，这些细节需要特别注意
    QKRTLAPI void clear();
    QKRTLAPI bool empty() const;
    QKRTLAPI int count() const;
    QKRTLAPI int64_t dataSize() const;
    inline bool closed() const { return closed_; }

    //积压在数组中，并不需要太多。如果积压太多，反而有问题。
    static const int kBufferSize = 256;
private:
    std::mutex popGuard_;
    Buffer buffers_[kBufferSize];
    int head_;
    int tail_;
    int64_t pushSize_;
    int64_t popSize_;

    std::mutex pushGuard_;
    std::deque<Buffer> cache_;
    bool closed_;
};

}


#endif /**QKRTL_STREAM_H*/
