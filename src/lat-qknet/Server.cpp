
#include "Server.h"
#include "qkrtl/Logger.h"


Session::Session(qknet::SocketConnectionHandlerManager& sessions)
    : sessions_(sessions) ,readSize_(0) , writedSize_(0)
{
    LOGDEBUG("Session[%p] created ", this);
}
Session::~Session()
{
    LOGDEBUG("Session[%p] will be freed", this);
}
bool Session::allocInBuffer(qkrtl::Buffer& buffer)
{
    bool result = buffer.malloc(1 << 16);
    LOGDEBUG("Session[%p] handle[%p] allocInBuffer[%p:%d]",
        this, getHandle(), buffer.cache(), buffer.capacity());
    return result;
}
bool Session::freeInBuffer(qkrtl::Buffer& buffer)
{
    LOGDEBUG("Session[%p] handle[%p] freeInBuffer[%p:%d:%d]",
        this, getHandle(), buffer.cache(), buffer.capacity() , buffer.dataSize());
    buffer.free();
    return true;
}
bool Session::freeOutBuffer(qkrtl::Buffer& buffer)
{
    LOGDEBUG("Session[%p] handle[%p] freeOutBuffer[%p:%d:%d]",
        this, getHandle(), buffer.cache(), buffer.capacity() , buffer.dataSize());
    if (buffer.empty() == false || buffer.assigned() == true)
    {
        LOGERR("Session[%p] handle[%p] freeOutBuffer maybe error",this, getHandle());
    }
    buffer.free();
    return true;
}
bool Session::handleInput(qkrtl::Buffer& buffer)
{
    readSize_ += buffer.dataSize();
    LOGDEBUG("Session[%p] handle[%p] handleInput [%d] bytes , readSize[%lld]", 
        this , getHandle() , buffer.dataSize() , readSize_);
    if (buffer.empty() == true || inStream_.push(buffer) == false)
    {
        buffer.free();
        return true;
    }
    else
    {
        buffer.clear();
    }

    return startOutput();
}
bool Session::handleOutput(qkrtl::Buffer& buffer)
{
    bool result = inStream_.pop(buffer);
    writedSize_ += buffer.dataSize();
    LOGDEBUG("Session[%p] handle[%p] handleOutput [%p:%d] bytes , writedSize[%lld]",
        this, getHandle(), buffer.cache() , buffer.dataSize() , writedSize_);
    return result;
}
bool Session::handleStop()
{
    LOGDEBUG("Session[%p] handleStop", this);
    return sessions_.remove(this);
}

Server::Server(qkrtl::Poller& poller)
    :poller_(poller), finaled_(false), acceptor_(NULL) , sessions_(poller)
{
    //
}
Server::~Server()
{
    final();
}
bool Server::init(uint16_t port)
{
    std::unique_lock<std::mutex> locker(guard_);
    if (acceptor_ != NULL)
        return false;
    acceptor_ = new qknet::SocketAcceptHandle(poller_);
    acceptor_->resetHandler(this);

    return acceptor_->init(port, SOMAXCONN);
}
void Server::final()
{
    std::unique_lock<std::mutex> locker(guard_);
    if (finaled_ == true)
        return;
    finaled_ = true;
    qknet::SocketAcceptHandle* acceptor = acceptor_;
    acceptor_ = NULL;

    if (acceptor != NULL)
        acceptor->close();

    sessions_.final();
}
bool Server::handleAccepted(qknet::Socket& socket)
{
    if (socket.valid() == false)
        return false;

    qknet::SocketConnectionHandle* conn = new qknet::SocketConnectionHandle(poller_);
    Session* sess = new Session(sessions_);
    conn->swap(socket);
    conn->connected(true);
    conn->resetHandler(sess);
    conn->start();

    return sessions_.insert(sess);
}
bool Server::handleStop()
{
    std::unique_lock<std::mutex> locker(guard_);
    LOGINFO("Server[%p] handle stop , acceptor[%p]", this, acceptor_);
    acceptor_ = NULL;
    sessions_.clear();
    return true;
}
