
#include "qkrtl/Logger.h"
#include "qkrtl/IoEvent.h"
#include "qkrtl/IoHandle.h"
#include "IocpPoller.h"

namespace qkrtl {

IocpPoller::IocpPoller() 
    :finaled_(false) , iocp_(NULL), threadsRunning_(0), isShutdown_(false)
{
    iocp_ = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
    events_.setWaiter(this);
}
IocpPoller::~IocpPoller()
{
    final();
}
bool IocpPoller::monitor(EventNode* node)
{
    if (node == NULL)
        return false;

    int nodeType = node->nodeType();

    if (nodeType == EventNode::kTypeFile ||
        nodeType == EventNode::kTypePipe ||
        nodeType == EventNode::kTypeSock)
    {
        IoHandler* handler = (IoHandler*)node;
        if (handler->valid() == false)
            return false;

        HANDLE handle = handler->getOsHandle();

        std::unique_lock<std::mutex> locker(guard_);
        std::map<HANDLE , int>::iterator hiter = handles_.find(handle);
        if (hiter == handles_.end())
        {
            if (::CreateIoCompletionPort(handle, iocp_, 0, 0) == NULL)
            {
                DWORD errCode = ::GetLastError();
                LOGERR("failed to bind file handle[%p] , errCode[%d]", handle, (int)errCode);
                return false;
            }
            else
            {
                LOGINFO("succeed to bind file handle[%p] ", handle);
            }
            handles_.insert(std::pair<HANDLE , int>(handle , 1));
        }
        else
        {
            hiter->second += 1;
            LOGINFO("Handle[%p] had succeed to bind , counter[%d] ", handle , hiter->second);
        }
        events_.monitor(node);
        return handler->start();        
    }
    else if (nodeType == EventNode::kTypeTimer)
    {
        TimerTask* timer = (TimerTask*)node;
        events_.monitor(timer);
        return timer_.add(timer);
    }
    else
    {
        return events_.monitor(node);
    }
}

bool IocpPoller::unmonitor(EventNode* node)
{
    if (node == NULL)
        return false;

    int nodeType = node->nodeType();
    if (events_.unmonitor(node) == false)
    {
        LOGERR("node[%p] failed to unmonitored , nodeType[%s]" , 
            node , EventNode::nodeTypeName(nodeType));
        return false;
    }

    LOGINFO("node[%p] will unmonitor , nodeType[%s]",
        node, EventNode::nodeTypeName(nodeType));

    if (nodeType == EventNode::kTypeFile ||
        nodeType == EventNode::kTypePipe ||
        nodeType == EventNode::kTypeSock)
    {
        IoHandler* handler = (IoHandler*)node;
        if (handler->valid() == false)
        {
            LOGERR("node[%p] nodeType[%s] handle is invalid",
                node, EventNode::nodeTypeName(nodeType) );
            return false;
        }
        HANDLE handle = handler->getOsHandle();
        handler->stop();
        std::unique_lock<std::mutex> locker(guard_);
        bool result = true;
        std::map<HANDLE , int>::iterator hiter = handles_.find(handle);
        if (hiter != handles_.end())
        {
            hiter->second -= 1;
            int count = hiter->second;
            if (count <= 0)
            {
                handles_.erase(hiter);
                result = UnbindIoCompletionPort(handle);
                LOGDEBUG("Handle[%p] had to unbindIoCompletionPort , result[%s]",
                    handle, result ? "TRUE" : "FALSE");
            }
            else
            {
                LOGINFO("Handle[%p] still has [%d] bindings , no need unbind",handle, count);
            }
        }
        else
        {
            LOGERR("Handle[%p] can not find" , handle);
            result = false;
        }

        return result;
    }
    else if (nodeType == EventNode::kTypeTimer)
    {
        TimerTask* timer = (TimerTask*)node;
        return timer_.remove(timer);
    }
    else
    {
        return events_.unmonitor(node);
    }
}
bool IocpPoller::post(const Event& event)
{
    //if (isShutdown_ == true || finaled_ == true)
    //    return false;
    return events_.post(event);
}
int IocpPoller::wait(Event* events, int maxCount, int timeout)
{
    //if (isShutdown_ == true || finaled_ == true)
    //    return 0;

    waitFor(timeout);
    timer_.process();

    return events_.peek(events, maxCount);
}

bool IocpPoller::shutdown()
{
    std::unique_lock<std::mutex> locker(guard_);
    if (isShutdown_ == true)
        return true;
    isShutdown_ = true;
    ::PostQueuedCompletionStatus(iocp_, 0, 1, NULL);
    return true;
}
void IocpPoller::final()
{
    std::unique_lock<std::mutex> locker(guard_);
    if (finaled_ == true)
        return;
    finaled_ = true;
    isShutdown_ = true;
    while (threadsRunning_ > 0)
    {
        ::PostQueuedCompletionStatus(iocp_, 0, 1, NULL);
        notifier_.wait_for(locker, std::chrono::milliseconds(100));
    }

    if (iocp_ != NULL)
    {
        ::CloseHandle(iocp_);
        iocp_ = NULL;
    }

    handles_.clear();
    timer_.final();
    events_.final();
}
bool IocpPoller::waitFor(int timeout)
{
    const ULONG kEntryMaxCount = 32;
    OVERLAPPED_ENTRY entries[kEntryMaxCount];
    ULONG completedCount = 0;
    BOOL result = FALSE;
    ULONG entryCount = kEntryMaxCount;

    threadsRunning_.fetch_add(1, std::memory_order_release);
    int count = events_.backlog();
    if (count > 0)
        timeout = 0;
    LOGDEBUG("Poller GetQueuedCompletionStatusEx , events backlog[%d] timeout[%d]",
        count , timeout);

    //如果已经有events在积压，那么文件部分不再等待。
    result = ::GetQueuedCompletionStatusEx(iocp_,
        entries, entryCount, &completedCount, timeout, FALSE);

    LOGDEBUG("Poller GetQueuedCompletionStatusEx , reuslt[%s] , completedCount[%d] timeout[%d]",
        result==TRUE?"True":"False" , (int)completedCount , timeout);


    threadsRunning_.fetch_sub(1, std::memory_order_release);

    if (result == TRUE)
    {
        for (ULONG cidx = 0; cidx < completedCount; ++cidx)
        {
            OVERLAPPED_ENTRY& entry = entries[cidx];
            ULONG_PTR key = entry.lpCompletionKey;
            DWORD bytes = entry.dwNumberOfBytesTransferred;
            OVERLAPPED* ovlp = entry.lpOverlapped;
            ULONG_PTR internal = entry.Internal;

            LOGDEBUG("Poller GetQueuedCompletionStatusEx , ovlp[%p] key[%u] bytes[%u] , internal[%p] ",
                ovlp, (uint32_t)key, (uint32_t)bytes, (HANDLE)internal);

            if (key == 1)
            {
                if (threadsRunning_.load(std::memory_order_acquire) > 0)
                    ::PostQueuedCompletionStatus(iocp_, 0, 1, NULL);
                continue;
            }
            else if (key == 2 || ovlp == NULL)
            {
                continue;
            }

            windows::FileEvent* ioEvent = (windows::FileEvent*)ovlp;
            if (internal == 0)
            {
                ioEvent->success(bytes);
            }
            else
            {
                //以后再使用RtlNtStatusToDosError 处理NTSTATUS的转换逻辑
                //目前返回0XC0000120 = STATUS_CANCLED
                int errCode = (int)(internal & 0X0FFFFFFFULL);
                LOGERR("GetQueuedCompletionStatus recv an internal[%x] , ovlp[%p] errCode[%d]" , 
                    internal , ovlp , errCode);
                ioEvent->failure(errCode);
            }

        }

        return true;
    }
    else
    {
        DWORD errCode = ::GetLastError();
        if (errCode != WAIT_TIMEOUT)
        {
            LOGERR("failed to GetQueuedCompletionStatus , errCode[%d]", (int)errCode);
            return false;
        }
        else
        {
            return true;
        }
    }
}
void IocpPoller::notifyOne()
{
    if(waiterCount() > 0)
        ::PostQueuedCompletionStatus(iocp_, 0, 2, NULL);
}
void IocpPoller::notifyAll()
{
    notifyOne();
}
int IocpPoller::waiterCount() const
{
    return threadsRunning_.load(std::memory_order_acquire);
}

class IocpPollerlCreator : public PollerImplCreator{
public:
    virtual PollerImpl* operator()(void* param)
    {
        return new IocpPoller();
    }
};

static PollerImplCreatorAutoRegister __IocpCreatorRegister__("iocp" , new IocpPollerlCreator());

}
