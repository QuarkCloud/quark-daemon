#include "Server.h"
#include "qkrtl/Logger.h"
#include "qkrtl/FileSystem.h"

Session::Session(qkrtl::Poller& poller):qkitc::Connection(poller)
{
    //
}
Session::~Session()
{
    //
}
bool Session::handleInput(int errCode)
{
    int handle = getHandle();
    std::unique_lock<std::mutex> locker(guard_);
    if (errCode != 0)
    {
        LOGERR("Session[%p] handle[%d] handleInput failed , errCode[%d]",
            this, handle, errCode);
        return false;
    }

    qkrtl::Buffer buffer;
    int dataSize = FsRead(handle, buffer);
    if (dataSize > 0)
    {
        LOGDEBUG("Session[%p] handle[%d] handleInput succeed to read data , size[%d]",
            this, handle, dataSize);

        FsWrite(handle, buffer);
    }
    else
    {
        LOGERR("Session[%p] handle[%d] handleInput failed to read data , size[%d]",
            this, handle, dataSize);
    }

    return startOutput();
}
bool Session::handleOutput(int errCode)
{
    int handle = getHandle();
    std::unique_lock<std::mutex> locker(guard_);
    if (errCode != 0)
    {
        LOGERR("Session[%p] handle[%d] handleOutput failed , errCode[%d]",
            this, handle, errCode);
        return false;
    }
    return true;
}
Server::Server(qkrtl::Poller& poller):qkitc::Acceptor(poller) , finaled_(false)
{
    //
}
Server::~Server()
{
    final();
}
void Server::final()
{
    std::unique_lock<std::mutex> locker(guard_);
    if (finaled_ == true)
        return;
    finaled_ = true;

    close();
}
bool Server::handleError(int errCode)
{
    return true;
}
bool Server::handleAccept(int* handles, int size)
{
    std::unique_lock<std::mutex> locker(guard_);

    for (int idx = 0; idx < size; ++idx)
    {
        Session* sess = new Session(poller_);
        int handle = handles[idx];
        sess->setHandle(handle);
        FsNotifier(handle, sess);
        sessions_.push_back(sess);
    }
    return true;
}



