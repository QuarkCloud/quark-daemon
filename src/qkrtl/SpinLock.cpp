
#include <atomic>
#include <thread>
#include "qkrtl/SpinLock.h"
#include "qkrtl/PowerTwo.h"

namespace qkrtl {

SpinLock::SpinLock():value_(0) , gating_(1)
{
    //
}
SpinLock::~SpinLock()
{
    //
}
bool SpinLock::lock()
{
    const int64_t kLoopShift = 6;
    const int64_t kLoopMask = (1LL << kLoopShift) - 1;
    const int64_t kLoopMaxTime = 1LL << (kLoopShift + 2);

    int64_t currentValue = value_.fetch_add(1, std::memory_order_release) + 1;
    int64_t gateValue = 0;
    int64_t counter = 0;
    while ((gateValue= gating_.load(std::memory_order_relaxed)) != currentValue)
    {
        if (counter >= kLoopMaxTime)
        {
            counter = 0;
            gateValue = gating_.load(std::memory_order_acquire);
            if (gateValue == currentValue)
                return true;

            std::this_thread::yield();
        }
        else
        {
            ++counter;
        }
    }
    return (currentValue == gating_.load(std::memory_order_acquire));
}
void SpinLock::unlock()
{
    gating_.fetch_add(1, std::memory_order_release);
}


}
