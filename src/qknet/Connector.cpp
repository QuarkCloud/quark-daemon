
#include "qknet/Connector.h"
#include "qkrtl/Logger.h"

namespace qknet {

ConnectTimeout::ConnectTimeout(Connection& connection)
    :connection_(connection)
{
    //
}
ConnectTimeout::~ConnectTimeout()
{
    //
}
void ConnectTimeout::execute()
{
    connection_.notifyOpen(Connector::kTimeout);
}

Connector::Connector(qkrtl::Poller& poller)
    :Connection(poller) , finaled_(false) , timeout_(*this) , connector_(*this)
{
    LOGDEBUG("Connector[%p] created", this);
}
Connector::~Connector()
{
    LOGDEBUG("Connector[%p] will be freed" , this);
    final();
}
bool Connector::connect(const std::string& host, uint16_t port, int timeout)
{
    const int kUrlMaxSize = 1024;
    char url[kUrlMaxSize] = { '\0' };

    ::sprintf(url, "tcp://%s:%hu", host.c_str() , port);
    int handle = FsOpen(url, this);
    if (FsValid(handle) == false)
        return false;

    setHandle(handle);
    osHandle_ = FsGetOsHandle(handle);

    ioStatus_.setDoing();

    if (timeout < 0)
        timeout = 3600 * 1000;  //一个小时
    timeout_.runAfter(timeout * 1000);

    LOGDEBUG("Connection[%p] handle[%d] osHandle[%p] will monitor eventNode , timer[%p] connector[%p]",
        this, handle, osHandle_, &timeout_, &connector_);

    poller_.monitor(&timeout_);
    poller_.monitor(&connector_);

    return true;
}
void Connector::final()
{
    {
        std::unique_lock<std::mutex> locker(guard_);
        if (finaled_ == true)
            return;
        finaled_ = true;
    }

    if (timeout_.isMonitored() == true)
        poller_.unmonitor(&timeout_);

    if (connector_.isMonitored() == true)
        poller_.unmonitor(&connector_);

    Connection::final();
}
void Connector::doAsynDestroy()
{
    LOGDEBUG("Connector[%p] will do asynch destroy", this);
    if (timeout_.isMonitored() == true)
        poller_.unmonitor(&timeout_);

    poller_.asynDestroy(this);
}
void Connector::notifyOpen(int errCode)
{
    int handle = getHandle();
    LOGCRIT("Connector[%p] handle[%d] notifyOpen , errCode[%d]" , this , handle , errCode);

    //只要有一个返回，先取消timeout判断
    if (timeout_.isMonitored() == true)
        poller_.unmonitor(&timeout_);

    if (errCode == 0)
    {
        if (connected() == false)
        {
            connected(true);
            handleStart();
        }
        return;
    }

    if (errCode == kTimeout)
    {
        //由于超时事件触发，connector还在，所以先关闭connect事件
        close();
    }
    handleStop();
    if (connector_.isMonitored() == true)
        poller_.unmonitor(&connector_);
}


}
