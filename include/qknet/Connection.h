
#ifndef QKNET_CONNECTION_H
#define QKNET_CONNECTION_H 1

#include <time.h>
#include <set>
#include <list>
#include "qknet/Compile.h"
#include "qkrtl/Poller.h"
#include "qkrtl/Buffer.h"
#include "qkrtl/Stream.h"
#include "qknet/IoEvent.h"
#include "qknet/IoHandle.h"

namespace qknet{

/**
    这块需要更精妙的设计，需要解决两个问题。以后优化再处理
    1、接口简洁，但要覆盖可能场景。
    2、释放时，要确保不能崩溃。
*/
class Connection : public IoHandle {
public:
    QKNETAPI Connection(qkrtl::Poller& poller);
    QKNETAPI virtual ~Connection();
    QKNETAPI virtual void final();
    QKNETAPI virtual void close();

    QKNETAPI virtual bool start();

    QKNETAPI virtual bool handleStart();
    QKNETAPI virtual bool handleStop();

    QKNETAPI virtual bool handleInput(int errCode = 0);
    QKNETAPI virtual bool startInput();

    /**
        handleRead和handleOutput需要由子类实现。
    */
    QKNETAPI virtual bool handleRead(qkrtl::Buffer& buffer);
    QKNETAPI bool write(qkrtl::Buffer& buffer);
    QKNETAPI virtual bool handleWrited(qkrtl::Buffer& buffer);
    QKNETAPI virtual bool handleOutput(int errCode = 0);

    QKNETAPI virtual bool startOutput();

    QKNETAPI virtual bool handleError(int errCode);

    inline bool connected() const { return connected_; }
    inline void connected(bool value) { connected_ = value; }

    QKNETAPI virtual void notifyOpen(int errCode);
    QKNETAPI virtual void notifyClose(int errCode);
    QKNETAPI virtual void notifyRead(int errCode);
    QKNETAPI virtual void notifyWrite(int bytes, int errCode);

private:
    std::mutex guard_;
    bool finaled_;
    TcpSocketReader reader_;
    TcpSocketWriter writer_;
    qkrtl::Buffer outBuffer_;
    bool connected_;
    QKNETAPI virtual void doAsynDestroy();
};


class ConnectionManager {
public:
    QKNETAPI ConnectionManager();
    QKNETAPI virtual ~ConnectionManager();

    QKNETAPI bool insert(Connection* conn);
    QKNETAPI bool remove(Connection* conn);
    QKNETAPI void clear();
    QKNETAPI void final();

private:
    std::mutex guard_;
    bool finaled_;
    std::set<Connection*> conns_;
};

}
#endif /**QKNET_CONNECTION_H*/
