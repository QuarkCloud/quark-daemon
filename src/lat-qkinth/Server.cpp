#include "Server.h"
#include "qkrtl/Logger.h"
#include "qkinth/FileIo.h"

Server::Server(qkrtl::Poller& poller):qkinth::Connection(poller) , finaled_(false)
{
    buffer_.malloc(1 << 16);
}
Server::~Server()
{
    final();
}
void Server::final()
{
    std::unique_lock<std::mutex> locker(guard_);
    if (finaled_ == true)
        return;
    finaled_ = true;

    close();
    buffer_.free();
}
bool Server::handleInput(int errCode)
{
    std::unique_lock<std::mutex> locker(guard_);
    if (finaled_ == true || errCode != 0)
    {
        LOGERR("Server[%p] handle[%d] handleInput failed , finaled[%s] errCode[%d]" ,
            this , handle() , finaled_?"True":"False" , errCode);
        return false;
    }

    char* buffer = buffer_.tail();
    int buflen = buffer_.avaibleSize();

    if (buffer == NULL || buflen <= 0)
    {
        LOGERR("Server[%p] handle[%d] handleInput has no buffer , avaibleSize[%d]",
            this, handle(), buflen);
        startInput();
        return false;
    }

    int size = qkinth::FileRead(handle(), buffer, buflen);
    if (size > 0)
    {
        LOGDEBUG("Server[%p] handle[%d] handleInput succeed to read data , size[%d]",
            this, handle(), size);
        if (size >= buflen)
        {
            LOGDEBUG("Server[%p] handle[%d] handleInput has more data, size[%d] , buflen[%d]",
                this, handle(), size , buflen);
            startInput();       //继续读取后续的
        }

        buffer_.extend(size);
    }
    else
    {
        LOGERR("Server[%p] handle[%d] handleInput failed to read data , size[%d]",
            this, handle(), size);
    }

    return startOutput();
}
bool Server::handleOutput(int errCode)
{
    std::unique_lock<std::mutex> locker(guard_);
    if (finaled_ == true || errCode != 0)
    {
        LOGERR("Server[%p] handle[%d] handleOutput failed , finaled[%s] errCode[%d]",
            this, handle(), finaled_ ? "True" : "False", errCode);
        return false;
    }

    char* buffer = buffer_.head();
    int dataSize = buffer_.dataSize();
    if (buffer == NULL || dataSize <= 0)
    {
        LOGERR("Server[%p] handle[%d] handleOutput has no buffer , dataSize[%d]",
            this, handle(), dataSize);
        return false;
    }

    int size = qkinth::FileWrite(handle(), buffer, dataSize);
    if (size > 0)
    {
        LOGDEBUG("Server[%p] handle[%d] handleOutput succeed to write data , size[%d]",
            this, handle(), size);
        buffer_.shrink(size);
        if (buffer_.empty() == true)
            buffer_.squish();
    }
    else
    {
        LOGERR("Server[%p] handle[%d] handleOutput failed to write data , size[%d]",
            this, handle(), size);
    }


    return true;
}

