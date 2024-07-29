
#ifndef LAT_QKITC_CLIENT_H
#define LAT_QKITC_CLIENT_H 1

#include <mutex>
#include <condition_variable>
#include <set>

#include "qkrtl/Buffer.h"
#include "qkrtl/Poller.h"
#include "qkrtl/Stream.h"
#include "qkrtl/StateMonitor.h"
#include "qkrtl/PerfInfo.h"
#include "qkitc/Connector.h"

/**
    �򵥵�echo�����ڲ����ӳ�
*/
class Client : public qkitc::Connector {
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
    qkrtl::TimeElapse  timeElapse_;
    int maxTimes_;
    int bufferSize_;
    int64_t expectSize_;
    int64_t readSize_;
    int64_t writedSize_;
    qkrtl::StateMonitor states_;
};

#endif /**LAT_QKITC_CLIENT_H*/
