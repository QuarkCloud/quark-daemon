
#include "Client.h"
#include "qkrtl/Logger.h"

ClientConnector::ClientConnector(qkrtl::Poller& poller, Client* owner)
    :qknet::ClientConnector(poller) ,owner_(owner)
{
    LOGCRIT("ClientConnector[%p] created", this);
}
ClientConnector::~ClientConnector()
{
    LOGCRIT("ClientConnector[%p] will be freed" , this);
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
    :finaled_(false) ,poller_(poller) ,connector_(NULL) , connection_(NULL) , 
    maxTimes_(0) , bufferSize_(0) , expectSize_(0) , readSize_(0) , writedSize_(0)
{
    LOGDEBUG("Client[%p] created ", this);
}
Client::~Client()
{
    LOGDEBUG("Client[%p] will be freed", this);
}
bool Client::init(int maxTimes, int bufferSize)
{
    maxTimes_ = maxTimes;
    bufferSize_ = bufferSize;

    expectSize_ = maxTimes;
    expectSize_ *= bufferSize_;

    if (maxTimes_ <= 0 || bufferSize_ <= 0)
        return false;

    if (inBuffer_.malloc(bufferSize_) == false ||
        outBuffer_.malloc(bufferSize_) == false)
        return false;

    char* out = outBuffer_.cache();
    int size = outBuffer_.capacity();
    if (out == NULL || size <= 0)
        return false;
    ::memset(out, 0, size);
    return true;
}
void Client::final()
{
    std::unique_lock<std::mutex> locker(guard_);
    if (finaled_ == true)
        return ;
    finaled_ = true;

    LOGCRIT("Clien[%p] will final" , this);

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

    inBuffer_.free();
    outBuffer_.free();
}
bool Client::connect(const std::string& host, uint16_t port, int timeout)
{
    std::unique_lock<std::mutex> locker(guard_);
    if (connector_ != NULL || finaled_ == true)
        return false;

    connector_ = new ClientConnector(poller_ , this);
    return connector_->connect(host, port, timeout);
}
bool Client::allocInBuffer(qkrtl::Buffer& buffer)
{
    buffer.swap(inBuffer_);
    return true;
}
bool Client::freeInBuffer(qkrtl::Buffer& buffer)
{
    inBuffer_.swap(buffer);
    inBuffer_.squish();
    return true;
}
bool Client::freeOutBuffer(qkrtl::Buffer& buffer)
{
    outBuffer_.discard();
    return true;
}
bool Client::handleInput(qkrtl::Buffer& buffer)
{
    readSize_ += buffer.dataSize();

    LOGCRIT("Client[%p] handle[%p] handleInput , readSize[%lld] dataSize[%d]",
        this, qknet::SocketConnectionHandler::getHandle() , readSize_ , buffer.dataSize());

    buffer.discard();

    if (readSize_ < writedSize_)
        return true;

    if (readSize_ >= expectSize_)
    {
        LOGCRIT("Client[%p] handle[%p] time elapse completed",
            this, qknet::SocketConnectionHandler::getHandle());
        timeElapse_.stop();
        states_.notify(maxTimes_);
        return true;
    }

    return startOutput();
}
bool Client::handleOutput(qkrtl::Buffer& buffer)
{
    if (writedSize_ == 0)
    {
        LOGCRIT("Client[%p] handle[%p] time elapse start" , 
            this , qknet::SocketConnectionHandler::getHandle());
        timeElapse_.start();
    }
    LOGDEBUG("Client[%p] handle[%p] handleOutput , writedSize[%lld]",
        this, qknet::SocketConnectionHandler::getHandle() , writedSize_);

    if (writedSize_ >= expectSize_)
    {
        return false;
    }

    int64_t* outData = (int64_t*)outBuffer_.cache();
    if (outData == NULL)
        return false;

    *outData = writedSize_;
    if(outBuffer_.extend(bufferSize_) == false)
    { 
        return false;
    }
    writedSize_ += bufferSize_;
    buffer.refer(outBuffer_);
    return true;
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

    LOGERR("Client[%p] handle[%p] handleStop , connector and connection is NULL", this , handle);
    return false;
}
bool Client::handleConnectSucceed(qknet::Socket& socket)
{
    std::unique_lock<std::mutex> locker(guard_);
    if (connection_ != NULL)
    {
        LOGERR("Client[%p] handle[%p] address[%s]'s connection had exist" , 
            this , socket.getHandle() , socket.getFullAddress().c_str());
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
    std::unique_lock<std::mutex> locker(guard_);
    states_.notify(kConnectFailed);
    return true;
}
bool Client::waitForCompleted()
{
    return states_.waitFor({ maxTimes_ , kConnectFailed }, 60 * 1000);
}
