
#include "qkrtl/Synch.h"

namespace qkrtl {

Mutex::Mutex()
{
    //
}
Mutex::~Mutex()
{
    //
}
bool Mutex::lock()
{
    return false;
}
void Mutex::unlock()
{
    //
}

Locker::Locker(Mutex& mutex) :guard_(mutex)
{
    guard_.lock();
}
Locker::~Locker()
{
    guard_.unlock();
}
bool Locker::lock()
{
    return guard_.lock();
}
void Locker::unlock()
{
    guard_.unlock();
}

Waiter::Waiter()
{
    //
}
Waiter::~Waiter()
{
    //
}
bool Waiter::wait()
{
    return waitFor(-1);
}
bool Waiter::waitFor(int timeout)
{
    return false;
}
void Waiter::notifyOne()
{
    //
}
void Waiter::notifyAll()
{
    //
}
int Waiter::waiterCount() const
{
    return 0;
}
}
