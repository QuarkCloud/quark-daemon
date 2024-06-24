
#include "qkrtl/IoHandle.h"

namespace qkrtl {

IoHandle::IoHandle()
{
    //
}
IoHandle::~IoHandle()
{
    //
}
void IoHandle::close()
{
    //
}
bool IoHandle::handleStart()
{
    return false;
}
bool IoHandle::handleStop()
{
    return false;
}
bool IoHandle::handleInput(int errCode)
{
    return true;
}
bool IoHandle::startInput()
{
    return true;
}
bool IoHandle::handleOutput(int errCode)
{
    return true;
}
bool IoHandle::startOutput()
{
    return true;
}
const HANDLE IoHandle::getHandle() const
{
    return INVALID_HANDLE_VALUE;
}
bool IoHandle::valid() const
{
    return false;
}

IoHandler::IoHandler()
{
    //
}
IoHandler::~IoHandler()
{
    //
}
void IoHandler::close()
{
    //
}
bool IoHandler::start()
{
    return false;
}
void IoHandler::stop()
{
    //
}
bool IoHandler::handleStart()
{
    return false;
}
bool IoHandler::handleStop()
{
    return false;
}
bool IoHandler::handle(int errCode)
{
    return false;
}
const HANDLE IoHandler::getHandle() const
{
    return INVALID_HANDLE_VALUE;
}
bool IoHandler::valid() const
{
    return false;
}

}