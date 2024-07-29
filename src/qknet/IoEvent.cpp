
#include "qknet/IoEvent.h"
#include "qkrtl/Logger.h"
#include "qkrtl/DateTime.h"
#include "qkrtl/FileSystem.h"

namespace qknet{

TcpSocketReader::TcpSocketReader(qkrtl::IoHandle& handle)
    :qkrtl::InEvent(handle , qkrtl::IoEvent::kIoRead) 
{
    nodeType_ = qkrtl::EventNode::kTypeSock;
}
TcpSocketReader::~TcpSocketReader()
{
    //
}
bool TcpSocketReader::read(qkrtl::Buffer& buffer)
{
    return (FsRead(getHandle(), buffer) > 0);
}
void TcpSocketReader::close()
{
    //
}
bool TcpSocketReader::start()
{
    int handle = getHandle();
    return FsStartRead(handle);
}
void TcpSocketReader::stop()
{
    close();
}

TcpSocketWriter::TcpSocketWriter(qkrtl::IoHandle& handle)
    :qkrtl::OutEvent(handle, qkrtl::IoEvent::kIoWrite)
{
    nodeType_ = qkrtl::EventNode::kTypeSock;
}
TcpSocketWriter::~TcpSocketWriter()
{
    //
}
bool TcpSocketWriter::write(qkrtl::Buffer& buffer)
{
    return (FsWrite(getHandle(), buffer) > 0);
}
void TcpSocketWriter::close()
{
    //
}
bool TcpSocketWriter::start()
{
    int handle = getHandle();
    return FsStartWrite(handle);
}
void TcpSocketWriter::stop()
{
    close();
}

TcpSocketAcceptor::TcpSocketAcceptor(qkrtl::IoHandle& handle)
    :qkrtl::InEvent(handle, qkrtl::IoEvent::kIoAccept)
{
    nodeType_ = qkrtl::EventNode::kTypeSock;
}
TcpSocketAcceptor::~TcpSocketAcceptor()
{
    //
}
void TcpSocketAcceptor::close()
{

}
bool TcpSocketAcceptor::start()
{
    return FsStartRead(getHandle());
}
void TcpSocketAcceptor::stop()
{
    close();
}


TcpSocketConnector::TcpSocketConnector(qkrtl::IoHandle& handle)
    :qkrtl::InEvent(handle, qkrtl::IoEvent::kIoConnect)
{
    nodeType_ = qkrtl::EventNode::kTypeSock;
}
TcpSocketConnector::~TcpSocketConnector()
{
    //
}
void TcpSocketConnector::close()
{
    //
}
bool TcpSocketConnector::start()
{
    return FsStartRead(getHandle());
}
void TcpSocketConnector::stop()
{
    close();
}


}
