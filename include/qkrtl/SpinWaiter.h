
#ifndef QKRTL_SPIN_WAITER_H
#define QKRTL_SPIN_WAITER_H 1

#include <atomic>
#include "qkrtl/Compile.h"
#include "qkrtl/Synch.h"

namespace qkrtl {

class SpinWaiter : public Waiter{
public:
    QKRTLAPI SpinWaiter();
    QKRTLAPI ~SpinWaiter();

    QKRTLAPI virtual bool waitFor(int timeout); //���뼶
    QKRTLAPI virtual void notifyOne();
    QKRTLAPI virtual void notifyAll();
    QKRTLAPI int waiterCount() const;
private:
    HANDLE semaphore_;  //linux�汾����
    std::atomic<int> waiters_;
};

}
#endif /**QKRTL_SPIN_WAITER_H*/
