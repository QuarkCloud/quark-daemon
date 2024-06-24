
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

    QKRTLAPI void ready();           //���Ա�����
    /**
        ���÷������£�
        int prepareValue = 0 ;
        do{
            prepareValue = prepare() ;
            execute() ;
        }while(completed(prepareValue) == false) ;
        ȷ��execute�ڼ䣬û��ready������
    */
    QKRTLAPI int prepare() const;
    QKRTLAPI bool completed(int prepareValue);      //ȫ��������ɣ������ٴα�����
    QKRTLAPI void completed();                      //ǿ������ȫ��������ɣ������ٴα�����

    QKRTLAPI bool isMonitored() const;

    inline int nodeType() const { return nodeType_; }

    static const int kTypeNone = 0;
    static const int kTypeFile = 1;
    static const int kTypeSock = 2;
    static const int kTypePipe = 3;     //ֻ֧�������ܵ�
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
        link��output�Ƿǳ���Ҫ�ĺ�����
        link��Ҫ֧�ֶ��̰߳�ȫ����output����ǿ��Ϊ���̣߳�
        ��Ϊ���̶߳����ܵ�Ӱ�컹��Ҫ����������û��ʱ��ȥ����
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
