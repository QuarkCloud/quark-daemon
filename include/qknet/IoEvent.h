
#ifndef QKNET_IO_EVENT_H
#define QKNET_IO_EVENT_H 1

#include <mutex>
#include "qkrtl/IoEvent.h"
#include "qkrtl/Buffer.h"
#include "qkrtl/IoHandle.h"

#include "qknet/Compile.h"

namespace qknet{

/**
    read 事件主要是由系统底层驱动的，是单向事件。
    当没有内容可以被读取时，会一致阻塞在底层。
*/
class TcpSocketReader : public qkrtl::InEvent {
public:
    QKNETAPI TcpSocketReader(qkrtl::IoHandle& handle);
    QKNETAPI virtual ~TcpSocketReader();

    QKNETAPI bool read(qkrtl::Buffer& buffer);
    QKNETAPI virtual void close();
    QKNETAPI virtual bool start();
    QKNETAPI virtual void stop();
private:
    mutable std::mutex guard_;
};
class TcpSocketWriter : public qkrtl::OutEvent {
public:
    QKNETAPI TcpSocketWriter(qkrtl::IoHandle& handle);
    QKNETAPI virtual ~TcpSocketWriter();

    /**
        当端口在写入时，内存页会被系统锁定。所以必须及时退出。
        当结果返回0时，可能正在写，或者没有足够空间
    */
    QKNETAPI bool write(qkrtl::Buffer& buffer);
    QKNETAPI virtual void close();

    QKNETAPI virtual bool start();
    QKNETAPI virtual void stop();

private:
    mutable std::mutex guard_;
};

class TcpSocketAcceptor : public qkrtl::InEvent {
public:
    QKNETAPI TcpSocketAcceptor(qkrtl::IoHandle& handle);
    QKNETAPI virtual ~TcpSocketAcceptor();
    QKNETAPI virtual void close();

    QKNETAPI virtual bool start();
    QKNETAPI virtual void stop();
};

/**
    本来要取消这个类，但需要有个eventNode，绑定到poller
*/
class TcpSocketConnector : public qkrtl::InEvent {
public:
    QKNETAPI TcpSocketConnector(qkrtl::IoHandle& handle);
    QKNETAPI virtual ~TcpSocketConnector();
    QKNETAPI virtual void close();

    QKNETAPI virtual bool start();
    QKNETAPI virtual void stop();
};

}
#endif /**QKNET_IO_EVENT_H*/
