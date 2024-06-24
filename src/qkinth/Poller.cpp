
#include "qkrtl/Logger.h"
#include "qkinth/Poller.h"

namespace qkinth {

Poller::Poller():finaled_(false) , isShutdown_(false)
{
    events_.setWaiter(this);
}
Poller::~Poller()
{
    final();
}
bool Poller::monitor(qkrtl::EventNode* node)
{
    if (node == NULL)
        return false;

    int nodeType = node->nodeType();
    if (nodeType == qkrtl::EventNode::kTypeTimer)
    {
        qkrtl::TimerTask* timer = (qkrtl::TimerTask*)node;
        events_.monitor(timer);
        return timer_.add(timer);
    }
    else if(nodeType == qkrtl::EventNode::kTypeNone)
    {
        return events_.monitor(node);
    }
    return false;
}
bool Poller::unmonitor(qkrtl::EventNode* node)
{
    if (node == NULL)
        return false;

    if (events_.unmonitor(node) == false)
        return false;

    int nodeType = node->nodeType();
    if (nodeType == qkrtl::EventNode::kTypeTimer)
    {
        qkrtl::TimerTask* timer = (qkrtl::TimerTask*)node;
        return timer_.remove(timer);
    }
    else if(nodeType == qkrtl::EventNode::kTypeNone)
    {
        return events_.monitor(node);
    }

    return false;
}
bool Poller::post(const qkrtl::Event& event)
{
    return events_.post(event);
}
int Poller::wait(qkrtl::Event* events, int maxCount, int timeout)
{
    if (isShutdown_ == true || finaled_ == true)
        return 0;

    waitFor(timeout);
    timer_.process();

    return events_.peek(events, maxCount);
}

bool Poller::shutdown()
{
    std::unique_lock<std::mutex> locker(guard_);
    if (isShutdown_ == true)
        return true;
    isShutdown_ = true;
    notifyOne();
    return true;
}
void Poller::final()
{
    std::unique_lock<std::mutex> locker(guard_);
    if (finaled_ == true)
        return;
    finaled_ = true;
    isShutdown_ = true;
    timer_.final();
    events_.final();
}
bool Poller::waitFor(int timeout)
{
    int waiters = waiterCount();
    if (waiters <= 0 && events_.backlog() > 0)
    {
        timeout = 0;
    }

    return SpinWaiter::waitFor(timeout);
}

class PollerlCreator : public qkrtl::PollerImplCreator{
public:
    virtual qkrtl::PollerImpl* operator()(void* param)
    {
        return new Poller();
    }
};

static qkrtl::PollerImplCreatorAutoRegister __CreatorRegister__("inth" , new PollerlCreator());

}
