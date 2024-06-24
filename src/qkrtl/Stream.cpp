
#include "qkrtl/Stream.h"

namespace qkrtl {

Stream::Stream() 
    : head_(0) , tail_(0) , pushSize_(0) , popSize_(0) , closed_(false)
{
    //
}
Stream::~Stream()
{
    clear();
}
void Stream::close()
{
    if (closed_ == true)
        return;
    closed_ = true;
}
bool Stream::push(const Buffer& buffer)
{
    if (closed_ == true)
        return false;

    {
        std::unique_lock<std::mutex> locker(pushGuard_);
        if (buffer.empty() == true)
            return false;

        pushSize_ += buffer.dataSize();
        cache_.push_back(buffer);
    }
    return true;
}
bool Stream::pop(Buffer& buffer)
{
    std::unique_lock<std::mutex> olocker(popGuard_);
    if (head_ == tail_)
    {
        head_ = tail_ = 0;
        std::unique_lock<std::mutex> ulocker(pushGuard_);
        int counter = 0;
        while (cache_.empty() == false && counter < kBufferSize)
        {
            buffers_[tail_++] = cache_.front();
            cache_.pop_front();
            counter++;
        }

        if (counter == 0)
            return false;
    }

    buffer = buffers_[head_++];
    popSize_ += buffer.dataSize();
    return true;
}
bool Stream::pop(std::deque<Buffer>& buffers)
{
    std::unique_lock<std::mutex> olocker(popGuard_);
    for (; head_ < tail_; ++head_)
    {
        const Buffer& buffer = buffers_[head_];
        popSize_ += buffer.dataSize();
        buffers.push_back(buffer);
    }
    head_ = tail_ = 0;

    {
        std::unique_lock<std::mutex> ulocker(pushGuard_);
        while (cache_.empty() == false)
        {
            const Buffer& buffer = cache_.front();
            popSize_ += buffer.dataSize();
            buffers.push_back(buffer);
            cache_.pop_front();
        }
    }

    return true;
}
void Stream::clear()
{
    std::unique_lock<std::mutex> olocker(popGuard_);
    for (int idx = head_; idx < tail_; ++idx)
    {
        Buffer& buffer =  buffers_[idx];
        buffer.free();
    }
    head_ = tail_ = 0;

    {
        std::unique_lock<std::mutex> ulocker(pushGuard_);
        for (std::deque<Buffer>::iterator iter = cache_.begin(); iter != cache_.end(); ++iter)
        {
            Buffer& buffer = (*iter);
            buffer.free();
        }
        cache_.clear();
    }
}

bool Stream::empty() const
{
    return (head_ == tail_) && cache_.empty();
}
int Stream::count() const
{
    return (tail_ - head_) + (int)cache_.size();
}
int64_t Stream::dataSize() const
{
    return (pushSize_ - popSize_);
}
}

