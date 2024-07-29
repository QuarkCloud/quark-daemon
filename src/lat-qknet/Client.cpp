
#include "Client.h"
#include "qkrtl/Logger.h"


Client::Client(qkrtl::Poller& poller)
    :qknet::Connector(poller) , finaled_(false) ,
    maxTimes_(0) , bufferSize_(0) , expectSize_(0) , readSize_(0) , writedSize_(0)
{
    LOGDEBUG("Client[%p] created ", this);
}
Client::~Client()
{
    LOGDEBUG("Client[%p] will be freed", this);
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

    if (outBuffer_.malloc(bufferSize_) == false)
        return false;

    char* out = outBuffer_.cache();
    int size = outBuffer_.capacity();
    if (out == NULL || size <= 0)
        return false;
    ::memset(out, 0, size);
    return true;
}
void Client::final()
{
    std::unique_lock<std::mutex> locker(guard_);
    if (finaled_ == true)
        return ;
    finaled_ = true;

    LOGCRIT("Clien[%p] will final" , this);

    outBuffer_.free();
    close();
}
bool Client::handleStop()
{
    int handle = getHandle();
    std::unique_lock<std::mutex> locker(guard_);
    LOGERR("Client[%p] handle[%d] handleStop ", this , handle);
    return false;
}

bool Client::handleRead(qkrtl::Buffer& buffer)
{
    int handle = getHandle();
    readSize_ += buffer.dataSize();

    LOGCRIT("Client[%p] handle[%d] handleRead , readSize[%lld] dataSize[%d]",
        this, handle , readSize_, buffer.dataSize());

    buffer.discard();

    if (readSize_ < writedSize_)
        return true;

    if (readSize_ >= expectSize_)
    {
        LOGCRIT("Client[%p] handle[%d] time elapse completed",this, handle);
        timeElapse_.stop();
        states_.notify(maxTimes_);
        return true;
    }

    return startOutput();
}
bool Client::handleWrited(qkrtl::Buffer& buffer)
{
    //outBuffer不释放
    //发送请求的所有内存已经全部完成，
    outBuffer_.discard();
    return true;
}
bool Client::handleOutput(int errCode)
{
    int handle = getHandle();
    if (writedSize_ == 0)
    {
        LOGCRIT("Client[%p] handle[%d] time elapse start",this, handle);
        timeElapse_.start();
    }
    LOGDEBUG("Client[%p] handle[%d] handleOutput , writedSize[%lld]",this, handle, writedSize_);

    if (writedSize_ >= expectSize_)
    {
        LOGDEBUG("Client[%p] handle[%d] handleOutput , writed completed", this, handle);
        return false;
    }
    if (writedSize_ > readSize_)
    {
        LOGDEBUG("Client[%p] handle[%d] handleOutput , read not completed . writedSize[%lld] readSize[%lld]", 
            this, handle , writedSize_ , readSize_);
        return false;
    }


    int64_t* outData = (int64_t*)outBuffer_.cache();
    if (outData == NULL)
        return false;

    *outData = writedSize_;
    if (outBuffer_.extend(bufferSize_) == false)
    {
        LOGERR("Client[%p] handle[%d] handleOutput , failed to extend out buffer", this, handle);
        return false;
    }
    writedSize_ += bufferSize_;

    if (write(outBuffer_) == false)
    {
        LOGERR("Client[%p] handle[%d] handleOutput , failed to write , dataSize[%d]", 
            this, handle , outBuffer_.dataSize());
        return false;
    }
    else
    {
        LOGDEBUG("Client[%p] handle[%d] handleOutput , succeed to write , dataSize[%d]",
            this, handle, outBuffer_.dataSize());
        return true;
    }
}
bool Client::waitForCompleted()
{
    return states_.waitFor({ maxTimes_ , kConnectFailed }, 60 * 1000);
}
