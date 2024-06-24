
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
    由外部继承实现
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
    SocketConnector在完成之后，需要被释放，在外面实际上无法知道这个状态。
    在成功连接后，状态切换到SocketConnection。
    在成功链接之前，处于连接中，这时候断开连接的话，需要超时释放，
    但SocketConnectEvent仍然挂接在IOCP中。

    SocketConnector只是一次性的，所以这个类的设计，和Acceptor是完全不同的。
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
    在尝试连接后，需要确保在特定时间后，必须能够连接成功，否则视为失败。
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
    SocketConnectHandle和SocketConnectionHandle不能共享同一个handler，所以只能分别实现。
    SocketConnectHandler是临时性，最好不需要外部了解太多细节，需要进一步封装
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
