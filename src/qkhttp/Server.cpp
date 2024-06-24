
#include "qkhttp/Server.h"
#include "qkrtl/Logger.h"

namespace qkhttp {

bool UriHandler::handle(const Request& request, Response& response)
{
    return false;
}

Session::Session(qknet::SocketConnectionHandlerManager& sessions):sessions_(sessions)
{
    inBuffer_.malloc(1 << 16);
}
Session::~Session()
{
    //
}
bool Session::allocInBuffer(qkrtl::Buffer& buffer)
{
    buffer.swap(inBuffer_);
    return true;
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
    RequestDecoder decoder;
    if (decoder.decode(buffer) <= 0)
        return false;

    return handleRequest(decoder.request());
}
bool Session::handleOutput(qkrtl::Buffer& buffer)
{
    return outStream_.pop(buffer);
}
bool Session::handleStop()
{
    return sessions_.remove(this);
}
bool Session::handleRequest(const Request& request)
{
    if (handler_ == NULL)
        return false;
    Response response;
    if (handler_->handle(request, response) == false)
        return false;

    ResponseEncoder encoder(response);
    int calcSize = encoder.calcSize();
    qkrtl::Buffer buffer;
    buffer.malloc(calcSize);
    int encodedSize = encoder.encode(buffer);
    if (encodedSize <= 0)
    {
        buffer.free();
        return false;
    }

    buffer.extend(encodedSize);

    outStream_.push(buffer);
    return startOutput();
}
bool Session::resetHandler(UriHandler* handler)
{
    handler_ = handler;
    return true;
}

Server::Server(qkrtl::Poller& poller)
    : poller_(poller), finaled_(false), acceptor_(NULL) , sessions_(poller) , handler_(NULL)
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
    sess->resetHandler(handler_);

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
bool Server::resetHandler(UriHandler* handler)
{
    std::unique_lock<std::mutex> locker(guard_);
    handler_ = handler;
    return true;
}

}
