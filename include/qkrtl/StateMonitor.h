
#ifndef QKRTL_STATE_MONITOR_H
#define QKRTL_STATE_MONITOR_H 1

#include "qkrtl/Compile.h"
#include <mutex>
#include <condition_variable>
#include <set>

namespace qkrtl {

class StateMonitor {
public:
    QKRTLAPI StateMonitor();
    QKRTLAPI virtual ~StateMonitor();

    QKRTLAPI bool init(const std::set<int>& states);
    QKRTLAPI void final();

    QKRTLAPI bool wait(const std::set<int>& states);
    QKRTLAPI bool wait(int state);
    QKRTLAPI bool waitFor(const std::set<int>& states  , int timeout); //毫秒级
    QKRTLAPI bool waitFor(int state, int timeout); //毫秒级
    QKRTLAPI void notify(int state);
    QKRTLAPI void clear();
private:
    std::mutex guard_;
    std::condition_variable notifier_;
    std::set<int> states_;
    bool finaled_;
    bool intersection(const std::set<int>& src, const std::set<int>& dst) const;
};

class CounterMonitor {
public:
    QKRTLAPI CounterMonitor();
    QKRTLAPI virtual ~CounterMonitor();
    QKRTLAPI void final();

    QKRTLAPI bool addCounter(int counter = 1);
    //超时以毫秒为单位，当计数达到maxCounter，则返回true
    QKRTLAPI bool waitForCompleted(int maxCounter , int timeout);
    inline int currentCounter() const { return counter_; }
    inline bool completed() const { return (counter_ >= maxCounter_); }
    inline bool finaled() const { return finaled_; }
private:
    std::mutex guard_;
    std::condition_variable notifier_;
    bool finaled_;
    int counter_;
    int maxCounter_;
};

class StateMachine {
public:
    StateMachine();
    virtual ~StateMachine();
    bool init(const std::set<int>& states);
    virtual bool execute(int state, int& nextState);
    bool isMonitor(int state) const;

private:
    mutable std::mutex guard_;
    std::set<int> states_;  //需要监控的状态集合
};

}
#endif /**QKRTL_STATE_MONITOR_H*/
