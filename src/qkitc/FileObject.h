
#ifndef QKITC_FILE_OBJECT_H
#define QKITC_FILE_OBJECT_H 1

#include <time.h>
#include <string>
#include "qkrtl/SpinLock.h"
#include "qkrtl/Buffer.h"
#include "qkrtl/Stream.h"
#include "qkrtl/FileSystem.h"
#include "qkitc/Compile.h"

namespace qkitc {

/**
    FileObject����FileSystem������ڲ����󣬲������ⲿ���ʣ����Ҫ�ر�ע�⡣
*/
class FileObject : public qkrtl::FileObject{
public:
    FileObject();
    virtual ~FileObject();

    inline const std::string& localAddr() const { return localAddr_; }
    inline const std::string& remoteAddr() const { return remoteAddr_; }
    inline int fileType() const { return fileType_; }

    static const int kTypeNone = 0;
    static const int kTypeAcceptor = 1;
    static const int kTypeConnection = 2;

    static const int kFstItc = 0x0100;
protected:
    friend class ItcFileSystem;
    std::string localAddr_;
    std::string remoteAddr_;
    int fileType_;
};

class AcceptObject : public FileObject {
public:
    AcceptObject();
    virtual ~AcceptObject();
    virtual void close();

    //writeԤ�ȴ���handles���ȴ���ȡ���󡣷�ֹû��read������ʧ
    bool write(int handle);
    virtual int read(qkrtl::Buffer& buffer);

    inline const std::string& name() const { return localAddr(); }
    static const int kMaxHandleSize = 64;

    static AcceptObject* fromFileObject(qkrtl::FileObject* fobj);
    static const AcceptObject* fromFileObject(const qkrtl::FileObject* fobj);

private:
    qkrtl::SpinLock guard_;
    int handles_[kMaxHandleSize];
    int head_;
    int tail_;
};

class ConnectionObject : public FileObject {
public:
    ConnectionObject();
    virtual ~ConnectionObject();
    virtual void close();

    virtual int read(qkrtl::Buffer& buffer);
    virtual int write(qkrtl::Buffer& buffer);

    inline int peerHandle() const { return peerHandle_; }
    inline void peerHandle(int handle) { peerHandle_ = handle; }

    static ConnectionObject* fromFileObject(qkrtl::FileObject* fobj);
    static const ConnectionObject* fromFileObject(const qkrtl::FileObject* fobj);
private:
    qkrtl::Stream buffers_;
    int peerHandle_;
    //�ɶԶ�д��
    int peerWrite(qkrtl::Buffer& buffer);
};

}

#endif /**QKITC_FILE_OBJECT_H*/
