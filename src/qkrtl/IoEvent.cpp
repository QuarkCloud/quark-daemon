
#include "qkrtl/IoEvent.h"
#include <string>

namespace qkrtl {

IoStatus::IoStatus():status_(kStatusNone)
{
    //
}
static const int kStatusNameMaxSize = 5;
static const char* __StatusNames__[kStatusNameMaxSize + 1] = {
    "None" , "Doing" , "Done" , "Closing" , "Closed" , "Invalid"
};
const char* IoStatus::toString(int status)
{
    if (status < 0 || status >= kStatusNameMaxSize)
        return __StatusNames__[kStatusNameMaxSize];
    else
        return __StatusNames__[status];
}

IoEvent::IoEvent(IoHandle& ioHandle)
    : IoHandler(ioHandle) , errCode_(0) , ioType_(kIoNone)
{
    //
}
IoEvent::~IoEvent()
{
    //
}
void IoEvent::handleCompleted(int errCode)
{
    errCode_ = errCode;
    if (isDoing() == true)
        setDone();
    ready();
}
InEvent::InEvent(IoHandle& ioHandle, int ioType):IoEvent(ioHandle)
{
    ioType_ = ioType;
}
InEvent::~InEvent()
{
    //
}
void InEvent::close()
{
    //
}
bool InEvent::start()
{
    return true;
}
void InEvent::stop()
{
    //
}
bool InEvent::handle(int errCode)
{
    return ioHandle_.handleInput(errCode);
}
void InEvent::execute()
{
    completed();
    handle(errCode_);
}

OutEvent::OutEvent(IoHandle& ioHandle, int ioType) :IoEvent(ioHandle)
{
    ioType_ = ioType;
}
OutEvent::~OutEvent()
{
    //
}
void OutEvent::close()
{
    //
}
bool OutEvent::start()
{
    return true;
}
void OutEvent::stop()
{
    //
}
bool OutEvent::handle(int errCode)
{
    return ioHandle_.handleOutput(errCode);
}
void OutEvent::execute()
{
    completed();
    handle(errCode_);
}


namespace windows {

FileEvent::FileEvent():errCode_(0), bytes_(0)
{
    OVERLAPPED* ovlp = (OVERLAPPED*)this;
    ::memset(ovlp, 0, sizeof(OVERLAPPED));
}
FileEvent::~FileEvent()
{
    //
}
void FileEvent::failure(int errCode)
{
    errCode_ = errCode;
    bytes_ = 0;
    ioCompleted();
}
void FileEvent::success(int bytes)
{
    errCode_ = 0;
    bytes_ = bytes;
    ioCompleted();
}
void FileEvent::ioCompleted()
{
    //
}

}
}

