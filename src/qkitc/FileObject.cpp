
#include "qkrtl/Synch.h"
#include "FileObject.h"
#include "FileSystem.h"

namespace qkitc {

FileObject::FileObject():fileType_(kTypeNone)
{
    fsType_ = kFstItc;
}
FileObject::~FileObject()
{
    //
}
AcceptObject::AcceptObject():handles_{0} , head_(0) , tail_(0)
{
    fileType_ = FileObject::kTypeAcceptor;
    ::memset(handles_, 0, sizeof(handles_));
}
AcceptObject::~AcceptObject()
{
    //
}
void AcceptObject::close()
{
    //
}
bool AcceptObject::write(int handle)
{
    {
        qkrtl::Locker locker(guard_);

        int index = tail_ + 1;
        if (index >= kMaxHandleSize)
            index = 0;
        if (index == head_)
            return false;

        handles_[tail_] = handle;
        tail_ = index;
    }
    //通知上层应用，可以读取
    notifyRead(0);
    return true;
}
int AcceptObject::read(qkrtl::Buffer& buffer)
{
    int* handles = (int*)buffer.tail();
    int handleCount = buffer.avaibleSize() / sizeof(int);
    if (handles == NULL || handleCount <= 0)
        return 0;

    qkrtl::Locker locker(guard_);
    if (head_ == tail_)
        return 0;

    int counter = 0;
    for (int idx = 0; idx < handleCount; ++idx)
    {
        if (head_ == tail_)
            break;

        handles[idx] = handles_[head_];
        handles_[head_] = 0;
        ++head_;
        if (head_ >= kMaxHandleSize)
            head_ = 0;

        ++counter;
    }

    return counter * sizeof(int);
}
AcceptObject* AcceptObject::fromFileObject(qkrtl::FileObject* fobj)
{
    if (fobj == NULL || fobj->fsType() != kFstItc)
        return NULL;

    FileObject* itc = (FileObject*)fobj;
    if (itc->fileType() != kTypeAcceptor)
        return NULL;

    return (AcceptObject*)itc;
}
const AcceptObject* AcceptObject::fromFileObject(const qkrtl::FileObject* fobj)
{
    if (fobj == NULL || fobj->fsType() != kFstItc)
        return NULL;

    const FileObject* itc = (FileObject*)fobj;
    if (itc->fileType() != kTypeAcceptor)
        return NULL;

    return (const AcceptObject*)itc;
}
ConnectionObject::ConnectionObject():peerHandle_(qkrtl::FileSystem::kInvalidIndex)
{
    fileType_ = kTypeConnection;
}
ConnectionObject::~ConnectionObject()
{
    //
}
void ConnectionObject::close()
{
    //
}
int ConnectionObject::read(qkrtl::Buffer& buffer)
{
    if (buffers_.pop(buffer) == false)
        return -1;

    return buffer.dataSize();
}
int ConnectionObject::write(qkrtl::Buffer& buffer)
{
    ConnectionObject* conn = ConnectionObject::fromFileObject(FsFind(peerHandle_));
    if (conn == NULL)
        return -1;

    int dataSize = conn->peerWrite(buffer);
    if (dataSize < 0)
        return dataSize;

    notifyWrite(dataSize, 0);
    return dataSize;
}
int ConnectionObject::peerWrite(qkrtl::Buffer& buffer)
{
    int dataSize = buffer.dataSize();
    if (buffers_.push(buffer) == false)
        return -1;

    notifyRead(0);
    return dataSize;
}
ConnectionObject* ConnectionObject::fromFileObject(qkrtl::FileObject* fobj)
{
    if (fobj == NULL || fobj->fsType() != kFstItc)
        return NULL;

    FileObject* itc = (FileObject*)fobj;
    if (itc->fileType() != kTypeConnection)
        return NULL;

    return (ConnectionObject*)itc;
}
const ConnectionObject* ConnectionObject::fromFileObject(const qkrtl::FileObject* fobj)
{
    if (fobj == NULL || fobj->fsType() != kFstItc)
        return NULL;

    const FileObject* itc = (const FileObject*)fobj;
    if (itc->fileType() != kTypeConnection)
        return NULL;

    return (const ConnectionObject*)itc;
}
}
