
#include "qknet/IoHandle.h"


namespace qknet {

IoHandle::IoHandle(qkrtl::Poller& poller):poller_(poller)
{
    //
}
IoHandle::~IoHandle()
{
    final();
}
void IoHandle::final()
{
    //
}
void IoHandle::close()
{
    //
}
void IoHandle::doAsynDestroy()
{
    //
}
void IoHandle::setHandle(int handle)
{
    qkrtl::IoHandle::setHandle(handle);
    qkrtl::IoHandle::setOsHandle(FsGetOsHandle(handle));
}
bool IoHandle::postAsynDestroy()
{
    qkrtl::Event event = qkrtl::Event::Initializer;
    event.dataType = qkrtl::Event::kDataTypeObj;
    event.data.object = this;
    event.executor = IoHandleAsynDestroy;
    return poller_.post(event);
}
bool IoHandle::valid() const
{
    return FsValid(getHandle());
}
void IoHandle::notifyOpen(int errCode)
{
    //
}
void IoHandle::notifyClose(int errCode)
{
    //
}
void IoHandle::notifyRead(int errCode)
{
    //
}
void IoHandle::notifyWrite(int bytes, int errCode)
{
    //
}

bool IoHandleAsynDestroy(const qkrtl::Event& event)
{
    if (event.dataType != qkrtl::Event::kDataTypeObj)
        return false;

    IoHandle* ioHandle = (IoHandle*)event.data.object;
    if (ioHandle == NULL)
        return false;

    ioHandle->doAsynDestroy();
    return true;
}
}
