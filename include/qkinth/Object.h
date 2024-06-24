
#ifndef QKINTH_OBJECGT_H
#define QKINTH_OBJECGT_H 1

#include <time.h>
#include <string>
#include "qkrtl/RingBuffer.h"
#include "qkrtl/Buffer.h"
#include "qkrtl/Object.h"

#include "qkinth/Compile.h"
#include "qkinth/IoEvent.h"

namespace qkinth {

/**
    不实现和FileSystem之间的交互。用于管理对象本身的行为。
*/
class Object : public qkrtl::Object {
public:
    QKINTHAPI Object();
    QKINTHAPI virtual ~Object();

    QKINTHAPI int read(char* buffer, int size);

    QKINTHAPI void inEvent(int bytes);
    QKINTHAPI void outEvent(int bytes);

    inline int handle() const { return identifier(); }
    inline int peerHandle() const { return peerHandle_; }

    inline const std::string& name() const { return name_; }

    inline void inEvent(IoEvent* evt) { inEvent_ = evt; }
    inline void outEvent(IoEvent* evt) { outEvent_ = evt; }

    static const int kMaxBufferSize = 1 << 16;
private:
    friend class FileSystem;
    std::string name_;
    time_t freeTime_;
    int peerHandle_;
    qkrtl::SafeRingBuffer inBuffer_;
    IoEvent* inEvent_;
    IoEvent* outEvent_;
    int peerWrite(const char* buffer, int size);
    void doEvent(IoEvent* evt, int bytes);
};
}

#endif /**QKINTH_OBJECGT_H*/
