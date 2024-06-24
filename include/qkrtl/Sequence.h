
#ifndef QKRTL_SEQUENCE_H
#define QKRTL_SEQUENCE_H 1

#include "qkrtl/Compile.h"
#include "qkrtl/SystemInfo.h"
#include <atomic>

namespace qkrtl {

class Sequence {
public:
    QKRTLAPI Sequence();
    QKRTLAPI explicit Sequence(int64_t initialValue);
    QKRTLAPI virtual ~Sequence();

    QKRTLAPI int64_t tryGetValue() const;
    QKRTLAPI int64_t getValue() const ;
    QKRTLAPI void setValue(int64_t value) ;
    QKRTLAPI bool compareAndSet(int64_t expected, int64_t next);
    inline int64_t incrementAndGet() { return addAndGet(1); }
    QKRTLAPI int64_t addAndGet(int64_t value);

    inline int64_t decrementAndGet() { return subAndGet(1); }
    QKRTLAPI int64_t subAndGet(int64_t value);

    static const int kPaddingSize = SystemInfo::kCacheLine - sizeof(std::atomic<int64_t>);
private:
    char padding0[kPaddingSize] = {};
    std::atomic<int64_t> value_;    //·ÖÅä
    char padding1[kPaddingSize] = {};
};

}
#endif /**QKRTL_SEQUENCE_H*/
