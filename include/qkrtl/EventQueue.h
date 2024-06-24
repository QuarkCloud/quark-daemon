
#ifndef QKRTL_EVENT_QUEUE_H
#define QKRTL_EVENT_QUEUE_H 1

#include <mutex>

#include "qkrtl/Compile.h"
#include "qkrtl/Sequencer.h"
#include "qkrtl/Synch.h"
#include "qkrtl/RingBuffer.h"
#include "qkrtl/Event.h"

namespace qkrtl {

/**
    ��EventPoller��ͬ��ʱ��MessageQueueֻ֧��һ���Ե���Ϣ��
    ���Բ���Ҫ�ͷţ�Ҳ���ܱ����ظ����͡�
*/
class EventQueue {
public:
    QKRTLAPI EventQueue();
    QKRTLAPI virtual ~EventQueue();
    QKRTLAPI bool init(int maxSize);
    QKRTLAPI virtual void final();
    QKRTLAPI int64_t post(const Event& event);
    QKRTLAPI bool cancel(int64_t sequence);
    QKRTLAPI bool peek(Event& event);
    QKRTLAPI int peek(Event * events , int maxCount = 1);
    QKRTLAPI bool wait(int timeout);

    QKRTLAPI void setWaiter(Waiter* waiter);

    inline int backlog() const { return index_.backlog(); }
protected:
    bool finaled_;
    RingBuffer<Event> events_;
    SafeRingIndex index_;
    Waiter * waiter_;
};

}

#endif /**QKRTL_EVENT_QUEUE_H*/
