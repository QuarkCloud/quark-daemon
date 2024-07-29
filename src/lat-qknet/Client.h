
#ifndef LAT_QKNET_CLIENT_H
#define LAT_QKNET_CLIENT_H 1

#include <mutex>
#include <condition_variable>
#include <set>

#include "qknet/Connector.h"
#include "qkrtl/Buffer.h"
#include "qkrtl/IoService.h"
#include "qkrtl/StateMonitor.h"
#include "qkrtl/PerfInfo.h"


class Client : public qknet::Connector {
public:
    Client(qkrtl::Poller& poller);
    virtual ~Client();
    bool init(int maxTimes, int bufferSize);
    virtual void final();
    virtual bool handleStop();
    virtual bool handleRead(qkrtl::Buffer& buffer);
    virtual bool handleWrited(qkrtl::Buffer& buffer);
    virtual bool handleOutput(int errCode = 0);
    bool waitForCompleted();

    inline int64_t elapse() const { return timeElapse_.elapse(); }

    static const int kConnectFailed = -1;
private:
    std::mutex guard_;
    bool finaled_;
    qkrtl::Buffer outBuffer_;
    qkrtl::TimeElapse  timeElapse_;
    int maxTimes_;
    int bufferSize_;
    int64_t expectSize_;
    int64_t readSize_;
    int64_t writedSize_;
    qkrtl::StateMonitor states_;
};


#endif /**LAT_QKNET_CLIENT_H*/
