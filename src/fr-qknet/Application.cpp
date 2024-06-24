
#include "Application.h"
#include "qkrtl/Logger.h"


Application::Application(qkrtl::IoService& ioService):finaled_(false), server_(ioService)
{
    //
}
Application::~Application()
{
    final();
}

bool Application::init(uint16_t port)
{
    std::unique_lock<std::mutex> locker(guard_);
    return server_.init(port);
}
void Application::final()
{
    std::unique_lock<std::mutex> locker(guard_);
    if (finaled_ == true)
        return;
    finaled_ = true;

    server_.final();
}

