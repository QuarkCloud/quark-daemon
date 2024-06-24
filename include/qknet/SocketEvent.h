
#ifndef QKNET_SOCKET_EVENT_H
#define QKNET_SOCKET_EVENT_H 1

#include <mutex>
#include "qknet/Compile.h"
#include "qkrtl/IoEvent.h"
#include "qkrtl/Buffer.h"
#include "qkrtl/IoHandle.h"
#include "qknet/Socket.h"

namespace qknet{

class SocketEvent : public qkrtl::IoEvent {
public:
    QKNETAPI SocketEvent();
    QKNETAPI virtual ~SocketEvent();
    QKNETAPI void reset(const Socket& socket);

    inline virtual const HANDLE getHandle() const { return socket_.getHandle(); }
    inline virtual bool valid() const { return socket_.valid(); }

    static const int kActionNone = 0;
    static const int kActionDoing = 1;
    static const int kActionDone = 2;
    static const int kActionClosing = 3;
    static const int kActionClosed = 4;

    static const char* ActionToString(int action);

    inline bool none() const { return (action_ == SocketEvent::kActionNone); }
    inline bool doing() const { return (action_ == SocketEvent::kActionDoing); }
    inline bool done() const { return (action_ == SocketEvent::kActionDone); }
    inline bool closing() const { return (action_ == SocketEvent::kActionClosing); }
    inline bool closed() const { return (action_ == SocketEvent::kActionClosed); }

    inline Socket& socket() { return socket_; }
    inline int action() const { return action_; }

    inline const char * actionName() const {return ActionToString(action_);}
protected:
    Socket socket_;
    int action_;
};

class SocketIoEvent : public SocketEvent {
public:
    QKNETAPI SocketIoEvent();
    QKNETAPI virtual ~SocketIoEvent();

    inline WSABUF* wsabuf() { return &wsabuf_; }
    inline const WSABUF* wsabuf() const { return &wsabuf_; }

protected:
    WSABUF wsabuf_;
};

/**
    read 事件主要是由系统底层驱动的，是单向事件。
    当没有内容可以被读取时，会一致阻塞在底层。
*/
class TcpSocketReadEvent : public SocketIoEvent {
public:
    QKNETAPI TcpSocketReadEvent();
    QKNETAPI virtual ~TcpSocketReadEvent();


    /**
        调用流程
        1、prepare 准备读缓冲区，替换buffer_
        2、startRead 启动读操作，使用buffer_，直到读完成
        3、read 完成读操作之后，替换buffer_。如果buffer是有效缓冲区，可以不用调用prepare
    */
    QKNETAPI bool prepare(qkrtl::Buffer& buffer);

    /**
        read函数在调用之前，不知道实际可以读取的字节数，
        就能设置一个合理的缓冲区
    */
    QKNETAPI int avaibleSize() const;
    QKNETAPI bool read(qkrtl::Buffer& buffer);
    QKNETAPI virtual bool startRead();
    QKNETAPI virtual void ioCompleted();
    QKNETAPI virtual void close();
    QKNETAPI virtual bool start();
    QKNETAPI virtual void stop();

    QKNETAPI virtual void execute();
private:
    mutable std::mutex guard_;
    qkrtl::Buffer buffer_;
};
class TcpSocketWriteEvent : public SocketIoEvent {
public:
    QKNETAPI TcpSocketWriteEvent();
    QKNETAPI virtual ~TcpSocketWriteEvent();

    /**
        当端口在写入时，内存页会被系统锁定。所以必须及时退出。
        当结果返回0时，可能正在写，或者没有足够空间
    */
    QKNETAPI bool write(qkrtl::Buffer& buffer);
    QKNETAPI bool startWrite(bool allowEmpty = false);

    QKNETAPI virtual void ioCompleted();
    QKNETAPI virtual void close();

    QKNETAPI virtual bool start();
    QKNETAPI virtual void stop();

    /*
        如果没有全部发送完成，buffer仍然有遗留，则继续发送。
    */
    QKNETAPI virtual void execute();
private:
    mutable std::mutex guard_;
    qkrtl::Buffer buffer_;     //初始为空
    bool doWrite(bool allowEmpty = false);
};

class SocketAcceptEvent : public SocketEvent {
public:
    QKNETAPI SocketAcceptEvent();
    QKNETAPI virtual ~SocketAcceptEvent();
    QKNETAPI bool accept(Socket& connection);
    QKNETAPI bool startAccept();
    QKNETAPI virtual void close();
    QKNETAPI virtual void ioCompleted();

    QKNETAPI virtual bool start();
    QKNETAPI virtual void stop();
    QKNETAPI virtual void execute();

    static const int kSockAddrMaxSize = 64;
    static const int kSockAddrBufferSize = kSockAddrMaxSize << 1;
private:
    std::mutex guard_;
    Socket newSocket_;
    char addressBuffer_[kSockAddrBufferSize];
};

class SocketConnectEvent : public SocketEvent {
public:
    QKNETAPI SocketConnectEvent();
    QKNETAPI virtual ~SocketConnectEvent();
    QKNETAPI bool connect(const std::string& host, uint16_t port);
    QKNETAPI bool startConnect();
    QKNETAPI virtual void close();
    QKNETAPI virtual void ioCompleted();

    QKNETAPI virtual bool start();
    QKNETAPI virtual void stop();
    QKNETAPI virtual void execute();

    inline bool connected() const { return connected_; }
    inline bool connecting() const { return (action_ == SocketEvent::kActionDoing); }
private:
    std::mutex guard_;
    bool connected_;
    SocketAddr address_;
};

}
#endif /**QKNET_SOCKET_EVENT_H*/
