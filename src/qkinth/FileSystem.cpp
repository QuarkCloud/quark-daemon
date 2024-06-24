
#include "qkinth/FileSystem.h"
#include "qkrtl/Logger.h"

namespace qkinth {

FileSystem::FileSystem() :finaled_(false)
{
    init(kMaxHandleSize);
    worker_ = std::thread(&FileSystem::process, this);
}
FileSystem::~FileSystem()
{
    final();
}
bool FileSystem::init(int maxSize)
{
    std::unique_lock<std::mutex> locker(guard_);
    return objects_.init(maxSize);
}
void FileSystem::final()
{
    {
        std::unique_lock<std::mutex> locker(guard_);
        if (finaled_ == true)
            return;
        finaled_ = true;
        names_.clear();
        /**
            释放所有的对象
        */
        time_t now = ::time(NULL);
        int maxSize = objects_.maxSize();
        for (int idx = 0; idx < maxSize; ++idx)
        {
            qkrtl::Object* obj = NULL;
            if (objects_.remove(idx, obj) == false || obj == NULL)
                continue;

            Object* inth = (Object*)obj;
            inth->freeTime_ = now;
            removeds_.push_back(inth);
        }

        objects_.final();
    }

    if (worker_.joinable())
        worker_.join();
}
int FileSystem::create(const std::string& name)
{
    int index = Object::kInvalidIdentifier;

    if (name.empty() == true)
        return index;
    std::unique_lock<std::mutex> locker(guard_);
    std::map<std::string, int>::iterator niter = names_.find(name);
    if (niter != names_.end())
    {
        return niter->second;
    }

    Object* obj = new Object();
    if (objects_.insert(obj) == false)
    {
        delete obj;
    }
    else
    {
        obj->name_ = name;
        index = obj->identifier();
        names_.insert(std::pair<std::string, int>(name, index));
    }

    return index;
}
int FileSystem::open(const std::string& name)
{
    int index = qkrtl::Object::kInvalidIdentifier;

    if (name.empty() == true)
        return index;

    std::unique_lock<std::mutex> locker(guard_);
    std::map<std::string, int>::iterator niter = names_.find(name);
    if (niter == names_.end())
        return index;

    Object* obj = new Object();
    if (objects_.insert(obj) == false)
    {
        delete obj;
        return index;
    }
    obj->name_ = name;
    index = obj->identifier();
    int peer = niter->second;

    Object* peerObj = (Object*)objects_.find(peer);
    obj->peerHandle_ = peer;
    peerObj->peerHandle_ = index;
    return index;
}
void FileSystem::close(int handle)
{
    std::unique_lock<std::mutex> locker(guard_);

    Object* object = (Object*)objects_.find(handle);
    if (object == NULL || object->identifier() != handle)
        return;

    const std::string& name = object->name();
    std::map<std::string, int>::iterator niter = names_.find(name);
    if (niter == names_.end())
    {
        LOGERR("failed to close a not exist inth object , name[%s] handle[%d]" ,
            name.c_str() , handle);
        return;
    }
    else
    {
        LOGCRIT("succeed to close a  inth object , name[%s] handle[%d]",
            name.c_str(), handle);
    }

    bool isCreator = false;
    if (niter->second == handle)
    {
        isCreator = true;
        names_.erase(niter);
    }
    int peerHandle = object->peerHandle();
    object->peerHandle_ = Object::kInvalidIdentifier;
    objects_.remove(object);

    object->freeTime_ = ::time(NULL);
    removeds_.push_back(object);

    Object* peerObj = (Object*)objects_.find(peerHandle);
    if (peerObj != NULL)
    {
        if (peerObj->peerHandle() == handle)
            peerObj->peerHandle_ = Object::kInvalidIdentifier;
    }
}
int FileSystem::read(int handle, char* buffer, int size)
{
    if (buffer == NULL || size <= 0)
        return -1;

    Object* object = (Object*)objects_.find(handle);
    if (object == NULL)
        return -1;

    return object->read(buffer, size);
}
int FileSystem::write(int handle, const char* buffer, int size)
{
    if (buffer == NULL || size <= 0)
        return -1;

    Object* object = (Object*)objects_.find(handle);
    if (object == NULL)
        return -1;

    int peerHandle = object->peerHandle();
    Object* peerObj = (Object*)objects_.find(peerHandle);
    if (peerObj == NULL)
        return -1;

    int writedSize = peerObj->peerWrite(buffer, size);
    peerObj->outEvent(writedSize);
    return writedSize;
}
Object* FileSystem::find(int handle)
{
    return (Object *)objects_.find(handle);
}
const Object* FileSystem::find(int handle) const
{
    return (const Object*)objects_.find(handle);
}
void FileSystem::process()
{
    bool completed = false;
    while (completed == false)
    {
        std::list<Object*> removeds;
        {
            time_t now = ::time(NULL);
            std::unique_lock<std::mutex> locker(guard_);
            while (removeds_.empty() == false)
            {
                std::list<Object*>::iterator riter = removeds_.begin();
                Object* obj = (*riter);
                if (obj->freeTime_ + kRemoveExpired < now)
                {
                    break;
                }
                removeds.push_back(obj);
                removeds_.erase(riter);
            }

            if (finaled_ == true)
                completed = removeds_.empty();
        }

        if (removeds.empty() == true)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        else
        {
            for (std::list<Object*>::iterator riter = removeds.begin(); riter != removeds.end(); ++riter)
            {
                Object* obj = (*riter);
                if (obj == NULL)
                    continue;
                delete obj;
            }
        }
    }
}

FileSystem& FileSystem::singleton()
{
    static FileSystem __FileSystemSingleton__;
    return __FileSystemSingleton__;
}

}
