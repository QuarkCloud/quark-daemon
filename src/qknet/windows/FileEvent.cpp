

#include "qkrtl/Logger.h"
#include "qkrtl/DateTime.h"
#include "FileEvent.h"
#include "WinSocketInit.h"
namespace qknet {
namespace windows {

FileEvent::FileEvent(qkrtl::FileObject& fileObject):fileObject_(fileObject)
{
    //
}
FileEvent::~FileEvent()
{
    //
}
void FileEvent::setSocket(const Socket& socket)
{
    socket_ = socket;
}

FileIoEvent::FileIoEvent(qkrtl::FileObject& fileObject) 
    :FileEvent(fileObject) , wsabuf_{ 0 , NULL }
{
    //
}
FileIoEvent::~FileIoEvent()
{
    //
}
TcpReadEvent::TcpReadEvent(qkrtl::FileObject& fileObject)
    :FileIoEvent(fileObject) , recvBufferSize_(0)
{
    //
}
TcpReadEvent::~TcpReadEvent()
{
    //
}
int TcpReadEvent::avaibleSize() const
{
    std::unique_lock<std::mutex> locker(guard_);
    if (socket_.valid() == false || isClosed() == true || isClosing() == true)
        return -1;

    if (isDone() == false)
        return 0;

    return socket_.avaibleSize();
}
bool TcpReadEvent::read(qkrtl::Buffer& buffer)
{
    std::unique_lock<std::mutex> locker(guard_);
    if (socket_.valid() == false || isDone() == false)
        return false;

    buffer.swap(buffer_);
    if (buffer_.assigned() == true)
        buffer_.free();

    return true;
}
bool TcpReadEvent::startRead()
{
    std::unique_lock<std::mutex> locker(guard_);
    if (socket_.valid() == false || isClosed() == true || isClosing() == true)
        return false;

    if (isDoing() == true)
        return true;

    errCode_ = 0;
    bytes_ = 0;
    SOCKET socket = socket_.getSocket();
    if (recvBufferSize_ == 0)
    {
        recvBufferSize_ = socket_.recvBufsize();
        if (recvBufferSize_ == 0)
            return false;
    }
    if (buffer_.malloc(recvBufferSize_) == false)
        return false;

    DWORD bytes = 0, flags = 0;
    wsabuf_.buf = buffer_.tail();
    wsabuf_.len = buffer_.avaibleSize();

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
    setDoing();
    return true;
}
void TcpReadEvent::ioCompleted()
{
    int errCode = 0;
    bool isRead = false;

    if (processCompleted(errCode, isRead) == false)
        return;

    if (isRead == false)
    {
        fileObject_.notifyClose(errCode);
        return;
    }
    fileObject_.notifyRead(errCode);
    startRead();
}
bool TcpReadEvent::processCompleted(int& errCode, bool& isRead)
{
    errCode = 0;
    isRead = true;
    std::unique_lock<std::mutex> locker(guard_);
    if (socket_.valid() == true)
    {
        HANDLE handle = socket_.getOsHandle();
        OVERLAPPED* ovlp = (OVERLAPPED*)this;
        if (errCode_ == 0)
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
                    errCode_ = WSAEDISCON;
                }
            }
        }
        else
        {
            LOGERR("SOCKET[%p] ovlp[%p] 's reading is completed , status[%d]", handle, ovlp, errCode_);
        }
    }
    errCode = errCode_;
    if (isDoing() == true)
        setDone();
    else if (isClosing() == true)
    {
        isRead = false;
        setClosed();
    }

    return true;
}
void TcpReadEvent::close()
{
    std::unique_lock<std::mutex> locker(guard_);
    if (socket_.valid() == false)
    {
        if (isClosed() == false)
            setClosed();
        return;
    }
    if (isClosed() == true || isClosing() == true)
        return;

    HANDLE handle = socket_.getOsHandle();
    if (isDoing() == true)
    {
        OVERLAPPED* ovlp = (OVERLAPPED*)this;
        LOGCRIT("TcpReadEvent[%p] socket[%p] cancel reading event[%p]",
            this, handle, ovlp);
        ::CancelIoEx(handle, this);
        setClosing();
    }
    else
    {
        LOGCRIT("TcpSocketReadEvent[%p] socket[%p] had closed",
            this, handle);
        setClosed();
    }
}
TcpWriteEvent::TcpWriteEvent(qkrtl::FileObject& fileObject):FileIoEvent(fileObject)
{
    //
}
TcpWriteEvent::~TcpWriteEvent()
{
    //
}
bool TcpWriteEvent::write(qkrtl::Buffer& buffer)
{
    std::unique_lock<std::mutex> locker(guard_);
    if (socket_.valid() == false || isClosed() == true || isClosing() == true)
    {
        LOGERR("TcpWriteEvent[%p] socket's writer , socket is NULL", this);
        return false;
    }
    HANDLE handle = socket_.getOsHandle();
    if (isDoing() == true)
    {
        LOGERR("TcpWriteEvent[%p] SOCKET[%p] is writing , so return ", this, handle);
        return false;
    }
    if (buffer_.empty() == false)
    {
        LOGERR("TcpWriteEvent[%p] SOCKET[%p] still have data not writed , dataSize[%d] ",
            this, handle, buffer_.dataSize());
        return false;
    }

    buffer_.refer(buffer);
    return doWrite();
}
bool TcpWriteEvent::startWrite(bool allowEmpty)
{
    std::unique_lock<std::mutex> locker(guard_);
    return doWrite(allowEmpty);
}
bool TcpWriteEvent::doWrite(bool allowEmpty)
{
    if (socket_.valid() == false || isClosed() == true || isClosing() == true)
        return false;

    if (isDoing() == true)
        return true;

    errCode_ = 0;
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
            LOGERR("TcpWriteEvent[%p] SOCKET[%p] failed to send , buffer[%p:%d]  , errCode[%d]",
                this, getOsHandle(), wsabuf_.buf, (int)wsabuf_.len, errCode);
            return false;
        }
    }
    setDoing();
    return true;
}
void TcpWriteEvent::ioCompleted()
{
    std::unique_lock<std::mutex> locker(guard_);
    HANDLE handle = INVALID_HANDLE_VALUE;
    if (socket_.valid() == true)
        handle = socket_.getOsHandle();
    if (errCode_ == 0)
    {
        if (bytes_ > 0)
        {
            buffer_.shrink(bytes_);
        }
        LOGINFO("SOCKET[%p]'s writer completed [%d] bytes", handle, bytes_);
    }
    else
    {
        LOGERR("SOCKET[%p]'s writer failed , status[%d]", handle, errCode_);
    }

    int errCode = errCode_, bytes = bytes_;
    if (isDoing() == true)
    {
        setDone();
    }
    else if (isClosing() == true)
        setClosed();

    fileObject_.notifyWrite(bytes, errCode_);
    if (buffer_.empty() == false)
        doWrite();
}
void TcpWriteEvent::close()
{
    std::unique_lock<std::mutex> locker(guard_);
    if (socket_.valid() == false)
    {
        if (isClosed() == false)
            setClosed();
        return;
    }
    if (isClosed() == true || isClosing() == true)
        return;

    HANDLE handle = socket_.getOsHandle();
    OVERLAPPED* ovlp = (OVERLAPPED*)this;
    if (isDoing() == true)
    {
        LOGCRIT("TcpWriteEvent[%p] SOCKET[%p] cancel writing event[%p]",
            this, handle, ovlp);
        ::CancelIoEx(handle, this);
        setClosing();
    }
    else
    {
        LOGCRIT("TcpWriteEvent[%p] SOCKET[%p] event[%p] had closed", this, handle, ovlp);
        setClosed();
    }
}

