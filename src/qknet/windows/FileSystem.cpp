
#include "qkrtl/Logger.h"
#include "FileSystem.h"
#include "Socket.h"
#include "FileObject.h"

namespace qknet {
namespace windows {

NetFileSystem::NetFileSystem() :finaled_(false)
{
    //
}
NetFileSystem::~NetFileSystem()
{
    final();
}
void NetFileSystem::final()
{
    std::unique_lock<std::mutex> locker(guard_);
    if (finaled_ == true)
        return;
    finaled_ = true;
    urls_.clear();
}
qkrtl::FileObject* NetFileSystem::create(const qkrtl::Url& url, qkrtl::FileNotifier* notifier)
{
    if (url.valid() == false)
        return NULL;

    std::string addr;
    if (url.toAddress(addr) == false)
        return NULL;

    {
        std::unique_lock<std::mutex> locker(guard_);
        std::map<std::string, int>::iterator niter = urls_.find(addr);
        if (niter != urls_.end())
        {
            return FsFind(niter->second);
        }
    }

    const std::string& protocol = url.protocol();
    if (protocol == kTcpProtocol)
    {
        AcceptObject* acceptor = new AcceptObject();
        if (acceptor->init() == false || acceptor->listen(url.host(), url.port()) == false)
        {
            delete acceptor;
            return NULL;
        }

        int index = FsInsert(acceptor);
        if (FsValid(index) == false)
        {
            delete acceptor;
            return NULL;
        }
        else
        {
            if (notifier != NULL)
                acceptor->setNotifier(notifier);

            std::string address = acceptor->toUrl();
            urls_.insert(std::pair<std::string, int>(address, index));
            return acceptor;
        }
    }
    else if (protocol == kUdpProtocol)
    {
        UdpServerObject* udpObj = new UdpServerObject();
        udpObj->init();
        if (udpObj->init() == false || udpObj->listen(url.host(), url.port()) == false)
        {
            delete udpObj;
            return NULL;
        }

        int index = FsInsert(udpObj);
        if (FsValid(index) == false)
        {
            delete udpObj;
            return NULL;
        }
        else
        {
            if (notifier != NULL)
                udpObj->setNotifier(notifier);

            std::string address = udpObj->toUrl();
            urls_.insert(std::pair<std::string, int>(address, index));
            return udpObj;
        }
    }

    return NULL;
}

qkrtl::FileObject* NetFileSystem::open(const qkrtl::Url& url, qkrtl::FileNotifier* notifier)
{
    if(url.valid(true) == false)
        return NULL;

    std::unique_lock<std::mutex> locker(guard_);
    const std::string& protocol = url.protocol();

    if (protocol == kTcpProtocol)
    {
        ConnectorObject* connector = new ConnectorObject();
        if (connector->init() == false || connector->connect(url.host() , url.port()) == false)
            return NULL;

        int connectorId = FsInsert(connector);
        if (FsValid(connectorId) == false)
        {
            delete connector;
            return NULL;
        }
        else
        {
            if(notifier != NULL)
                connector->setNotifier(notifier);
            return connector;
        }
    }
    else if (protocol == kUdpProtocol)
    {
        UdpClientObject* udpObj = new UdpClientObject();
        udpObj->init();
        if (udpObj->init() == false || udpObj->open(url.host(), url.port()) == false)
        {
            delete udpObj;
            return NULL;
        }

        int index = FsInsert(udpObj);
        if (FsValid(index) == false)
        {
            delete udpObj;
            return NULL;
        }
        else
        {
            if (notifier != NULL)
                udpObj->setNotifier(notifier);

            return udpObj;
        }
    }

    return NULL;
}

const std::string NetFileSystem::kTcpProtocol = "tcp";
const std::string NetFileSystem::kUdpProtocol = "udp";

static qkrtl::FileSystemMounter __TcpFileSystemMounter__(NetFileSystem::kTcpProtocol, new NetFileSystem());
static qkrtl::FileSystemMounter __UdpFileSystemMounter__(NetFileSystem::kUdpProtocol, new NetFileSystem());

}
}