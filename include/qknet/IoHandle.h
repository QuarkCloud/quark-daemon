
#ifndef QKNET_IO_HANDLE_H
#define QKNET_IO_HANDLE_H 1

#include "qknet/Compile.h"
#include "qkrtl/IoHandle.h"
#include "qkrtl/FileSystem.h"
#include "qkrtl/Poller.h"

namespace qknet{

/**
    IoHandle不再直接继承自EventNode，所以没有execute函数。
    直接用调用IoHandleAsynDestroy，简化类继承关系。
*/
class IoHandle : public qkrtl::IoHandle , public qkrtl::FileNotifier{
public:
    QKNETAPI IoHandle(qkrtl::Poller& poller);
    QKNETAPI virtual ~IoHandle();
    QKNETAPI virtual void final();
    QKNETAPI virtual void close();

    QKNETAPI virtual bool valid() const;
    QKNETAPI virtual void doAsynDestroy();

    QKNETAPI virtual void setHandle(int handle);

    inline qkrtl::Poller& poller() { return poller_; }
protected:
    qkrtl::Poller& poller_;
    QKNETAPI bool postAsynDestroy();

    QKNETAPI virtual void notifyOpen(int errCode);
    QKNETAPI virtual void notifyClose(int errCode);
    QKNETAPI virtual void notifyRead(int errCode);
    QKNETAPI virtual void notifyWrite(int bytes, int errCode);
};

QKNETAPI bool IoHandleAsynDestroy(const qkrtl::Event& event);

}

#endif /**QKNET_IO_HANDLE_H*/
