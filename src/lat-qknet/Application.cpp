
#include "Application.h"
#include "qkrtl/Logger.h"


Application::Application()
    :finaled_(false) , ioService_("iocp") , server_(ioService_), client_(ioService_)
{
    //
}
Application::~Application()
{
    final();
}

bool Application::init(uint16_t port, int maxTimes, int bufferSize)
{
    std::unique_lock<std::mutex> locker(guard_);
    if (server_.init(port) == false)
        return false;
 
    if (client_.init(maxTimes, bufferSize) == false)
        return false;

    ioService_.run();

    return client_.connect("127.0.0.1", port , 1000);
}
void Application::final()
{
    std::unique_lock<std::mutex> locker(guard_);
    if (finaled_ == true)
        return;
    finaled_ = true;

    client_.final();
    server_.final();
    ioService_.final();
}
bool Application::waitForCompleted()
{
    return client_.waitForCompleted();
}

