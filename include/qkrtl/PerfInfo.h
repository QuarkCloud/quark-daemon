
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
        �ο�wrk��stats�����ͳ�ƽ����
        1��avgValue:ƽ����
        2��stdev:����ƽ�����ı�׼��
        3��within stdev����׼�Χ�ڵı���
        4��percentile���ٷ����ı���
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
    //ͳ��ÿ����ֵ������
    void calcCounters(size_t maxIndex, std::map<int64_t, uint64_t>& counters) const;
};

}

#endif /**QKRTL_PERF_INFO_H*/
