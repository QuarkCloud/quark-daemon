
#ifndef QKRTL_FILE_SYSTEM_H
#define QKRTL_FILE_SYSTEM_H 1

#include <time.h>
#include <string>
#include <mutex>
#include <map>
#include <list>

#include "qkrtl/Compile.h"
#include "qkrtl/Object.h"
#include "qkrtl/Url.h"
#include "qkrtl/OsHandle.h"

namespace qkrtl {

/**
    FileNotifier�����ã�����ĳ��������ɺ�֪ͨ��������������
    ����ӿڶ�Ӧ��IOCP�ĺ��壬��Ҫע����EPOLL�Ĳ�ͬ�㡣
*/
class QKRTLAPI FileNotifier {
public:
    FileNotifier();
    virtual ~FileNotifier();

    virtual void notifyOpen(int errCode);
    virtual void notifyClose(int errCode);

    //֪ͨ�ϲ�Ӧ�ã����Կ�ʼ���������Ƕ�д��ɡ������IOCP��һ������Ҫ�ر�ע��
    virtual void notifyRead(int errCode);
    virtual void notifyWrite(int bytes, int errCode);

    static FileNotifier kDummyNotifier;
};

class QKRTLAPI FileObject : public Object {
public:
    FileObject();
    virtual ~FileObject();

    /**
        2024-07-19
        ����open�ӿڡ��������ⲿ����������Ҳ��ͬ��
    */
    virtual void close();
    virtual bool start();
    virtual bool startRead();
    virtual bool startWrite();
    virtual int read(qkrtl::Buffer& buffer);
    virtual int write(qkrtl::Buffer& buffer);

    virtual void notifyOpen(int errCode);
    virtual void notifyClose(int errCode);
    virtual void notifyRead(int errCode);
    virtual void notifyWrite(int bytes, int errCode);

    virtual const OsHandle getOsHandle() const;

    inline int getHandle() const { return identifier(); }
    inline void setHandle(int handle) { identifier_ = handle; }
    inline time_t freeTime() const { return freeTime_; }
    inline int fsType() const { return fsType_; }
    void setNotifier(FileNotifier* notifier);

    static const int kFstNone = 0;
protected:
    friend class FileSystem;
    FileNotifier* notifier_;
    time_t freeTime_;
    int fsType_;
};

class QKRTLAPI FileSystemImpl {
public:
    FileSystemImpl();
    virtual ~FileSystemImpl();

    virtual void final();

    virtual FileObject* create(const Url& url, FileNotifier* notifier = NULL);
    virtual FileObject* open(const Url& url, FileNotifier* notifier = NULL);
};

/**
    ֻ����������;�������漰���������
    2024-07-01
    ȡ��singleton�����ⲿʵ�֡�
*/
class FileSystem {
public:
    QKRTLAPI FileSystem();
    QKRTLAPI virtual ~FileSystem();

    QKRTLAPI bool init(int maxSize);
    QKRTLAPI void final();

    QKRTLAPI int create(const std::string& url, FileNotifier* notifier = NULL);
    QKRTLAPI int create(const Url& url, FileNotifier* notifier = NULL);
    QKRTLAPI int open(const std::string& url, FileNotifier* notifier = NULL);
    QKRTLAPI int open(const Url& url, FileNotifier* notifier = NULL);
    QKRTLAPI bool setNotifier(int handle, FileNotifier* notifier = NULL);

    QKRTLAPI void close(int handle);
    //����handle��ָ���FileObject��read/write����
    QKRTLAPI int read(int handle, Buffer& buffer);
    QKRTLAPI int write(int handle, Buffer& buffer);

    QKRTLAPI bool start(int handle);
    QKRTLAPI bool startRead(int handle);
    QKRTLAPI bool startWrite(int handle);


    QKRTLAPI int insert(FileObject* obj);
    QKRTLAPI bool remove(FileObject* obj);
    QKRTLAPI bool remove(int handle, FileObject*& obj);
    QKRTLAPI bool exchange(int handle, FileObject * obj , FileObject*& old);

    QKRTLAPI FileObject* find(int handle);
    QKRTLAPI const FileObject* find(int handle) const;

    QKRTLAPI bool mount(const std::string& protocol, FileSystemImpl* impl);
    QKRTLAPI int listMounts(std::list<std::string>& mounts) const;

    inline bool valid(int handle) const { return objects_.valid(handle); }
    inline int size() const { return objects_.size(); }
    inline int maxSize() const { return objects_.maxSize(); }

    QKRTLAPI static FileSystem& singleton();

    static const int kRemoveExpired = 3;    //����3���ɾ��
    static const int kMaxHandleSize = 1 << 16;
    static const int kInvalidIndex = Object::kInvalidIdentifier;
private:
    mutable std::mutex guard_;
    bool finaled_;
    std::map<std::string, FileSystemImpl*> fsImpls_;
    std::list<FileObject*> removeds_;

    std::thread worker_;
    void process();
    ObjectManager objects_;
};

class QKRTLAPI FileSystemMounter {
public:
    explicit FileSystemMounter(const std::string& protocol, FileSystemImpl* impl);
};

}

#ifdef __cplusplus
extern "C" {
#endif

QKRTLAPI bool FsInit(int maxSize);
QKRTLAPI void FsFinal();

QKRTLAPI int FsCreate(const std::string& url , qkrtl::FileNotifier * notifier = NULL);
QKRTLAPI int FsOpen(const std::string& url, qkrtl::FileNotifier* notifier = NULL);
QKRTLAPI bool FsNotifier(int handle, qkrtl::FileNotifier* notifier = NULL);
QKRTLAPI void FsClose(int handle);
QKRTLAPI bool FsValid(int handle);

QKRTLAPI int FsInsert(qkrtl::FileObject* obj);
QKRTLAPI bool FsRemove(int handle, qkrtl::FileObject*& obj);
QKRTLAPI bool FsExchange(int handle, qkrtl::FileObject* obj, qkrtl::FileObject*& old);

QKRTLAPI int FsRead(int handle , qkrtl::Buffer & buffer);
QKRTLAPI int FsWrite(int handle, qkrtl::Buffer& buffer);
QKRTLAPI bool FsStart(int handle);
QKRTLAPI bool FsStartRead(int handle);
QKRTLAPI bool FsStartWrite(int handle);

QKRTLAPI qkrtl::FileObject* FsFind(int handle);
QKRTLAPI const OsHandle FsGetOsHandle(int handle);

QKRTLAPI int FsHandleSize();

const int kFsInvalidIndex = qkrtl::FileSystem::kInvalidIndex;

#ifdef __cplusplus
}
#endif

#endif /**QKRTL_FILE_SYSTEM_H*/
