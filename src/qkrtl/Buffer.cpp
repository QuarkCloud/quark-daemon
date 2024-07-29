
#include <stdlib.h>
#include <string.h>
#include "qkrtl/Buffer.h"
#include "qkrtl/PowerTwo.h"
#include "qkrtl/SystemInfo.h"

namespace qkrtl {
Buffer::Buffer()
    :cache_(NULL) , capacity_(0) , dataStart_(0) , dataSize_(0) , status_(kStatusNone)
{
    //
}
Buffer::Buffer(const Buffer& buffer) 
    : cache_(NULL), capacity_(0), dataStart_(0), dataSize_(0), status_(kStatusNone)
{
    refer(buffer);
}
Buffer::Buffer(char* buffer, int buflen) 
    : cache_(NULL), capacity_(0), dataStart_(0), dataSize_(0), status_(kStatusNone)
{
    refer(buffer, buflen);
}
Buffer::~Buffer()
{
    free();
}

bool Buffer::extend(int length)
{
    if (cache_ == NULL || capacity_ == 0 || length < 0)
        return false;

    int offset = length + dataStart_ + dataSize_;
    if (offset > capacity_)
        return false;
    dataSize_ += length;
    return true;
}
bool Buffer::shrink(int length)
{
    if (cache_ == NULL || capacity_ == 0 || length < 0)
        return false;
    if (dataSize_ < length)
        return false;
    dataSize_ -= length;
    dataStart_ += length;
    return true;
}
void Buffer::squish()
{
    if (dataStart_ != 0)
    {
        if (cache_ != NULL && dataSize_ != 0)
        {
            ::memmove(cache_, cache_ + dataStart_, dataSize_);
        }
        dataStart_ = 0;
    }
}
void Buffer::discard()
{
    dataStart_ = 0;
    dataSize_ = 0;
}
bool Buffer::refer(const char* buffer, int buflen, int start, int size)
{
    free();

    if (buffer == NULL || buflen <= 0 || start < 0 || size < 0)
        return false;

    if (start + size > buflen)
        return false;

    cache_ = (char *)buffer;
    capacity_ = buflen;
    dataStart_ = start;
    dataSize_ = size;
    status_ = kStatusNone;
    return true;
}
bool Buffer::refer(const Buffer& buffer)
{
    free();

    cache_ = buffer.cache_;
    capacity_ = buffer.capacity_;
    dataStart_ = buffer.dataStart_;
    dataSize_ = buffer.dataSize_;
    status_ = kStatusNone;
    return true;
}
bool Buffer::move(Buffer& buffer)
{
    free();

    cache_ = buffer.cache_;
    capacity_ = buffer.capacity_;
    dataStart_ = buffer.dataStart_;
    dataSize_ = buffer.dataSize_;
    status_ = buffer.status_;

    buffer.clear();
    return true;
}
void Buffer::clear()
{
    cache_ = NULL;
    capacity_ = 0;
    dataStart_ = dataSize_ = 0;
    status_ = kStatusNone;
}
int Buffer::append(const char* buffer, int size)
{
    if (buffer == NULL || size <= 0)
        return -1;

    int copySize = avaibleSize();
    if (copySize < size)
    {
        squish();
        copySize = avaibleSize();
    }
    if (copySize > size)
        copySize = size;
    if (copySize <= 0)
        return 0;

    char* dst = tail();
    ::memcpy(dst, buffer, copySize);
    dataSize_ += copySize;
    return copySize;
}
void Buffer::swap(Buffer& buffer)
{
    char* tmpCache = cache_;
    int tmpCapacity = capacity_;
    int tmpDataStart = dataStart_;
    int tmpDataSize = dataSize_;
    int tmpStatus_ = status_;

    cache_ = buffer.cache_;
    capacity_ = buffer.capacity_;
    dataStart_ = buffer.dataStart_;
    dataSize_ = buffer.dataSize_;
    status_ = buffer.status_;

    buffer.cache_ = tmpCache;
    buffer.capacity_ = tmpCapacity;
    buffer.dataStart_ = tmpDataStart;
    buffer.dataSize_ = tmpDataSize;
    buffer.status_ = tmpStatus_;
}
const char* Buffer::head() const
{
    if (cache_ == NULL)
        return NULL;
    return cache_ + dataStart_;
}
char* Buffer::head()
{
    if (cache_ == NULL)
        return NULL;
    return cache_ + dataStart_;
}
const char* Buffer::tail() const
{
    if (cache_ == NULL)
        return NULL;
    return (cache_ + dataStart_ + dataSize_);
}
char* Buffer::tail()
{
    if (cache_ == NULL)
        return NULL;
    return (cache_ + dataStart_ + dataSize_);
}
bool Buffer::malloc(int size)
{
    clear();
    if (size <= 0)
        return false;

    char* addr = (char*)::malloc(size);
    if (addr == NULL)
        return false;

    cache_ = addr;
    capacity_ = size;
    status_ = kStatusOwned;
    return true;
}
void Buffer::free()
{
    char* addr = cache_;
    clear();

    if (status_ == kStatusOwned && addr != NULL)
        ::free(addr);
}

Allocator::Allocator()
{
    //
}
Allocator::~Allocator()
{
    //
}
bool Allocator::alloc(Buffer& buffer, int size)
{
    return buffer.malloc(size);
}
void Allocator::free(Buffer& buffer)
{
    buffer.free();
}
}

