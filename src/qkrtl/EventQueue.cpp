
#include "qkrtl/EventQueue.h"

namespace qkrtl {

EventQueue::EventQueue():finaled_(false), waiter_(NULL)
{
    //
}
EventQueue::~EventQueue()
{
    final();
}
bool EventQueue::init(int maxSize)
{
    if (events_.init(maxSize) == false)
        return false;

    index_.ringSize(events_.dataSize());
    return true;
}
void EventQueue::final()
{
    if (finaled_ == true)
        return;
    finaled_ = true;

    if(waiter_ != NULL)
        waiter_->notifyAll();
    events_.final();
}
int64_t EventQueue::post(const Event& event)
{
    //if (finaled_ == true)
    //    return -1;

    int64_t sequence = 0;
    if (index_.nextTail(sequence) <= 0)
        return -1;
    int index = events_.calcIndex(sequence);
    Event& evt = events_.get(index);
    evt = event;
    if(event.status == Event::kStatusNone)
        evt.status = Event::kStatusReady;

    if (index_.commitTail(sequence) == false)
    {
        //可能有多个线程同时处理，需要善后处理，再尝试一次
        int64_t trySequence = index_.getTail().getValue();
        int64_t gatingValue = index_.getTail().getGating();
        index = events_.calcIndex(trySequence);
        int dataSize = events_.dataSize();
        int maxSize = 0;
        for (int64_t idx = trySequence; idx < gatingValue; ++idx , ++index)
        {
            if (index >= dataSize)
                index = 0;
            if (events_.get(index).status == Event::kStatusNone)
                break;

            ++maxSize;
        }
        index_.commitTail(trySequence, maxSize);
    }
    if(waiter_ != NULL)
        waiter_->notifyOne();
    return sequence;
}
bool EventQueue::cancel(int64_t sequence)
{
    if (finaled_ == true)
        return false;

    int64_t head = index_.getHead().tryGetValue();
    int64_t tail = index_.getTail().tryGetGating();

    if (head > sequence || tail <= sequence)
        return false;

    int index = events_.calcIndex(sequence);
    Event& evt = events_.get(index);
    evt.status = Event::kStatusNone;
    return true;
}
bool EventQueue::peek(Event& event)
{
    //if (finaled_ == true)
    //    return false;

    int64_t sequence = 0;
    if (index_.nextHead(sequence) <= 0)
        return false;

    int index = events_.calcIndex(sequence);
    Event& evt = events_.get(index);
    event = evt;
    evt.status = Event::kStatusNone;
    if (index_.commitHead(sequence) == false)
    {
        //可能有多个线程同时处理，需要善后处理，再尝试一次
        sequence = index_.getHead().getValue();
        int64_t gatingValue = index_.getHead().getGating();
        index = events_.calcIndex(sequence);
        int dataSize = events_.dataSize();
        int maxSize = 0;
        for (int64_t idx = sequence; idx < gatingValue; ++idx, ++index)
        {
            if (index >= dataSize)
                index = 0;
            if (events_.get(index).status != Event::kStatusNone)
                break;

            ++maxSize;
        }
        index_.commitHead(sequence, maxSize);
    }
    
    return true;
}
int EventQueue::peek(Event* events, int maxCount)
{
    //if (finaled_ == true)
    //    return -1;

    int64_t sequence = 0;
    int maxSize = 0;
    if ((maxSize = index_.nextHead(sequence , maxCount , false)) <= 0)
        return 0;

    int index = events_.calcIndex(sequence);
    int dataSize = events_.dataSize();
    for (int eidx = 0; eidx < maxSize; ++eidx , ++index)
    {
        if (index >= dataSize)
            index = 0;
        Event& evt = events_.get(index);
        events[eidx] = evt;
        evt.status = Event::kStatusNone;
    }
    if (index_.commitHead(sequence , maxSize) == false)
    {
        //可能有多个线程同时处理，需要善后处理，再尝试一次
        sequence = index_.getHead().getValue();
        int64_t gatingValue = index_.getHead().getGating();
        index = events_.calcIndex(sequence);
        int totalSize = 0;
        for (int64_t idx = sequence; idx < gatingValue; ++idx, ++index)
        {
            if (index >= dataSize)
                index = 0;
            if (events_.get(index).status != Event::kStatusNone)
                break;

            ++totalSize;
        }
        index_.commitHead(sequence, totalSize);
    }

    return maxSize;
}
bool EventQueue::wait(int timeout)
{
    if (waiter_ == NULL)
        return false;

    return waiter_->waitFor(timeout);
}

void EventQueue::setWaiter(Waiter* waiter)
{
    waiter_ = waiter;
}


}

