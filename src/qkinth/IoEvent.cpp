
#include "qkinth/IoEvent.h"
#include "qkrtl/Logger.h"

namespace qkinth {

IoEvent::IoEvent() :bytes_(0), status_(0)
{
    //
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
