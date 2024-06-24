
#ifndef QKINTH_POLLER_H
#define QKINTH_POLLER_H 1

#include <mutex>
#include <set>
#include <atomic>
#include "qkrtl/Poller.h"
#include "qkrtl/EventPoller.h"
#include "qkrtl/Timer.h"
#include "qkrtl/SpinWaiter.h"
#include "qkinth/Compile.h"

namespace qkinth {

class Poller : public qkrtl::PollerImpl , public qkrtl::SpinWaiter {
public:
    QKINTHAPI Poller();
    QKINTHAPI virtual ~Poller();
    QKINTHAPI virtual bool monitor(qkrtl::EventNode* node);
    QKINTHAPI virtual bool unmonitor(qkrtl::EventNode* node);
    QKINTHAPI virtual bool post(const qkrtl::Event& event);
    QKINTHAPI virtual int wait(qkrtl::Event* events, int maxCount, int timeout = 0); //∫¡√Î
    QKINTHAPI virtual bool shutdown();
    QKINTHAPI virtual void final();

    QKINTHAPI virtual bool waitFor(int timeout);
private:
    std::mutex guard_;
    bool finaled_;
    SpinWaiter waiter_;
    bool isShutdown_;
    qkrtl::Timer timer_;
    qkrtl::EventPoller events_;
};

}

#endif /**QKINTH_POLLER_H*/
