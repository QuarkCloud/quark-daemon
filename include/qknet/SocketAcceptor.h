
#ifndef QKNET_SOCKET_ACCEPTOR_H
#define QKNET_SOCKET_ACCEPTOR_H 1

#include <time.h>
#include <set>
#include <list>
#include "qknet/Compile.h"
#include "qkrtl/Poller.h"
#include "qkrtl/Buffer.h"
#include "qknet/SocketEvent.h"
#include "qknet/SocketIoHandle.h"

namespace qknet{

/**
    ���ⲿ�̳�ʵ��
*/
class SocketAcceptHandler : public SocketIoHandler {
public:
    QKNETAPI SocketAcceptHandler();
    QKNETAPI virtual ~SocketAcceptHandler();

    QKNETAPI virtual bool handleAccepted(Socket& socket);
    QKNETAPI virtual bool handleStop();
};

/**
    �ڲ�ʵ�֣�������Ҫ�̳С��ⲿ��SocketAcceptHandlerʵ��
*/
class SocketAcceptor : public SocketAcceptEvent {
public:
    QKNETAPI SocketAcceptor(SocketIoHandle* ioHandle);
    QKNETAPI virtual ~SocketAcceptor();
    QKNETAPI virtual bool handle(int errCode = 0);
private:
    SocketIoHandle* ioHandle_;
};
class SocketAcceptHandle : public SocketIoHandle{
public:
    QKNETAPI SocketAcceptHandle(qkrtl::Poller& poller);
    QKNETAPI virtual ~SocketAcceptHandle();

    QKNETAPI bool init(uint16_t port, int backlog);
    QKNETAPI void final();

    QKNETAPI virtual void doAsynDestroy();

    QKNETAPI virtual bool start();
    QKNETAPI virtual void close();

    //Ψһִ�а�ȫ�رռ��
    QKNETAPI virtual void execute();

    //��Poller�Ĺ����ж����
    QKNETAPI virtual bool handleStop();

    QKNETAPI virtual bool handleInput(int errCode = 0);
    QKNETAPI virtual bool startInput();

    QKNETAPI void resetHandler(SocketAcceptHandler* handler);
private:
    std::mutex guard_;
    bool finaled_;
    SocketAcceptor acceptor_;
    qkrtl::Poller& poller_;

    SocketAcceptHandler* handler_;
    SocketAcceptHandler dummyHandler_;
};

}
#endif /**QKNET_SOCKET_ACCEPTOR_H*/
