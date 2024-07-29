
#include "qkrtl/Logger.h"
#include "FileSystem.h"
#include "FileObject.h"

namespace qkitc {

ItcFileSystem::ItcFileSystem():finaled_(false)
{
    //
}
ItcFileSystem::~ItcFileSystem()
{
    final();
}
void ItcFileSystem::final()
{
    std::unique_lock<std::mutex> locker(guard_);
    if (finaled_ == true)
        return;
    finaled_ = true;
    urls_.clear();
}
qkrtl::FileObject* ItcFileSystem::create(const qkrtl::Url& url, qkrtl::FileNotifier* notifier)
{
    if (url.protocol() != kProtocol)
        return NULL;
    const std::string& addr = url.host();
    std::unique_lock<std::mutex> locker(guard_);
    std::map<std::string, int>::iterator uiter = urls_.find(addr);
    if (uiter != urls_.end())
    {
        int handle = uiter->second;
        return FsFind(handle);
    }

    AcceptObject* obj = new AcceptObject();
    obj->localAddr_ = addr;
    obj->setNotifier(notifier);
    int index = FsInsert(obj);
    if (index == kFsInvalidIndex)
    {
        delete obj;
        return NULL;
    }

    urls_.insert(std::pair<std::string, int>(addr, index));
    return obj;
}
qkrtl::FileObject* ItcFileSystem::open(const qkrtl::Url& url, qkrtl::FileNotifier* notifier)
{
    if (url.protocol() != kProtocol)
        return NULL;
    const std::string& addr = url.host();
    std::unique_lock<std::mutex> locker(guard_);
    std::map<std::string, int>::iterator uiter = urls_.find(addr);
    if (uiter == urls_.end())
        return NULL;
    int acceptHandle = uiter->second;
    AcceptObject* acceptor = AcceptObject::fromFileObject(FsFind(acceptHandle));
    if (acceptor == NULL)
        return NULL;

    ConnectionObject* conn = new ConnectionObject();
    conn->remoteAddr_ = addr;
    conn->setNotifier(notifier);

    int connIndex = FsInsert(conn);
    if (connIndex == kFsInvalidIndex)
    {
        delete conn;
        return NULL;
    }

    ConnectionObject* sess = new ConnectionObject();
    sess->localAddr_ = addr;
    int sessIndex = FsInsert(sess);
    if (sessIndex == kFsInvalidIndex)
    {
        delete sess;
        return NULL;
    }

    conn->peerHandle(sessIndex);
    sess->peerHandle(connIndex);

    acceptor->write(sessIndex);
    return conn;
}
const std::string ItcFileSystem::kProtocol = "itc";

static qkrtl::FileSystemMounter __ItcFileSystemMounter__(ItcFileSystem::kProtocol, new ItcFileSystem());

}
