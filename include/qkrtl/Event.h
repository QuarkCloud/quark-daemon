
#ifndef QKRTL_EVENT_H
#define QKRTL_EVENT_H 1

#include <mutex>

#include "qkrtl/Compile.h"
#include "qkrtl/Sequencer.h"
#include "qkrtl/Synch.h"
#include "qkrtl/RingBuffer.h"
#include "qkrtl/Object.h"

namespace qkrtl {

class QKRTLAPI EventObject : public Object{
public:
    EventObject();
    virtual ~EventObject();
    virtual void execute();

    inline int64_t sequence() const { return sequence_; }
    inline void sequence(int64_t value) { sequence_ = value; }

    static const int64_t kInvalidSequence = -1;
private:
    int64_t sequence_;
};

typedef struct Event Event;
typedef bool (*EventExecutor)(const Event& event);

/**
    控制在64字节，不需要释放
*/
struct Event {
    int64_t id;
    uint32_t type;
    int16_t status;
    int16_t dataType;
    union {
        int32_t i32;
        uint32_t u32;
        int64_t i64;
        uint64_t u64;
        float f32;
        double f64;
        void* ptr;
        char* str;
        EventObject* event;
        Object* object;
        uintptr_t value;
    } data;

    EventExecutor executor;

    static const uint32_t kEventTypeNone = 0;

    static const int16_t kStatusNone    = 0;  //数据无效，初始状态
    static const int16_t kStatusReady   = 1;  //数据有效，待处理
    static const int16_t kStatusFree    = -1; //数据有效，等待释放

    static const int16_t kDataTypeNone = 0;
    static const int16_t kDataTypeI32 = 1;
    static const int16_t kDataTypeU32 = 2;
    static const int16_t kDataTypeI64 = 3;
    static const int16_t kDataTypeU64 = 4;
    static const int16_t kDataTypeF32 = 5;
    static const int16_t kDataTypeF64 = 6;
    static const int16_t kDataTypePtr = 7;
    static const int16_t kDataTypeStr = 8;
    static const int16_t kDataTypeObj = 9;
    static const int16_t kDataTypeEvt = 10;

    QKRTLAPI static const Event Initializer;
};
/**
    注意执行事件的优先级，设置对应成员的值
    1、status != kStatusNone ;
    2、executor != NULL ，整个事件当做参数传给executor
    3、dataType == kDataObject ;
    3.1、status == kStatusFree，直接释放
    3.2、EventObject::execute
*/
QKRTLAPI bool ExecuteEvent(const Event& event);

/**
    直接使用默认事件队列
*/
QKRTLAPI bool PostEvent(const Event& event);
QKRTLAPI bool PeekEvent(Event& event);
QKRTLAPI int PeekEvent(Event* events, int maxCount = 1);
QKRTLAPI bool WaitEvent(int timeout);

}


#endif /**QKRTL_EVENT_H*/
