
#include "Server.h"
#include "qkrtl/Logger.h"


Session::Session(qkrtl::Poller& poller, qknet::ConnectionManager& sessions)
    :qknet::Connection(poller) ,sessions_(sessions)
{
    LOGDEBUG("Session[%p] created ", this );
}
Session::~Session()
{
    LOGDEBUG("Session[%p] will be freed" , this);
}
bool Session::handleInput(int errCode)
{
    return true;
}
bool Session::handleOutput(int errCode)
{
    return true;
}
bool Session::handleError(int errCode)
{
    return true;
}
bool Session::handleStop()
{
    LOGDEBUG("Session[%p] handleStop", this);
    return sessions_.remove(this);
}

Server::Server(qkrtl::Poller& poller)
    :qknet::Acceptor(poller) , finaled_(false)
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
    if (qknet::Acceptor::init(port) == false)
        return false;
    return true;
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
    return 0;
}
bool Server::handleStop()
{
    std::unique_lock<std::mutex> locker(guard_);
    sessions_.clear();
    return true;
}



