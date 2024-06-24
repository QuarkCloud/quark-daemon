
#include "qkrtl/Sequence.h"
#include <atomic>
#include <thread>

namespace qkrtl {

Sequence::Sequence():value_(0)
{
    //
}
Sequence::Sequence(int64_t initialValue) :value_(initialValue)
{
    //
}
Sequence::~Sequence()
{
    //
}
int64_t Sequence::tryGetValue() const
{
    return std::atomic_load_explicit(&value_, std::memory_order_relaxed);
}
int64_t Sequence::getValue() const
{
    return std::atomic_load_explicit(&value_, std::memory_order_acquire);
}
void Sequence::setValue(int64_t value)
{
    std::atomic_store_explicit(&value_, value, std::memory_order_release);
}
bool Sequence::compareAndSet(int64_t expected, int64_t next)
{
    return std::atomic_compare_exchange_strong(&value_, &expected, next);
}
int64_t Sequence::addAndGet(int64_t value)
{
    return std::atomic_fetch_add_explicit(&value_, value, std::memory_order_release) + value;
}
int64_t Sequence::subAndGet(int64_t value)
{
    return std::atomic_fetch_sub_explicit(&value_, value, std::memory_order_release) - value;
}

}
