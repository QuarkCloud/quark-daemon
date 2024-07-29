
#include "qkrtl/Timer.h"

namespace qkrtl {

TimerTask::TimerTask(const std::string& name)
    :name_(name) , fireTime_(0) , period_(0)
{
    nodeType_ = EventNode::kTypeTimer;
}
TimerTask::~TimerTask()
{
    //
}
bool TimerTask::runAfter(int64_t afterTime)
{
    return runAt(afterTime + UTimeNow());
}
bool TimerTask::runAt(int64_t fireTime, int64_t period)
{
    fireTime_ = fireTime;
    period_ = period;
    return true;
}
void TimerTask::fire()
{
    ready();
    fireTime_ += period_;
}
int64_t TimerTask::period()const
{
    return period_;
}
int64_t TimerTask::fireTime() const
{
    return fireTime_;
}

Timer::Timer():finaled_(false) , lastTime_(0)
{
    //
}
Timer::~Timer()
{
    final();
}
void Timer::final()
{
    std::unique_lock<std::mutex> locker(guard_);
    if (finaled_ == true)
        return;
    finaled_ = true;

    if (tasks_.empty() == true)
        return;
}
bool Timer::checkExpired(std::list<TimerTask*>& expired)
{
    std::unique_lock<std::mutex> locker(guard_);
    lastTime_ = UTimeNow();
    if (tasks_.empty() == true)
        return false;
    while (times_.empty() == false)
    {
        std::map<int64_t, std::set<TimerTask*> >::iterator titer = times_.begin();
        int64_t fireTime = titer->first;
        if (fireTime > lastTime_)
            break;

        std::set<TimerTask*>& tasks = titer->second;
        for (std::set<TimerTask*>::iterator iter = tasks.begin(); iter != tasks.end(); ++iter)
        {
            TimerTask* task = (*iter);
            if (task == NULL)
                continue;
            expired.push_back(task);
        }
        times_.erase(titer);
    }

    return (expired.empty() == false);
}
void Timer::process()
{
    std::list<TimerTask*> expired;
    if (checkExpired(expired) == false)
        return;

    for (std::list<TimerTask*>::iterator eiter = expired.begin(); eiter != expired.end(); ++eiter)
    {
        TimerTask* task = (*eiter);
        if (task == NULL)
            continue;
        task->fire();

        int64_t nextFireTime = task->fireTime();
        if (nextFireTime <= lastTime_)
        {
            remove(task);
        }
        else
        {
            add(task);
        }
    }
}
bool Timer::add(TimerTask* task)
{
    std::unique_lock<std::mutex> locker(guard_);
    return innerAdd(task);
}
bool Timer::remove(TimerTask* task)
{
    std::unique_lock<std::mutex> locker(guard_);
    return innerRemove(task);
}
bool Timer::innerAdd(TimerTask* task)
{
    if (task == NULL)
        return false;

    int64_t fireTime = task->fireTime();
    int64_t oldTime = 0;
    std::map<TimerTask*, int64_t>::iterator titer = tasks_.find(task);
    if (titer == tasks_.end())
        titer = tasks_.insert(std::pair<TimerTask*, int64_t>(task, fireTime)).first;

    oldTime = titer->second;
    if (oldTime != fireTime)
    {
        titer->second = fireTime;
    }

    //先删除时间序列
    std::map<int64_t, std::set<TimerTask*> >::iterator siter = times_.find(oldTime);
    if (siter != times_.end())
    {
        std::set<TimerTask*>& tasks = siter->second;
        std::set<TimerTask*>::iterator iter = tasks.find(task);
        if (iter != tasks.end())
        {
            tasks.erase(iter);
            if (tasks.empty() == true)
            {
                times_.erase(siter);
            }
        }
    }

    //再添加该任务
    siter = times_.find(fireTime);
    if (siter == times_.end())
        siter = times_.insert(std::pair<int64_t, std::set<TimerTask*> >
            (fireTime, std::set<TimerTask*>())).first;
    std::set<TimerTask*>& tasks = siter->second;
    tasks.insert(task);
    return true;
}
bool Timer::innerRemove(TimerTask* task)
{
    if (task == NULL)
        return false;

    int64_t fireTime = 0;
    std::map<TimerTask*, int64_t>::iterator titer = tasks_.find(task);
    if (titer != tasks_.end())
    {
        fireTime = titer->second;
        tasks_.erase(titer);
    }
    else
    {
        fireTime = task->fireTime();
    }

    //先删除时间序列
    std::map<int64_t, std::set<TimerTask*> >::iterator siter = times_.find(fireTime);
    if (siter != times_.end())
    {
        std::set<TimerTask*>& tasks = siter->second;
        std::set<TimerTask*>::iterator iter = tasks.find(task);
        if (iter != tasks.end())
        {
            tasks.erase(iter);
            if (tasks.empty() == true)
            {
                times_.erase(siter);
            }
        }
    }

    return true;
}
}

