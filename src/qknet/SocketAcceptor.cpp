
#include "qknet/SocketAcceptor.h"
#include "qkrtl/Logger.h"

namespace qknet {

SocketAcceptor::SocketAcceptor(SocketIoHandle* ioHandle):ioHandle_(ioHandle)
{
    //
}
SocketAcceptor::~SocketAcceptor()
{
    //
}

bool SocketAcceptor::handle(int errCode)
{
    if(errCode == 0)
        LOGDEBUG("SocketAcceptor[%p] listener[%p] handle a success input" , this , getHandle());
    else
        LOGERR("SocketAcceptor[%p] listener[%p] handle an errCode[%d]", 
            this , getHandle(), errCode);

    return ioHandle_->handleInput(errCode);
}

SocketAcceptHandler::SocketAcceptHandler()
{
    //
}
SocketAcceptHandler::~SocketAcceptHandler()
{
    //
}
bool SocketAcceptHandler::handleAccepted(Socket& socket)
{
    return false;
}
bool SocketAcceptHandler::handleStop()
{
    return false;
}

SocketAcceptHandle::SocketAcceptHandle(qkrtl::Poller& poller)
    :finaled_(false) , acceptor_(this) , poller_(poller) , handler_(&dummyHandler_)
{
    //
}
SocketAcceptHandle::~SocketAcceptHandle()
{
    final();
}
bool SocketAcceptHandle::init(uint16_t port, int backlog)
{
    std::unique_lock<std::mutex> locker(guard_);
    SocketAddr addr;
    if (addr.assign("", port) == false)
        return false;

    TcpSocket listener;
    if (listener.init() == false)
        return false;

    if (listener.bind(addr) == false || listener.listen(backlog) == false)
    {
        listener.final();
        return false;
    }

    socket_ = listener;
    acceptor_.reset(listener);

    return poller_.monitor(&acceptor_);
}
void SocketAcceptHandle::final()
{
    std::unique_lock<std::mutex> locker(guard_);
    if (finaled_ == true)
        return;
    finaled_ = true;

    if (socket_.valid() == true)
    {
        LOGDEBUG("SocketAcceptHandle[%p] Handle[%p] will be closed", this, getHandle());
        socket_.final();
    }
}
void SocketAcceptHandle::doAsynDestroy()
{
    if (acceptor_.isMonitored() == true)
        poller_.unmonitor(&acceptor_);
    if (isMonitored() == true)
        poller_.unmonitor(this);

    LOGDEBUG("SocketAcceptHandle[%p] will do asynch destroy" , this);

    poller_.asynDestroy(this);
}
bool SocketAcceptHandle::start()
{
    LOGERR("SocketAcceptHandle[%p] Socket[%p]'s start no implemented" ,
        this , getHandle());
    return false;
}
void SocketAcceptHandle::close()
{
    LOGDEBUG("SocketAcceptHandle[%p] listener[%p] close [%s]",
        this, getHandle() , acceptor_.actionName());

    //由handler主动调用，不需要再回调
    resetHandler(NULL);

    if (isMonitored() == false)
    {
        poller_.monitor(this);
    }
    ready();
}
void SocketAcceptHandle::execute()
{
    int counter = prepare();
    LOGDEBUG("SocketAcceptHandle[%p] Socket[%p] execute counter[%d]", this, getHandle() , counter);
    if (acceptor_.closed() == false)
    {
        LOGDEBUG("SocketAcceptHandle[%p] Socket[%p] is not closed , actionName[%s] ", 
            this, getHandle() , acceptor_.actionName());
        if (acceptor_.closing() == false)
            acceptor_.close();
        completed();
        ready();
        return;
    }

    if (acceptor_.isMonitored() == true)
        poller_.unmonitor(&acceptor_);

    poller_.unmonitor(this);
    doAsynDestroy();
}
bool SocketAcceptHandle::handleStop()
{
    LOGDEBUG("SocketAcceptHandle[%p] Socket[%p] handleStop", this, getHandle());

    bool result= handler_->handleStop();
    resetHandler(NULL);

    if (acceptor_.isMonitored() == true)
        poller_.unmonitor(&acceptor_);

    if (isMonitored() == false)
    {
        poller_.monitor(this);
    }
    ready();
    return result;
}
bool SocketAcceptHandle::handleInput(int errCode)
{
    if (errCode != 0)
    {
        LOGERR("SocketAcceptHandle[%p] listener[%p] recv an errCode[%d] , action[%s]" , 
            this , getHandle() , errCode , acceptor_.actionName());

        if (acceptor_.closing() == false)
        {
            acceptor_.close();
        }
    }

    if (acceptor_.closing() == true)
    {
        LOGINFO("SocketAcceptHandle[%p] listener[%p] is closing" , this , getHandle());
        return true;
    }
    if (acceptor_.closed() == true)
    {
        LOGINFO("SocketAcceptHandle[%p] listener[%p] had closed", this, getHandle());
        handleStop();
        return true;
    }

    Socket conn;
    if (acceptor_.accept(conn) == false)
    {
        LOGERR("SocketAcceptHandle[%p] listener[%p] failed to accept a new socket" , 
            this , getHandle() );
        return false;
    }
    else
    {
        LOGINFO("SocketAcceptHandle[%p] listener[%p]  succeed to accept a new socket[%p]", 
            this , getHandle() , conn.getHandle());
    }
    HANDLE connHandle = conn.getHandle();

    if (handler_->handleAccepted(conn) == false)
    {
        LOGERR("SocketAcceptHandle[%p] listener[%p] failed to handle new socket[%p]", 
            this , getHandle() , conn.getHandle());
        conn.final();
        return false;
    }
    else
    {
        LOGINFO("SocketAcceptHandle[%p] listener[%p] succeed to handle new socket[%p]",
            this , getHandle(), connHandle);
    }

    return startInput();;
}
bool SocketAcceptHandle::startInput()
{
    return acceptor_.startAccept();
}
void SocketAcceptHandle::resetHandler(SocketAcceptHandler* handler)
{
    if (handler_ == handler)
        return;

    SocketAcceptHandler* oldHandler = handler_;
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

}
