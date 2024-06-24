
#include "qkrtl/SpinWaiter.h"

namespace qkrtl {

SpinWaiter::SpinWaiter():semaphore_(INVALID_HANDLE_VALUE) , waiters_(0)
{
    semaphore_ = ::CreateSemaphore(NULL, 1, MAXLONG  , NULL);
}
SpinWaiter::~SpinWaiter()
{
    if (semaphore_ != INVALID_HANDLE_VALUE)
    {
        ::CloseHandle(semaphore_);
        semaphore_ = INVALID_HANDLE_VALUE;
    }
}
bool SpinWaiter::waitFor(int timeout)
{
    waiters_.fetch_add(1, std::memory_order_release);

    DWORD msec = 0;
    if (timeout < 0)
        msec = INFINITE;
    else
        msec = timeout;

    DWORD retvalue = ::WaitForSingleObject(semaphore_, msec);

    waiters_.fetch_sub(1, std::memory_order_release);

    return (retvalue == WAIT_OBJECT_0);
}
void SpinWaiter::notifyOne()
{
    LONG currentValue = waiterCount();
    if(currentValue > 0)
        ::ReleaseSemaphore(semaphore_, 1, NULL);
}
void SpinWaiter::notifyAll()
{
    LONG currentValue = waiterCount();
    if(currentValue > 0)
        ::ReleaseSemaphore(semaphore_, currentValue, NULL);
}
int SpinWaiter::waiterCount() const
{
    return waiters_.load(std::memory_order_acquire);
}

}
