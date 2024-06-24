
#ifndef QKNET_SOCKET_IO_HANDLE_H
#define QKNET_SOCKET_IO_HANDLE_H 1

#include "qknet/Compile.h"
#include "qkrtl/IoHandle.h"
#include "qknet/Socket.h"

namespace qknet{

/**
    SocketIoHandleֻ�Ǳ�IoHandle����һ���򵥵�Socket��Ա��
    ԭ�����ڣ�Socket������IoHandle�����ࡣ
*/
class SocketIoHandle : public qkrtl::IoHandle {
public:
    QKNETAPI SocketIoHandle();
    QKNETAPI virtual ~SocketIoHandle();
    QKNETAPI virtual void final();
    QKNETAPI virtual void close();
    QKNETAPI virtual void execute();

    QKNETAPI virtual const HANDLE getHandle() const;
    QKNETAPI virtual bool valid() const;

    inline const Socket& socket() const { return socket_; }
    inline Socket& socket() { return socket_; }
    inline operator Socket& () { return socket_; }
    inline operator const Socket& () const{ return socket_; }
protected:
    Socket socket_;
    QKNETAPI virtual void doAsynDestroy();
};

/**
    handler�Ǳ����첽���õģ����Բ���Ҫ����start��
    close���������ã���Ҫ��SocketIoHandleʵ�֡�
*/
class SocketIoHandler : public qkrtl::IoHandler {
public:
    QKNETAPI SocketIoHandler();
    QKNETAPI virtual ~SocketIoHandler();
    QKNETAPI virtual void close();
    QKNETAPI virtual bool handleStart();
    QKNETAPI virtual bool handleStop();
    QKNETAPI virtual bool handle(int errCode = 0);

    QKNETAPI virtual const HANDLE getHandle() const;
    QKNETAPI virtual bool valid() const;
    QKNETAPI void resetHandle(SocketIoHandle* ioHandle);
protected:
    SocketIoHandle* ioHandle_;
    SocketIoHandle dummyIoHandle_;
};

}
#endif /**QKNET_SOCKET_IO_HANDLE_H*/
