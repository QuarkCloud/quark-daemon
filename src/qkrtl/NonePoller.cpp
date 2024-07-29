
#include "qkrtl/Logger.h"
#include "qkrtl/IoEvent.h"
#include "qkrtl/IoHandle.h"
#include "NonePoller.h"

namespace qkrtl {

NonePoller::NonePoller()
    :finaled_(false) ,  isShutdown_(false)
{
    events_.setWaiter(&waiter_);
}
NonePoller::~NonePoller()
{
    final();
}
bool NonePoller::monitor(EventNode* node)
{
    if (node == NULL)
        return false;

    int nodeType = node->nodeType();
    if (nodeType != EventNode::kTypeNone)
        return false;
    return events_.monitor(node);
}

bool NonePoller::unmonitor(EventNode* node)
{
    if (node == NULL)
        return false;

    int nodeType = node->nodeType();
    if (nodeType != EventNode::kTypeNone)
        return false;

    if (events_.unmonitor(node) == false)
    {
        LOGERR("node[%p] failed to unmonitored , nodeType[%s]" , 
            node , EventNode::nodeTypeName(nodeType));
        return false;
    }
    LOGINFO("node[%p] will unmonitor , nodeType[%s]",
        node, EventNode::nodeTypeName(nodeType));

    return events_.unmonitor(node);
}
bool NonePoller::post(const Event& event)
{
    return events_.post(event);
}
int NonePoller::wait(Event* events, int maxCount, int timeout)
{
    if(events_.backlog() <= 0)
        waiter_.waitFor(timeout);
    return events_.peek(events, maxCount);
}
bool NonePoller::shutdown()
{
    std::unique_lock<std::mutex> locker(guard_);
    if (isShutdown_ == true)
        return true;
    isShutdown_ = true;
    waiter_.notifyAll();
    return true;
}
void NonePoller::final()
{
    std::unique_lock<std::mutex> locker(guard_);
    if (finaled_ == true)
        return;
    finaled_ = true;
    isShutdown_ = true;
    events_.final();
}

class NonePollerlCreator : public PollerImplCreator{
public:
    virtual PollerImpl* operator()(void* param)
    {
        return new NonePoller();
    }
};

static PollerImplCreatorAutoRegister __NoneCreatorRegister__("none" , new NonePollerlCreator());

}
