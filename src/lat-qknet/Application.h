
#ifndef LAT_QKNET_APPLICATION_H
#define LAT_QKNET_APPLICATION_H 1

#include <mutex>
#include <condition_variable>
#include <set>

#include "qkrtl/Buffer.h"
#include "qkrtl/IoService.h"
#include "qkrtl/PerfInfo.h"
#include "Client.h"
#include "Server.h"


class Application {
public:
    Application();
    virtual ~Application();

    bool init(uint16_t port , int maxTimes, int bufferSize);
    void final();
    bool waitForCompleted();
    inline int64_t elapse() const { return client_.elapse(); }
private:
    std::mutex guard_;
    bool finaled_;
    qkrtl::IoService ioService_;
    Server server_;
    Client client_;
};

#endif /**LAT_QKNET_APPLICATION_H*/
