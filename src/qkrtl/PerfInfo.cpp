#include "qkrtl/PerfInfo.h"
#include <string>
#include <math.h>

namespace qkrtl {

int64_t HrFrequency()
{
    static int64_t __PerformanceFrequency__ = 0;
    if (__PerformanceFrequency__ == 0)
    {
        LARGE_INTEGER li;
        if (::QueryPerformanceFrequency(&li) == FALSE)
            __PerformanceFrequency__ = 1;
        else
            __PerformanceFrequency__ = li.QuadPart;
    }
    return __PerformanceFrequency__;
}

int64_t HrCounter()
{
    LARGE_INTEGER li;
    if (::QueryPerformanceCounter(&li) == FALSE)
        return -1;
    return li.QuadPart;
}
double CounterToS(int64_t counter)
{
    double frequency = (double)HrFrequency();
    double value = (double)counter;
    return (value / frequency);
}
double CounterToMS(int64_t counter)
{
    double frequency = (double)HrFrequency();
    double value = (double)counter;
    return ((value / frequency) * 1000.0);
}
double CounterToUS(int64_t counter)
{
    double frequency = (double)HrFrequency();
    double value = (double)counter;
    return ((value / frequency) * 1000000.0);
}
double CounterToNS(int64_t counter)
{
    double frequency = (double)HrFrequency();
    double value = (double)counter;
    return ((value / frequency) * 1000000000.0);
}


static const int kMaxUnitSize = 4;
static const char* __units__[kMaxUnitSize] = { "Byte" , "KB" , "MB" , "GB" };
static const int64_t  __unitMaxBytes__[kMaxUnitSize] =
{ 1LL , (1LL << 10) , (1LL << 20) , (1LL << 30) };

std::string BytesToUnit(int64_t bytes)
{
    int stage = kMaxUnitSize - 1;
    for (; stage >= 0; --stage)
    {
        if (bytes >= __unitMaxBytes__[stage])
            break;
    }
    if (stage < 0)
        stage = 0;
    return __units__[stage];
}
std::string CalcThroughout(int64_t bytes, int64_t counter)
{
    double thr = ((double)bytes);
    double sec = CounterToS(counter);
    thr = thr / sec;
    int64_t speed = (int64_t)thr;

    int stage = kMaxUnitSize - 1;
    for (; stage >= 0; --stage)
    {
        if (speed >= __unitMaxBytes__[stage])
            break;
    }
    if (stage < 0)
        stage = 0;

    double unit = 1.0;
    if (stage > 0)
        unit = (double)__unitMaxBytes__[stage];

    thr = thr / unit;

    char str[256] = { '\0' };
    ::sprintf(str, "%0.3f%s", thr, __units__[stage]);

    return str;
}
std::string NanoToStr(double nsec)
{
    char str[256] = { '\0' };
    if (nsec >= 1000000000.0)
    {
        ::sprintf(str, "%0.3f sec", nsec / 1000000000.0);
    }
    else if (nsec >= 1000000.0)
    {
        ::sprintf(str, "%0.3f msec", nsec / 1000000.0);
    }
    else if (nsec >= 1000.0)
    {
        ::sprintf(str, "%0.3f usec", nsec / 1000.0);
    }
    else
    {
        ::sprintf(str, "%0.0f nsec", nsec);
    }
    return str;

}
std::string CalcLatency(int64_t hrCounter, int64_t times)
{
    double totalTime = CounterToNS(hrCounter);
    double ct = (double)times;
    double avg = (totalTime / ct);   //纳秒

    return NanoToStr(avg);
}
std::string CalcCounter(int64_t hrCounter)
{
    double nsec = CounterToNS(hrCounter);

    return NanoToStr(nsec);
}


TimeElapse::TimeElapse()
{
    startTime = 0;
    endTime = 0;
    start();
}
TimeElapse::TimeElapse(int64_t st, int64_t et)
{
    startTime = st;
    endTime = et;
}
TimeElapse::TimeElapse(const TimeElapse& te)
{
    startTime = te.startTime;
    endTime = te.endTime;
}
TimeElapse& TimeElapse::operator=(const TimeElapse& te)
{
    startTime = te.startTime;
    endTime = te.endTime;
    return (*this);
}
void TimeElapse::start()
{
    startTime = HrCounter();
}
void TimeElapse::stop()
{
    endTime = HrCounter();
}
int64_t TimeElapse::elapse() const
{
    if (endTime == 0)
    {
        return (HrCounter() - startTime);
    }
    else
    {
        return (endTime - startTime);
    }
}


TimeStatManager::TimeStatManager() :times_(NULL), capacity_(0), index_(0)
{
    //
}
TimeStatManager::TimeStatManager(size_t cap) : times_(NULL), capacity_(0), index_(0)
{
    init(cap);
}
TimeStatManager::~TimeStatManager()
{
    final();
}
bool TimeStatManager::init(size_t cap)
{
    if (times_ != NULL || cap == 0)
        return false;

    size_t nsize = sizeof(TimeStat) * cap;
    times_ = (TimeStat*)::malloc(nsize);
    if (times_ == NULL)
        return false;

    ::memset(times_, 0, nsize);
    capacity_ = cap;
    return true;
}
void TimeStatManager::final()
{
    if (times_ != NULL)
    {
        ::free(times_);
        times_ = NULL;
    }
    capacity_ = 0;
    index_ = 0;
}
bool TimeStatManager::append(const TimeStat& item)
{
    if (item.endTime < item.startTime)
        return false;

    if (index_.load(std::memory_order_relaxed) >= capacity_)
        return false;

    size_t index = index_.fetch_add(1, std::memory_order_release);
    if (index >= capacity_)
        return false;
    times_[index] = item;
    return true;
}
bool TimeStatManager::append(int64_t startTime, int64_t endTime)
{
    TimeStat item = { startTime , endTime };
    return append(item);
}
bool TimeStatManager::calc(int64_t& totalValue, int64_t& minValue,
    int64_t& maxValue, int64_t& avgValue) const
{
    size_t maxIndex = index_.load();
    if (maxIndex > capacity_)
        maxIndex = capacity_;
    int64_t tv = 0, minv = 0, maxv = 0;
    int counter = 0;
    for (size_t tidx = 0; tidx < maxIndex; ++tidx)
    {
        TimeStat& ts = times_[tidx];
        int64_t curValue = ts.endTime - ts.startTime;
        if (curValue < 0)
            continue;

        tv += curValue;
        if (minv == 0 || minv > curValue)
            minv = curValue;
        if (maxv == 0 || maxv < curValue)
            maxv = curValue;
        ++counter;
    }
    if (counter == 0)
        return false;

    totalValue = tv;
    minValue = minv;
    maxValue = maxv;
    avgValue = tv / counter;
    return true;
}
size_t TimeStatManager::calcMaxIndex() const
{
    size_t maxIndex = index_.load();
    if (maxIndex > capacity_)
        maxIndex = capacity_;
    return maxIndex;
}
double TimeStatManager::calcAvgValue() const
{
    size_t maxIndex = calcMaxIndex();
    int64_t totalValue = 0;
    int64_t counter = 0;
    for (size_t tidx = 0; tidx < maxIndex; ++tidx)
    {
        const TimeStat& ts = times_[tidx];
        int64_t curValue = ts.endTime - ts.startTime;
        if (curValue < 0)
            continue;

        totalValue += curValue;
        ++counter;
    }
    if (counter == 0)
        return 0.0;

    return ((double)totalValue) / ((double)counter);
}
void TimeStatManager::calcCounters(size_t maxIndex, std::map<int64_t, uint64_t>& counters) const
{
    for (size_t tidx = 0; tidx < maxIndex; ++tidx)
    {
        const TimeStat& ts = times_[tidx];
        int64_t curValue = ts.endTime - ts.startTime;
        if (curValue < 0)
            continue;

        std::map<int64_t, uint64_t>::iterator iter = counters.find(curValue);
        if (iter == counters.end())
            iter = counters.insert(std::pair<int64_t, uint64_t>(curValue, 0)).first;
        iter->second++;
    }
}
double TimeStatManager::calcStdev(double avgValue) const
{
    size_t maxIndex = calcMaxIndex();
    if (maxIndex < 2)
        return 0.0;

    //统计每个数值的数量
    std::map<int64_t, uint64_t> counters;
    calcCounters(maxIndex, counters);


    //计算标准差
    double sum = 0.0;
    for (std::map<int64_t, uint64_t>::const_iterator citer = counters.begin();
        citer != counters.end(); ++citer)
    {
        int64_t curValue = citer->first;
        uint64_t counter = citer->second;

        if (counter == 0)
            continue;

        sum += powl(curValue - avgValue, 2) * counter;
    }

    return sqrtl(sum / (maxIndex - 1));
}
double TimeStatManager::calcWithinStdev(double avgValue, double stdev, int64_t value) const
{
    size_t maxIndex = calcMaxIndex();

    double upper = avgValue + (stdev * value);
    double lower = avgValue - (stdev * value);

    uint64_t counter = 0;
    for (size_t tidx = 0; tidx < maxIndex; ++tidx)
    {
        const TimeStat& ts = times_[tidx];
        int64_t curValue = ts.endTime - ts.startTime;
        if (curValue < 0)
            continue;

        if (curValue >= lower && curValue <= upper)
        {
            ++counter;
        }
    }
    return ((double)counter / (double)maxIndex) * 100;
}
double TimeStatManager::calcPercentile(double percent) const
{
    size_t maxIndex = calcMaxIndex();
    std::map<int64_t, uint64_t> counters;
    calcCounters(maxIndex, counters);
    if (counters.empty() == true)
        return 0.0;

    uint64_t rank = (uint64_t)round((percent / 100.0) * maxIndex + 0.5);
    uint64_t counter = 0;
    for (std::map<int64_t, uint64_t>::const_iterator citer = counters.begin();
        citer != counters.end(); ++citer)
    {
        counter += citer->second;
        if (counter >= rank)
            return (double)citer->first;
    }

    return 0.0;
}
}