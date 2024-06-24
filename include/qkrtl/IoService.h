
#ifndef QKRTL_IO_SERVICE_H
#define QKRTL_IO_SERVICE_H 1

#include <thread>
#include <mutex>
#include <vector>
#include "qkrtl/Compile.h"
#include "qkrtl/Poller.h"
#include "qkrtl/StateMonitor.h"

namespace qkrtl {

/**
    调整架构，每个IoService只支持单个线程，避免多线程之间互锁，提高性能。
*/
class IoService : public Poller{
public:
    QKRTLAPI IoService(const std::string& name = "");
    QKRTLAPI virtual ~IoService();

    QKRTLAPI void final();
    QKRTLAPI void run(bool standalone = true);

private:
    std::mutex guard_;
    bool finaled_;
    std::thread worker_;
    void process();
    CounterMonitor counter_;
};
}
#endif /**QKRTL_IO_SERVICE_H*/
