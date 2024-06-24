
#ifndef FR_QKNET_APPLICATION_H
#define FR_QKNET_APPLICATION_H 1

#include <mutex>
#include <condition_variable>
#include <set>

#include "qkrtl/Buffer.h"
#include "qkrtl/IoService.h"
#include "qkrtl/PerfInfo.h"
#include "Server.h"


class Application {
public:
    Application(qkrtl::IoService& ioService);
    virtual ~Application();

    bool init(uint16_t port);
    void final();
private:
    std::mutex guard_;
    bool finaled_;
    Server server_;
};

#endif /**FR_QKNET_APPLICATION_H*/
