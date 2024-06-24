
#ifndef QKRTL_DATE_TIME_H
#define QKRTL_DATE_TIME_H 1

#include "qkrtl/Compile.h"
#include <time.h>
#include <string>

namespace qkrtl {

QKRTLAPI int64_t UTimeNow();
QKRTLAPI time_t MakeTime(int year, int month, int day, int hour = 0, int minute = 0, int second = 0);
QKRTLAPI bool LocalTime(time_t ts , int& year, int& month, int& day, int& hour, int& minute, int& second);
QKRTLAPI bool GmTime(time_t ts, int& year, int& month, int& day, int& hour, int& minute, int& second);

class QKRTLAPI Date {
public:
    Date();
    Date(const Date& date);
    Date(time_t ts);
    Date(int year , int month , int day);
    virtual ~Date();

    void reset();
    void calcNow(time_t ts = 0);

    Date& operator=(time_t ts);
    bool operator==(time_t ts);
    bool operator!=(time_t ts);

    bool operator==(const Date& d);
    bool operator!=(const Date& d);

    bool operator<(const Date& d);
    bool operator<=(const Date& d);
    bool operator>(const Date& d);
    bool operator>=(const Date& d);


    std::string toString() const;
    time_t time();
    time_t gmttime();

    inline int year() const { return year_; }
    inline int month() const { return month_; }
    inline int day() const { return day_; }

    Date& nextDay(int dc = 1);

    static Date& today();
    static int timezone();

private:
    time_t time_;   //当地时间00:00:00的秒数
    int year_;
    int month_;
    int day_;
};

//精度微秒
class QKRTLAPI DateTime {
public:
    DateTime();
    DateTime(const DateTime& dt);
    DateTime(int64_t ts);
    virtual ~DateTime();

    void reset();
    void calcNow(int64_t ts = 0);
    DateTime& operator=(int64_t ts);
    std::string toString(bool hasDate = false) const;

    inline int year() const { return year_; }
    inline int month() const { return month_; }
    inline int day() const { return day_; }
    inline int hour() const { return hour_; }
    inline int minute() const { return minute_; }
    inline int second() const { return second_; }
    inline int usec() const { return usec_; }

private:
    int64_t time_;   
    int year_;
    int month_;
    int day_;
    int hour_;
    int minute_;
    int second_;
    int usec_;
};
}
#endif /**QKRTL_DATE_TIME_H*/
