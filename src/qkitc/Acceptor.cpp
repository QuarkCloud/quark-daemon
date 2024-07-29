
#include "qkitc/Acceptor.h"
#include "qkrtl/FileSystem.h"
#include "qkrtl/Logger.h"

namespace qkitc {

Acceptor::Acceptor(qkrtl::Poller& poller)
    :poller_(poller) ,acceptor_(*this , qkrtl::IoEvent::kIoAccept) 
{
    poller_.monitor(&acceptor_);
}
Acceptor::~Acceptor()
{
    //
}
bool Acceptor::create(const std::string& name)
{
    int index = FsCreate(name, this);
    setHandle(index);
    return valid();
}
void Acceptor::close()
{
    //
}
bool Acceptor::handleStart()
{
    return true;
}
bool Acceptor::handleStop()
{
    return true;
}
bool Acceptor::handleInput(int errCode)
{
    if (errCode != 0)
    {
        LOGERR("Acceptor[%p] handleInput failed , errCode[%d]" , this , errCode);
        return handleError(errCode);
    }

    const int kHandleMaxSize = 64;
    int handles[kHandleMaxSize];

    qkrtl::Buffer buffer((char*)&handles, sizeof(handles));

    int dataSize = FsRead(getHandle(), buffer);
    if (dataSize <= 0)
        return false;

    int handleSize = dataSize / sizeof(int);

    return handleAccept(handles, handleSize);
}
bool Acceptor::handleError(int errCode)
{
    return true;
}
bool Acceptor::handleAccept(int* handles, int size)
{
    return true;
}
bool Acceptor::startInput()
{
    return true;
}
bool Acceptor::valid() const
{
    return FsValid(getHandle());
}
void Acceptor::notifyRead(int errCode)
{
    acceptor_.handleCompleted(errCode);
}
}