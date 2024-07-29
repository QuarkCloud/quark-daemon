
#include "Server.h"
#include "qkrtl/Logger.h"


Session::Session(qkrtl::Poller& poller, qknet::ConnectionManager& sessions)
    : qknet::Connection(poller) , sessions_(sessions) ,readSize_(0) , writedSize_(0)
{
    LOGDEBUG("Session[%p] created ", this);
}
Session::~Session()
{
    LOGDEBUG("Session[%p] will be freed", this);
}
bool Session::handleRead(qkrtl::Buffer& buffer)
{
    int dataSize = buffer.dataSize();
    if (inStream_.push(buffer) == false)
        return false;

    readSize_ += dataSize;
    buffer.clear();

    return startOutput();
}
bool Session::handleWrited(qkrtl::Buffer& buffer)
{
    int dataSize = buffer.dataSize();
    writedSize_ += dataSize;
    buffer.free();
    return true;
}
bool Session::handleOutput(int errCode)
{
    LOGDEBUG("Session[%p] handle[%d] handleOutput , errCode[%d]", this, getHandle(), errCode);
    if (inStream_.pop(outBuffer_) == false)
        return false;

    return write(outBuffer_);
}
bool Session::handleStop()
{
    LOGDEBUG("Session[%p] handleStop", this);
    return sessions_.remove(this);
}

Server::Server(qkrtl::Poller& poller)
    :qknet::Acceptor(poller) , poller_(poller), finaled_(false)
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
    qknet::Acceptor::final();
    sessions_.final();
}
int Server::handleAccept(int* handles, int size)
{
    int counter = 0;
    for (int idx = 0; idx < size; ++idx)
    {
        int handle = handles[idx];

        Session* sess = new Session(poller_, sessions_);
        sess->setHandle(handle);
        sess->connected(true);
        FsNotifier(handle, sess);

        sessions_.insert(sess);
        sess->start();
        ++counter;
    }

    return counter;
}
bool Server::handleStop()
{
    std::unique_lock<std::mutex> locker(guard_);
    LOGINFO("Server[%p] handle stop , acceptor[%d]", this, getHandle());
    sessions_.clear();
    return true;
}
