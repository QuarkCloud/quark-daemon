
#ifndef QKITC_FILE_SYSTEM_H
#define QKITC_FILE_SYSTEM_H 1

#include <time.h>
#include <string>
#include <map>
#include <list>
#include <mutex>
#include <thread>

#include "qkrtl/FileSystem.h"
#include "qkitc/Compile.h"

namespace qkitc {

/**
    这是一个纯内存的，进程内部，模拟tcp传输的文件系统，用于评估内部组件的性能指标。
    IPC: Inter Process Communication，和该类型相对应，我们建立进程内部的通讯方式
    ITC: Inter Thread Communication。
    由于是在进程内部，所以没有类似socket那样的接口，反而更接近于Pipe，根据Name来查找服务端
*/

class ItcFileSystem : public qkrtl::FileSystemImpl {
public:
    ItcFileSystem();
    virtual ~ItcFileSystem();

    virtual void final();

    virtual qkrtl::FileObject* create(const qkrtl::Url& url, qkrtl::FileNotifier* notifier = NULL);
    virtual qkrtl::FileObject* open(const qkrtl::Url& url, qkrtl::FileNotifier* notifier = NULL);

    static const std::string kProtocol;
private:
    std::mutex guard_;
    bool finaled_;
    std::map<std::string, int> urls_;
};

}

#endif /**QKITC_FILE_SYSTEM_H*/
