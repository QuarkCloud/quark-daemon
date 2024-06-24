
#include "qkrtl/Event.h"
#include "qkrtl/EventQueue.h"
#include "qkrtl/Logger.h"

namespace qkrtl {

EventObject::EventObject():sequence_(kInvalidSequence)
{
    //    
}
EventObject::~EventObject()
{
    //
}
void EventObject::execute()
{

}
const Event Event::Initializer = { 0 , kEventTypeNone , kStatusNone , kDataTypeNone , {0} , NULL };

bool ExecuteEvent(const Event& event)
{
    if (event.status == Event::kStatusNone)
    {
        LOGERR("event[%p]'s status is invalid" , &event);
        return false;
    }

    EventExecutor executor = event.executor;
    if (executor != NULL)
    {
        LOGDEBUG("event[%p] has executor", &event);
        return executor(event);
    }

    if (event.dataType == Event::kDataTypeObj)
    {
        Object* obj = event.data.object;
        if (obj == NULL)
        {
            LOGERR("Event[%p]'s object data is NULL" , &event);
            return false;
        }
        if (event.status == Event::kStatusFree)
        {
            LOGDEBUG("event[%p] is object type , [%p] will be freed", &event , obj);
            delete obj;
        }
    }
    if (event.dataType == Event::kDataTypeEvt)
    {
        EventObject* evt = event.data.event;
        if (evt == NULL)
        {
            LOGERR("Event[%p]'s event data is NULL", &event);
            return false;
        }
        if (event.status == Event::kStatusFree)
        {
            LOGDEBUG("event[%p] is event type , [%p] will be freed", &event, evt);
            delete evt;
        }
        else
        {
            LOGDEBUG("event[%p] is event type , has executor [%p]", &event, evt);
            evt->sequence(EventObject::kInvalidSequence);
            evt->execute();
        }
    }
    else
    {
        LOGERR("Event[%p]'s dataType[%d] is invalid", &event, event.dataType);
    }

    return true;
}

class EventQueueInitialer {
public:
    EventQueueInitialer() :inited_(false), defaultQueue_(NULL)
    {
        //
    }
    virtual ~EventQueueInitialer()
    {
        std::unique_lock<std::mutex> locker(guard_);
        if (inited_ == false)
            return;
        inited_ = false;
        if (defaultQueue_ == NULL)
            return;

        defaultQueue_->final();
        delete defaultQueue_;
        defaultQueue_ = NULL;
    }
    EventQueue* getDefaultQueue()
    {
        if (inited_ == false)
        {
            std::unique_lock<std::mutex> locker(guard_);
            if (defaultQueue_ == NULL)
            {
                defaultQueue_ = new EventQueue();
                defaultQueue_->init(1 << 10);
                inited_ = true;
            }
        }

        return defaultQueue_;
    }
private:
    std::mutex guard_;
    bool inited_;
    EventQueue* defaultQueue_;
};

static EventQueueInitialer __EventQueueInitialer__;

bool PostEvent(const Event& event)
{
    EventQueue* defaultQueue = __EventQueueInitialer__.getDefaultQueue();
    if (defaultQueue == NULL)
        return false;
    return (defaultQueue->post(event) >= 0);
}
bool PeekEvent(Event& event)
{
    EventQueue* defaultQueue = __EventQueueInitialer__.getDefaultQueue();
    if (defaultQueue == NULL)
        return false;
    return defaultQueue->peek(event);
}
int PeekEvent(Event* events, int maxCount)
{
    EventQueue* defaultQueue = __EventQueueInitialer__.getDefaultQueue();
    if (defaultQueue == NULL)
        return -1;
    return defaultQueue->peek(events , maxCount);
}
bool WaitEvent(int timeout)
{
    EventQueue* defaultQueue = __EventQueueInitialer__.getDefaultQueue();
    if (defaultQueue == NULL)
        return false;
    return defaultQueue->wait(timeout);
}

}


