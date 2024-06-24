
#include "qkrtl/RingBuffer.h"

namespace qkrtl {

RingIndex::RingIndex() :ringSize_(0)
{
    //
}
RingIndex::~RingIndex()
{
    //
}
int RingIndex::nextHead(int64_t& sequence, int size, bool fulfill)
{
    return getHead().next(sequence, getTail().getGating(), size, fulfill);
}
bool RingIndex::commitHead(int64_t sequence, int maxSize)
{
    return getHead().commit(sequence, maxSize);
}
int RingIndex::nextTail(int64_t& sequence, int size, bool fulfill)
{
    int64_t barrier = getHead().getGating() + ringSize_;
    return getTail().next(sequence, barrier, size, fulfill);
}
bool RingIndex::commitTail(int64_t sequence, int maxSize)
{
    return getTail().commit(sequence, maxSize);
}
int RingIndex::backlog() const
{
    int64_t tail = getTail().getGating();
    int64_t head = getHead().getValue();

    return (int)(tail - head) - 1;
}
void RingIndex::ringSize(int size)
{
    ringSize_ = size;
}

NotSafeRingIndex::NotSafeRingIndex()
{
    //
}
NotSafeRingIndex::~NotSafeRingIndex()
{
    //
}

SafeRingIndex::SafeRingIndex()
{
    //
}
SafeRingIndex::~SafeRingIndex()
{
    //
}

SafeRingBuffer::SafeRingBuffer()
{
    //
}
SafeRingBuffer::~SafeRingBuffer()
{
    final();
}
bool SafeRingBuffer::init(int maxSize)
{
    if (buffer_.init(maxSize) == false)
        return false;

    index_.ringSize(buffer_.dataSize());
    return true;
}
void SafeRingBuffer::final()
{
    buffer_.final();
}
int SafeRingBuffer::read(char* buffer, int size)
{
    if (buffer == NULL || size <= 0)
        return -1;

    int64_t sequence = 0;
    int maxSize = index_.nextHead(sequence, size, false);
    if (maxSize <= 0)
        return maxSize;

    char* start = NULL;
    int copySize = 0;
    if (buffer_.getAvaible(start, copySize, sequence, size) == true)
    {
        ::memcpy(buffer, start, copySize);
        if (copySize < maxSize)
        {
            buffer += copySize;
            start = buffer_.getAddr(0);
            copySize = maxSize - copySize;
        }
    }

    if (copySize > 0)
        ::memcpy(buffer, start, copySize);

    index_.commitHead(sequence, maxSize);
    return maxSize;
}
int SafeRingBuffer::write(const char* buffer, int size)
{
    if (buffer == NULL || size <= 0)
        return -1;

    int64_t sequence = 0;
    int maxSize = index_.nextTail(sequence, size, false);
    if (maxSize <= 0)
    {
        return maxSize;
    }

    char* start = NULL;
    int copySize = 0;
    if (buffer_.getAvaible(start, copySize, sequence, size) == true)
    {
        ::memcpy(start, buffer, copySize);
        if (copySize < maxSize)
        {
            buffer += copySize;
            start = buffer_.getAddr(0);
            copySize = maxSize - copySize;
        }
    }

    if (copySize > 0)
        ::memcpy(start, buffer, copySize);

    index_.commitTail(sequence, maxSize);
    return maxSize;
}


}

