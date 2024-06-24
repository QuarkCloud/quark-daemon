
#ifndef QKRTL_THREAD_HELPER_H
#define QKRTL_THREAD_HELPER_H 1

#include "qkrtl/Compile.h"

namespace qkrtl {

QKRTLAPI uint32_t GetCurrentThreadId();

QKRTLAPI uint32_t GetCurrentProcessor();
QKRTLAPI uint32_t GetProcessorCount();

QKRTLAPI bool SetThreadAffinity(uint64_t mask);
QKRTLAPI uint64_t GetThreadAffinity();

class QKRTLAPI ThreadLocal {
public:
    ThreadLocal();
    virtual ~ThreadLocal();

    bool setValue(void* value);
    void * getValue();

private:
    DWORD tlsKey_;
};

}
#endif /**QKRTL_THREAD_HELPER_H*/
