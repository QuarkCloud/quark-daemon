
#include "qkrtl/Logger.h"
#include "qkitc/IoHandle.h"

namespace qkitc {

IoHandle::IoHandle()
{
    //
}
IoHandle::~IoHandle()
{
    //
}
void IoHandle::close()
{
    //
}
bool IoHandle::handleStart()
{
    return false;
}
bool IoHandle::handleStop()
{
    return false;
}
bool IoHandle::handleInput(int errCode)
{
    return false;
}
bool IoHandle::startInput()
{
    return false;
}
bool IoHandle::handleOutput(int errCode)
{
    return false;
}
bool IoHandle::startOutput()
{
    return false;
}
void IoHandle::setHandle(int handle)
{
    identifier_ = handle;
}
void IoHandle::notifyOpen(int errCode)
{
    if(errCode == 0)
        handleStart();
}
void IoHandle::notifyClose(int errCode)
{
    handleStop();
}
void IoHandle::notifyRead(int errCode)
{
    //
}
void IoHandle::notifyWrite(int bytes, int errCode)
{
    //
}

}
