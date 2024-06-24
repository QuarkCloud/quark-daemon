
#ifndef QKRTL_POLLER_H
#define QKRTL_POLLER_H 1

#include "qkrtl/Compile.h"
#include "qkrtl/Event.h"
#include "qkrtl/EventPoller.h"
#include <mutex>
#include <map>

namespace qkrtl {

/**
    后续再完善，先从最简单的封装开始。
    在monitor之后，
    1、Connector才开始执行connect函数，
    2、Connection自动触发Read/Write
    3、Acceptor自动触发accept
*/
class QKRTLAPI PollerImpl {
public:
    PollerImpl();
    virtual ~PollerImpl();
    virtual bool monitor(EventNode* node);
    virtual bool unmonitor(EventNode* node);
    virtual bool post(const Event& event);
    virtual bool asynDestroy(EventObject * object);
    virtual int wait(Event* events, int maxCount, int timeout = 0); //毫秒
    virtual bool shutdown();
    virtual void final();
};

class QKRTLAPI PollerImplCreator {
public:
    virtual PollerImpl* operator()(void* param);
};

class PollerImplRegistry {
public:
    QKRTLAPI PollerImplRegistry();
    QKRTLAPI virtual ~PollerImplRegistry();
    QKRTLAPI bool add(const std::string& name, PollerImplCreator * creator);
    QKRTLAPI PollerImpl* create(const std::string& name , void * param);

    QKRTLAPI static PollerImplRegistry& singleton();
private:
    std::mutex guard_;
    std::map<std::string, PollerImplCreator *> creators_;
};
class QKRTLAPI PollerImplCreatorAutoRegister {
public:
    PollerImplCreatorAutoRegister(const std::string& name, PollerImplCreator* creator);
};

class Poller {
public:
    /**
        默认为inth驱动，不支持文件模式的IOCP，只支持纯粹的内存模式
    */
    QKRTLAPI Poller(const std::string& name = "");
    QKRTLAPI virtual ~Poller();
    QKRTLAPI virtual bool monitor(EventNode* node);
    QKRTLAPI virtual bool unmonitor(EventNode* node);
    QKRTLAPI virtual bool post(const Event& event);
    QKRTLAPI virtual bool asynDestroy(EventObject* object);
    QKRTLAPI virtual int wait(Event* events, int maxCount, int timeout = 0); //毫秒
    QKRTLAPI bool shutdown();
    QKRTLAPI void final();
private:
    PollerImpl* impl_;
};
}


#endif /**QKRTL_POLLER_H*/
