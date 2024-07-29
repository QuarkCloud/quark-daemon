
#ifndef QKITC_IO_HANDLE_H
#define QKITC_IO_HANDLE_H 1

#include <time.h>
#include <string>
#include "qkrtl/RingBuffer.h"
#include "qkrtl/Buffer.h"
#include "qkrtl/IoHandle.h"
#include "qkrtl/IoEvent.h"
#include "qkrtl/FileSystem.h"

#include "qkitc/Compile.h"


namespace qkitc {

/**
    不实现和FileSystem之间的交互。用于管理对象本身的行为。
*/
class IoHandle : public qkrtl::IoHandle , public qkrtl::FileNotifier{
public:
    QKITCAPI IoHandle();
    QKITCAPI virtual ~IoHandle();

    QKITCAPI virtual void close();

    QKITCAPI virtual bool handleStart();
    QKITCAPI virtual bool handleStop();

    QKITCAPI virtual bool handleInput(int errCode = 0);
    QKITCAPI virtual bool startInput();

    QKITCAPI virtual bool handleOutput(int errCode = 0);
    QKITCAPI virtual bool startOutput();

    QKITCAPI void setHandle(int handle);

protected:
    QKITCAPI virtual void notifyOpen(int errCode);
    QKITCAPI virtual void notifyClose(int errCode);
    QKITCAPI virtual void notifyRead(int errCode);
    QKITCAPI virtual void notifyWrite(int bytes , int errCode);
};
}

#endif /**QKITC_IO_HANDLE_H*/