TcpAcceptEvent::TcpAcceptEvent(qkrtl::FileObject& fileObject) 
    :FileEvent(fileObject) , addressBuffer_{ '\0' }
{
}
TcpAcceptEvent::~TcpAcceptEvent()
{
    //
}
bool TcpAcceptEvent::accept(Socket& connection)
{
    std::unique_lock<std::mutex> locker(guard_);
    if (isDone() == false)
    {
        LOGERR("TcpAcceptEvent[%p] listener[%p] is accepting , so return",
            this, socket_.getOsHandle());
        return false;
    }
    connection.swap(newSocket_);
    return (connection.valid());
}
bool TcpAcceptEvent::startAccept()
{
    std::unique_lock<std::mutex> locker(guard_);
    HANDLE handle = socket_.getOsHandle();
    LOGINFO("TcpAcceptEvent[%p] listener[%p] begin to start accept", this, handle);
    if (isDoing() == true)
    {
        LOGDEBUG("TcpAcceptEvent[%p] listener[%p] had started accept", this, handle);
        return true;
    }
    if (socket_.valid() == false || isClosed() == true || isClosing() == true)
        return false;

    errCode_ = 0;
    bytes_ = 0;

    if (newSocket_.init() == false)
        return false;

    DWORD bytes = 0;
    int64_t startTime = qkrtl::UTimeNow();
    BOOL result = AcceptEx(socket_, newSocket_, addressBuffer_, 0,
        kSockAddrMaxSize, kSockAddrMaxSize, &bytes, this);
    int64_t elapseTime = qkrtl::UTimeNow() - startTime;
    if (result == FALSE)
    {
        int errCode = ::WSAGetLastError();
        if (errCode != WSA_IO_PENDING)
        {
            newSocket_.final();
            LOGERR("TcpAcceptEvent[%p] listener[%p] failed to start accept , errCode[%d]",
                this, handle, errCode);
            return false;
        }
    }
    setDoing();
    LOGINFO("TcpAcceptEvent[%p] listener[%p] had started accept , elapse [%lld] usecs",
        this, handle, elapseTime);
    return true;
}
void TcpAcceptEvent::close()
{
    std::unique_lock<std::mutex> locker(guard_);
    if (isClosed() == true || isClosing() == true || socket_.valid() == false)
        return;

    HANDLE handle = socket_.getOsHandle();
    if (isDoing() == true)
    {
        OVERLAPPED* ovlp = (OVERLAPPED*)this;
        LOGCRIT("TcpAcceptEvent[%p] listner[%p] cancel accept event[%p]",
            this, handle, ovlp);
        ::CancelIoEx(handle, this);
        setClosing();
    }
    else
    {
        setClosed();
    }
}
void TcpAcceptEvent::ioCompleted()
{
    int errCode = 0;
    bool isRead = false;

    if (processCompleted(errCode, isRead) == false)
        return;

    if (isRead == false)
    {
        fileObject_.notifyClose(errCode);
        return;
    }
    fileObject_.notifyRead(errCode);
    startAccept();
}
bool TcpAcceptEvent::processCompleted(int& errCode, bool& isRead)
{
    errCode = 0;
    isRead = true;

    std::unique_lock<std::mutex> locker(guard_);
    HANDLE handle = socket_.getOsHandle();
    LOGDEBUG("TcpAcceptEvent[%p] listener[%p] completed , errCode[%d]", this, handle , errCode_);
    if (errCode_ != 0)
    {
        OVERLAPPED* ovlp = (OVERLAPPED*)this;
        LOGERR("TcpAcceptEvent[%p] listener[%p] ovlp[%p]'s  recv an errCode[%d]",
            this, handle, ovlp, errCode_);

        if (isDoing() == true)
        {
            setDone();
            LOGERR("TcpAcceptEvent[%p] listener[%p] completed accept , status[%d]",
                this, handle, errCode_);
        }
        else if (isClosing() == true)
        {
            LOGINFO("TcpAcceptEvent[%p] listener[%p] completed accept , status[%d]",
                this, handle, errCode_);
            setClosed();
        }
        else
        {
            LOGERR("TcpAcceptEvent[%p] listener[%p] completed accept , uncatch action[%s]",
                this, handle, statusName());
        }
        errCode = errCode_;
        return true;
    }

    if (isDoing() == true)
    {
        SOCKET listener = socket_.getSocket();
        int retval = ::setsockopt(newSocket_, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT,
            (char*)&listener, sizeof(listener));
        if (retval == SOCKET_ERROR)
        {
            errCode_ = ::WSAGetLastError();
            LOGERR("TcpAcceptEvent[%p] listener[%p] completed accept , update failed , status[%d]",
                this, handle, errCode_);
        }
        else
        {
            LOGINFO("TcpAcceptEvent[%p] listener[%p] completed accept a new socket[%p]",
                this, handle, newSocket_.getOsHandle());

        }
        setDone();
        LOGINFO("TcpAcceptEvent[%p] listener[%p] completed accept , status[%d]",
            this, handle, errCode_);

        errCode = errCode_;
        return true;
    }

    if (isClosing() == true)
    {
        LOGINFO("TcpAcceptEvent[%p] listener[%p] completed accept , status[%d]",
            this, handle, errCode_);
        setClosed();

        errCode = errCode_;
        isRead = false;
        return true;
    }
    else
    {
        LOGERR("TcpAcceptEvent[%p] listener[%p] completed accept , uncatch action[%s]",
            this, handle, statusName());
        return false;
    }
}

