
#ifndef QKRTL_STREAM_H
#define QKRTL_STREAM_H 1

#include <string.h>
#include <deque>
#include <mutex>

#include "qkrtl/Compile.h"
#include "qkrtl/Buffer.h"

namespace qkrtl {
/**
    stream��ҪӦ���ڵ���д���������Զ�����Ҫ�󲢲��ϸ�
    ��Ҫע����ǣ���Ҫ�ֹ��ͷ�Buffer���ڴ�,Buffer����Ҳ�������ͷ��ڴ档
    Stream��û�еȴ������������Ҫʱ����������Ҫ�����Ż���
    ��Ϊʱ��Ĺ�ϵ���ȷ����Ż����ܡ�
*/ 
class Stream {
public:
    QKRTLAPI Stream();
    QKRTLAPI virtual ~Stream();
    QKRTLAPI void close();
    QKRTLAPI bool push(const Buffer& buffer);
    QKRTLAPI bool pop(Buffer& buffer);
    QKRTLAPI bool pop(std::deque<Buffer>& buffers);
    //�������û���ͷ��ڴ棬���������඼û���ͷ��ڴ棬��Щϸ����Ҫ�ر�ע��
    QKRTLAPI void clear();
    QKRTLAPI bool empty() const;
    QKRTLAPI int count() const;
    QKRTLAPI int64_t dataSize() const;
    inline bool closed() const { return closed_; }

    //��ѹ�������У�������Ҫ̫�ࡣ�����ѹ̫�࣬���������⡣
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
