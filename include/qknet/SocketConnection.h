
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
    �����ǳ���Ҫ����SocketConnection��Ӧ�ò�֮���Ψһ�ӿڣ���Ӧ�ò�ʵ�֡�
    ��Ա�����Ķ��壬������SocketIoEvent�����̡�
    ��Ҫ��ϸ�����Ҫ�����Ĺ��ܶ��塣
*/
class SocketConnectionHandler : public SocketIoHandler {
public:
    QKNETAPI SocketConnectionHandler();
    QKNETAPI virtual ~SocketConnectionHandler();
    
    /**
        ��Щinput����������ͳһ�߳��У���ioHandle���ã������������£�
        1��allocBuffer������һ��buffer������startRead���ȴ��Զ����롣
        2��handleInput������һ����ȡ��ɵ�buffer��
        3��freeBuffer���ͷ�һ���Ѿ���������ɵ�buffer��
    */
    QKNETAPI virtual bool allocInBuffer(qkrtl::Buffer& buffer);
    QKNETAPI virtual bool freeInBuffer(qkrtl::Buffer& buffer);
    QKNETAPI virtual bool freeOutBuffer(qkrtl::Buffer& buffer);
    QKNETAPI virtual bool handleInput(qkrtl::Buffer& buffer);

    /**
        ��handler���ã���ioHandle�����ڲ�ͬ�߳�ִ�С���������£�����Ҫ���ã�
        ֻ������������£�inputѭ�����жϣ�Ȼ������������iocp����һ��ϵͳ���á�
    */
    QKNETAPI bool startInput();

    /**
        handleOutput��ioHandle���ã�֮ǰbuffer�Ѿ�ȫ��������ɡ�
        handler����һ��buffer����buffer����������Ҫ��д��socket�С�
    */
    QKNETAPI virtual bool handleOutput(qkrtl::Buffer& buffer);
    /**
        ��handler���ã���ioHandle�����ڲ�ͬ�߳�ִ�С��ú�����startInput��ͬ��
        ����Ҫ��iocp����һ��ϵͳ���ã�ֻ����EventPoller����һ����ִ���¼���
        �ɹ�֮�󣬺�����������handleOutput��
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
    �����Ҫ���������ƣ���Ҫ����������⡣�Ժ��Ż��ٴ���
    1���ӿڼ�࣬��Ҫ���ǿ��ܳ�����
    2���ͷ�ʱ��Ҫȷ�����ܱ�����

    2024-05-30
    SocketConnection����֧�ּ̳У����ⲿʵ��SocketConnectionHandler��
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
