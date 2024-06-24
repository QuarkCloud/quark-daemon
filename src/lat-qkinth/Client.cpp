#include "Client.h"
#include "qkrtl/Logger.h"
#include "qkinth/FileIo.h"

Client::Client(qkrtl::Poller& poller) 
    :qkinth::Connection(poller), finaled_(false) ,
    maxTimes_(0), bufferSize_(0), expectSize_(0), readSize_(0), writedSize_(0)
{
    //
}
Client::~Client()
{
    final();
}
bool Client::init(int maxTimes, int bufferSize)
{
    maxTimes_ = maxTimes;
    bufferSize_ = bufferSize;

    expectSize_ = maxTimes;
    expectSize_ *= bufferSize_;

    if (maxTimes_ <= 0 || bufferSize_ <= 0)
        return false;

    if (inBuffer_.malloc(bufferSize_) == false )
        return false;

    if (outBuffer_.malloc(bufferSize_) == false)
        return false;

    return true;
}
void Client::final()
{
    std::unique_lock<std::mutex> locker(guard_);
    if (finaled_ == true)
        return;
    finaled_ = true;

    close();
    inBuffer_.free();
    outBuffer_.free();
}
bool Client::handleInput(int errCode)
{
    std::unique_lock<std::mutex> locker(guard_);
    if (finaled_ == true || errCode != 0)
    {
        LOGERR("Client[%p] handle[%d] handleInput failed , finaled[%s] errCode[%d]",
            this, handle(), finaled_ ? "True" : "False", errCode);
        return false;
    }

    char* buffer = inBuffer_.cache();
    int buflen = inBuffer_.capacity();

    int dataSize = qkinth::FileRead(handle(), buffer, buflen);
    if (dataSize > 0)
    {
        readSize_ += dataSize;
        LOGDEBUG("Client[%p] handle[%d] handleInput succeed to read [%d] bytes , read size[%d]",
            this, handle(), dataSize , readSize_);
    }

    if (readSize_ < writedSize_)
    {
        LOGDEBUG("Client[%p] handle[%d] handleInput , read size[%d] write size[%d]",
            this, handle(), readSize_ , writedSize_);
        return true;
    }

    if (readSize_ >= expectSize_)
    {
        timeElapse_.stop();
        LOGCRIT("Client[%p] handle[%d] time elapse completed",
            this, handle());
        states_.notify(maxTimes_);
        return true;
    }

    return startOutput();
}
bool Client::handleOutput(int errCode)
{
    std::unique_lock<std::mutex> locker(guard_);
    if (finaled_ == true || errCode != 0)
    {
        LOGERR("Client[%p] handle[%d] handleInput failed , finaled[%s] errCode[%d]",
            this, handle(), finaled_ ? "True" : "False", errCode);
        return false;
    }

    if (writedSize_ == 0)
    {
        LOGCRIT("Client[%p] handle[%d] time elapse start",this, handle());
        timeElapse_.start();
    }
    LOGDEBUG("Client[%p] handle[%d] handleOutput , writedSize[%lld]",this, handle(), writedSize_);

    if (writedSize_ >= expectSize_)
    {
        return false;
    }

    char* buffer = outBuffer_.head();
    int dataSize = outBuffer_.dataSize();
    if (dataSize <= 0)
    {
        outBuffer_.discard();
        outBuffer_.extend(bufferSize_);

        buffer = outBuffer_.head();
        dataSize = outBuffer_.dataSize();
    }

    int size = qkinth::FileWrite(handle(), buffer, dataSize);
    if (size > 0)
    {
        outBuffer_.shrink(size);
        writedSize_ += size;
        LOGDEBUG("Client[%p] handle[%d] handleOutput succeed to write [%d] bytes , writedSize[%d]", 
            this, handle(), size , writedSize_);
    }
    else
    {
        LOGERR("Client[%p] handle[%d] handleOutput failed to write data , size[%d]",
            this, handle(), size);
    }

    return true;
}

bool Client::waitForCompleted()
{
    return states_.waitFor(maxTimes_, 60 * 1000);
}
