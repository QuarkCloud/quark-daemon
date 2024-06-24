
#include "qknet/SocketConnection.h"
#include "qkrtl/Logger.h"

namespace qknet {

SocketConnectionHandler::SocketConnectionHandler()
{
    //
}
SocketConnectionHandler::~SocketConnectionHandler()
{
    //
}
bool SocketConnectionHandler::allocInBuffer(qkrtl::Buffer& buffer)
{
    return false;
}
bool SocketConnectionHandler::freeInBuffer(qkrtl::Buffer& buffer)
{
    return false;
}
bool SocketConnectionHandler::freeOutBuffer(qkrtl::Buffer& buffer)
{
    return false;
}
bool SocketConnectionHandler::handleInput(qkrtl::Buffer& buffer)
{
    return false;
}
bool SocketConnectionHandler::startInput()
{
    return ioHandle_->startInput();
}
bool SocketConnectionHandler::handleOutput(qkrtl::Buffer& buffer)
{
    return false;
}
bool SocketConnectionHandler::startOutput()
{
    return ioHandle_->startOutput();
}


TcpSocketReader::TcpSocketReader(SocketIoHandle* ioHandle) :ioHandle_(ioHandle)
{
    //
}
TcpSocketReader::~TcpSocketReader()
{
    //
}
bool TcpSocketReader::handle(int errCode)
{
    return ioHandle_->handleInput(errCode);
}

TcpSocketWriter::TcpSocketWriter(SocketIoHandle* ioHandle) :ioHandle_(ioHandle)
{
    //
}
TcpSocketWriter::~TcpSocketWriter()
{
    //
}
bool TcpSocketWriter::handle(int errCode)
{
    return ioHandle_->handleOutput(errCode);
}


SocketConnectionHandle::SocketConnectionHandle(qkrtl::Poller& poller)
    : reader_(this) , writer_(this) , connected_(false) , poller_(poller) , 
    handler_(&dummyHandler_)
{
    LOGDEBUG("SocketConnectionHandle[%p] created", this);
}
SocketConnectionHandle::~SocketConnectionHandle()
{
    LOGDEBUG("SocketConnectionHandle[%p] Handle[%p] will be freed" , this , getHandle());
    final();
}
void SocketConnectionHandle::final()
{
    if (socket_.valid() == true)
    {
        LOGCRIT("SocketConnectionHandle[%p] Handle[%p] Address[%s] will be closed", 
            this, getHandle() , socket_.getFullAddress().c_str());
        socket_.final();
    }
}
void SocketConnectionHandle::close()
{
    LOGDEBUG("SocketConnectionHandle[%p] handle[%p] close , connected[%s]" ,
        this , getHandle() , connected_?"True":"False");
    //由handler主动调用，不需要再回调
    resetHandler(NULL);

    if (isMonitored() == false)
    {
        poller_.monitor(this);
    }
    ready();
}
void SocketConnectionHandle::execute()
{
    int counter = prepare();
    LOGDEBUG("SocketConnectionHandle[%p] Socket[%p] execute counter[%d]", 
        this, getHandle(), counter);

    int closeTimes = 0;
    if (reader_.closed() == false)
    {
        LOGDEBUG("SocketConnectionHandle[%p] Socket[%p]'s reader is not closed , actionName[%s] ",
            this, getHandle(), reader_.actionName());
        if (reader_.closing() == false)
            reader_.close();
        closeTimes++;
    }
    if (writer_.closed() == false)
    {
        LOGDEBUG("SocketConnectionHandle[%p] Socket[%p]'writer is not closed , actionName[%s] ",
            this, getHandle(), writer_.actionName());
        if (writer_.closing() == false)
            writer_.close();
        closeTimes++;
    }
    if (closeTimes > 0)
    {
        completed();
        ready();
        return;
    }

    if (reader_.isMonitored() == true)
        poller_.unmonitor(&reader_);
    if (writer_.isMonitored() == true)
        poller_.unmonitor(&writer_);

    poller_.unmonitor(this);
    doAsynDestroy();
}
bool SocketConnectionHandle::start()
{
    if (connected_ == false)
    {
        LOGERR("SocketConnectionHandle[%p] Socket[%p] is not connected , failed to start" , 
            this , getHandle());
        return false;
    }

    OVERLAPPED* rovlp = (OVERLAPPED*)&reader_;
    OVERLAPPED* wovlp = (OVERLAPPED*)&writer_;
    LOGINFO("SocketConnectionHandle[%p]'s Handle[%p] fullAddress[%s]" , 
        this , getHandle() , socket_.getFullAddress().c_str());
    LOGINFO("SocketConnectionHandle[%p]'s Handle[%p] read ovlp[%p] write ovlp[%p] start",
        this, getHandle(), rovlp, wovlp);

    qkrtl::Buffer buffer;
    if (handler_->allocInBuffer(buffer) == true)
        reader_.prepare(buffer);

    poller_.monitor(&reader_);
    poller_.monitor(&writer_);

    return handleStart();
}

void SocketConnectionHandle::doAsynDestroy()
{
    LOGDEBUG("SocketConnectionHandle[%p] will do asynch destroy", this);

    if (reader_.isMonitored() == true)
        poller_.unmonitor(&reader_);
    if (writer_.isMonitored() == true)
        poller_.unmonitor(&writer_);
    if (isMonitored() == true)
        poller_.unmonitor(this);

    poller_.asynDestroy(this);
}
void SocketConnectionHandle::swap(Socket& socket)
{
    LOGCRIT("SocketConnectionHandle[%p] assign socket[%p] , address[%s]" ,
        this , socket.getHandle() , socket.getFullAddress().c_str());
    socket_.swap(socket);
    reader_.reset(socket_);
    writer_.reset(socket_);
}
bool SocketConnectionHandle::handleStart()
{
    return handler_->handleStart();
}
bool SocketConnectionHandle::handleStop()
{
    LOGDEBUG("SocketConnectionHandle[%p] Socket[%p] handleStop" , this , getHandle());
    bool result = handler_->handleStop();
    resetHandler(NULL);

    if (reader_.isMonitored() == true)
        poller_.unmonitor(&reader_);
    if (writer_.isMonitored() == true)
        poller_.unmonitor(&writer_);

    if (isMonitored() == false)
    {
        poller_.monitor(this);
    }
    ready();
    return result;
}
bool SocketConnectionHandle::handleInput(int errCode)
{
    if (errCode != 0)
    {
        return handleError(errCode);
    }

    qkrtl::Buffer buffer;
    if (reader_.read(buffer) == true && buffer.empty() == false)
    {
        handler_->handleInput(buffer);
    }
    if (buffer.assigned() == true)
        handler_->freeInBuffer(buffer);

    if (handler_->allocInBuffer(buffer) == true && buffer.assigned() == true)
    {
        reader_.prepare(buffer);
    }

    return startInput();
}
bool SocketConnectionHandle::startInput()
{
    return reader_.startRead();
}
bool SocketConnectionHandle::handleOutput(int errCode)
{
    if (errCode != 0)
    {
        return handleError(errCode);
    }

    qkrtl::Buffer buffer;
    if (handler_->handleOutput(buffer) == false)
    {
        LOGERR("SocketConnectionHandle[%p] handle[%p] failed to output buffer",
            this, getHandle());
        return false;
    }
    else
    {
        LOGDEBUG("SocketConnectionHandle[%p] handle[%p] succeed to output buffer size[%d]",
            this, getHandle() , buffer.dataSize());
    }

    bool result = writer_.write(buffer);
    if (result == false)
    {
        LOGERR("SocketConnectionHandle[%p] handle[%p] failed to write buffer",
            this, getHandle());
    }

    handler_->freeOutBuffer(buffer);

    if (result == true)
        return startOutput();

    return result;
}
bool SocketConnectionHandle::startOutput()
{
    writer_.ready();
    return true;
}
int SocketConnectionHandle::forceCloseAll()
{
    int changedTimes = 0;
    if (reader_.closed() == false)
    {
        reader_.close();
        ++changedTimes;
    }
    if (reader_.closed() == true && reader_.isMonitored() == true)
        poller_.unmonitor(&reader_);

    if (writer_.closed() == false)
    {
        writer_.close();
        ++changedTimes;
    }
    if (writer_.closed() == true && writer_.isMonitored() == true)
        poller_.unmonitor(&writer_);

    return changedTimes;
}
bool SocketConnectionHandle::handleError(int errCode)
{
    LOGERR("SocketConnectionHandle[%p] handle[%p] handle an errCode[%d]" , 
        this , getHandle() , errCode);
    connected_ = false;

    int changedTimes = forceCloseAll() ;

    if (reader_.closed() == true && writer_.closed() == true && changedTimes > 0)
    {
        handleStop();
    }

    return true;
}
void SocketConnectionHandle::resetHandler(SocketConnectionHandler* handler)
{
    if (handler_ == handler)
        return;

    SocketConnectionHandler* oldHandler = handler_;
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

SocketConnectionHandlerManager::SocketConnectionHandlerManager(qkrtl::Poller& poller)
    :poller_(poller) , finaled_(false)
{
    //
}
SocketConnectionHandlerManager::~SocketConnectionHandlerManager()
{
    final();
}
void SocketConnectionHandlerManager::final()
{
    {
        std::unique_lock<std::mutex> locker(guard_);
        if (finaled_ == true)
            return;
        finaled_ = true;
    }
    clear();
}
bool SocketConnectionHandlerManager::insert(SocketConnectionHandler* handler)
{
    if (handler == NULL)
        return false;
    std::unique_lock<std::mutex> locker(guard_);
    std::set<SocketConnectionHandler*>::iterator hiter = handlers_.find(handler);
    if (hiter != handlers_.end())
        return false;
    handlers_.insert(handler);
    return true;
}
bool SocketConnectionHandlerManager::remove(SocketConnectionHandler* handler)
{
    if (handler == NULL)
        return false;
    std::unique_lock<std::mutex> locker(guard_);
    std::set<SocketConnectionHandler*>::iterator hiter = handlers_.find(handler);
    if (hiter == handlers_.end())
        return true;
    handlers_.erase(hiter);
    handler->close();

    return poller_.asynDestroy(handler);
}
bool SocketConnectionHandlerManager::find(SocketConnectionHandler* handler) const
{
    std::unique_lock<std::mutex> locker(guard_);
    std::set<SocketConnectionHandler*>::iterator hiter = handlers_.find(handler);
    return (hiter != handlers_.end());
}
void SocketConnectionHandlerManager::clear()
{
    std::unique_lock<std::mutex> locker(guard_);
    for (std::set<SocketConnectionHandler*>::iterator hiter = handlers_.begin(); 
        hiter != handlers_.end(); ++hiter)
    {
        SocketConnectionHandler* handler = (*hiter);
        if (handler == NULL)
            continue;

        handler->close();

        poller_.asynDestroy(handler);
    }
    handlers_.clear();
}


}
