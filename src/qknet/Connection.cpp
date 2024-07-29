
#include "qknet/Connection.h"
#include "qkrtl/Logger.h"

namespace qknet {

Connection::Connection(qkrtl::Poller& poller)
    : IoHandle(poller) , reader_(*this) , writer_(*this) ,
    finaled_(false) , connected_(false)
{
    LOGDEBUG("Connection[%p] created", this);
}
Connection::~Connection()
{
    LOGDEBUG("Connection[%p] Handle[%p] will be freed" , this , getOsHandle());
    final();
}
void Connection::final()
{
    {
        std::unique_lock<std::mutex> locker(guard_);
        if (finaled_ == true)
            return;
        finaled_ = true;
    }

    close();

    if (reader_.isMonitored() == true)
        poller_.unmonitor(&reader_);
    if (writer_.isMonitored() == true)
        poller_.unmonitor(&writer_);
}
void Connection::close()
{
    int handle = getHandle();
    if (FsValid(handle) == true)
    {
        FsClose(getHandle());
        setHandle(::kFsInvalidIndex);
    }
}
bool Connection::start()
{
    int handle = getHandle();
    if (connected_ == false)
    {
        LOGERR("Connection[%p] handle[%d] is not connected , failed to start" , this , handle);
        return false;
    }
    LOGCRIT("Connection[%p] handle[%d] connected , succeed to start",this, handle);

    LOGDEBUG("Connection[%p] handle[%d] osHandle[%p] will monitor eventNode , reader[%p] writer[%p]",
        this, handle, osHandle_, &reader_ , &writer_);

    poller_.monitor(&reader_);
    poller_.monitor(&writer_);

    startOutput();

    return true;
}

void Connection::doAsynDestroy()
{
    LOGDEBUG("Connection[%p] will do asynch destroy", this);

    if (reader_.isMonitored() == true)
        poller_.unmonitor(&reader_);
    if (writer_.isMonitored() == true)
        poller_.unmonitor(&writer_);

    poller_.asynDestroy(this);
}
bool Connection::handleStart()
{
    connected_ = true;
    return start();
}
bool Connection::handleStop()
{
    return true;
}
bool Connection::handleInput(int errCode)
{
    LOGDEBUG("Connection[%p] handle[%d] handleInput , errCode[%d]" , this ,getHandle() , errCode);
    if (errCode != 0)
    {
        return handleError(errCode);
    }
    qkrtl::Buffer buffer;
    while (reader_.read(buffer) == true)
    {
        handleRead(buffer);
        buffer.free();
    }
    return true;
}
bool Connection::startInput()
{
    return true;
}
bool Connection::handleRead(qkrtl::Buffer& buffer)
{
    return true;
}
bool Connection::write(qkrtl::Buffer& buffer)
{
    std::unique_lock<std::mutex> locker(guard_);
    if (outBuffer_.assigned() == true)
        return false;

    outBuffer_.refer(buffer);
    writer_.write(outBuffer_) ;
    return true;
}
bool Connection::handleWrited(qkrtl::Buffer& buffer)
{
    if (buffer.empty() == false)
        return false;
    buffer.free();
    return true;
}
bool Connection::handleOutput(int errCode)
{
    LOGDEBUG("Connection[%p] handle[%d] handleOutput , errCode[%d]", this, getHandle(), errCode);

    if (errCode != 0)
    {
        return handleError(errCode);
    }

    return true;
}
bool Connection::startOutput()
{
    writer_.ready();
    return true;
}
bool Connection::handleError(int errCode)
{
    LOGERR("Connection[%p] handle[%p] handle an errCode[%d]" , 
        this , getHandle() , errCode);
    connected_ = false;

    return true;
}
void Connection::notifyOpen(int errCode)
{
    //
}
void Connection::notifyClose(int errCode)
{
    //
}
void Connection::notifyRead(int errCode)
{
    reader_.handleCompleted(errCode);
}
void Connection::notifyWrite(int bytes, int errCode)
{
    {
        std::unique_lock<std::mutex> locker(guard_);
        outBuffer_.shrink(bytes);
        if (outBuffer_.empty() == false)
            return;
        handleWrited(outBuffer_);
        outBuffer_.clear();
    }

    writer_.handleCompleted(errCode);
}

ConnectionManager::ConnectionManager():finaled_(false)
{
    //
}
ConnectionManager::~ConnectionManager()
{
    final();
}
bool ConnectionManager::insert(Connection* conn)
{
    if (conn == NULL)
        return false;

    std::unique_lock<std::mutex> locker(guard_);
    return conns_.insert(conn).second;
}
bool ConnectionManager::remove(Connection* conn)
{
    if (conn == NULL)
        return false;

    std::unique_lock<std::mutex> locker(guard_);
    std::set<Connection*>::iterator iter = conns_.find(conn);
    if (iter != conns_.end())
        conns_.erase(iter);
    return true;
}
void ConnectionManager::clear()
{
    std::unique_lock<std::mutex> locker(guard_);

    for (std::set<Connection*>::iterator iter = conns_.begin();
        iter != conns_.end(); ++iter)
    {
        Connection* conn = (*iter);
        if (conn == NULL)
            continue;

        delete conn;
    }
    conns_.clear();
}
void ConnectionManager::final()
{
    {
        std::unique_lock<std::mutex> locker(guard_);
        if (finaled_ == true)
            return;
        finaled_ = true;
    }
    clear();
}



}
