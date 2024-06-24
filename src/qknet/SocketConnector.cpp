
#include "qknet/SocketConnector.h"
#include "qkrtl/Logger.h"

namespace qknet {

SocketConnector::SocketConnector(SocketIoHandle* ioHandle):ioHandle_(ioHandle)
{
    //
}
SocketConnector::~SocketConnector()
{
    //
}
bool SocketConnector::handle(int errCode)
{
    return ioHandle_->handleInput(errCode);
}

SocketConnectTimeout::SocketConnectTimeout(SocketIoHandle* ioHandle)
    :ioHandle_(ioHandle)
{
    //
}
SocketConnectTimeout::~SocketConnectTimeout()
{
    //
}
void SocketConnectTimeout::execute()
{
    ioHandle_->handleInput(SocketConnectHandle::kTimeout);
}

SocketConnectHandler::SocketConnectHandler()
{

}
SocketConnectHandler::~SocketConnectHandler()
{
    //
}
bool SocketConnectHandler::handleConnectSucceed(Socket& socket)
{
    return false;
}
bool SocketConnectHandler::handleConnectFailed(int errCode)
{
    return false;
}
bool SocketConnectHandler::handleConnectCompleted()
{
    return false;
}
SocketConnectHandle::SocketConnectHandle(qkrtl::Poller& poller)
    :finaled_(false) , handled_(false) , poller_(poller) , timeout_(this) , connector_(this) ,
    handler_(&dummyHandler_) 
{
    LOGDEBUG("SocketConnectHandle[%p] created", this);
}
SocketConnectHandle::~SocketConnectHandle()
{
    LOGDEBUG("SocketConnectHandle[%p] will be freed" , this);
    final();
}
bool SocketConnectHandle::connect(const std::string& host, uint16_t port, int timeout)
{
    std::unique_lock<std::mutex> locker(guard_);
    if (socket_.init() == false)
    {
        LOGERR("SocketConnectHandle[%p] failed to init a socket" , this);
        return false;
    }
    else
    {
        LOGDEBUG("SocketConnectHandle[%p] handle[%p] init a socket", this , getHandle());
    }
    socket_.reuse(true);
    connector_.reset(socket_);

    timeout_.init(qkrtl::UTimeNow() + timeout * 1000);
    poller_.monitor(&timeout_);
    poller_.monitor(&connector_);
    if(connector_.connect(host , port) == false)
    {
        LOGERR("SocketConnectHandle[%p] handle[%p] failed to connect[%s:%hu]", 
            this , socket_.getHandle() , host.c_str() , port);
        poller_.unmonitor(&timeout_);
        poller_.unmonitor(&connector_);
        return false;
    }

    OVERLAPPED* ovlp = (OVERLAPPED*)&connector_;
    LOGINFO("SocketConnectHandle[%p]'s Handle[%p] create ovlp[%p]", this , getHandle(), ovlp);
    return true;
}
void SocketConnectHandle::final()
{
    std::unique_lock<std::mutex> locker(guard_);
    if (finaled_ == true)
        return;
    finaled_ = true;

    if (valid() == true && handled_ == false)
    {
        socket_.final();
        return;
    }
}
void SocketConnectHandle::close()
{
    LOGDEBUG("SocketConnectHandle[%p] socket[%p] close [%s]",
        this, getHandle(), connector_.actionName());

    //由handler主动调用，不需要再回调
    resetHandler(NULL);

    //不管怎么样，都先停止timeout
    if (timeout_.isMonitored() == true)
        poller_.unmonitor(&timeout_);

    if (isMonitored() == false)
    {
        poller_.monitor(this);
    }
    ready();
}
void SocketConnectHandle::execute()
{
    if (timeout_.isMonitored() == true)
        poller_.unmonitor(&timeout_);

    int counter = prepare();
    LOGDEBUG("SocketConnectHandle[%p] Socket[%p] execute counter[%d]", this, getHandle(), counter);
    if (connector_.closed() == false)
    {
        LOGDEBUG("SocketConnectHandle[%p] Socket[%p] is not closed , actionName[%s] ",
            this, getHandle(), connector_.actionName());
        if (connector_.closing() == false)
            connector_.close();
        completed();
        ready();
        return;
    }

    if (connector_.isMonitored() == true)
        poller_.unmonitor(&connector_);

    poller_.unmonitor(this);
    doAsynDestroy();
}
void SocketConnectHandle::doAsynDestroy()
{
    LOGDEBUG("SocketConnectHandle[%p] will do asynch destroy", this);
    if (timeout_.isMonitored() == true)
        poller_.unmonitor(&timeout_);
    if(connector_.isMonitored() == true)
        poller_.unmonitor(&connector_);
    if (isMonitored() == true)
        poller_.unmonitor(this);

    poller_.asynDestroy(this);
}
bool SocketConnectHandle::handleStop()
{
    LOGDEBUG("SocketConnectHandle[%p] socket[%p] handleStop",this, getHandle());
    bool result = handler_->handleConnectCompleted();

    resetHandler(NULL);

    if (connector_.isMonitored() == true)
        poller_.unmonitor(&connector_);
    if (isMonitored() == false)
    {
        poller_.monitor(this);
    }
    ready();
    return result;
}
bool SocketConnectHandle::handleInput(int errCode)
{
    //只要有一个返回，先取消timeout判断
    if(timeout_.isMonitored() == true)
        poller_.unmonitor(&timeout_);

    bool result = false;
    if (errCode == 0 && connector_.connected() == true)
    {
        handled_ = handler_->handleConnectSucceed(socket_);
        result = handled_;
    }
    else
    {
        result = handler_->handleConnectFailed(errCode);
    }

    if(connector_.doing() == true || connector_.done() == true)
    { 
        connector_.close();
    }
    
    if (connector_.closed() == true)
    {
        handleStop();
    }

    resetHandler(NULL);
    return result;
}
void SocketConnectHandle::resetHandler(SocketConnectHandler* handler)
{
    if (handler_ == handler)
        return;

    SocketConnectHandler* oldHandler = handler_;
    if (handler == NULL)
    {
        if (handler_ == &dummyHandler_)
            return;
        handler_ = &dummyHandler_;
    }
    else
    {
        handler_ = handler;
        handler_->resetHandle(this);
    }

    if (oldHandler != &dummyHandler_)
        oldHandler->resetHandle(NULL);
}

ClientConnector::ClientConnector(qkrtl::Poller& poller)
    :finaled_(false), poller_(poller), connector_(NULL)
{
    LOGDEBUG("ClientConnector[%p] created", this);
}
ClientConnector::~ClientConnector()
{
    LOGDEBUG("ClientConnector[%p] destroyed", this);
    final();
}
bool ClientConnector::connect(const std::string& host, uint16_t port, int timeout)
{
    std::unique_lock<std::mutex> locker(guard_);
    if (connector_ != NULL || finaled_ == true)
        return false;

    connector_ = new qknet::SocketConnectHandle(poller_);
    connector_->resetHandler(this);
    return connector_->connect(host, port, timeout);
}
void ClientConnector::final()
{
    std::unique_lock<std::mutex> locker(guard_);
    if (finaled_ == true)
        return;
    finaled_ = true;
}
bool ClientConnector::handleConnectSucceed(qknet::Socket& socket)
{
    LOGCRIT("ClientConnector[%p] Handle[%p] Address[%s] connected",
        this, getHandle(), socket.getFullAddress().c_str());
    return false;
}
bool ClientConnector::handleConnectFailed(int errCode)
{
    qknet::Socket& socket = ioHandle_->socket();
    LOGERR("ClientConnector[%p] Handle[%p] Address[%s] recv an errCode[%d]",
        this, getHandle(), socket.getFullAddress().c_str(), errCode);
    return true;
}
bool ClientConnector::handleConnectCompleted()
{
    qknet::Socket& socket = ioHandle_->socket();
    LOGINFO("ClientConnector[%p] Handle[%p] Address[%s] handleConnectCompleted",
        this, getHandle(), socket.getFullAddress().c_str());
    return false;
}


}
