
#include "FileObject.h"
#include "FileSystem.h"

namespace qknet {
namespace windows {

FileObject::FileObject():freeTime_(0) , fileType_(kTypeNone)
{
    fsType_ = kFstNet;
}
FileObject::~FileObject()
{
    close();
}
bool FileObject::init()
{
    return false;
}
void FileObject::close()
{
    if (socket_.valid() == true)
        socket_.close();
}
bool FileObject::start()
{
    return (startRead() && startWrite());
}
bool FileObject::startRead()
{
    return true;
}
bool FileObject::startWrite()
{
    return true;
}
void FileObject::setSocket(const Socket& socket)
{
    socket_ = socket;
}
const OsHandle FileObject::getOsHandle() const
{
    return socket_.getOsHandle();
}
AcceptObject::AcceptObject() :handles_{ 0 }, head_(0), tail_(0) , acceptEvent_(*this)
{
    fileType_ = FileObject::kTypeAcceptor;
    ::memset(handles_, 0, sizeof(handles_));
}
AcceptObject::~AcceptObject()
{
    //
}
bool AcceptObject::init()
{
    if (socket_.valid() == true)
        return true;

    return socket_.init(Socket::kTypeTcp);
}
bool AcceptObject::listen(const std::string& host, uint16_t port)
{
    Address addr;
    if (addr.assign(host, port) == false)
        return false;

    if (socket_.bind(addr) == false || socket_.listen(SOMAXCONN) == false)
    {
        socket_.final();
        return false;
    }
    //因为port可能为0，只有在绑定后，才知道被系统绑定的端口号
    socket_.localAddr(localAddr_);
    acceptEvent_.setSocket(socket_);
    return true;
}
void AcceptObject::close()
{
    //
}
bool AcceptObject::startRead()
{
    return acceptEvent_.startAccept();
}
std::string AcceptObject::toUrl() const
{
    return NetFileSystem::kTcpProtocol + "://" + localAddr_.toString();
}
int AcceptObject::read(qkrtl::Buffer& buffer)
{
    int* handles = (int*)buffer.tail();
    int handleCount = buffer.avaibleSize() / sizeof(int);
    if (handles == NULL || handleCount <= 0)
        return 0;

    qkrtl::Locker locker(guard_);
    if (head_ == tail_)
        return 0;

    int counter = 0;
    for (int idx = 0; idx < handleCount; ++idx)
    {
        if (head_ == tail_)
            break;

        handles[idx] = handles_[head_];
        handles_[head_] = 0;
        ++head_;
        if (head_ >= kMaxHandleSize)
            head_ = 0;

        ++counter;
    }

    return counter * sizeof(int);
}
bool AcceptObject::write(int handle)
{
    {
        qkrtl::Locker locker(guard_);

        int index = tail_ + 1;
        if (index >= kMaxHandleSize)
            index = 0;
        if (index == head_)
            return false;

        handles_[tail_] = handle;
        tail_ = index;
    }
    qkrtl::FileObject::notifyRead(0);
    return true;
}
bool AcceptObject::accept(int& handle)
{
    qkrtl::Locker locker(guard_);
    if (head_ == tail_)
        return false;

    handle = handles_[head_];
    handles_[head_] = 0;
    ++head_;
    if (head_ >= kMaxHandleSize)
        head_ = 0;
    return true;
}
void AcceptObject::notifyRead(int errCode)
{
    if (errCode != 0)
    {
        qkrtl::FileObject::notifyRead(errCode);
        return;
    }

    Socket sess;
    if (acceptEvent_.accept(sess) == false || sess.valid() == false)
    {
        qkrtl::FileObject::notifyRead(-1);
        return;
    }

    ConnectionObject* conn = new ConnectionObject();
    conn->setSocket(sess);

    int sessIndex = FsInsert(conn);
    if (sessIndex == kFsInvalidIndex)
    {
        conn->close();
        delete  conn;
        return;
    }

    write(sessIndex);
}
ConnectionObject::ConnectionObject():readEvent_(*this) , writeEvent_(*this)
{
    fileType_ = kTypeConnection;
}
ConnectionObject::~ConnectionObject()
{
    //
}
void ConnectionObject::close()
{
    readEvent_.close();
    writeEvent_.close();
    FileObject::close();
}
void ConnectionObject::setSocket(const Socket& socket)
{
    FileObject::setSocket(socket);
    readEvent_.setSocket(socket_);
    writeEvent_.setSocket(socket_);
}
bool ConnectionObject::startRead()
{
    return readEvent_.startRead();
}
bool ConnectionObject::startWrite()
{
    return writeEvent_.startWrite();
}
int ConnectionObject::read(qkrtl::Buffer& buffer)
{
    if (inStream_.pop(buffer) == false)
        return -1;
    return buffer.dataSize();
}
int ConnectionObject::write(qkrtl::Buffer& buffer)
{
    if (writeEvent_.write(buffer) == false)
        return -1;
    return 0;
}
void ConnectionObject::notifyRead(int errCode)
{
    if (errCode == 0)
    {
        qkrtl::Buffer buffer;
        if (readEvent_.read(buffer) == true)
            inStream_.push(buffer);
    }
    qkrtl::FileObject::notifyRead(errCode);
}
ConnectorObject::ConnectorObject() :connectEvent_(*this)
{
    fileType_ = kTypeConnector;
}
ConnectorObject::~ConnectorObject()
{
    //
}
bool ConnectorObject::init()
{
    if (socket_.valid() == true)
        return true;
    if (socket_.init(Socket::kTypeTcp) == false)
        return false;

    connectEvent_.setSocket(socket_);
    readEvent_.setSocket(socket_);
    writeEvent_.setSocket(socket_);
    return true;
}
void ConnectorObject::close()
{
    if (isConnector() == false)
    {
        ConnectionObject::close();
    }
    else
    {
        connectEvent_.close();
    }
}
bool ConnectorObject::startRead()
{
    if (isConnector() == false)
        return ConnectionObject::startRead();
    return connectEvent_.startConnect();        
}
bool ConnectorObject::connect(const std::string& host, uint16_t port)
{
    return connectEvent_.connect(host, port);
}
void ConnectorObject::notifyOpen(int errCode)
{
    fileType_ = kTypeConnection;
    FileObject::notifyOpen(errCode);
}

UdpSessionObject::UdpSessionObject() :writeEvent_(*this)
{
    fileType_ = kTypeUdpSession;
}
UdpSessionObject::~UdpSessionObject()
{
    //
}
void UdpSessionObject::close()
{

}
bool UdpSessionObject::startWrite()
{
    return writeEvent_.startWrite();
}
int UdpSessionObject::write(qkrtl::Buffer& buffer)
{
    if (writeEvent_.write(remoteAddr(), buffer) == false)
        return -1;
    return 0;
}

UdpServerObject::UdpServerObject():readEvent_(*this)
{
    fileType_ = kTypeUdpServer;
}
UdpServerObject::~UdpServerObject()
{

}
bool UdpServerObject::init()
{
    if (socket_.valid() == true)
        return true;
    return socket_.init(Socket::kTypeUdp);
}
bool UdpServerObject::listen(const std::string& host, uint16_t port)
{
    Address addr;
    if (addr.assign(host, port) == false)
        return false;

    if (socket_.bind(addr) == false)
    {
        socket_.final();
        return false;
    }

    socket_.localAddr(localAddr_);
    readEvent_.setSocket(socket_);
    return true;
}
void UdpServerObject::close()
{
    //
}
bool UdpServerObject::startRead()
{
    return readEvent_.startRead();
}
std::string UdpServerObject::toUrl() const
{
    return NetFileSystem::kTcpProtocol + "://" + localAddr_.toString();
}
int UdpServerObject::read(qkrtl::Buffer& buffer)
{
    if (readEvent_.read(remoteAddr_, buffer) == false)
        return -1;
    return 0;
}

UdpClientObject::UdpClientObject() :readEvent_(*this)
{
    fileType_ = kTypeUdpClient;
}
UdpClientObject::~UdpClientObject()
{
    //
}
bool UdpClientObject::init()
{
    if (socket_.valid() == true)
        return true;
    return socket_.init(Socket::kTypeUdp);
}
bool UdpClientObject::open(const std::string& host, uint16_t port)
{
    if (remoteAddr_.assign(host, port) == false)
        return false;

    if (socket_.bind(Address()) == false)
    {
        socket_.final();
        return false;
    }
    socket_.localAddr(localAddr_);
    readEvent_.setSocket(socket_);
    writeEvent_.setSocket(socket_);
    return true;
}
void UdpClientObject::close()
{
    //
}
bool UdpClientObject::startRead()
{
    return readEvent_.startRead();
}
std::string UdpClientObject::toUrl() const
{
    return "udp://" + localAddr_.toString();
}
int UdpClientObject::read(qkrtl::Buffer& buffer)
{
    if (readEvent_.read(remoteAddr_, buffer) == false)
        return -1;
    return 0;
}

}
}
