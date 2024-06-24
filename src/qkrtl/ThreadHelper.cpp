

#include "qkrtl/ThreadHelper.h"
#include "qkrtl/SystemInfo.h"

namespace qkrtl {

uint32_t GetCurrentThreadId()
{
    return (uint32_t)::GetCurrentThreadId();
}
uint32_t GetCurrentProcessor()
{
    return (uint32_t)::GetCurrentProcessorNumber();
}
uint32_t GetProcessorCount()
{
    return SystemInfo::singleton().numberOfProcessors();
}
bool SetThreadAffinity(uint64_t mask)
{
    return (::SetThreadAffinityMask(::GetCurrentThread(), mask) != 0);
}
uint64_t GetThreadAffinity()
{
    DWORD_PTR temp = 1;
    DWORD_PTR currentMask = ::SetThreadAffinityMask(::GetCurrentThread(), temp);
    if (currentMask)
        ::SetThreadAffinityMask(::GetCurrentThread(), currentMask);
    return currentMask;
}

ThreadLocal::ThreadLocal():tlsKey_(TLS_OUT_OF_INDEXES)
{
    tlsKey_ = ::TlsAlloc();
}
ThreadLocal::~ThreadLocal()
{
    if (tlsKey_ != TLS_OUT_OF_INDEXES)
    {
        ::TlsFree(tlsKey_);
        tlsKey_ = TLS_OUT_OF_INDEXES;
    }
}
bool ThreadLocal::setValue(void* value)
{
    return (::TlsSetValue(tlsKey_, value) == TRUE);
}
void* ThreadLocal::getValue()
{
    return ::TlsGetValue(tlsKey_);
}

}

