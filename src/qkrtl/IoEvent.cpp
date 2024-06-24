
#include "qkrtl/IoEvent.h"
#include <string>

namespace qkrtl {

IoEvent::IoEvent():status_(0) , ioType_(kIoNone) , bytes_(0)
{
    OVERLAPPED* ovlp = (OVERLAPPED *)this;
    ::memset(ovlp, 0, sizeof(OVERLAPPED));
}
IoEvent::~IoEvent()
{
    //
}
void IoEvent::failure(int errCode)
{
    status_ = errCode;
    bytes_ = 0;
    ioCompleted();
    ready();
}
void IoEvent::success(int bytes)
{
    status_ = 0;
    bytes_ = (int)bytes;
    ioCompleted();
    ready();
}
void IoEvent::ioCompleted()
{
    //
}
}

