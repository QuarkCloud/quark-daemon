
#include "qkrtl/Poller.h"
#include "qkrtl/Logger.h"

namespace qkrtl {

PollerImpl::PollerImpl()
{
    //
}
PollerImpl::~PollerImpl()
{
    final();
}
bool PollerImpl::monitor(EventNode* node)
{
    return false;
}
bool PollerImpl::unmonitor(EventNode* node)
{
    return false;
}
bool PollerImpl::post(const Event& event)
{
    return false;
}
bool PollerImpl::asynDestroy(Object* object)
{
    qkrtl::Event event = qkrtl::Event::Initializer;
    event.dataType = qkrtl::Event::kDataTypeObj;
    event.status = qkrtl::Event::kStatusFree;
    event.data.object = object;
    return post(event);
}
int PollerImpl::wait(Event* events, int maxCount, int timeout)
{
    return 0;
}
bool PollerImpl::shutdown()
{
    return false;
}
void PollerImpl::final()
{
    //
}
PollerImpl* PollerImplCreator::operator()(void* param)
{
    return NULL;
}
PollerImplRegistry::PollerImplRegistry()
{
    //
}
PollerImplRegistry::~PollerImplRegistry()
{
    std::unique_lock<std::mutex> locker(guard_);
    for (std::map<std::string, PollerImplCreator*>::iterator iter = creators_.begin();
        iter != creators_.end(); ++iter)
    {
        PollerImplCreator* creator = iter->second; 
        if (creator == NULL)
            continue;
        delete creator;
    }
    creators_.clear();
}
bool PollerImplRegistry::add(const std::string& name, PollerImplCreator * creator)
{
    std::unique_lock<std::mutex> locker(guard_);
    if (name.empty() == true || creator == NULL)
        return false;
    std::map<std::string, PollerImplCreator *>::iterator iter = creators_.find(name);
    if (iter != creators_.end())
        return false;

    return creators_.insert(std::pair<std::string, PollerImplCreator *>(name, creator)).second;
}
PollerImpl* PollerImplRegistry::create(const std::string& name, void* param)
{
    PollerImplCreator * creator;
    {
        std::unique_lock<std::mutex> locker(guard_);
        std::map<std::string, PollerImplCreator *>::iterator iter = creators_.find(name);
        if (iter == creators_.end() || iter->second == NULL )
            return NULL;
        creator = iter->second;
    }

    return (*creator)(param);
}
PollerImplRegistry& PollerImplRegistry::singleton()
{
    static PollerImplRegistry __PollerImplRegistrySingleton__;
    return __PollerImplRegistrySingleton__;
}
PollerImplCreatorAutoRegister::PollerImplCreatorAutoRegister
    (const std::string& name, PollerImplCreator* creator)
{
    PollerImplRegistry::singleton().add(name, creator);
}
Poller::Poller(const std::string& name):impl_(NULL)
{
    impl_ = PollerImplRegistry::singleton().create(name, NULL);
}
Poller::~Poller()
{
    final();
}
void Poller::final()
{
    PollerImpl* impl = impl_;
    impl_ = NULL;
    if (impl != NULL)
    {
        impl->final();
        delete impl;
    }
}
bool Poller::monitor(EventNode* node)
{
    if (impl_ == NULL || node == NULL)
        return false;
    return impl_->monitor(node);
}
bool Poller::unmonitor(EventNode* node)
{
    if (impl_ == NULL || node == NULL)
        return false;
    return impl_->unmonitor(node);
}
bool Poller::post(const Event& event)
{
    if (impl_ == NULL )
        return false;
    return impl_->post(event);
}
bool Poller::asynDestroy(Object* object)
{
    if (impl_ == NULL)
        return false;
    return impl_->asynDestroy(object);
}
int Poller::wait(Event* events, int maxCount, int timeout)
{
    if (impl_ == NULL || events == NULL || maxCount <= 0)
        return 0;
    return impl_->wait(events, maxCount , timeout);
}
bool Poller::shutdown()
{
    if (impl_ == NULL)
        return false;
    return impl_->shutdown();
}
}
