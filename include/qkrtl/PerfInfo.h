
#ifndef QKRTL_PERF_INFO_H
#define QKRTL_PERF_INFO_H 1

#include "qkrtl/Compile.h"
#include <string>
#include <atomic>
#include <map>

namespace qkrtl {

QKRTLAPI int64_t HrCounter();
QKRTLAPI double CounterToS(int64_t counter);
QKRTLAPI double CounterToMS(int64_t counter);
QKRTLAPI double CounterToUS(int64_t counter);
QKRTLAPI double CounterToNS(int64_t counter);

QKRTLAPI std::string BytesToUnit(int64_t bytes);
QKRTLAPI std::string CalcThroughout(int64_t bytes, int64_t counter);
QKRTLAPI std::string CalcLatency(int64_t hrCounter, int64_t times);
QKRTLAPI std::string CalcCounter(int64_t hrCounter);


struct TimeStat {
    int64_t startTime;
    int64_t endTime;
};

class QKRTLAPI TimeElapse : public TimeStat {
public:
    TimeElapse();
    TimeElapse(int64_t st, int64_t et);
    TimeElapse(const TimeElapse& te);
    TimeElapse& operator=(const TimeElapse& te);

    void start();
    void stop();
    int64_t elapse() const;
};

class TimeStatManager {
public:
    QKRTLAPI TimeStatManager();
    QKRTLAPI explicit TimeStatManager(size_t cap);
    QKRTLAPI virtual ~TimeStatManager();

    QKRTLAPI bool init(size_t cap);
    QKRTLAPI void final();

    QKRTLAPI bool append(const TimeStat& item);
    QKRTLAPI bool append(int64_t startTime, int64_t endTime);

    QKRTLAPI bool calc(int64_t& totalValue, int64_t& minValue, int64_t& maxValue, int64_t& avgValue) const;

    /**
        参考wrk的stats，输出统计结果。
        1、avgValue:平均数
        2、stdev:基于平均数的标准差
        3、within stdev：标准差范围内的比例
        4、percentile：百分数的比例
    */
    QKRTLAPI double calcAvgValue() const;
    QKRTLAPI double calcStdev(double avgValue) const;
    QKRTLAPI double calcWithinStdev(double avgValue, double stdev, int64_t value) const;
    QKRTLAPI double calcPercentile(double percent) const;
protected:
    TimeStat* times_;
    size_t    capacity_;
    std::atomic<size_t>    index_;
    size_t calcMaxIndex() const;
    //统计每个数值的数量
    void calcCounters(size_t maxIndex, std::map<int64_t, uint64_t>& counters) const;
};

}

#endif /**QKRTL_PERF_INFO_H*/
