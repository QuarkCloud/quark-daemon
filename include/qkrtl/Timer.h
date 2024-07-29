
#ifndef QKRTL_TIMER_H
#define QKRTL_TIMER_H 1

#include <stdint.h>
#include <time.h>
#include <list>
#include <string>
#include <mutex>
#include <map>
#include <set>
#include <condition_variable>

#include "qkrtl/Compile.h"
#include "qkrtl/DateTime.h"
#include "qkrtl/EventPoller.h"

namespace qkrtl {

/**
    时间以微妙为单位，基于UTimeNow()
*/
class TimerTask : public EventNode {
public:
    QKRTLAPI TimerTask(const std::string& name = "");
    QKRTLAPI virtual ~TimerTask();
    QKRTLAPI bool runAfter(int64_t afterTime);
    QKRTLAPI bool runAt(int64_t fireTime , int64_t period = 0);
    QKRTLAPI void fire();
    QKRTLAPI int64_t period()const;
    QKRTLAPI int64_t fireTime() const;
private:
    std::string name_;
    int64_t fireTime_;
    int64_t period_;
};

class Timer {
public:
    QKRTLAPI Timer();
    QKRTLAPI virtual ~Timer();
    QKRTLAPI void final();
    QKRTLAPI void process();
    QKRTLAPI bool add(TimerTask* task);
    QKRTLAPI bool remove(TimerTask* task);

    inline int64_t lastTime() const { return lastTime_; }
    inline bool finaled() const { return finaled_; }
private:
    std::mutex guard_;
    bool finaled_;
    int64_t lastTime_;
    std::map<int64_t, std::set<TimerTask *> > times_;
    std::map<TimerTask* , int64_t> tasks_;

    bool innerAdd(TimerTask* task);
    bool innerRemove(TimerTask* task);

    bool checkExpired(std::list<TimerTask*>& expired);
};

}

#endif /**QKRTL_TIMER_H*/
