
#include "qkrtl/Logger.h"
#include "qkhttp/Client.h"

namespace qkhttp {

 
ClientRequest::ClientRequest():finaled_(false)
{
    //
}
ClientRequest::~ClientRequest()
{
    final();
}
bool ClientRequest::init(const Request& request)
{
    RequestEncoder encoder(request);
    int encodedSize = encoder.calcSize();
    if (encodedSize <= 0)
        return false;

    if (outBuffer_.malloc(encodedSize) == false)
        return false;

    encodedSize = encoder.encode(outBuffer_);
    if (encodedSize <= 0)
        return false;
    outBuffer_.extend(encodedSize);

    return true;
}
bool ClientRequest::init(qkrtl::Buffer& request)
{
    outBuffer_.swap(request);
    return true;
}
void ClientRequest::final()
{
    if (finaled_ == true)
        return;
    finaled_ = true;

    if (outBuffer_.assigned() == true)
        outBuffer_.free();

    if (inStream_.empty() == false)
        inStream_.clear();
}
bool ClientRequest::request(qkrtl::Buffer& buffer)
{
    outBuffer_.swap(buffer);
    return (buffer.empty() == false);
}
bool ClientRequest::reply(qkrtl::Buffer& buffer)
{
    if (inStream_.push(buffer) == false)
        return false;
    buffer.clear();
    return true;
}
bool ClientRequest::replyEnd()
{
    qkrtl::Buffer buffer;
    if (inStream_.pop(buffer) == false)
        return false;

    ResponseDecoder decoder;
    int decodedSize = decoder.decode(buffer);
    if (decodedSize <= 0)
        return false;

    buffer.free();
    return onRely(decoder.response());
}
bool ClientRequest::onRely(const Response& response)
{
    return false;
}

ClientConnector::ClientConnector(qkrtl::Poller& poller, Client* owner)
    :qknet::ClientConnector(poller), owner_(owner)
{
    LOGCRIT("ClientConnector[%p] created", this);
}
ClientConnector::~ClientConnector()
{
    LOGCRIT("ClientConnector[%p] will be freed", this);
}
bool ClientConnector::handleConnectSucceed(qknet::Socket& socket)
{
    return owner_->handleConnectSucceed(socket);
}
bool ClientConnector::handleConnectFailed(int errCode)
{
    return owner_->handleConnectFailed(errCode);
}
bool ClientConnector::handleConnectCompleted()
{
    owner_ = NULL;
    return true;
}

Client::Client(qkrtl::Poller& poller)
    :finaled_(false), poller_(poller), connector_(NULL), connection_(NULL),request_(NULL)
{
    LOGDEBUG("Client[%p] created ", this);
}
Client::~Client()
{
    LOGDEBUG("Client[%p] will be freed", this);
}
bool Client::init(ClientRequest* request)
{
    std::unique_lock<std::mutex> locker(guard_);
    if (request == NULL)
        return false;

    request_ = request;
    return true;
}
void Client::final()
{
    std::unique_lock<std::mutex> locker(guard_);
    if (finaled_ == true)
        return;
    finaled_ = true;

    LOGCRIT("Clien[%p] will final", this);

    if (connector_ != NULL)
    {
        delete connector_;
        connector_ = NULL;
    }

    if (connection_ != NULL)
    {
        connection_->close();
        connection_ = NULL;
    }
}
bool Client::connect(const std::string& host, uint16_t port, int timeout)
{
    std::unique_lock<std::mutex> locker(guard_);
    if (connector_ != NULL || finaled_ == true)
        return false;

    connector_ = new ClientConnector(poller_, this);
    return connector_->connect(host, port, timeout);
}
bool Client::allocInBuffer(qkrtl::Buffer& buffer)
{
    buffer.malloc(1 << 16);
    return true;
}
bool Client::freeInBuffer(qkrtl::Buffer& buffer)
{
    buffer.free();
    return true;
}
bool Client::freeOutBuffer(qkrtl::Buffer& buffer)
{
    buffer.free();
    return true;
}
bool Client::handleInput(qkrtl::Buffer& buffer)
{
    if (request_ == NULL)
        return false;
    return request_->reply(buffer);
}
bool Client::handleOutput(qkrtl::Buffer& buffer)
{
    if (request_ == NULL)
        return false;

    if (request_->request(buffer) == false)
        return false;

    return (buffer.empty() == false);
}
bool Client::handleStop()
{
    std::unique_lock<std::mutex> locker(guard_);
    HANDLE handle = INVALID_HANDLE_VALUE;
    if (connection_ != NULL)
    {
        handle = qknet::SocketConnectionHandler::getHandle();
        LOGDEBUG("Client[%p] handle[%p] handleStop , connection reset",
            this, handle);
        connection_ = NULL;
        return true;
    }

    LOGERR("Client[%p] handle[%p] handleStop , connector and connection is NULL", this, handle);
    return false;
}
bool Client::handleConnectSucceed(qknet::Socket& socket)
{
    std::unique_lock<std::mutex> locker(guard_);
    if (connection_ != NULL)
    {
        LOGERR("Client[%p] handle[%p] address[%s]'s connection had exist",
            this, socket.getHandle(), socket.getFullAddress().c_str());
        return false;
    }
    else
    {
        LOGERR("Client[%p] handle[%p] address[%s]'s connected",
            this, socket.getHandle(), socket.getFullAddress().c_str());
    }

    connection_ = new qknet::SocketConnectionHandle(poller_);
    connection_->swap(socket);
    connection_->connected(true);
    connection_->resetHandler(this);
    connection_->start();
    return true;
}
bool Client::handleConnectFailed(int errCode)
{
    return true;
}

}

