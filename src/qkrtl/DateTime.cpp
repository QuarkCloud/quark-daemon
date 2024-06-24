
#include "qkrtl/DateTime.h"

namespace qkrtl {

int64_t UTimeNow()
{
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);

    /*
        2011-03-08
        参考BOOST的microsec_time_clock.hpp	将FILETIME转化为毫秒

        shift is difference between 1970-Jan-01 & 1601-Jan-01
        in 100-nanosecond intervals
    */
    const uint64_t shift = 116444736000000000ULL; // (27111902 << 32) + 3577643008
    uint64_t caster = ft.dwHighDateTime;
    caster = ((caster << 32) + ft.dwLowDateTime - shift) / 10;
    
    return (int64_t)caster;
}

time_t MakeTime(int year, int month, int day, int hour, int minute, int second)
{
    struct tm tm;
    memset(&tm, 0, sizeof(tm));

    tm.tm_year = year - 1900;
    tm.tm_mon = month - 1;
    tm.tm_mday = day;

    tm.tm_hour = hour;
    tm.tm_min = minute;
    tm.tm_sec = second;

    return ::mktime(&tm);
}
bool LocalTime(time_t ts, int& year, int& month, int& day, int& hour, int& minute, int& second)
{
    struct tm tm;

    if (::localtime_s(&tm, &ts) != 0)
        return false;

    year = tm.tm_year + 1900;
    month = tm.tm_mon + 1;
    day = tm.tm_mday;

    hour = tm.tm_hour;
    minute = tm.tm_min;
    second = tm.tm_sec;
    return true;
}
bool GmTime(time_t ts, int& year, int& month, int& day, int& hour, int& minute, int& second)
{
    struct tm tm;

    if (::gmtime_s(&tm, &ts) != 0)
        return false;

    year = tm.tm_year + 1900;
    month = tm.tm_mon + 1;
    day = tm.tm_mday;

    hour = tm.tm_hour;
    minute = tm.tm_min;
    second = tm.tm_sec;
    return true;
}
Date::Date():time_(0) , year_(0) , month_(0) , day_(0)
{
    //
}
Date::Date(const Date& date):time_(0) ,year_(date.year_) , month_(date.month_) , day_(date.day_)
{
    time_ = MakeTime(year_, month_, day_);
}
Date::Date(time_t ts) :time_(0), year_(0), month_(0), day_(0)
{
    int hour = 0, minute = 0, second = 0;
    LocalTime(ts, year_, month_, day_, hour, minute, second);
}
Date::Date(int year, int month, int day) :time_(0), year_(year), month_(month), day_(day)
{
    time_ = MakeTime(year_, month_, day_);
}
Date::~Date()
{
    //
}
void Date::reset()
{
    time_ = 0;
    year_ = month_ = day_ = 0;
}
void Date::calcNow(time_t ts)
{
    if (ts == 0)
        ts = ::time(NULL);
    int hour = 0, minute = 0, second = 0;
    if (LocalTime(ts, year_, month_, day_, hour, minute, second) == false)
    {
        reset();
    }
    else
    {
        time_ = MakeTime(year_, month_, day_);
    }
}
Date& Date::operator=(time_t ts)
{
    calcNow(ts);
    return (*this);
}
bool Date::operator==(time_t ts)
{
    Date date(ts);
    return ((*this) == date);
}
bool Date::operator!=(time_t ts)
{
    Date date(ts);
    return ((*this) != date);
}
bool Date::operator==(const Date& d)
{
    return (time_ == d.time_);
}
bool Date::operator!=(const Date& d)
{
    return (time_ != d.time_);
}

bool Date::operator<(const Date& d)
{
    return (time_ < d.time_);
}
bool Date::operator<=(const Date& d)
{
    return (time_ <= d.time_);
}
bool Date::operator>(const Date& d)
{
    return (time_ > d.time_);
}
bool Date::operator>=(const Date& d)
{
    return (time_ >= d.time_);
}

std::string Date::toString() const
{
    char str[256] = { '\0' };
    ::snprintf(str, sizeof(str), "%04d%02d%02d", year_, month_, day_);
    return str;
}
time_t Date::time()
{
    return time_;
}
time_t Date::gmttime()
{
    return (time_ - (Date::timezone() * 3600));
}
Date& Date::nextDay(int dc)
{
    time_t t = time_ + (86400 * dc);
    int hour = 0, minute = 0, second = 0;
    if (LocalTime(t, year_, month_, day_ , hour , minute , second) == true)
    {
        time_ = MakeTime(year_, month_, day_);
    }
    else
    {
        reset();
    }
    return (*this);
}
Date& Date::today()
{
    static Date __today__;
    static time_t __lastTime__ = __today__.time();

    time_t now = ::time(NULL);
    if (now - __lastTime__ >= 86400)
    {
        __today__.calcNow();
        __lastTime__ = __today__.time();
    }

    return __today__;
}
int Date::timezone()
{
    static int __timeZone__ = 0;
    static bool __timeZoneInited__ = false;

    if (__timeZoneInited__ == false)
    {
        time_t now = ::time(NULL);
        struct tm ltm, gtm;

        localtime_s(&ltm , &now);
        gmtime_s(&gtm , &now );


        __timeZone__ = (ltm.tm_hour + 24 - gtm.tm_hour) % 24;
        __timeZoneInited__ = true;
    }

    return __timeZone__;
}

DateTime::DateTime():
    time_(0) , year_(0) , month_(0) , day_(0) ,
    hour_(0) , minute_(0) , second_(0) , usec_(0)
{
    //
}
DateTime::DateTime(const DateTime& dt):
    time_(dt.time_), year_(dt.year_), month_(dt.month_), day_(dt.day_),
    hour_(dt.hour_), minute_(dt.minute_), second_(dt.second_), usec_(dt.usec_)
{
    //
}
DateTime::DateTime(int64_t ts) :
    time_(0), year_(0), month_(0), day_(0),
    hour_(0), minute_(0), second_(0), usec_(0)
{
    calcNow(ts);
}
DateTime::~DateTime()
{
    //
}
void DateTime::reset()
{
    time_ = 0;
    year_ = month_ = day_ = 0;
    hour_ = minute_ = second_ = usec_ = 0;
}
void DateTime::calcNow(int64_t ts)
{
    time_t sec = (time_t)(ts / 1000000LL);
    int64_t usec = ts % 1000000LL;

    if (LocalTime(sec, year_, month_, day_, hour_, minute_, second_) == false)
    {
        reset();
        return;
    }

    usec_ = (int)usec;
    time_ = ts;
}
DateTime& DateTime::operator=(int64_t ts)
{
    calcNow(ts);
    return (*this);
}
std::string DateTime::toString(bool hasDate) const
{
    char  str[256] = { '\0' };
    if (hasDate == true)
    {
        ::snprintf(str, sizeof(str), "%04d%02d%02d %02d:%02d:%02d.%06d",
            year_, month_, day_, hour_, minute_, second_, usec_);
    }
    else
    {
        ::snprintf(str, sizeof(str), "%02d:%02d:%02d.%06d",hour_, minute_, second_, usec_);
    }
    return str;
}
}
