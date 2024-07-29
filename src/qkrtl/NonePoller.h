
#ifndef QKRTL_NONE_POLLER_H
#define QKRTL_NONE_POLLER_H 1

#include <mutex>
#include <atomic>
#include "qkrtl/Compile.h"
#include "qkrtl/Poller.h"
#include "qkrtl/EventPoller.h"
#include "qkrtl/SpinWaiter.h"

namespace qkrtl {

class NonePoller : public PollerImpl{
public:
    NonePoller();
    virtual ~NonePoller();
    virtual bool monitor(EventNode* node);
    virtual bool unmonitor(EventNode* node);
    virtual bool post(const Event& event);
    virtual int wait(Event* events, int maxCount, int timeout = 0); //∫¡√Î
    virtual bool shutdown();
    virtual void final();
private:
    std::mutex guard_;
    bool finaled_;
    bool isShutdown_;
    SpinWaiter waiter_;
    EventPoller events_;
};

}

#endif /**QKRTL_NONE_POLLER_H*/