TcpConnectEvent::TcpConnectEvent(qkrtl::FileObject& fileObject)
    :FileEvent(fileObject) , connected_(false)
{
    LOGDEBUG("TcpConnectEvent[%p] created", this);
}
TcpConnectEvent::~TcpConnectEvent()
{
    LOGDEBUG("TcpConnectEvent[%p] will be freed", this);
}
bool TcpConnectEvent::connect(const std::string& host, uint16_t port)
{
    LOGDEBUG("TcpConnectEvent[%p] try to connect[%s:%hu]", this, host.c_str(), port);
    {
        std::unique_lock<std::mutex> locker(guard_);
        if (isClosed() == true || isClosing() == true)
        {
            LOGERR("TcpConnectEvent[%p] is closed", this);
            return false;
        }
        if (address_.assign(host, port) == false)
        {
            LOGERR("TcpConnectEvent[%p] failed to assign server address", this);
            return false;
        }
    }

    return true;
}
bool TcpConnectEvent::startConnect()
{
    std::unique_lock<std::mutex> locker(guard_);
    if (isDoing() == true || isClosed() == true || isClosing() == true)
    {
        LOGERR("TcpConnectEvent[%p] handle[%p] failed to start connect , action[%s]",
            this, getOsHandle(), statusName());
        return false;
    }
    if (socket_.valid() == false)
    {
        LOGERR("TcpConnectEvent[%p]'s handle is invalid", this);
        return false;
    }

    //默认情况下，connection没有执行bind
    Address sockAddr("", 0);
    if (socket_.bind(sockAddr) == false)
    {
        LOGERR("TcpConnectEvent[%p] handle[%p] failed to bind", this, getOsHandle());
        return false;
    }
    else
    {
        LOGDEBUG("TcpConnectEvent[%p] handle[%p] succedd to bind at[%s]",
            this, getOsHandle(), socket_.getFullAddress().c_str());
    }

    DWORD bytes = 0;
    BOOL retval = ::ConnectEx(socket_, address_.getSockAddr(), address_.getSocklen(),
        NULL, 0, &bytes, this);
    if (retval == FALSE)
    {
        int errCode = ::WSAGetLastError();
        if (errCode != WSA_IO_PENDING)
        {
            LOGERR("TcpConnectEvent[%p] handle[%p] failed to connect , errCode[%d]",
                this, getOsHandle(), errCode);
            return false;
        }
    }
    setDoing();
    return true;
}
void TcpConnectEvent::close()
{
    std::unique_lock<std::mutex> locker(guard_);
    if (socket_.valid() == false)
    {
        if (isClosed() == false)
            setClosed();
        return;
    }
    if (isClosed() == true || isClosing() == true)
        return;

    HANDLE handle = socket_.getOsHandle();
    if (isDoing() == true)
    {
        OVERLAPPED* ovlp = (OVERLAPPED*)this;
        LOGCRIT("TcpConnectEvent[%p] SOCKET[%p] cancel connect event[%p]", this, handle, ovlp);
        ::CancelIoEx(handle, this);
        setClosing();
    }
    else
    {
        setClosed();
    }
    //socket_.reset();
}
bool TcpConnectEvent::processCompleted(int& errCode)
{
    errCode = errCode_;

    LOGDEBUG("TcpConnectEvent[%p] socket[%p] completed", this, getOsHandle());
    std::unique_lock<std::mutex> locker(guard_);
    if (isDoing() == true)
        setDone();
    else if (isClosing() == true)
        setClosed();

    LOGDEBUG("TcpConnectEvent[%p] socket[%p] completed , status[%s] errCode[%d]",
        this, getOsHandle(), statusName(), errCode_);

    if (errCode_ != 0)
        return false;

    int retval = ::setsockopt(socket_, SOL_SOCKET, SO_UPDATE_CONNECT_CONTEXT, NULL, 0);
    if (retval == SOCKET_ERROR)
    {
        errCode_ = ::WSAGetLastError();
        LOGERR("TcpConnectEvent[%p] socket[%p] completed , update failed , errCode[%d]",
            this, getOsHandle(), errCode_);
        errCode = errCode_;
        return false;
    }
    connected_ = true;
    LOGINFO("TcpConnectEvent[%p] socket[%p] connected", this, getOsHandle());
    return true;
}
void TcpConnectEvent::ioCompleted()
{
    int errCode = 0;
    if (processCompleted(errCode) == false || errCode != 0)
    {
        fileObject_.notifyOpen(errCode);
    }
    else if(isDone() == true && connected() == true)
    {
        fileObject_.notifyOpen(0);
    }

    if (isClosed() == false)
        close();
}

