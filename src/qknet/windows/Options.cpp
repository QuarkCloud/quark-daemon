
#include "qkrtl/FileSystem.h"
#include "qknet/Options.h"

#include "Socket.h"
#include "FileObject.h"

namespace qknet {
namespace windows {

FileObject* GetNetFileObject(int handle)
{
    qkrtl::FileObject* fobj = FsFind(handle);
    if (fobj == NULL || fobj->fsType() != FileObject::kFstNet)
        return NULL;

    return (FileObject*)fobj;
}

}

bool OptionSetNodelay(int handle, bool value)
{
    windows::FileObject* fobj = windows::GetNetFileObject(handle);
    if (fobj == NULL)
        return false;

    return fobj->getSocket().nodelay(value);
}
bool OptionGetNodelay(int handle, bool& value)
{
    windows::FileObject* fobj = windows::GetNetFileObject(handle);
    if (fobj == NULL)
        return false;

    value = fobj->getSocket().nodelay();
    return true;
}
bool OptionSetNonBlocking(int handle, bool value)
{
    windows::FileObject* fobj = windows::GetNetFileObject(handle);
    if (fobj == NULL)
        return false;

    return fobj->getSocket().nonblocking(value);
}
bool OptionGetNonBlocking(int handle, bool& value)
{
    windows::FileObject* fobj = windows::GetNetFileObject(handle);
    if (fobj == NULL)
        return false;

    value = fobj->getSocket().nodelay();
    return true;
}
bool OptionSetReuse(int handle, bool value)
{
    windows::FileObject* fobj = windows::GetNetFileObject(handle);
    if (fobj == NULL)
        return false;

    return fobj->getSocket().reuse(value);
}
bool OptionGetReuse(int handle, bool& value)
{
    windows::FileObject* fobj = windows::GetNetFileObject(handle);
    if (fobj == NULL)
        return false;

    value = fobj->getSocket().reuse();
    return true;
}

bool OptionSetKeepAlive(int handle, bool value)
{
    windows::FileObject* fobj = windows::GetNetFileObject(handle);
    if (fobj == NULL)
        return false;

    return fobj->getSocket().keepalive(value);
}
bool OptionGetKeepAlive(int handle, bool& value)
{
    windows::FileObject* fobj = windows::GetNetFileObject(handle);
    if (fobj == NULL)
        return false;

    value = fobj->getSocket().keepalive();
    return true;
}
bool OptionGetAvaibleSize(int handle, int& value)
{
    windows::FileObject* fobj = windows::GetNetFileObject(handle);
    if (fobj == NULL)
        return false;

    value = fobj->getSocket().avaibleSize();
    return true;
}
bool OptionSetSendBufferSize(int handle, int value)
{
    windows::FileObject* fobj = windows::GetNetFileObject(handle);
    if (fobj == NULL)
        return false;

    return fobj->getSocket().sendBufsize(value);
}
bool OptionGetSendBufferSize(int handle, int& value)
{
    windows::FileObject* fobj = windows::GetNetFileObject(handle);
    if (fobj == NULL)
        return false;

    value = fobj->getSocket().sendBufsize();
    return true;
}
bool OptionSetRecvBufferSize(int handle, int value)
{
    windows::FileObject* fobj = windows::GetNetFileObject(handle);
    if (fobj == NULL)
        return false;

    return fobj->getSocket().recvBufsize(value);
}
bool OptionGetRecvBufferSize(int handle, int& value)
{
    windows::FileObject* fobj = windows::GetNetFileObject(handle);
    if (fobj == NULL)
        return false;

    value = fobj->getSocket().recvBufsize();
    return true;
}
bool OptionGetSockType(int handle, int& value)
{
    windows::FileObject* fobj = windows::GetNetFileObject(handle);
    if (fobj == NULL)
        return false;

    value = fobj->getSocket().sockType();
    return true;
}

}

