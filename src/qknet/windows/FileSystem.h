
#ifndef QKNET_WINDOWS_FILE_SYSTEM_H
#define QKNET_WINDOWS_FILE_SYSTEM_H 1

#include <string>
#include <map>
#include <mutex>

#include "qkrtl/Buffer.h"
#include "qkrtl/FileSystem.h"
#include "qkrtl/Url.h"
#include "qknet/Compile.h"
#include "qknet/Address.h"

namespace qknet {
namespace windows {

/**
    2024-07-06
    将socket对应于FileObject，做为内部对象管理。外部统一以整数提供唯一索引。
*/

class NetFileSystem : public qkrtl::FileSystemImpl {
public:
    NetFileSystem();
    virtual ~NetFileSystem();

    virtual void final();

    virtual qkrtl::FileObject* create(const qkrtl::Url& url, qkrtl::FileNotifier* notifier = NULL);
    virtual qkrtl::FileObject* open(const qkrtl::Url& url, qkrtl::FileNotifier* notifier = NULL);

    static const std::string kTcpProtocol;
    static const std::string kUdpProtocol;
private:
    std::mutex guard_;
    bool finaled_;
    std::map<std::string, int> urls_;
};

}
}
#endif /**QKNET_FILE_SYSTEM_H*/
