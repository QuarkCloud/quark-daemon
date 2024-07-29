
#include <stdio.h>
#include <stdlib.h>

#include "Application.h"
#include "qkrtl/Logger.h"
#include "qkrtl/PerfInfo.h"

int main(int argc, char* argv[])
{
    GetDefaltLogger()->fileLevel(qkrtl::Logger::kMAXLEVEL);
    
    const int kDefaultPort = 12345;

    const int kMaxTimes = 1 ;
    const int kBufferMaxSize = 1 << 8;

    Application app;

    if (app.init(kDefaultPort, kMaxTimes, kBufferMaxSize) == false)
    {
        LOGERR("failed to init application");
        return -1;
    }

    app.waitForCompleted();

    int64_t elapse = app.elapse();
    app.final();

    ::printf("times[%d] Counter[%s] latency = %s \n" , kMaxTimes ,
        qkrtl::CalcCounter(elapse).c_str() ,
        qkrtl::CalcLatency(elapse , kMaxTimes).c_str());

    return 0;
}
