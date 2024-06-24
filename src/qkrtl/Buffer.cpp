
#include "qkrtl/Buffer.h"
#include "qkrtl/PowerTwo.h"
#include "qkrtl/SystemInfo.h"

namespace qkrtl {
Buffer::Buffer():cache_(NULL) , capacity_(0) , dataStart_(0) , dataSize_(0)
{
    //
}
Buffer::Buffer(const Buffer& buffer) 
    : cache_(NULL), capacity_(0), dataStart_(0), dataSize_(0)
{
    refer(buffer);
}
Buffer::Buffer(char* buffer, int buflen) 
    : cache_(NULL), capacity_(0), dataStart_(0), dataSize_(0)
{
    refer(buffer, buflen);
}
Buffer::~Buffer()
{
    //
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
bool Buffer::refer(char* buffer, int buflen, int start, int size)
{
    if (buffer == NULL || buflen <= 0 || start < 0 || size < 0)
        return false;

    if (start + size > buflen)
        return false;

    cache_ = buffer;
    capacity_ = buflen;
    dataStart_ = start;
    dataSize_ = size;
    return true;
}
bool Buffer::refer(const Buffer& buffer)
{
    cache_ = buffer.cache_;
    capacity_ = buffer.capacity_;
    dataStart_ = buffer.dataStart_;
    dataSize_ = buffer.dataSize_;
    return true;
}
bool Buffer::move(Buffer& buffer)
{
    cache_ = buffer.cache_;
    capacity_ = buffer.capacity_;
    dataStart_ = buffer.dataStart_;
    dataSize_ = buffer.dataSize_;

    buffer.clear();
    return true;
}
void Buffer::clear()
{
    cache_ = NULL;
    capacity_ = 0;
    dataStart_ = dataSize_ = 0;
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

    cache_ = buffer.cache_;
    capacity_ = buffer.capacity_;
    dataStart_ = buffer.dataStart_;
    dataSize_ = buffer.dataSize_;

    buffer.cache_ = tmpCache;
    buffer.capacity_ = tmpCapacity;
    buffer.dataStart_ = tmpDataStart;
    buffer.dataSize_ = tmpDataSize;
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

    int exactSize = 0;
    char* page = AllocPage(size , &exactSize);
    if (page == NULL || exactSize <= 0)
        return false;

    cache_ = page;
    capacity_ = exactSize;
    return true;
}
void Buffer::free()
{
    char* page = cache_;
    clear();
    if (page != NULL)
        ::FreePage(page);
}

}

char* AllocPage(int size, int* exactSize)
{
    if (size <= 0)
        return NULL;

    int pageSize = (int)qkrtl::SystemInfo::singleton().pageSize();
    int expectSize = qkrtl::AlignSize(size, pageSize);

    if (exactSize != NULL)
        *exactSize = expectSize;

    return (char*)::VirtualAlloc(NULL, expectSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
}
void FreePage(char* addr)
{
    ::VirtualFree(addr, 0, MEM_RELEASE);
}
