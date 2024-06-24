
#include "qkrtl/StateMonitor.h"
#include "qkrtl/DateTime.h"

namespace qkrtl {

StateMonitor::StateMonitor():finaled_(false)
{
    //
}
StateMonitor::~StateMonitor()
{
    final();
}
bool StateMonitor::init(const std::set<int>& states)
{
    std::unique_lock<std::mutex> locker(guard_);
    states_ = states;
    return true;
}
void StateMonitor::final()
{
    std::unique_lock<std::mutex> locker(guard_);
    if (finaled_ == true)
        return;
    finaled_ = true;
    notifier_.notify_all();
}
bool StateMonitor::wait(const std::set<int>& states)
{
    std::unique_lock<std::mutex> locker(guard_);
    while (finaled_ == false)
    {
        if(intersection(states, states_) == true)
            return true;
        notifier_.wait(locker);
    }

    return false; 
}
bool StateMonitor::wait(int state)
{
    std::unique_lock<std::mutex> locker(guard_);
    while (finaled_ == false)
    {
        if (states_.find(state) != states_.end())
            return true;
        notifier_.wait(locker);
    }

    return false;
}
bool StateMonitor::waitFor(const std::set<int>& states, int timeout)
{
    int64_t endTime = 0;
    if (timeout < 0)
        endTime = std::numeric_limits<int64_t>::max();
    else
        endTime = UTimeNow() + timeout * 1000;
    std::unique_lock<std::mutex> locker(guard_);
    while (finaled_ == false)
    {
        if (intersection(states, states_) == true)
            return true;
        if (endTime <= UTimeNow())
            break;

        notifier_.wait_for(locker , std::chrono::milliseconds(1));
    }

    return false;
}
bool StateMonitor::waitFor(int state, int timeout)
{
    int64_t endTime = 0;
    if (timeout < 0)
        endTime = std::numeric_limits<int64_t>::max();
    else
        endTime = UTimeNow() + timeout * 1000;
    std::unique_lock<std::mutex> locker(guard_);
    while (finaled_ == false)
    {
        if (states_.find(state) != states_.end())
            return true;
        if (endTime <= UTimeNow())
            break;

        notifier_.wait_for(locker, std::chrono::milliseconds(1));
    }

    return false;
}
void StateMonitor::notify(int state)
{
    std::unique_lock<std::mutex> locker(guard_);
    states_.insert(state);
    notifier_.notify_all();    
}
void StateMonitor::clear()
{
    std::unique_lock<std::mutex> locker(guard_);
    states_.clear();
    notifier_.notify_all();
}
bool StateMonitor::intersection(const std::set<int>& src, const std::set<int>& dst) const
{
    std::set<int>::const_iterator siter = src.begin();
    std::set<int>::const_iterator diter = dst.begin();

    bool result = false;

    while (siter != src.end() && diter != dst.end())
    {
        int svalue = (*siter);
        int dvalue = (*diter);

        if (svalue == dvalue)
            return true;

        if (svalue < dvalue)
        {
            ++siter;
            continue;
        }

        do
        {
            ++diter;
            if (diter == dst.end())
                return false;
            dvalue = (*diter);
        } while (svalue > dvalue);
    }

    return false;
}

CounterMonitor::CounterMonitor()
    :finaled_(false) , counter_(0) , maxCounter_(INT_MAX)
{
    //
}
void CounterMonitor::final()
{
    std::unique_lock<std::mutex> locker(guard_);
    if (finaled_ == true)
        return;
    finaled_ = true;
    //Ç¿ÖÆ»½ÐÑ
    notifier_.notify_one();
}
CounterMonitor::~CounterMonitor()
{
    final();
}
bool CounterMonitor::addCounter(int counter)
{
    std::unique_lock<std::mutex> locker(guard_);
    counter_ += counter;
    if (completed() == true)
    {
        notifier_.notify_all();
    }
    return true;
}
bool CounterMonitor::waitForCompleted(int maxCounter, int timeout)
{
    std::unique_lock<std::mutex> locker(guard_);
    maxCounter_ = maxCounter;
    if (timeout == 0)
        return completed();

    std::chrono::steady_clock::time_point tp;
    if (timeout > 0)
        tp = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeout);

    while ((completed() == false) && (finaled_ == false))
    {
        if (notifier_.wait_for(locker, std::chrono::milliseconds(1)) == std::cv_status::no_timeout)
            continue;

        if (timeout > 0)
        {
            if (std::chrono::steady_clock::now() >= tp)
                break;
        }
    }
    return completed();
}

StateMachine::StateMachine()
{
    //
}
StateMachine::~StateMachine()
{
    //
}
bool StateMachine::init(const std::set<int>& states)
{
    if (states.empty() == false)
        return false;

    std::unique_lock<std::mutex> locker(guard_);
    states_ = states;
    return true;
}
bool StateMachine::execute(int state, int& nextState)
{
    nextState = 0;
    return false;
}
bool StateMachine::isMonitor(int state) const
{
    std::unique_lock<std::mutex> locker(guard_);
    return (states_.find(state) != states_.end());
}



}
