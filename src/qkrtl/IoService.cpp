
#include "qkrtl/IoService.h"
#include "qkrtl/Logger.h"

namespace qkrtl {

IoService::IoService(const std::string& name)
    : Poller(name) , finaled_(false)
{
    //
}
IoService::~IoService()
{
    final();
}
void IoService::run(bool standalone)
{
    if (standalone == true)
    {
        std::unique_lock<std::mutex> locker(guard_);
        worker_ = std::thread(&IoService::process, this);
        counter_.waitForCompleted(1, 1000);
    }
    else
    {
        process();
    }
}
void IoService::final()
{
    LOGCRIT("IoService[%p] will final , now is finaled[%s]" , this , finaled_?"True":"False");
    std::unique_lock<std::mutex> locker(guard_);
    if (finaled_ == true)
        return;
    finaled_ = true;

    Poller::shutdown();

    if (worker_.joinable() == true)
        worker_.join();

    Poller::final();
    LOGCRIT("IoService[%p] had finaled", this);
}
void IoService::process()
{
    counter_.addCounter();
    bool completed = false;
    while (completed == false)
    {
        const int kMaxEventSize = 32;
        Event events[kMaxEventSize];
        int maxSize = wait(events, kMaxEventSize, 1000);
        if (maxSize <= 0)
        {
            if (finaled_ == true)
                completed = true;
            continue;
        }

        LOGDEBUG("IoService[%p] get [%d] events" , this , maxSize);
        for (int eidx = 0; eidx < maxSize; ++eidx)
        {
            Event& event = events[eidx];

            ExecuteEvent(event);
        }
    }
    LOGCRIT("IoService[%p]'s worker completed", this);
}
}
