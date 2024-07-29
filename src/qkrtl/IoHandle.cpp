
#include "qkrtl/IoHandle.h"

namespace qkrtl {


IoHandle::IoHandle() : osHandle_(kDefaultHandleValue)
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
    return true;
}
bool IoHandle::startInput()
{
    return true;
}
bool IoHandle::handleOutput(int errCode)
{
    return true;
}
bool IoHandle::startOutput()
{
    return true;
}
bool IoHandle::valid() const
{
    return false;
}
void IoHandle::setHandle(int handle)
{
    identifier_ = handle;
}
void IoHandle::setOsHandle(const OsHandle& handle)
{ 
    osHandle_ = handle; 
}

IoHandler::IoHandler(IoHandle& ioHandle) : ioHandle_(ioHandle)
{
    //
}
IoHandler::~IoHandler()
{
    //
}
void IoHandler::close()
{
    //
}
bool IoHandler::start()
{
    return false;
}
void IoHandler::stop()
{
    //
}
bool IoHandler::handleStart()
{
    return false;
}
bool IoHandler::handleStop()
{
    return false;
}
bool IoHandler::handle(int errCode)
{
    return false;
}

}