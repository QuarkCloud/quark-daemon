
#ifndef QKRTL_SPINLOCK_H
#define QKRTL_SPINLOCK_H 1

#include <atomic>
#include "qkrtl/Compile.h"
#include "qkrtl/SystemInfo.h"
#include "qkrtl/Synch.h"

namespace qkrtl {

class SpinLock : public Mutex{
public:
    QKRTLAPI SpinLock();
    QKRTLAPI ~SpinLock();

    QKRTLAPI bool lock();
    QKRTLAPI void unlock();

    static const int kPaddingSize = SystemInfo::kCacheLine - sizeof(std::atomic<int64_t>);
private:
    char padding0[kPaddingSize] = {};
    std::atomic<int64_t> value_;    
    char padding1[kPaddingSize] = {};
    std::atomic<int64_t> gating_;
    char padding2[kPaddingSize] = {};
};

}
#endif /**QKRTL_SPINLOCK_H*/
