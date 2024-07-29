
#include "qkrtl/EventPoller.h"
#include "qkrtl/DateTime.h"
#include "qkrtl/Logger.h"
#include <thread>

namespace qkrtl {

EventNode::EventNode():nodeType_(kTypeNone) , monitor_(NULL), readyCounter_(0)
{
    //
}
EventNode::~EventNode()
{
    //
}
void EventNode::ready()
{
    int oldValue = readyCounter_.fetch_add(1, std::memory_order_release);
    if (oldValue == 0)
    {
        EventPoller* monitor = monitor_;
        if (monitor != NULL)
        {
            if (monitor->link(this) == false)
            {
                LOGERR("EventNode[%p] failed to link monitor", this);
            }
            else
            {
                LOGDEBUG("EventNode[%p] succeed to link monitor", this);
            }
        }
        else
        {
            LOGERR("EventNode[%p] has no monitor", this);
        }
    }
    else
    {
        LOGDEBUG("EventNode[%p] not link , old vlaue[%d]" , this ,oldValue);
    }
}
int EventNode::prepare() const
{
    return readyCounter_.load(std::memory_order_acquire);
}
bool EventNode::completed(int prepareValue)
{
    return readyCounter_.compare_exchange_strong(prepareValue, 0 , std::memory_order_release);
}
void EventNode::completed()
{
    readyCounter_.store(0, std::memory_order_release);
}
bool EventNode::isMonitored() const
{
    if (monitor_ == NULL)
        return false;
    return monitor_->isMonitored(this);
}
const char* EventNode::nodeTypeName(int type)
{
    static const int kNodeTypeMaxSize = 5;
    static const char* __InvalidNodeTypeName__ = "Invalid Type";
    static const char* __NodeTypeNames__[kNodeTypeMaxSize] =
    { "None" , "File" , "Sock" , "Pipe" , "Timer" };

    if (type < 0 || type >= kNodeTypeMaxSize)
        return __InvalidNodeTypeName__;
    else
        return __NodeTypeNames__[type];
}

EventPoller::EventPoller()
{
    init(kMaxNodeSize);
}
EventPoller::~EventPoller()
{
    final();
}
void EventPoller::final()
{
    std::unique_lock<std::mutex> locker(guard_);
    if (finaled_ == true)
        return;
    
    EventQueue::final();

    if (nodes_.empty() == false)
        nodes_.clear();
}
bool EventPoller::monitor(EventNode* node)
{
    if (node == NULL)
        return false;

    std::unique_lock<std::mutex> locker(guard_);
    std::set<EventNode*>::iterator niter = nodes_.find(node);
    if (niter != nodes_.end())
    {
        LOGDEBUG("EventNode[%p] nodeType[%s] had monitored at[%p]" , 
            node , EventNode::nodeTypeName(node->nodeType()) , node->monitor_);
        return (node->monitor_ == this);
    }

    node->monitor_ = this;
    LOGDEBUG("EventNode[%p] nodeType[%s] will be monitored at[%p]",
        node, EventNode::nodeTypeName(node->nodeType()), node->monitor_);


    nodes_.insert(node);
    return true;
}
bool EventPoller::unmonitor(EventNode* node)
{
    if (node == NULL || node->monitor_ != this)
        return false;

    std::unique_lock<std::mutex> locker(guard_);
    LOGDEBUG("EventNode[%p] nodeType[%s] will be unmonitored at[%p]", 
        node, EventNode::nodeTypeName(node->nodeType()), node->monitor_);
    std::set<EventNode*>::iterator niter = nodes_.find(node);
    if (niter == nodes_.end())
    {
        if (node->monitor_ == this)
            node->monitor_ = NULL;
        return true;
    }
    nodes_.erase(niter);
    node->monitor_ = NULL;

    int64_t sequence = node->sequence();
    if (sequence != EventObject::kInvalidSequence)
        cancel(sequence);

    return true;
}
bool EventPoller::isMonitored(const EventNode* node) const
{
    if (node == NULL)
        return false;
    return (node->monitor_ == this);
}
bool EventPoller::link(EventNode* node)
{
    if(node == NULL || node->monitor_ != this)
        return false;

    Event evt = Event::Initializer; 

    evt.dataType = Event::kDataTypeEvt;
    evt.data.event = node;
    int64_t sequence = EventQueue::post(evt); 
    if (sequence < 0)
        return false;
    node->sequence(sequence);
    return true;
}

}
