
#include "Application.h"
#include "qkrtl/Logger.h"


Application::Application()
    :finaled_(false) , ioService_("inth") , server_(ioService_), client_(ioService_)
{
    //
}
Application::~Application()
{
    final();
}

bool Application::init(const std::string& name, int maxTimes, int bufferSize)
{
    std::unique_lock<std::mutex> locker(guard_);
    if (server_.create(name) == false)
        return false;

    if (client_.open(name) == false)
        return false;
 
    if (client_.init(maxTimes, bufferSize) == false)
        return false;

    ioService_.run();

    return client_.startOutput();
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

