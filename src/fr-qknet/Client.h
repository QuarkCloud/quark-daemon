
#ifndef FR_QKNET_CLIENT_H
#define FR_QKNET_CLIENT_H 1

#include <mutex>
#include <condition_variable>
#include <set>

#include "qknet/SocketConnector.h"
#include "qkrtl/Buffer.h"
#include "qkrtl/IoService.h"
#include "qkrtl/StateMonitor.h"

/**
    ���ڸ���Ŀֻ�ǲ������Ӻ��ͷţ����Բ���Ҫʵ��SocketConnectionHandler��
*/
class Client : public qknet::ClientConnector {
public:
    Client(qkrtl::Poller& poller);
    virtual ~Client();
    virtual bool handleConnectCompleted();
    bool waitForCompleted();
private:
    qkrtl::CounterMonitor waiter_;
};

#endif /**FR_QKNET_CLIENT_H*/
