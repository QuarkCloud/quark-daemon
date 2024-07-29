
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
    ����һ�����ڴ�ģ������ڲ���ģ��tcp������ļ�ϵͳ�����������ڲ����������ָ�ꡣ
    IPC: Inter Process Communication���͸��������Ӧ�����ǽ��������ڲ���ͨѶ��ʽ
    ITC: Inter Thread Communication��
    �������ڽ����ڲ�������û������socket�����Ľӿڣ��������ӽ���Pipe������Name�����ҷ����
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
