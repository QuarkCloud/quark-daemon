
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
    read �¼���Ҫ����ϵͳ�ײ������ģ��ǵ����¼���
    ��û�����ݿ��Ա���ȡʱ����һ�������ڵײ㡣
*/
class TcpSocketReadEvent : public SocketIoEvent {
public:
    QKNETAPI TcpSocketReadEvent();
    QKNETAPI virtual ~TcpSocketReadEvent();


    /**
        ��������
        1��prepare ׼�������������滻buffer_
        2��startRead ������������ʹ��buffer_��ֱ�������
        3��read ��ɶ�����֮���滻buffer_�����buffer����Ч�����������Բ��õ���prepare
    */
    QKNETAPI bool prepare(qkrtl::Buffer& buffer);

    /**
        read�����ڵ���֮ǰ����֪��ʵ�ʿ��Զ�ȡ���ֽ�����
        ��������һ������Ļ�����
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
        ���˿���д��ʱ���ڴ�ҳ�ᱻϵͳ���������Ա��뼰ʱ�˳���
        ���������0ʱ����������д������û���㹻�ռ�
    */
    QKNETAPI bool write(qkrtl::Buffer& buffer);
    QKNETAPI bool startWrite(bool allowEmpty = false);

    QKNETAPI virtual void ioCompleted();
    QKNETAPI virtual void close();

    QKNETAPI virtual bool start();
    QKNETAPI virtual void stop();

    /*
        ���û��ȫ��������ɣ�buffer��Ȼ����������������͡�
    */
    QKNETAPI virtual void execute();
private:
    mutable std::mutex guard_;
    qkrtl::Buffer buffer_;     //��ʼΪ��
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
