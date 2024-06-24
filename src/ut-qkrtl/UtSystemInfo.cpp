
#include "qkrtl/TestKits.h"
#include "qkrtl/SystemInfo.h"

bool testSystemInfo()
{
    qkrtl::SystemInfo sysinfo;
    sysinfo.load();

    ::printf("numberOfProcessors[%u] pageSize[%u] \n",
        sysinfo.numberOfProcessors(), sysinfo.pageSize());

    ::printf("allocationGranularity[%u] processorMask[%llu] \n",
        sysinfo.allocationGranularity(), sysinfo.processorMask());

    return true;
}

//TESTCASE(testSystemInfo);