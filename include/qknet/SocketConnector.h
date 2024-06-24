
#ifndef QKNET_SOCKET_CONNECTOR_H
#define QKNET_SOCKET_CONNECTOR_H 1

#include "qknet/Compile.h"
#include "qkrtl/Poller.h"
#include "qkrtl/Timer.h"
#include "qknet/SocketIoHandle.h"
#include "qknet/SocketEvent.h"
#include "qknet/SocketConnection.h"

namespace qknet{

/**
    ���ⲿ�̳�ʵ��
*/
class SocketConnectHandler : public SocketIoHandler {
public:
    QKNETAPI SocketConnectHandler();
    QKNETAPI virtual ~SocketConnectHandler();
    QKNETAPI virtual bool handleConnectSucceed(Socket& socket);
    QKNETAPI virtual bool handleConnectFailed(int errCode);
    QKNETAPI virtual bool handleConnectCompleted();
};

/**
    SocketConnector�����֮����Ҫ���ͷţ�������ʵ�����޷�֪�����״̬��
    �ڳɹ����Ӻ�״̬�л���SocketConnection��
    �ڳɹ�����֮ǰ�����������У���ʱ��Ͽ����ӵĻ�����Ҫ��ʱ�ͷţ�
    ��SocketConnectEvent��Ȼ�ҽ���IOCP�С�

    SocketConnectorֻ��һ���Եģ�������������ƣ���Acceptor����ȫ��ͬ�ġ�
*/
class SocketConnector : public SocketConnectEvent {
public:
    QKNETAPI SocketConnector(SocketIoHandle* ioHandle);
    QKNETAPI virtual ~SocketConnector();
    QKNETAPI virtual bool handle(int errCode = 0);
private:
    SocketIoHandle* ioHandle_;
};

/**
    �ڳ������Ӻ���Ҫȷ�����ض�ʱ��󣬱����ܹ����ӳɹ���������Ϊʧ�ܡ�
*/
class SocketConnectTimeout : public qkrtl::TimerTask {
public:
    QKNETAPI SocketConnectTimeout(SocketIoHandle* ioHandle);
    QKNETAPI virtual ~SocketConnectTimeout();
    QKNETAPI virtual void execute();
private:
    SocketIoHandle* ioHandle_;
};
class SocketConnectHandle : public SocketIoHandle {
public:
    QKNETAPI SocketConnectHandle(qkrtl::Poller& poller);
    QKNETAPI virtual ~SocketConnectHandle();
    QKNETAPI bool connect(const std::string& host, uint16_t port , int timeout);

    QKNETAPI virtual void final();
    QKNETAPI virtual void close();
    QKNETAPI virtual void execute();


    QKNETAPI virtual bool handleStop();
    QKNETAPI virtual bool handleInput(int errCode = 0);

    QKNETAPI void resetHandler(SocketConnectHandler* handler);

    static const int kTimeout = 1000000;
private:
    std::mutex guard_;
    bool finaled_;
    bool handled_;
    qkrtl::Poller& poller_;
    SocketConnectTimeout timeout_;
    SocketConnector connector_;

    SocketConnectHandler* handler_;
    SocketConnectHandler dummyHandler_;
    QKNETAPI virtual void doAsynDestroy();
};

/**
    SocketConnectHandle��SocketConnectionHandle���ܹ���ͬһ��handler������ֻ�ֱܷ�ʵ�֡�
    SocketConnectHandler����ʱ�ԣ���ò���Ҫ�ⲿ�˽�̫��ϸ�ڣ���Ҫ��һ����װ
*/
class ClientConnector : public SocketConnectHandler {
public:
    QKNETAPI ClientConnector(qkrtl::Poller& poller);
    QKNETAPI virtual ~ClientConnector();
    QKNETAPI bool connect(const std::string& host, uint16_t port, int timeout);
    QKNETAPI void final();
    QKNETAPI virtual bool handleConnectSucceed(qknet::Socket& socket);
    QKNETAPI virtual bool handleConnectFailed(int errCode);
    QKNETAPI virtual bool handleConnectCompleted();
private:
    std::mutex guard_;
    bool finaled_;
    qkrtl::Poller& poller_;
    SocketConnectHandle* connector_;
};

}
#endif /**QKNET_SOCKET_CONNECTOR_H*/
