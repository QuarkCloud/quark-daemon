
#ifndef QKNET_WINDOWS_FILE_EVENT_H
#define QKNET_WINDOWS_FILE_EVENT_H 1

#include <mutex>
#include "qknet/Compile.h"
#include "qkrtl/IoEvent.h"
#include "qkrtl/FileSystem.h"
#include "qkrtl/Buffer.h"
#include "qkrtl/IoHandle.h"
#include "qknet/Address.h"
#include "Socket.h"

namespace qknet {
namespace windows {

class FileEvent : public qkrtl::windows::FileEvent , public qkrtl::IoStatus{
public:
    FileEvent(qkrtl::FileObject& fileObject);
    virtual ~FileEvent();
    void setSocket(const Socket& socket);

    inline virtual const HANDLE getOsHandle() const { return socket_.getOsHandle(); }
    inline virtual bool valid() const { return socket_.valid(); }
    inline Socket& socket() { return socket_; }
protected:
    Socket socket_;
    qkrtl::FileObject& fileObject_;
};

class FileIoEvent : public FileEvent {
public:
    FileIoEvent(qkrtl::FileObject& fileObject);
    virtual ~FileIoEvent();

    inline WSABUF* wsabuf() { return &wsabuf_; }
    inline const WSABUF* wsabuf() const { return &wsabuf_; }

protected:
    WSABUF wsabuf_;
};

/**
    read �¼���Ҫ����ϵͳ�ײ������ģ��ǵ����¼���
    ��û�����ݿ��Ա���ȡʱ����һ�������ڵײ㡣
*/
class TcpReadEvent : public FileIoEvent {
public:
    TcpReadEvent(qkrtl::FileObject& fileObject);
    virtual ~TcpReadEvent();

    int avaibleSize() const;
    bool read(qkrtl::Buffer& buffer);
    virtual bool startRead();
    virtual void ioCompleted();
    virtual void close();
private:
    mutable std::mutex guard_;
    int recvBufferSize_;
    qkrtl::Buffer buffer_;
    bool processCompleted(int& errCode, bool& isRead);
};
class TcpWriteEvent : public FileIoEvent {
public:
    TcpWriteEvent(qkrtl::FileObject& fileObject);
    virtual ~TcpWriteEvent();

    /**
        ���˿���д��ʱ���ڴ�ҳ�ᱻϵͳ���������Ա��뼰ʱ�˳���
        ���������0ʱ����������д������û���㹻�ռ�
    */
    bool write(qkrtl::Buffer& buffer);
    bool startWrite(bool allowEmpty = false);

    virtual void ioCompleted();
    virtual void close();
private:
    mutable std::mutex guard_;
    qkrtl::Buffer buffer_;     //��ʼΪ��
    bool doWrite(bool allowEmpty = false);
};

class TcpAcceptEvent : public FileEvent {
public:
    TcpAcceptEvent(qkrtl::FileObject& fileObject);
    virtual ~TcpAcceptEvent();
    bool accept(Socket& connection);
    bool startAccept();
    virtual void close();
    virtual void ioCompleted();

    static const int kSockAddrMaxSize = 64;
    static const int kSockAddrBufferSize = kSockAddrMaxSize << 1;
private:
    std::mutex guard_;
    Socket newSocket_;
    char addressBuffer_[kSockAddrBufferSize];
    bool processCompleted(int& errCode , bool& isRead);
};

class TcpConnectEvent : public FileEvent {
public:
    TcpConnectEvent(qkrtl::FileObject& fileObject);
    virtual ~TcpConnectEvent();
    bool connect(const std::string& host, uint16_t port);
    bool startConnect();
    virtual void close();
    virtual void ioCompleted();

    inline bool connected() const { return connected_; }
    inline bool connecting() const { return isDoing(); }
private:
    std::mutex guard_;
    bool connected_;
    Address address_;

    bool processCompleted(int& errCode);
};

/**
    read �¼���Ҫ����ϵͳ�ײ������ģ��ǵ����¼���
    ��û�����ݿ��Ա���ȡʱ����һ�������ڵײ㡣
*/
class UdpReadEvent : public FileIoEvent {
public:
    UdpReadEvent(qkrtl::FileObject& fileObject);
    virtual ~UdpReadEvent();
    bool read(Address& addr , qkrtl::Buffer& buffer);
    virtual bool startRead();
    virtual void ioCompleted();
    virtual void close();

    static const int kUdpBufferSize = 1 << 13;
private:
    mutable std::mutex guard_;
    Address address_;
    int addrLen_;
    qkrtl::Buffer buffer_;
};
class UdpWriteEvent : public FileIoEvent {
public:
    QKNETAPI UdpWriteEvent(qkrtl::FileObject& fileObject);
    QKNETAPI virtual ~UdpWriteEvent();

    /**
        ���˿���д��ʱ���ڴ�ҳ�ᱻϵͳ���������Ա��뼰ʱ�˳���
        ���������0ʱ����������д������û���㹻�ռ�
    */
    bool write(const Address& addr, qkrtl::Buffer& buffer);
    bool startWrite(bool allowEmpty = false);

    virtual void ioCompleted();
    virtual void close();
private:
    mutable std::mutex guard_;
    Address address_;
    qkrtl::Buffer buffer_;     //��ʼΪ��
    bool doWrite(bool allowEmpty = false);
};

}
}
#endif /**QKNET_WINDOWS_FILE_EVENT_H*/
