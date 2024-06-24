
#ifndef LAT_QKINTH_CLIENT_H
#define LAT_QKINTH_CLIENT_H 1

#include <mutex>
#include <condition_variable>
#include <set>

#include "qkrtl/Buffer.h"
#include "qkrtl/Poller.h"
#include "qkrtl/Stream.h"
#include "qkrtl/StateMonitor.h"
#include "qkrtl/PerfInfo.h"
#include "qkinth/Connection.h"

/**
    简单的echo，用于测试延迟
*/
class Client : public qkinth::Connection {
public:
    Client(qkrtl::Poller& poller);
    virtual ~Client();
    bool init(int maxTimes, int bufferSize);
    void final();
    virtual bool handleInput(int errCode = 0);
    virtual bool handleOutput(int errCode = 0);

    bool waitForCompleted();
    inline int64_t elapse() const { return timeElapse_.elapse(); }
private:
    std::mutex guard_;
    bool finaled_;
    qkrtl::Buffer inBuffer_;
    qkrtl::Buffer outBuffer_;
    qkrtl::TimeElapse  timeElapse_;
    int maxTimes_;
    int bufferSize_;
    int64_t expectSize_;
    int64_t readSize_;
    int64_t writedSize_;
    qkrtl::StateMonitor states_;
};

#endif /**LAT_QKINTH_CLIENT_H*/
