
#include "qkrtl/FileSystem.h"
#include "qkrtl/Logger.h"

namespace qkrtl {


FileNotifier::FileNotifier()
{
    //
}
FileNotifier::~FileNotifier()
{
    //
}
void FileNotifier::notifyOpen(int errCode)
{
    //
}
void FileNotifier::notifyClose(int errCode)
{
    //
}
void FileNotifier::notifyRead(int errCode)
{
    //
}
void FileNotifier::notifyWrite(int bytes, int errCode)
{
    //
}
FileNotifier FileNotifier::kDummyNotifier;

FileObject::FileObject()
    :notifier_(&FileNotifier::kDummyNotifier), 
    freeTime_(0) , fsType_(kFstNone) 
{
    //
}
FileObject::~FileObject()
{
    //
}
void FileObject::close()
{
    //
}
bool FileObject::start()
{
    return false;
}
bool FileObject::startRead()
{
    return false;
}
bool FileObject::startWrite()
{
    return false;
}
int FileObject::read(qkrtl::Buffer& buffer)
{
    return -1;
}
int FileObject::write(qkrtl::Buffer& buffer)
{
    return -1;
}

void FileObject::notifyOpen(int errCode)
{
    notifier_->notifyOpen(errCode);
}
void FileObject::notifyClose(int errCode)
{
    notifier_->notifyClose(errCode);
}
void FileObject::notifyRead(int errCode)
{
    notifier_->notifyRead(errCode);
}
void FileObject::notifyWrite(int bytes, int errCode)
{
    notifier_->notifyWrite(bytes, errCode);
}
const OsHandle FileObject::getOsHandle() const
{
    return kDefaultHandleValue;
}

void FileObject::setNotifier(FileNotifier* notifier)
{
    FileNotifier* old = notifier_;

    if (notifier == NULL)
        notifier_ = &FileNotifier::kDummyNotifier;
    else if(notifier != &FileNotifier::kDummyNotifier)
    {
        notifier_ = notifier;
    }
}

FileSystemImpl::FileSystemImpl()
{
    //
}
FileSystemImpl::~FileSystemImpl()
{
    final();
}
void FileSystemImpl::final()
{
    //
}
FileObject* FileSystemImpl::create(const Url& url, FileNotifier* notifier)
{
    return NULL;
}
FileObject* FileSystemImpl::open(const Url& url, FileNotifier* notifier)
{
    return NULL;
}

FileSystem::FileSystem():finaled_(false)
{
    worker_ = std::thread(&FileSystem::process, this);
}
FileSystem::~FileSystem()
{
    final();
}
bool FileSystem::init(int maxSize)
{
    return objects_.init(maxSize);
}
void FileSystem::final()
{
    {
        std::unique_lock<std::mutex> locker(guard_);
        if (finaled_ == true)
            return;
        finaled_ = true;
    }

    objects_.final();
    if (worker_.joinable() == true)
        worker_.join();


    for (std::map<std::string, FileSystemImpl*>::iterator fiter = fsImpls_.begin();
        fiter != fsImpls_.end(); ++fiter)
    {
        FileSystemImpl* impl = fiter->second;
        if (impl == NULL)
            continue;
        impl->final();
        delete impl;
    }
}
int FileSystem::create(const std::string& url, FileNotifier* notifier)
{
    Url parser;
    const char* str = url.c_str();
    int slen = (int)url.size();
    if (parser.decode(str, slen) == false)
        return kInvalidIndex;
    return create(parser, notifier);
}
int FileSystem::create(const Url& url, FileNotifier* notifier)
{
    FileObject* fobj = NULL;
    const std::string& protocol = url.protocol();

    std::unique_lock<std::mutex> locker(guard_);
    std::map<std::string, FileSystemImpl*>::iterator fiter = fsImpls_.find(protocol);
    if (fiter == fsImpls_.end())
        return kInvalidIndex;

    FileSystemImpl* impl = fiter->second;
    fobj = impl->create(url, notifier);
    if (fobj == NULL)
        return kInvalidIndex;

    return fobj->getHandle();
}
int FileSystem::open(const std::string& url, FileNotifier* notifier)
{
    Url parser;
    const char* str = url.c_str();
    int slen = (int)url.size();
    if (parser.decode(str, slen) == false)
        return kInvalidIndex;
    return open(parser, notifier);
}
int FileSystem::open(const Url& url, FileNotifier* notifier)
{
    FileObject* fobj = NULL;
    const std::string& protocol = url.protocol();

    std::unique_lock<std::mutex> locker(guard_);
    std::map<std::string, FileSystemImpl*>::iterator fiter = fsImpls_.find(protocol);
    if (fiter == fsImpls_.end())
        return kInvalidIndex;

    FileSystemImpl* impl = fiter->second;
    fobj = impl->open(url, notifier);
    if (fobj == NULL)
        return kInvalidIndex;

    return fobj->getHandle();
}
bool FileSystem::setNotifier(int handle, FileNotifier* notifier)
{
    FileObject* fobj = find(handle);
    if (fobj == NULL)
        return false;
    fobj->setNotifier(notifier);
    return true;
}
void FileSystem::close(int handle)
{
    FileObject* fobj = NULL;
    if (remove(handle, fobj) == true && fobj != NULL)
    {
        LOGCRIT("fileObject[%p] fsType[%d] handle[%d] will be closed" , 
            fobj , fobj->fsType() , handle);
        fobj->close();
        std::unique_lock<std::mutex> locker(guard_);
        fobj->freeTime_ = ::time(NULL);
        removeds_.push_back(fobj);
    }
    else
    {
        LOGERR("failed to close fileObject handle[%d] , which not exist",handle);
    }
}
int FileSystem::read(int handle, qkrtl::Buffer& buffer)
{
    FileObject* fobj = find(handle);
    if (fobj == NULL)
        return -1;
    return fobj->read(buffer);
}
int FileSystem::write(int handle, qkrtl::Buffer& buffer)
{
    FileObject* fobj = find(handle);
    if (fobj == NULL)
        return -1;
    return fobj->write(buffer);
}
bool FileSystem::start(int handle)
{
    FileObject* fobj = find(handle);
    if (fobj == NULL)
        return false;
    return fobj->start();
}
bool FileSystem::startRead(int handle)
{
    FileObject* fobj = find(handle);
    if (fobj == NULL)
        return false;
    return fobj->startRead();
}
bool FileSystem::startWrite(int handle)
{
    FileObject* fobj = find(handle);
    if (fobj == NULL)
        return false;
    return fobj->startWrite();
}
int FileSystem::insert(FileObject* obj)
{
    if (objects_.insert(obj) == false)
        return Object::kInvalidIdentifier;

    return obj->getHandle();
}
bool FileSystem::remove(FileObject* obj)
{
    return objects_.remove(obj);
}
bool FileSystem::remove(int handle, FileObject*& obj)
{
    Object* old = NULL;
    if (objects_.remove(handle, old) == false)
        return false;
    obj = (FileObject*)old;
    return true;
}
bool FileSystem::exchange(int handle, FileObject* obj, FileObject*& old)
{
    Object* fo = NULL;
    if (objects_.exchange(handle, obj, fo) == false)
        return false;
    old = (FileObject*)fo;
    return true;
}
FileObject* FileSystem::find(int handle)
{
    return (FileObject*)objects_.find(handle);
}
const FileObject* FileSystem::find(int handle) const
{
    return (const FileObject*)objects_.find(handle);
}
bool FileSystem::mount(const std::string& protocol, FileSystemImpl* impl)
{
    if (protocol.empty() == true || impl == NULL)
        return false;

    std::unique_lock<std::mutex> locker(guard_);
    std::map<std::string, FileSystemImpl*>::iterator iter = fsImpls_.find(protocol);
    if (iter != fsImpls_.end())
    {
        if (iter->second == impl)
            return true;
        return false;
    }

    return fsImpls_.insert(std::pair<std::string, FileSystemImpl*>(protocol, impl)).second;
}
int FileSystem::listMounts(std::list<std::string>& mounts) const
{
    std::unique_lock<std::mutex> locker(guard_);
    int counter = 0;
    for(std::map<std::string, FileSystemImpl*>::const_iterator fiter = fsImpls_.begin() ;
        fiter != fsImpls_.end() ; ++fiter)
    {
        const std::string& name = fiter->first;
        if (name.empty() == true)
            continue;
        mounts.push_back(name);
        ++counter;
    }
    return counter;
}
void FileSystem::process()
{
    bool completed = false;
    while (completed == false)
    {
        std::list<FileObject*> removeds;
        {
            time_t now = ::time(NULL);
            std::unique_lock<std::mutex> locker(guard_);
            while (removeds_.empty() == false)
            {
                std::list<FileObject*>::iterator riter = removeds_.begin();
                FileObject* obj = (*riter);
                if (obj->freeTime() + kRemoveExpired < now)
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
            for (std::list<FileObject*>::iterator riter = removeds.begin(); riter != removeds.end(); ++riter)
            {
                FileObject* obj = (*riter);
                if (obj == NULL)
                    continue;

                LOGCRIT("fileObject[%p] fsType[%d] handle[%d] will be freed" ,
                    obj , obj->fsType() , obj->getHandle());
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

FileSystemMounter::FileSystemMounter(const std::string& protocol, FileSystemImpl* impl)
{
    FileSystem::singleton().mount(protocol, impl);
}

}

bool FsInit(int maxSize)
{
    return qkrtl::FileSystem::singleton().init(maxSize);
}
void FsFinal()
{
    qkrtl::FileSystem::singleton().final();
}
int FsCreate(const std::string& url, qkrtl::FileNotifier* notifier)
{
    return qkrtl::FileSystem::singleton().create(url, notifier);
}
int FsOpen(const std::string& url, qkrtl::FileNotifier* notifier)
{
    return qkrtl::FileSystem::singleton().open(url, notifier);
}
bool FsNotifier(int handle, qkrtl::FileNotifier* notifier)
{
    return qkrtl::FileSystem::singleton().setNotifier(handle, notifier);
}
void FsClose(int handle)
{
    qkrtl::FileSystem::singleton().close(handle);
}
bool FsValid(int handle)
{
    return qkrtl::FileSystem::singleton().valid(handle);
}
int FsInsert(qkrtl::FileObject* obj)
{
    return qkrtl::FileSystem::singleton().insert(obj);
}
bool FsRemove(int handle, qkrtl::FileObject*& obj)
{
    return qkrtl::FileSystem::singleton().remove(handle , obj);
}
bool FsExchange(int handle, qkrtl::FileObject* obj, qkrtl::FileObject*& old)
{
    return qkrtl::FileSystem::singleton().exchange(handle, obj , old);
}
int FsRead(int handle, qkrtl::Buffer& buffer)
{
    return qkrtl::FileSystem::singleton().read(handle, buffer);
}
int FsWrite(int handle, qkrtl::Buffer& buffer)
{
    return qkrtl::FileSystem::singleton().write(handle, buffer);
}
bool FsStart(int handle)
{
    return qkrtl::FileSystem::singleton().start(handle);
}
bool FsStartRead(int handle)
{
    return qkrtl::FileSystem::singleton().startRead(handle);
}
bool FsStartWrite(int handle)
{
    return qkrtl::FileSystem::singleton().startWrite(handle);
}

qkrtl::FileObject* FsFind(int handle)
{
    return qkrtl::FileSystem::singleton().find(handle);
}
const OsHandle FsGetOsHandle(int handle)
{
    qkrtl::FileObject* fobj = FsFind(handle);
    if (fobj == NULL)
        return kDefaultHandleValue;
    return fobj->getOsHandle();
}
int FsHandleSize()
{
    return qkrtl::FileSystem::singleton().size();
}
