
#ifndef QKRTL_IO_HANDLE_H
#define QKRTL_IO_HANDLE_H 1

#include "qkrtl/Compile.h"
#include "qkrtl/Object.h"
#include "qkrtl/EventPoller.h"
#include "qkrtl/SpinLock.h"
#include "qkrtl/Synch.h"
#include "qkrtl/OsHandle.h"

namespace qkrtl {

/**
*   2024-06-25
    强化了EventPoller的接口，不需要IoHandle调用EventNode的execute。
*/
class IoHandle : public Object{
public:
    QKRTLAPI IoHandle();
    QKRTLAPI virtual ~IoHandle();

    QKRTLAPI virtual void close();

    QKRTLAPI virtual bool handleStart();
    QKRTLAPI virtual bool handleStop();

    QKRTLAPI virtual bool handleInput(int errCode = 0);
    QKRTLAPI virtual bool startInput();

    QKRTLAPI virtual bool handleOutput(int errCode = 0);
    QKRTLAPI virtual bool startOutput();

    QKRTLAPI virtual bool valid() const;

    inline int getHandle() const { return identifier(); }
    inline const OsHandle& getOsHandle() const { return osHandle_; }

    QKRTLAPI virtual void setHandle(int handle) ;
    QKRTLAPI virtual void setOsHandle(const OsHandle& handle);
protected:
    OsHandle osHandle_;
};

class IoHandler : public EventNode{
public:
    QKRTLAPI IoHandler(IoHandle& ioHandle);
    QKRTLAPI virtual ~IoHandler();

    QKRTLAPI virtual void close();

    QKRTLAPI virtual bool start();
    QKRTLAPI virtual void stop();

    QKRTLAPI virtual bool handleStart();
    QKRTLAPI virtual bool handleStop();
    QKRTLAPI virtual bool handle(int errCode = 0);

    inline int getHandle() const { return ioHandle_.getHandle(); }
    inline bool valid() const { return ioHandle_.valid(); }
    inline const OsHandle& getOsHandle() const { return ioHandle_.getOsHandle(); }
protected:
    IoHandle& ioHandle_;
};

}
#endif /**QKRTL_IO_HANDLE_H*/
