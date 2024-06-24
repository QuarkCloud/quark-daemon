
#ifndef QKRTL_IOCP_POLLER_H
#define QKRTL_IOCP_POLLER_H 1

#include "qkrtl/Compile.h"
#include "qkrtl/Poller.h"
#include "qkrtl/EventPoller.h"
#include "qkrtl/Timer.h"
#include <mutex>
#include <condition_variable>
#include <map>
#include <atomic>

namespace qkrtl {

class IocpPoller : public PollerImpl , public Waiter{
public:
    IocpPoller();
    virtual ~IocpPoller();
    virtual bool monitor(EventNode* node);
    virtual bool unmonitor(EventNode* node);
    virtual bool post(const Event& event);
    virtual int wait(Event* events, int maxCount, int timeout = 0); //毫秒
    virtual bool shutdown();
    virtual void final();

    //Waiter的实现
    virtual bool waitFor(int timeout); //毫秒级
    virtual void notifyOne();
    virtual void notifyAll();
    virtual int waiterCount() const;

private:
    std::mutex guard_;
    std::condition_variable notifier_;

    bool finaled_;
    std::map<HANDLE , int> handles_;
    HANDLE iocp_;
    std::atomic<int> threadsRunning_;
    bool isShutdown_;
    Timer timer_;
    EventPoller events_;
};

bool UnbindIoCompletionPort(HANDLE fileHandle);
}

#endif /**QKRTL_IOCP_POLLER_H*/
