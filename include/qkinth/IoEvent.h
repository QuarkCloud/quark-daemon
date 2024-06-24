
#ifndef QKINTH_IO_EVENT_H
#define QKINTH_IO_EVENT_H 1

#include <time.h>
#include <string>
#include "qkinth/Compile.h"
#include "qkrtl/RingBuffer.h"
#include "qkrtl/Object.h"
#include "qkrtl/IoHandle.h"

namespace qkinth {
/**
    ��FileSystem����������Inth�ļ�ϵͳ�ڲ�����
    ����ģ������Pipe��Socket����Ϊ
*/
class IoEvent : public qkrtl::IoHandler{
public:
    QKINTHAPI IoEvent();
    QKINTHAPI virtual ~IoEvent();

    QKINTHAPI void failure(int errCode);
    QKINTHAPI void success(int bytes);

    inline int bytes() const { return bytes_; }
    inline void bytes(int value) { bytes_ = value; }
    inline int status() const { return status_; }
    inline void status(int value) { status_ = value; }
protected:
    QKINTHAPI virtual void ioCompleted();
    int bytes_;
    int status_;
};

}

#endif /**QKINTH_IO_EVENT_H*/
