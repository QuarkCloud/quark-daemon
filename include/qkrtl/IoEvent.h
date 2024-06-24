
#ifndef QKRTL_IO_EVENT_H
#define QKRTL_IO_EVENT_H 1

#include "qkrtl/Compile.h"
#include "qkrtl/IoHandle.h"

namespace qkrtl {

/**
    针对windows版本的IoEvent。
    执行failure和success后，自动执行ready，挂接到eventpoller。
*/
class IoEvent : public OVERLAPPED , public IoHandler{
public:
    QKRTLAPI IoEvent();
    QKRTLAPI virtual ~IoEvent();

    QKRTLAPI void failure(int errCode);
    QKRTLAPI void success(int bytes);

    static const int kIoNone = 0;
    static const int kIoAccept = 1;
    static const int kIoConnect = 2;
    static const int kIoRead = 3;
    static const int kIoWrite = 4;

    inline int ioType() const { return ioType_; }
protected:
    QKRTLAPI virtual void ioCompleted();
    int status_;
    int ioType_;
    int bytes_;
};
}

#endif /**QKRTL_IO_EVENT_H*/
