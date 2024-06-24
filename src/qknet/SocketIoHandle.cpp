
#include "qknet/SocketIoHandle.h"


namespace qknet {

SocketIoHandle::SocketIoHandle()
{
    //
}
SocketIoHandle::~SocketIoHandle()
{
    final();
}
void SocketIoHandle::final()
{
    //
}
void SocketIoHandle::close()
{
    //
}
void SocketIoHandle::execute()
{
    //
}
void SocketIoHandle::doAsynDestroy()
{
    //
}
const HANDLE SocketIoHandle::getHandle() const
{
    return socket_.getHandle();
}
bool SocketIoHandle::valid() const
{
    return socket_.valid();
}

SocketIoHandler::SocketIoHandler() :ioHandle_(&dummyIoHandle_)
{
    //
}
SocketIoHandler::~SocketIoHandler()
{
    //
}
void SocketIoHandler::close()
{
    ioHandle_->close();
}
bool SocketIoHandler::handleStart()
{
    return false;
}
bool SocketIoHandler::handleStop()
{
    return false;
}
bool SocketIoHandler::handle(int errCode)
{
    return false;
}
const HANDLE SocketIoHandler::getHandle() const
{
    return ioHandle_->getHandle();
}
bool SocketIoHandler::valid() const
{
    return ioHandle_->valid();
}
void SocketIoHandler::resetHandle(SocketIoHandle* ioHandle)
{
    if (ioHandle == NULL)
        ioHandle_ = &dummyIoHandle_;
    else if(ioHandle_ != ioHandle)
        ioHandle_ = ioHandle;
}

}
