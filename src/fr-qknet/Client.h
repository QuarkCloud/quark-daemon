
#ifndef FR_QKNET_CLIENT_H
#define FR_QKNET_CLIENT_H 1

#include <mutex>
#include <condition_variable>
#include <set>

#include "qknet/Connector.h"
#include "qkrtl/Buffer.h"
#include "qkrtl/IoService.h"
#include "qkrtl/StateMonitor.h"

/**
    由于该项目只是测试连接和释放，所以不需要实现SocketConnectionHandler。
*/
class Client : public qknet::Connector {
public:
    Client(qkrtl::Poller& poller);
    virtual ~Client();
    virtual bool handleStart();
    virtual bool handleStop();

    bool waitForCompleted();
private:
    qkrtl::CounterMonitor waiter_;
};

#endif /**FR_QKNET_CLIENT_H*/
