
#include "qknet/Acceptor.h"
#include "qkrtl/Logger.h"
#include "qkrtl/FileSystem.h"
#include "qkrtl/OsHandle.h"

namespace qknet {

Acceptor::Acceptor(qkrtl::Poller& poller)
    :IoHandle(poller) , finaled_(false) , acceptor_(*this)
{
    //
}
Acceptor::~Acceptor()
{
    final();
}
bool Acceptor::init(uint16_t port)
{
    const int kUrlMaxSize = 1024;
    char url[kUrlMaxSize] = { '\0' };

    int ulen = ::sprintf(url, "tcp://0.0.0.0");
    if (port != 0)
        ::sprintf(url + ulen, ":%hu", port);

    int handle = FsCreate(url, this);
    if (FsValid(handle) == false)
        return false;

    setHandle(handle);
    osHandle_ = FsGetOsHandle(handle);

    qkrtl::EventNode* node = &acceptor_;
    LOGDEBUG("Acceptor[%p] handle[%d] osHandle[%p] will monitor eventNode[%p]" ,
        this , handle , osHandle_ , node);

    return poller_.monitor(&acceptor_);
}
void Acceptor::final()
{
    std::unique_lock<std::mutex> locker(guard_);
    if (finaled_ == true)
        return;
    finaled_ = true;
}
void Acceptor::doAsynDestroy()
{
    
}
void Acceptor::close()
{
    //
}

bool Acceptor::handleStop()
{
    return true;
}
bool Acceptor::handleInput(int errCode)
{
    int handle = getHandle();
    if (errCode != 0)
    {
        LOGERR("Acceptor[%p] handle[%d] failed to handleInput , errCode[%d]" , 
            this , handle, errCode);
        return false;
    }
    
    const int kHandleMaxSize = 64;
    int handles[kHandleMaxSize];
    qkrtl::Buffer buffer((char*)handles, sizeof(handles));
    int dataSize = FsRead(handle , buffer);
    if (dataSize <= 0)
        return false;

    int handleCount = dataSize / sizeof(int);
    return (handleAccept(handles, handleCount) > 0);
}
int Acceptor::handleAccept(int* handles, int size)
{
    int counter = 0;
    for (int idx = 0; idx < size; ++idx)
    {
        int handle = handles[idx];
        FsClose(handle);
        ++counter;
    }
    return counter;
}
void Acceptor::notifyRead(int errCode)
{
    acceptor_.handleCompleted(errCode);
}

}
