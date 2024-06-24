
#ifndef QKRTL_POLLER_H
#define QKRTL_POLLER_H 1

#include "qkrtl/Compile.h"
#include "qkrtl/Event.h"
#include "qkrtl/EventPoller.h"
#include <mutex>
#include <map>

namespace qkrtl {

/**
    ���������ƣ��ȴ���򵥵ķ�װ��ʼ��
    ��monitor֮��
    1��Connector�ſ�ʼִ��connect������
    2��Connection�Զ�����Read/Write
    3��Acceptor�Զ�����accept
*/
class QKRTLAPI PollerImpl {
public:
    PollerImpl();
    virtual ~PollerImpl();
    virtual bool monitor(EventNode* node);
    virtual bool unmonitor(EventNode* node);
    virtual bool post(const Event& event);
    virtual bool asynDestroy(EventObject * object);
    virtual int wait(Event* events, int maxCount, int timeout = 0); //����
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
        Ĭ��Ϊinth��������֧���ļ�ģʽ��IOCP��ֻ֧�ִ�����ڴ�ģʽ
    */
    QKRTLAPI Poller(const std::string& name = "");
    QKRTLAPI virtual ~Poller();
    QKRTLAPI virtual bool monitor(EventNode* node);
    QKRTLAPI virtual bool unmonitor(EventNode* node);
    QKRTLAPI virtual bool post(const Event& event);
    QKRTLAPI virtual bool asynDestroy(EventObject* object);
    QKRTLAPI virtual int wait(Event* events, int maxCount, int timeout = 0); //����
    QKRTLAPI bool shutdown();
    QKRTLAPI void final();
private:
    PollerImpl* impl_;
};
}


#endif /**QKRTL_POLLER_H*/