UdpReadEvent::UdpReadEvent(qkrtl::FileObject& fileObject) 
    :FileIoEvent(fileObject) , addrLen_(0)
{
    //
}
UdpReadEvent::~UdpReadEvent()
{
    //
}
bool UdpReadEvent::read(Address& addr,  qkrtl::Buffer& buffer)
{
    std::unique_lock<std::mutex> locker(guard_);
    if (socket_.valid() == false || isDone() == false)
        return false;

    addr.assign(address_);
    buffer.swap(buffer_);
    if (buffer_.assigned() == true)
        buffer_.free();

    address_.clear();
    return true;
}
bool UdpReadEvent::startRead()
{
    std::unique_lock<std::mutex> locker(guard_);
    if (socket_.valid() == false || isClosed() == true || isClosing() == true)
        return false;

    if (isDoing() == true)
        return true;

    errCode_ = 0;
    bytes_ = 0;
    SOCKET socket = socket_.getSocket();
    if (buffer_.malloc(kUdpBufferSize) == false)
        return false;

    DWORD bytes = 0 , flags = 0;
    wsabuf_.buf = buffer_.tail();
    wsabuf_.len = buffer_.avaibleSize();

    addrLen_ = address_.getSocklen();
    int retval = ::WSARecvFrom(socket, &wsabuf_, 1, &bytes, &flags,
        address_.getSockAddr() , &addrLen_ , this, NULL);
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
    setDoing();
    return true;
}
void UdpReadEvent::ioCompleted()
{
    std::unique_lock<std::mutex> locker(guard_);
    if (socket_.valid() == true)
    {
        HANDLE handle = socket_.getOsHandle();
        OVERLAPPED* ovlp = (OVERLAPPED*)this;
        if (errCode_ == 0)
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
                    errCode_ = WSAEDISCON;
                }
            }
        }
        else
        {
            LOGERR("SOCKET[%p] ovlp[%p] 's reading is completed , status[%d]", handle, ovlp, errCode_);
        }
    }
    if (isDoing() == true)
        setDone();
    else if (isClosing() == true)
        setClosed();

    fileObject_.notifyRead(errCode_);

}
void UdpReadEvent::close()
{
    std::unique_lock<std::mutex> locker(guard_);
    if (socket_.valid() == false)
    {
        if (isClosed() == false)
            setClosed();
        return;
    }
    if (isClosed() == true || isClosing() == true)
        return;

    HANDLE handle = socket_.getOsHandle();
    if (isDoing() == true)
    {
        OVERLAPPED* ovlp = (OVERLAPPED*)this;
        LOGCRIT("TcpReadEvent[%p] socket[%p] cancel reading event[%p]",
            this, handle, ovlp);
        ::CancelIoEx(handle, this);
        setClosing();
    }
    else
    {
        LOGCRIT("TcpSocketReadEvent[%p] socket[%p] had closed",
            this, handle);
        setClosed();
    }
}
UdpWriteEvent::UdpWriteEvent(qkrtl::FileObject& fileObject):FileIoEvent(fileObject)
{
    //
}
UdpWriteEvent::~UdpWriteEvent()
{
    //
}
bool UdpWriteEvent::write(const Address& addr, qkrtl::Buffer& buffer)
{
    std::unique_lock<std::mutex> locker(guard_);
    if (socket_.valid() == false || isClosed() == true || isClosing() == true)
    {
        LOGERR("TcpWriteEvent[%p] socket's writer , socket is NULL", this);
        return false;
    }
    HANDLE handle = socket_.getOsHandle();
    if (isDoing() == true)
    {
        LOGERR("TcpWriteEvent[%p] SOCKET[%p] is writing , so return ", this, handle);
        return false;
    }
    if (buffer_.empty() == false)
    {
        LOGERR("TcpWriteEvent[%p] SOCKET[%p] still have data not writed , dataSize[%d] ",
            this, handle, buffer_.dataSize());
        return false;
    }
    address_.assign(addr);
    buffer_.refer(buffer);

    return doWrite();
}
bool UdpWriteEvent::startWrite(bool allowEmpty)
{
    std::unique_lock<std::mutex> locker(guard_);
    return doWrite(allowEmpty);
}
bool UdpWriteEvent::doWrite(bool allowEmpty)
{
    if (socket_.valid() == false || isClosed() == true || isClosing() == true)
        return false;

    if (isDoing() == true)
        return true;

    errCode_ = 0;
    bytes_ = 0;

    if (allowEmpty == false && buffer_.empty() == true)
        return true;

    SOCKET s = socket_.getSocket();
    DWORD bytes = 0;
    wsabuf_.buf = buffer_.head();
    wsabuf_.len = buffer_.dataSize();

    int addrLen = address_.getSocklen();
    int retval = ::WSASendTo(s, wsabuf(), 1, &bytes, 0, 
        address_.getSockAddr() , addrLen , this, NULL);
    if (retval == SOCKET_ERROR)
    {
        int errCode = ::WSAGetLastError();
        if (errCode != WSA_IO_PENDING)
        {
            LOGERR("TcpWriteEvent[%p] SOCKET[%p] failed to send , buffer[%p:%d]  , errCode[%d]",
                this, getOsHandle(), wsabuf_.buf, (int)wsabuf_.len, errCode);
            return false;
        }
    }
    setDoing();
    return true;
}
void UdpWriteEvent::ioCompleted()
{
    std::unique_lock<std::mutex> locker(guard_);
    HANDLE handle = INVALID_HANDLE_VALUE;
    if (socket_.valid() == true)
        handle = socket_.getOsHandle();
    if (errCode_ == 0)
    {
        if (bytes_ > 0)
        {
            buffer_.shrink(bytes_);
        }
        LOGINFO("SOCKET[%p]'s writer completed [%d] bytes", handle, bytes_);
    }
    else
    {
        LOGERR("SOCKET[%p]'s writer failed , status[%d]", handle, errCode_);
    }

    int errCode = errCode_, bytes = bytes_;
    if (isDoing() == true)
        setDone();
    else if (isClosing() == true)
        setClosed();

    fileObject_.notifyWrite(bytes , errCode);
}
void UdpWriteEvent::close()
{
    std::unique_lock<std::mutex> locker(guard_);
    if (socket_.valid() == false)
    {
        if (isClosed() == false)
            setClosed();
        return;
    }
    if (isClosed() == true || isClosing() == true)
        return;

    HANDLE handle = socket_.getOsHandle();
    OVERLAPPED* ovlp = (OVERLAPPED*)this;
    if (isDoing() == true)
    {
        LOGCRIT("TcpWriteEvent[%p] SOCKET[%p] cancel writing event[%p]",
            this, handle, ovlp);
        ::CancelIoEx(handle, this);
        setClosing();
    }
    else
    {
        LOGCRIT("TcpWriteEvent[%p] SOCKET[%p] event[%p] had closed", this, handle, ovlp);
        setClosed();
    }
}

}
}