
#ifndef QKNET_SOCKET_CONNECTION_H
#define QKNET_SOCKET_CONNECTION_H 1

#include <time.h>
#include <set>
#include <list>
#include "qknet/Compile.h"
#include "qkrtl/Poller.h"
#include "qkrtl/Buffer.h"
#include "qkrtl/Stream.h"
#include "qknet/SocketEvent.h"
#include "qknet/SocketIoHandle.h"

namespace qknet{

/**
    这个类非常重要，是SocketConnection和应用层之间的唯一接口，由应用层实现。
    成员函数的定义，依赖于SocketIoEvent的流程。
    需要仔细理解主要函数的功能定义。
*/
class SocketConnectionHandler : public SocketIoHandler {
public:
    QKNETAPI SocketConnectionHandler();
    QKNETAPI virtual ~SocketConnectionHandler();
    
    /**
        这些input函数都是在统一线程中，由ioHandle调用，调用流程如下：
        1、allocBuffer，申请一个buffer，用于startRead，等待对端输入。
        2、handleInput，处理一个读取完成的buffer。
        3、freeBuffer，释放一个已经被处理完成的buffer。
    */
    QKNETAPI virtual bool allocInBuffer(qkrtl::Buffer& buffer);
    QKNETAPI virtual bool freeInBuffer(qkrtl::Buffer& buffer);
    QKNETAPI virtual bool freeOutBuffer(qkrtl::Buffer& buffer);
    QKNETAPI virtual bool handleInput(qkrtl::Buffer& buffer);

    /**
        由handler调用，和ioHandle可能在不同线程执行。正常情况下，不需要调用，
        只有在特殊情况下，input循环被中断，然后再重启，向iocp发起一个系统调用。
    */
    QKNETAPI bool startInput();

    /**
        handleOutput由ioHandle调用，之前buffer已经全部发送完成。
        handler返回一个buffer，该buffer的内容是需要被写入socket中。
    */
    QKNETAPI virtual bool handleOutput(qkrtl::Buffer& buffer);
    /**
        由handler调用，和ioHandle可能在不同线程执行。该函数和startInput不同，
        不需要向iocp发起一个系统调用，只是在EventPoller发起一个可执行事件。
        成功之后，后续必须会调用handleOutput。
    */
    QKNETAPI bool startOutput();
};


class TcpSocketReader : public TcpSocketReadEvent {
public:
    QKNETAPI TcpSocketReader(SocketIoHandle* ioHandle);
    QKNETAPI virtual ~TcpSocketReader();
    QKNETAPI virtual bool handle(int errCode = 0);
private:
    SocketIoHandle* ioHandle_;
};
class TcpSocketWriter : public TcpSocketWriteEvent {
public:
    QKNETAPI TcpSocketWriter(SocketIoHandle* ioHandle);
    QKNETAPI virtual ~TcpSocketWriter();
    QKNETAPI virtual bool handle(int errCode = 0);
private:
    SocketIoHandle* ioHandle_;
};

/**
    这块需要更精妙的设计，需要解决两个问题。以后优化再处理
    1、接口简洁，但要覆盖可能场景。
    2、释放时，要确保不能崩溃。

    2024-05-30
    SocketConnection不再支持继承，由外部实现SocketConnectionHandler。
*/
class SocketConnectionHandle : public SocketIoHandle {
public:
    QKNETAPI SocketConnectionHandle(qkrtl::Poller& poller);
    QKNETAPI virtual ~SocketConnectionHandle();
    QKNETAPI virtual void final();
    QKNETAPI virtual void close();
    QKNETAPI virtual void execute();

    QKNETAPI virtual bool start();

    QKNETAPI void swap(Socket& socket);

    QKNETAPI virtual bool handleStart();
    QKNETAPI virtual bool handleStop();

    QKNETAPI virtual bool handleInput(int errCode = 0);
    QKNETAPI virtual bool startInput();

    QKNETAPI virtual bool handleOutput(int errCode = 0);
    QKNETAPI virtual bool startOutput();

    QKNETAPI virtual bool handleError(int errCode);

    QKNETAPI void resetHandler(SocketConnectionHandler* handler);

    inline bool connected() const { return connected_; }
    inline void connected(bool value) { connected_ = value; }

protected:
    TcpSocketReader reader_;
    TcpSocketWriter writer_;
    bool connected_;
    qkrtl::Poller& poller_;
    SocketConnectionHandler* handler_;
    SocketConnectionHandler dummyHandler_;

    int forceCloseAll();
    QKNETAPI virtual void doAsynDestroy();
};

class SocketConnectionHandlerManager {
public:
    QKNETAPI SocketConnectionHandlerManager(qkrtl::Poller& poller);
    QKNETAPI virtual ~SocketConnectionHandlerManager();

    QKNETAPI void final();
    QKNETAPI bool insert(SocketConnectionHandler* handler);
    QKNETAPI bool remove(SocketConnectionHandler* handler);
    QKNETAPI bool find(SocketConnectionHandler* handler) const;
    QKNETAPI void clear();

private:
    mutable std::mutex guard_;
    qkrtl::Poller& poller_;
    bool finaled_;
    std::set<SocketConnectionHandler*> handlers_;
};

}
#endif /**QKNET_SOCKET_CONNECTION_H*/
