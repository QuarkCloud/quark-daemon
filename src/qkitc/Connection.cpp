
#include "qkitc/Connection.h"
#include "qkrtl/FileSystem.h"

namespace qkitc {

Connection::Connection(qkrtl::Poller& poller)
    :poller_(poller) ,
    reader_(*this , qkrtl::IoEvent::kIoRead) , 
    writer_(*this, qkrtl::IoEvent::kIoRead)
{
    poller_.monitor(&reader_);
    poller_.monitor(&writer_);
}
Connection::~Connection()
{
    //
}
void Connection::close()
{
    FsClose(getHandle());
}
bool Connection::handleStart()
{
    return true;
}
bool Connection::handleStop()
{
    return true;
}
bool Connection::handleInput(int errCode)
{
    return true;
}
bool Connection::startInput()
{
    reader_.ready();
    return true;
}
bool Connection::handleOutput(int errCode)
{
    return true;
}
bool Connection::startOutput()
{
    writer_.ready();
    return true;
}
bool Connection::valid() const
{
    return FsValid(getHandle());
}
void Connection::notifyRead(int errCode)
{
    reader_.handleCompleted(errCode);
}
void Connection::notifyWrite(int bytes, int errCode)
{
    writer_.handleCompleted(errCode);
}
}