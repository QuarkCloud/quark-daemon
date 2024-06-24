
#include "qkinth/Connection.h"
#include "qkinth/FileIo.h"
#include "qkinth/Object.h"

namespace qkinth {

Reader::Reader(Connection* owner):owner_(owner)
{
    //
}
Reader::~Reader()
{
    //
}
void Reader::execute()
{
    completed();
    owner_->handleInput(status_);
}
Writer::Writer(Connection* owner) :owner_(owner)
{
    //
}
Writer::~Writer()
{
    //
}
void Writer::execute()
{
    completed();
    owner_->handleOutput(status_);
}

Connection::Connection(qkrtl::Poller& poller)
    :poller_(poller) ,reader_(this) , writer_(this),
    handle_(Object::kInvalidIdentifier)
{
    //
}
Connection::~Connection()
{
    //
}
bool Connection::create(const std::string& name)
{
    int handle = FileCreate(name);
    if (FileValid(handle) == false)
        return false;

    if(FileSetIoEvent(handle , &reader_ , &writer_) == false)
        return false;

    handle_ = handle;
    poller_.monitor(&reader_);
    poller_.monitor(&writer_);

    return true;
}
bool Connection::open(const std::string& name)
{
    int handle = FileOpen(name);
    if (FileValid(handle) == false)
        return false;

    if (FileSetIoEvent(handle, &reader_, &writer_) == false)
        return false;

    handle_ = handle;
    poller_.monitor(&reader_);
    poller_.monitor(&writer_);

    return true;
}
void Connection::close()
{
    int handle = handle_;
    handle_ = Object::kInvalidIdentifier;
    FileClose(handle);
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
const HANDLE Connection::getHandle() const
{
    return NULL;
}
bool Connection::valid() const
{
    return FileValid(handle_);
}
}