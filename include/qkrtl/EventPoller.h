
#ifndef QKRTL_EVENT_POLLER_H
#define QKRTL_EVENT_POLLER_H 1

#include <mutex>
#include <set>
#include <atomic>

#include "qkrtl/Compile.h"
#include "qkrtl/EventQueue.h"

namespace qkrtl {

class EventNode : public EventObject {
public:
    QKRTLAPI EventNode();
    QKRTLAPI virtual ~EventNode();

    QKRTLAPI void ready();           //可以被消费
    /**
        调用范例如下：
        int prepareValue = 0 ;
        do{
            prepareValue = prepare() ;
            execute() ;
        }while(completed(prepareValue) == false) ;
        确保execute期间，没有ready被调用
    */
    QKRTLAPI int prepare() const;
    QKRTLAPI bool completed(int prepareValue);      //全部消费完成，允许再次被消费
    QKRTLAPI void completed();                      //强制设置全部消费完成，允许再次被消费

    QKRTLAPI bool isMonitored() const;

    inline int nodeType() const { return nodeType_; }

    static const int kTypeNone = 0;
    static const int kTypeFile = 1;
    static const int kTypeSock = 2;
    static const int kTypePipe = 3;     //只支持有名管道
    static const int kTypeTimer = 4;
    static const int kTypeInth = 5;

    static const  char* nodeTypeName(int type);
protected:
    int nodeType_;
private:
    friend class EventPoller;
    EventPoller* monitor_;
    std::atomic<int> readyCounter_;
};

class EventPoller : public EventQueue {
public:
    QKRTLAPI EventPoller();
    QKRTLAPI virtual ~EventPoller();

    QKRTLAPI void final();
    QKRTLAPI bool monitor(EventNode* node);
    QKRTLAPI bool unmonitor(EventNode* node);
    QKRTLAPI bool isMonitored(const EventNode* node) const;


    /**
        link和output是非常重要的函数。
        link需要支持多线程安全，但output可以强制为单线程，
        因为多线程对性能的影响还需要评估，但是没有时间去处理
    */
    QKRTLAPI bool link(EventNode* node);

    inline int size() const { return (int)nodes_.size(); }
    static const int kMaxNodeSize = 1 << 16;
private:
    std::mutex guard_;
    std::set<EventNode *> nodes_;
};

}

#endif /**QKRTL_EVENT_POLLER_H*/
