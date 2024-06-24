
#include <windows.h>
#include "qkrtl/SystemInfo.h"

namespace qkrtl {


SystemInfo::SystemInfo(): numberOfProcessors_(0) , pageSize_(0) ,
    allocationGranularity_(0) , cacheLine_(kCacheLine) , processorMask_(0)
{
    //
}
SystemInfo::~SystemInfo()
{
    //
}
void SystemInfo::load()
{
    SYSTEM_INFO info;
    ::GetSystemInfo(&info);

    numberOfProcessors_ = info.dwNumberOfProcessors;
    pageSize_ = info.dwPageSize;
    allocationGranularity_ = info.dwAllocationGranularity;
    processorMask_ = info.dwActiveProcessorMask;
}
static bool __SystemInfoSingletonInited__ = false;
static SystemInfo __SystemInfoSingleton__;
SystemInfo& SystemInfo::singleton()
{
    if (__SystemInfoSingletonInited__ == false)
    {
        __SystemInfoSingleton__.load();
        __SystemInfoSingletonInited__ = true;
    }
    return __SystemInfoSingleton__;
}

}
