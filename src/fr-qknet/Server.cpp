
#include "Server.h"
#include "qkrtl/Logger.h"


Session::Session(qknet::SocketConnectionHandlerManager& sessions):sessions_(sessions)
{
    LOGDEBUG("Session[%p] created ", this );
}
Session::~Session()
{
    LOGDEBUG("Session[%p] will be freed" , this);
}
bool Session::allocInBuffer(qkrtl::Buffer& buffer)
{
    return buffer.malloc(1 << 16);
}
bool Session::freeInBuffer(qkrtl::Buffer& buffer)
{
    buffer.free();
    return true;
}
bool Session::freeOutBuffer(qkrtl::Buffer& buffer)
{
    buffer.free();
    return true;
}
bool Session::handleInput(qkrtl::Buffer& buffer)
{
    if (buffer.empty() == true || inStream_.push(buffer) == false)
    {
        buffer.free();
        return true;
    }

    return startOutput();
}
bool Session::handleOutput(qkrtl::Buffer& buffer)
{
    return inStream_.pop(buffer);
}
bool Session::handleStop()
{
    LOGDEBUG("Session[%p] handleStop", this);
    return sessions_.remove(this);
}

Server::Server(qkrtl::Poller& poller)
    :poller_(poller) , finaled_(false), acceptor_(NULL) , sessions_(poller)
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
    LOGINFO("Server[%p] handle stop , acceptor[%p]", this , acceptor_);
    acceptor_ = NULL;

    sessions_.clear();
    return true;
}



