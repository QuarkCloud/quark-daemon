
#include <stdio.h>
#include <stdlib.h>

#include "qkrtl/Logger.h"
#include "qkrtl/PerfInfo.h"
#include "Application.h"
#include "Client.h"

int main(int argc, char* argv[])
{
    GetDefaltLogger()->fileLevel(qkrtl::Logger::kMINLEVEL);

    const int kDefaultPort = 12345;
    const int kMaxTimes = 1 << 12 ;

    qkrtl::IoService ioService("iocp");
    ioService.run();

    Application app(ioService);

    if (app.init(kDefaultPort) == false)
    {
        LOGERR("failed to init application");
        ioService.final();
        return -1;
    }

    qkrtl::TimeElapse timeElapse;

    for (int tidx = 0; tidx < kMaxTimes; ++tidx)
    {
        Client client(ioService);
        if (client.connect("127.0.0.1", kDefaultPort , 1000) == false)
        {
            LOGERR("failed to connect server");
            continue;
        }

        client.waitForCompleted();
        client.final();

        LOGCRIT("Client's Index[%d] had final" , tidx);
    }
    timeElapse.stop();

    app.final();
    ioService.final();

    std::string str = qkrtl::CalcLatency(timeElapse.elapse(), kMaxTimes);
    ::printf("latency[%s] , elapse[%s] times[%d] \n" ,
        str.c_str() , qkrtl::CalcCounter(timeElapse.elapse()).c_str(), kMaxTimes);

    return 0;
}

