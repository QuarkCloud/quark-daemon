;
#ifndef QKNET_WINDOWS_FILE_OBJECT_H
#define QKNET_WINDOWS_FILE_OBJECT_H 1

#include <time.h>
#include <string>
#include "qkrtl/FileSystem.h"
#include "qkrtl/Buffer.h"
#include "qkrtl/Stream.h"
#include "qknet/Compile.h"
#include "qknet/Address.h"
#include "FileEvent.h"

namespace qknet {
namespace windows {

class FileObject : public qkrtl::FileObject {
public:
    FileObject();
    virtual ~FileObject();
    //初始化socket
    virtual bool init();
    virtual void close();

    virtual bool start();
    virtual bool startRead();
    virtual bool startWrite();

    virtual void setSocket(const Socket& socket);

    inline const Socket& getSocket() const { return socket_; }
    inline Socket& getSocket() { return socket_; }

    virtual const OsHandle getOsHandle() const;

    inline const Address& localAddr() const { return localAddr_; }
    inline Address& localAddr() { return localAddr_; }
    inline const Address& remoteAddr() const { return remoteAddr_; }
    inline Address& remoteAddr() { return remoteAddr_; }
    inline time_t freeTime() const { return freeTime_; }
    inline int fileType() const { return fileType_; }

    static const int kTypeNone = 0;
    static const int kTypeAcceptor = 1;
    static const int kTypeConnection = 2;
    static const int kTypeConnector = 3;
    static const int kTypeUdpServer = 4;
    static const int kTypeUdpSession = 5;
    static const int kTypeUdpClient = 6;

    static const int kFstNet = 257;
protected:
    friend class FileSystem;
    Address localAddr_;
    Address remoteAddr_;
    time_t freeTime_;
    int fileType_;
    Socket socket_;
};

class AcceptObject : public FileObject {
public:
    AcceptObject();
    virtual ~AcceptObject();
    virtual bool init();
    bool listen(const std::string& host , uint16_t port);
    virtual void close();
    virtual bool startRead();

    std::string toUrl() const;

    virtual int read(qkrtl::Buffer& buffer);

    bool write(int handle);
    bool accept(int& handle);

    virtual void notifyRead(int errCode);

    static const int kMaxHandleSize = 64;
private:
    qkrtl::SpinLock guard_;
    int handles_[kMaxHandleSize];
    int head_;
    int tail_;
    TcpAcceptEvent acceptEvent_;
};

class ConnectionObject : public FileObject {
public:
    ConnectionObject();
    virtual ~ConnectionObject();
    virtual void close();
    virtual void setSocket(const Socket& socket);

    virtual bool startRead();
    virtual bool startWrite();

    virtual int read(qkrtl::Buffer& buffer);
    virtual int write(qkrtl::Buffer& buffer);

    virtual void notifyRead(int errCode);
protected:
    qkrtl::Stream inStream_;
    TcpReadEvent readEvent_;
    TcpWriteEvent writeEvent_;
};

/**
    ConnectorObject只比ConnectionObject多一个connect动作。
    只需要捕获这个动作就可以了，不需要重新创建Connection。
    这个非常重要，解决接口设计上的一个难点
*/
class ConnectorObject : public ConnectionObject {
public:
    ConnectorObject();
    virtual ~ConnectorObject();
    virtual bool init();
    virtual void close();
    virtual bool startRead();
    bool connect(const std::string& host, uint16_t port);

    virtual void notifyOpen(int errCode);
private:
    TcpConnectEvent connectEvent_;
    inline bool isConnector() const {return (fileType_ == kTypeConnector);}
};
 
class UdpSessionObject : public FileObject {
public:
    UdpSessionObject();
    virtual ~UdpSessionObject();
    virtual void close();

    virtual bool startWrite();
    virtual int write(qkrtl::Buffer& buffer);
protected:
    UdpWriteEvent writeEvent_;
};

/**
    2024-07-19
    没有实现自动创建UdpSession功能，留待后续完善
*/
class UdpServerObject : public UdpSessionObject {
public:
    UdpServerObject();
    virtual ~UdpServerObject();
    virtual bool init();
    //如果port=0，那么由系统自动分配
    bool listen(const std::string& host, uint16_t port = 0);
    virtual void close();
    virtual bool startRead();
    std::string toUrl() const;

    virtual int read(qkrtl::Buffer& buffer);
protected:
    UdpReadEvent readEvent_;
};

class UdpClientObject : public UdpSessionObject {
public:
    UdpClientObject();
    virtual ~UdpClientObject();
    virtual bool init();
    //如果port=0，那么由系统自动分配
    bool open(const std::string& host, uint16_t port = 0);
    virtual void close();
    virtual bool startRead();
    std::string toUrl() const;

    virtual int read(qkrtl::Buffer& buffer);
protected:
    UdpReadEvent readEvent_;
};

}
}
#endif /**QKNET_WINDOWS_FILE_OBJECT_H*/
