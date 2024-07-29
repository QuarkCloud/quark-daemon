
#ifndef QKRTL_IO_EVENT_H
#define QKRTL_IO_EVENT_H 1

#include "qkrtl/Compile.h"
#include "qkrtl/IoHandle.h"

namespace qkrtl {

class QKRTLAPI IoStatus {
public:
    IoStatus();

    inline operator int() const { return status_; }

    static const int kStatusNone = 0;
    static const int kStatusDoing = 1;
    static const int kStatusDone = 2;
    static const int kStatusClosing = 3;
    static const int kStatusClosed = 4;

    static const char* toString(int status);

    inline bool isNone() const { return (status_ == kStatusNone); }
    inline void setNone() { status_ = kStatusNone; }

    inline bool isDoing() const { return (status_ == kStatusDoing); }
    inline void setDoing() { status_ = kStatusDoing; }

    inline bool isDone() const { return (status_ == kStatusDone); }
    inline void setDone() { status_ = kStatusDone; }

    inline bool isClosing() const { return (status_ == kStatusClosing); }
    inline void setClosing() { status_ = kStatusClosing; }

    inline bool isClosed() const { return (status_ == kStatusClosed); }
    inline void setClosed() { status_ = kStatusClosed; }

    inline const char* statusName() const { return toString(status_); }
private:
    int status_;
};

/**
    针对windows版本的IoEvent。
    执行failure和success后，自动执行ready，挂接到eventpoller。
*/
class IoEvent : public IoHandler , public IoStatus{
public:
    QKRTLAPI IoEvent(IoHandle& ioHandle);
    QKRTLAPI virtual ~IoEvent();
    QKRTLAPI void handleCompleted(int errCode = 0);

    static const int kIoNone = 0;
    static const int kIoAccept = 1;
    static const int kIoConnect = 2;
    static const int kIoRead = 3;
    static const int kIoWrite = 4;

    inline int ioType() const { return ioType_; }
protected:
    int errCode_;
    int ioType_;
};

class InEvent : public IoEvent {
public:
    QKRTLAPI InEvent(IoHandle& ioHandle, int ioType);
    QKRTLAPI virtual ~InEvent();
    QKRTLAPI virtual void close();
    QKRTLAPI virtual bool start();
    QKRTLAPI virtual void stop();
    QKRTLAPI virtual bool handle(int errCode = 0);
    QKRTLAPI virtual void execute();
};

class OutEvent : public IoEvent {
public:
    QKRTLAPI OutEvent(IoHandle& ioHandle, int ioType);
    QKRTLAPI virtual ~OutEvent();
    QKRTLAPI virtual void close();
    QKRTLAPI virtual bool start();
    QKRTLAPI virtual void stop();
    QKRTLAPI virtual bool handle(int errCode = 0);
    QKRTLAPI virtual void execute();
};


#if defined(PLATFORM_WINDOWS)

namespace windows {

/**
    windows的事件模式和linux不同，windows强烈依赖于OVERLAPPED。
    这个类由IOCP调用，子类由SOCKET/FILE/NAMED PIPE实现
*/
class QKRTLAPI FileEvent : public OVERLAPPED {
public:
    FileEvent();
    virtual ~FileEvent();

    void failure(int errCode);
    void success(int bytes);
protected:
    virtual void ioCompleted();
    int errCode_;
    int bytes_;
};

}

#endif

}

#endif /**QKRTL_IO_EVENT_H*/
