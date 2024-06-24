
#ifndef QKRTL_SYSTEM_INFO_H
#define QKRTL_SYSTEM_INFO_H 1

#include "qkrtl/Compile.h"

namespace qkrtl {

class QKRTLAPI SystemInfo {
public:
    SystemInfo();
    virtual ~SystemInfo();
    void load();

    inline uint32_t numberOfProcessors() const { return numberOfProcessors_; }
    inline uint32_t pageSize() const { return pageSize_; }
    inline uint32_t allocationGranularity() const { return allocationGranularity_; }
    inline uint32_t cacheLine() const { return cacheLine_; }
    inline uint64_t processorMask() const { return processorMask_; }

    static const uint32_t kCacheLine = 64;
    static SystemInfo& singleton();
private:
    uint32_t numberOfProcessors_;
    uint32_t pageSize_;
    uint32_t allocationGranularity_;
    uint32_t cacheLine_;
    uint64_t processorMask_;
};



}
#endif /**QKRTL_SYSTEM_INFO_H*/
