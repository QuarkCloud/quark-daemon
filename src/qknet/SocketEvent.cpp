
#include "qknet/SocketEvent.h"
#include "WinSocketInit.h"
#include "qkrtl/Logger.h"
#include "qkrtl/DateTime.h"

namespace qknet{

SocketEvent::SocketEvent() :action_(kActionNone)
{
    nodeType_ = qkrtl::EventNode::kTypeSock;
}
SocketEvent::~SocketEvent()
{
    //
}
void SocketEvent::reset(const Socket& socket)
{
    socket_ = socket;
}

static const int kActionNameMaxSize = 5;
static const char* __ActionNames__[kActionNameMaxSize + 1] = {
    "None" , "Doing" , "Done" , "Closing" , "Closed" , "ErrorAction"
};
const char* SocketEvent::ActionToString(int action)
{
    if (action < 0 || action >= kActionNameMaxSize)
        return __ActionNames__[kActionNameMaxSize];
    else
        return __ActionNames__[action];
}
SocketIoEvent::SocketIoEvent():wsabuf_{ 0 , NULL }
{
    //
}
SocketIoEvent::~SocketIoEvent()
{
    //
}
TcpSocketReadEvent::TcpSocketReadEvent()
{
    ioType_ = IoEvent::kIoRead;
}
TcpSocketReadEvent::~TcpSocketReadEvent()
{
}
bool TcpSocketReadEvent::prepare(qkrtl::Buffer& buffer)
{
    std::unique_lock<std::mutex> locker(guard_);
    if (none() == false && done() == false)
        return false;

    buffer.swap(buffer_);
    return true;
}
int TcpSocketReadEvent::avaibleSize() const
{
    std::unique_lock<std::mutex> locker(guard_);
    if (socket_.valid() == false || closed() == true || closing() == true)
        return -1;

    if (done() == false)
        return 0;

    return socket_.avaibleSize();
}
bool TcpSocketReadEvent::read(qkrtl::Buffer& buffer)
{
    std::unique_lock<std::mutex> locker(guard_);
    if (socket_.valid() == false || done() == false)
        return false;

    buffer.swap(buffer_);
    return true;
}
bool TcpSocketReadEvent::startRead()
{
    std::unique_lock<std::mutex> locker(guard_);
    if (socket_.valid() == false || closed() == true || closing() == true)
        return false;

    if (doing() == true)
        return true;

    status_ = 0;
    bytes_ = 0;

    SOCKET socket = socket_.getSocket();
    DWORD bytes = 0, flags = 0;
    int avaibleSize = buffer_.avaibleSize();
    if (avaibleSize <= 0)
    {
        wsabuf_.buf = NULL;
        wsabuf_.len = 0;
    }
    else
    {
        wsabuf_.buf = buffer_.tail();
        wsabuf_.len = avaibleSize;
    }

    int retval = ::WSARecv(socket, &wsabuf_, 1, &bytes, &flags, this, NULL);
    if (retval == SOCKET_ERROR)
    {
        int errCode = ::WSAGetLastError();
        if (errCode != WSA_IO_PENDING)
        {
            LOGERR("failed to trigger SOCKET[%p]'s a read event , errCode[%d]",
                socket, errCode);
            return false;
        }
    }
    LOGINFO("SOCKET[%p] succeed to start a read event", socket);
    action_ = SocketEvent::kActionDoing;
    return true;
}
void TcpSocketReadEvent::ioCompleted()
{
    std::unique_lock<std::mutex> locker(guard_);
    if (socket_.valid() == true)
    {
        HANDLE handle = socket_.getHandle();
        OVERLAPPED* ovlp = (OVERLAPPED*)this;
        if (status_ == 0)
        {
            LOGINFO("SOCKET[%p] ovlp[%p] 's reading is completed , bytes[%d]", handle, ovlp, bytes_);
            if (bytes_ > 0)
            {
                buffer_.extend(bytes_);
            }
            else
            {
                //bytes_<=0 ，对端已经关闭
                if (wsabuf_.len > 0 && wsabuf_.buf != NULL)
                {
                    status_ = WSAEDISCON;
                }
            }
        }
        else
        {
            LOGERR("SOCKET[%p] ovlp[%p] 's reading is completed , status[%d]", handle, ovlp, status_);
        }
    }
    if (doing() == true)
        action_ = kActionDone;
    else if (closing() == true)
        action_ = kActionClosed;
}
void TcpSocketReadEvent::close()
{
    std::unique_lock<std::mutex> locker(guard_);
    if (socket_.valid() == false)
    {
        if (closed() == false)
            action_ = kActionClosed;
        return;
    }
    if (closed() == true || closing() == true)
        return;

    HANDLE handle = socket_.getHandle();
    if (doing() == true)
    {
        OVERLAPPED* ovlp = (OVERLAPPED*)this;
        LOGCRIT("TcpSocketReadEvent[%p] socket[%p] cancel reading event[%p]" , 
            this , handle , ovlp);
        ::CancelIoEx(handle, this);
        action_ = kActionClosing;
    }
    else
    {
        LOGCRIT("TcpSocketReadEvent[%p] socket[%p] had closed",
            this, handle);
        action_ = kActionClosed;
    }
    //socket_.reset();
}
bool TcpSocketReadEvent::start()
{
    return startRead();
}
void TcpSocketReadEvent::stop()
{
    close();
}
void TcpSocketReadEvent::execute()
{
    completed();
    handle(status_);
}

TcpSocketWriteEvent::TcpSocketWriteEvent()
{
    ioType_ = IoEvent::kIoWrite;
}
TcpSocketWriteEvent::~TcpSocketWriteEvent()
{
    //
}
bool TcpSocketWriteEvent::write(qkrtl::Buffer& buffer)
{
    std::unique_lock<std::mutex> locker(guard_);
    if (socket_.valid() == false || closed() == true || closing() == true)
    {
        LOGERR("TcpSocketWriteEvent[%p] socket's writer , socket is NULL" , this);
        return false;
    }
    HANDLE handle = socket_.getHandle();
    if (doing() == true)
    {
        LOGERR("TcpSocketWriteEvent[%p] SOCKET[%p] is writing , so return ", this , handle);
        return false;
    }
    if (buffer_.empty() == false)
    {
        LOGERR("TcpSocketWriteEvent[%p] SOCKET[%p] still have data not writed , dataSize[%d] ", 
            this , handle , buffer_.dataSize());
        return false;
    }

    buffer.swap(buffer_);
    return (buffer_.empty() == false);
}
bool TcpSocketWriteEvent::startWrite(bool allowEmpty)
{
    std::unique_lock<std::mutex> locker(guard_);
    return doWrite(allowEmpty); 
}
bool TcpSocketWriteEvent::doWrite(bool allowEmpty)
{
    if (socket_.valid() == false || closed() == true || closing() == true)
        return false;

    if (doing() == true)
        return true;

    status_ = 0;
    bytes_ = 0;

    if (allowEmpty == false && buffer_.empty() == true)
        return true;

    SOCKET s = socket_.getSocket();
    DWORD bytes = 0;
    wsabuf_.buf = buffer_.head();
    wsabuf_.len = buffer_.dataSize();

    int retval = ::WSASend(s, wsabuf(), 1, &bytes, 0, this, NULL);
    if (retval == SOCKET_ERROR)
    {
        int errCode = ::WSAGetLastError();
        if (errCode != WSA_IO_PENDING)
        {
            LOGERR("TcpSocketWriteEvent[%p] SOCKET[%p] failed to send , buffer[%p:%d]  , errCode[%d]",
                this, getHandle(), wsabuf_.buf, (int)wsabuf_.len, errCode);
            return false;
        }
    }

    action_ = kActionDoing;
    return true;
}
void TcpSocketWriteEvent::ioCompleted()
{
    std::unique_lock<std::mutex> locker(guard_);
    HANDLE handle = INVALID_HANDLE_VALUE;
    if (socket_.valid() == true)
        handle = socket_.getHandle();
    if (status_ == 0)
    {
        if (bytes_ > 0)
        {
            buffer_.shrink(bytes_);
        }
        LOGINFO("SOCKET[%p]'s writer completed [%d] bytes" , handle, bytes_);
    }
    else
    {
        LOGERR("SOCKET[%p]'s writer failed , status[%d]" , handle, status_);
    }

    if (doing() == true)
        action_ = kActionDone;
    else if (closing() == true)
        action_ = kActionClosed;
}
void TcpSocketWriteEvent::close()
{
    std::unique_lock<std::mutex> locker(guard_);
    if (socket_.valid() == false)
    {
        if(closed() == false)
            action_ = kActionClosed;
        return;
    }
    if (closed() == true || closing() == true)
        return;

    HANDLE handle = socket_.getHandle();
    OVERLAPPED* ovlp = (OVERLAPPED*)this;
    if (action_ == kActionDoing)
    {
        LOGCRIT("TcpSocketWriteEvent[%p] SOCKET[%p] cancel writing event[%p]", 
            this , handle , ovlp);
        ::CancelIoEx(handle, this);
        action_ = kActionClosing;
    }
    else
    {
        LOGCRIT("TcpSocketWriteEvent[%p] SOCKET[%p] event[%p] had closed",  this , handle , ovlp);
        action_ = kActionClosed;
    }
    //socket_.reset();
}
bool TcpSocketWriteEvent::start()
{
    return startWrite(true);
}
void TcpSocketWriteEvent::stop()
{
    close();
}
void TcpSocketWriteEvent::execute()
{
    OVERLAPPED* ovlp = (OVERLAPPED*)this;
    qkrtl::EventObject* event = (qkrtl::EventObject*)this;
    LOGCRIT("TcpSocketWriteEvent[%p] SOCKET[%p] ovlp[%p] event[%p] begin to execute", 
        this, getHandle(), ovlp, event);
    completed();
    {
        std::unique_lock<std::mutex> locker(guard_);
        if (status_ == 0 && buffer_.empty() == false)
        {
            doWrite();
            return;
        }
    }

    handle(status_);
    LOGCRIT("TcpSocketWriteEvent[%p] SOCKET[%p] ovlp[%p] event[%p] completed execute", 
        this, getHandle(), ovlp , event);
}

SocketAcceptEvent::SocketAcceptEvent(): addressBuffer_{ '\0' }
{
    nodeType_ = qkrtl::EventNode::kTypeSock;
    ioType_ = IoEvent::kIoAccept;
}
SocketAcceptEvent::~SocketAcceptEvent()
{
    //
}
bool SocketAcceptEvent::accept(Socket& connection)
{
    std::unique_lock<std::mutex> locker(guard_);
    if (doing() == true)
    {
        LOGERR("SocketAcceptEvent[%p] listener[%p] is accepting , so return" , 
            this , socket_.getHandle());
        return false;
    } 
    connection.swap(newSocket_);
    return (connection.valid());
}
bool SocketAcceptEvent::startAccept()
{
    std::unique_lock<std::mutex> locker(guard_);
    HANDLE handle = socket_.getHandle();
    LOGINFO("SocketAcceptEvent[%p] listener[%p] begin to start accept", this , handle);
    if (doing() == true)
    {
        LOGDEBUG("SocketAcceptEvent[%p] listener[%p] had started accept", this , handle);
        return true;
    }
    if (socket_.valid() == false || closed() == true || closing() == true)
        return false;

    status_ = 0;
    bytes_ = 0;

    if (newSocket_.init() == false)
        return false;
    
    DWORD bytes = 0;
    int64_t startTime = qkrtl::UTimeNow();
    BOOL result = AcceptEx(socket_, newSocket_, addressBuffer_, 0,
        kSockAddrMaxSize, kSockAddrMaxSize,&bytes, this);
    int64_t elapseTime = qkrtl::UTimeNow() - startTime;
    if (result == FALSE)
    {
        int errCode = ::WSAGetLastError();
        if (errCode != WSA_IO_PENDING)
        {
            newSocket_.final();
            LOGERR("SocketAcceptEvent[%p] listener[%p] failed to start accept , errCode[%d]",
                this, handle, errCode);
            return false;
        }
    }
    action_ = kActionDoing;
    LOGINFO("SocketAcceptEvent[%p] listener[%p] had started accept , elapse [%lld] usecs" , 
        this , handle , elapseTime);
    return true;
}
void SocketAcceptEvent::close()
{
    std::unique_lock<std::mutex> locker(guard_);
    if (closed() == true || closing() == true || socket_.valid() == false)
        return;

    HANDLE handle = socket_.getHandle();
    if (doing() == true)
    {
        OVERLAPPED* ovlp = (OVERLAPPED*)this;
        LOGCRIT("SocketAcceptEvent[%p] listner[%p] cancel accept event[%p]", 
            this , handle, ovlp);
        ::CancelIoEx(handle, this);
        action_ = kActionClosing;
    }
    else
    {
        action_ = kActionClosed;
    }
    //socket_.reset();
}
void SocketAcceptEvent::ioCompleted()
{
    std::unique_lock<std::mutex> locker(guard_);
    HANDLE handle = socket_.getHandle();
    LOGDEBUG("SocketAcceptEvent[%p] listener[%p] completed", this, handle);
    if (status_ != 0)
    {
        OVERLAPPED* ovlp = (OVERLAPPED*)this;
        LOGERR("SocketAcceptEvent[%p] listener[%p] ovlp[%p]'s  recv an errCode[%d]" , 
            this , handle ,ovlp , status_);

        if (doing() == true)
        {
            action_ = kActionDone;
            LOGERR("SocketAcceptEvent[%p] listener[%p] completed accept , status[%d]",
                this, handle, status_);
        }
        else if (closing() == true)
        {
            LOGINFO("SocketAcceptEvent[%p] listener[%p] completed accept , status[%d]",
                this, handle, status_);
            action_ = kActionClosed;
        }
        else
        {
            LOGERR("SocketAcceptEvent[%p] listener[%p] completed accept , uncatch action[%s]",
                this, handle, actionName());
        }

        return;
    }

    if(doing() == true)
    {
        SOCKET listener = socket_.getSocket();
        int retval = ::setsockopt(newSocket_, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT,
            (char*)&listener, sizeof(listener));
        if (retval == SOCKET_ERROR)
        {
            status_ = ::WSAGetLastError();
            LOGERR("SocketAcceptEvent[%p] listener[%p] completed accept , update failed , status[%d]", 
                this , handle, status_);
        }
        else
        {
            LOGINFO("SocketAcceptEvent[%p] listener[%p] completed accept a new socket[%p]",
                this, handle , newSocket_.getHandle());

        }

        action_ = kActionDone;
        LOGINFO("SocketAcceptEvent[%p] listener[%p] completed accept , status[%d]", 
            this , handle, status_);

        return;
    }

    if (closing() == true)
    {
        LOGINFO("SocketAcceptEvent[%p] listener[%p] completed accept , status[%d]", 
            this , handle, status_);
        action_ = kActionClosed;
    }
    else
    {
        LOGERR("SocketAcceptEvent[%p] listener[%p] completed accept , uncatch action[%s]", 
            this , handle, actionName());
    }
}
bool SocketAcceptEvent::start()
{
    return startAccept();
}
void SocketAcceptEvent::stop()
{
    close();
}
void SocketAcceptEvent::execute()
{
    handle(status_);
    completed();
}

SocketConnectEvent::SocketConnectEvent(): connected_(false) 
{
    nodeType_ = qkrtl::EventNode::kTypeSock;
    ioType_ = IoEvent::kIoConnect;
    LOGDEBUG("SocketConnectEvent[%p] created" , this);
}
SocketConnectEvent::~SocketConnectEvent()
{
    LOGDEBUG("SocketConnectEvent[%p] will be freed", this);
}
bool SocketConnectEvent::connect(const std::string& host, uint16_t port)
{
    LOGDEBUG("SocketConnectEvent[%p] try to connect[%s:%hu]", this , host.c_str() , port);
    {
        std::unique_lock<std::mutex> locker(guard_);
        if (closed() == true || closing() == true)
        {
            LOGERR("SocketConnectEvent[%p] is closed", this);
            return false;
        }
        if (address_.assign(host, port) == false)
        {
            LOGERR("SocketConnectEvent[%p] failed to assign server address", this);
            return false;
        }
    }

    return startConnect();
}
bool SocketConnectEvent::startConnect()
{
    std::unique_lock<std::mutex> locker(guard_);
    if (doing() == true || closed() == true || closing() == true)
    {
        LOGERR("SocketConnectEvent[%p] handle[%p] failed to start connect , action[%s]", 
            this , getHandle() , actionName());
        return false;
    }
    if (socket_.valid() == false)
    {
        LOGERR("SocketConnectEvent[%p]'s handle is invalid",this);
        return false;
    }

    //默认情况下，connection没有执行bind
    SocketAddr sockAddr("" , 0);
    if (socket_.bind(sockAddr) == false)
    {
        LOGERR("SocketConnectEvent[%p] handle[%p] failed to bind",this, getHandle());
        return false;
    }
    else
    {
        LOGDEBUG("SocketConnectEvent[%p] handle[%p] succedd to bind at[%s]", 
            this, getHandle() , socket_.getFullAddress().c_str());
    }

    DWORD bytes = 0;
    BOOL retval = ::ConnectEx(socket_, address_.getSockAddr(), address_.getSocklen(),
        NULL, 0, &bytes, this);
    if (retval == FALSE)
    {
        int errCode = ::WSAGetLastError();
        if (errCode != WSA_IO_PENDING)
        {
            LOGERR("SocketConnectEvent[%p] handle[%p] failed to connect , errCode[%d]", 
                this, getHandle() , errCode);
            return false;
        }
    }
    action_ = kActionDoing;
    return true;
}
void SocketConnectEvent::close()
{
    std::unique_lock<std::mutex> locker(guard_);
    if (socket_.valid() == false)
    {
        if(closed() == false)
            action_ = kActionClosed;
        return ;
    }
    if (closed() == true || closing() == true)
        return;

    HANDLE handle = socket_.getHandle();
    if (doing() == true)
    {
        OVERLAPPED* ovlp = (OVERLAPPED*)this;
        LOGCRIT("SocketConnectEvent[%p] SOCKET[%p] cancel connect event[%p]", this , handle, ovlp);
        ::CancelIoEx(handle, this);
        action_ = kActionClosing;
    }
    else
    {
        action_ = kActionClosed;
    }
    //socket_.reset();
}
void SocketConnectEvent::ioCompleted()
{
    LOGDEBUG("SocketConnectEvent[%p] socket[%p] completed" , this , getHandle());

    std::unique_lock<std::mutex> locker(guard_);
    if (doing() == true)
        action_ = kActionDone;
    else if (closing() == true)
        action_ = kActionClosed;

    LOGDEBUG("SocketConnectEvent[%p] socket[%p] completed , action[%s] status[%d]", 
        this, getHandle() , actionName() , status_);

    if (status_ != 0)
        return;

    int retval = ::setsockopt(socket_, SOL_SOCKET, SO_UPDATE_CONNECT_CONTEXT, NULL, 0);
    if (retval == SOCKET_ERROR)
    {
        status_ = ::WSAGetLastError();
        LOGERR("SocketConnectEvent[%p] socket[%p] completed , update failed , status[%d]", 
            this, getHandle() , status_);
        return;
    }
    connected_ = true;
    LOGINFO("SocketConnectEvent[%p] socket[%p] connected",this, getHandle());
}
bool SocketConnectEvent::start()
{
    return true;
}
void SocketConnectEvent::stop()
{
    close();
}
void SocketConnectEvent::execute()
{
    LOGINFO("SocketConnectEvent[%p] socket[%p] execute , status[%d]", this, getHandle() , status_);
    handle(status_);
    completed();
}

}
