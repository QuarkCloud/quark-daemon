
#ifndef QKINTH_FILE_SYSTEM_H
#define QKINTH_FILE_SYSTEM_H 1

#include <time.h>
#include <string>
#include <map>
#include <list>
#include <mutex>
#include <thread>

#include "qkinth/Compile.h"
#include "qkinth/Object.h"

namespace qkinth {

class FileSystem {
public:
    QKINTHAPI FileSystem();
    QKINTHAPI virtual ~FileSystem();

    QKINTHAPI bool init(int maxSize);
    QKINTHAPI void final();

    QKINTHAPI int create(const std::string& name);
    QKINTHAPI int open(const std::string& name);
    QKINTHAPI void close(int handle);

    QKINTHAPI int read(int handle, char* buffer, int size);
    QKINTHAPI int write(int handle, const char* buffer, int size);

    QKINTHAPI Object * find(int handle);
    QKINTHAPI const Object* find(int handle) const;

    inline bool valid(int handle) const { return objects_.valid(handle); }

    QKINTHAPI static FileSystem& singleton();
    static const int kMaxHandleSize = 1 << 10;
    static const int kRemoveExpired = 3;    //³¬¹ý3Ãë²ÅÉ¾³ý

private:
    std::mutex guard_;
    bool finaled_;
    std::map<std::string, int> names_;
    qkrtl::ObjectManager objects_;
    std::list<Object*> removeds_;

    std::thread worker_;
    void process();
};

}

#endif /**QKINTH_FILE_SYSTEM_H*/
