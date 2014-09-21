// **********************************************************************
//
// Copyright (c) 2010-2014 Bernard Luo. All rights reserved.
//
// <Email: luo (dot) xiaowei (at) hotmail (dot) com>
//
// **********************************************************************

#ifdef _WIN32
#   include <sys/timeb.h>
#   include <time.h>
#else
#   include <sys/time.h>
#endif

#ifdef __APPLE__
#   include <mach/mach.h>
#   include <mach/mach_time.h>
#endif

#include <iostream>
#include <iomanip>
#include <Util/Time.h>
#include <Util/Exception.h>
#include <Build/UsefulMacros.h>

using namespace Util;

#ifdef _WIN32

namespace
{

static double frequency = -1.0;

//
// Initialize the frequency
//
class InitializeFrequency
{
public:

    InitializeFrequency()
    {
        //
        // Get the frequency of performance counters. We also make a call to
        // QueryPerformanceCounter to ensure it works. If it fails or if the
        // call to QueryPerformanceFrequency fails, the frequency will remain
        // set to -1.0 and ftime will be used instead.
        //
        Int64 v;
        if (QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&v)))		// 获取高精确度性能计数器的值
        {
            if (QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&v)))		// 获得机器内部定时器的时钟频率
            {
                frequency = static_cast<double>(v);
            }
        }
    }
};
static InitializeFrequency frequencyInitializer;

}
#endif

#ifdef __APPLE__
namespace
{

double machMultiplier = 1.0;
class InitializeTime
{
public:

    InitializeTime()
    {
        mach_timebase_info_data_t initTimeBase = { 0, 0 };
        mach_timebase_info(&initTimeBase);
        machMultiplier = static_cast<double>(initTimeBase.numer) / initTimeBase.denom / UTIL_INT64(1000);
    }
};
InitializeTime initializeTime;

}
#endif

Util::Time::Time() throw() :
    m_microsec(0)
{
}

Util::Time::Time(int year, int month, int day, int hour, int min, int sec, int DST)
{
#pragma warning (push)
#pragma warning (disable: 4127)  // conditional expression constant

	ENSURE_THROW(year >= 1900, Util::IllegalArgumentException);
	ENSURE_THROW(month >= 1 && month <= 12, Util::IllegalArgumentException);
	ENSURE_THROW(day >= 1 && day <= 31, Util::IllegalArgumentException);
	ENSURE_THROW(hour >= 0 && hour <= 23, Util::IllegalArgumentException);
	ENSURE_THROW(min >= 0 && min <= 59, Util::IllegalArgumentException);
	ENSURE_THROW(sec >= 0 && sec <= 59, Util::IllegalArgumentException);

#pragma warning (pop)

	struct tm atm;

	atm.tm_sec = sec;
	atm.tm_min = min;
	atm.tm_hour = hour;
	atm.tm_mday = day;
	atm.tm_mon = month - 1;        // tm_mon is 0 based
	atm.tm_year = year - 1900;     // tm_year is 1900 based
	atm.tm_isdst = DST;

	__time64_t time = _mktime64(&atm);
	assert(-1 != time);       // indicates an illegal input time
	if(-1 == time)
	{
		throw Util::IllegalArgumentException(__FILE__, __LINE__);
	}

	m_microsec = static_cast<Int64>(time) * UTIL_INT64(1000000);
}

//Time 
//Util::Time::GetCurrentTime() throw()
//{
//	return (Time(::_time64(NULL) * UTIL_INT64(1000000)));
//}

Time
Util::Time::Now(Clock clock)
{
    if (clock == Realtime)
    {
#ifdef _WIN32
#  if defined(_MSC_VER)
        struct _timeb tb;
        _ftime(&tb);
		//struct __timeb64 tb;
		//_ftime64(&tb);
#  elif defined(__MINGW32__)
        struct timeb tb;
        ftime(&tb);
#  endif
        return Time(static_cast<Int64>(tb.time) * UTIL_INT64(1000000) + tb.millitm * 1000);
#else
        struct timeval tv;
        if (gettimeofday(&tv, 0) < 0)
        {
            assert(0);
            throw SyscallException(__FILE__, __LINE__, errno);
        }
        return Time(tv.tv_sec * UTIL_INT64(1000000) + tv.tv_usec);
#endif
    }
    else // Monotonic
    {
#if defined(_WIN32)
        if (frequency > 0.0)
        {
            Int64 count;
            if (!QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&count)))
            {
                assert(0);
                throw SyscallException(__FILE__, __LINE__, GetLastError());
            }
            return Time(static_cast<Int64>(count / frequency * 1000000.0));
        }
        else
        {
#  if defined(_MSC_VER)
            struct _timeb tb;
            _ftime(&tb);
#  elif defined(__MINGW32__)
            struct timeb tb;
            ftime(&tb);
#  endif
            return Time(static_cast<Int64>(tb.time) * UTIL_INT64(1000000) + tb.millitm * 1000);
        }
#elif defined(__hpux)
        //
        // HP does not support CLOCK_MONOTONIC
        //
        struct timeval tv;
        if (gettimeofday(&tv, 0) < 0)
        {
            assert(0);
            throw SyscallException(__FILE__, __LINE__, errno);
        }
        return Time(tv.tv_sec * UTIL_INT64(1000000) + tv.tv_usec);
#elif defined(__APPLE__)
       return Time(mach_absolute_time() * machMultiplier);
#else
        struct timespec ts;
        if (clock_gettime(CLOCK_MONOTONIC, &ts) < 0)
        {
            assert(0);
            throw SyscallException(__FILE__, __LINE__, errno);
        }
        return Time(ts.tv_sec * UTIL_INT64(1000000) + ts.tv_nsec / UTIL_INT64(1000));
#endif
    }
}

Time 
Time::TimeOfToday(size_t hour, size_t minute, size_t second)
{
	assert(minute < 60 && second < 60);

	Time base(Now(Realtime) + Hours(24) * ((int)hour / 24));
	time_t time = static_cast<long>(base.ToMicroSeconds() / 1000000);

	struct tm* t;
#ifdef _WIN32
	t = localtime(&time);
#else
	struct tm tr;
	localtime_r(&time, &tr);
	t = &tr;
#endif
	t->tm_hour = hour % 24;
	t->tm_min = minute;
	t->tm_sec = second;

	return Seconds(mktime(t));
}

//Time
//Util::Time::HourOfDay(size_t hour)
//{
//	Time base(Now(Realtime) + Hours(24) * ((int)hour / 24));
//	time_t time = static_cast<long>(base.ToMicroSeconds() / 1000000);
//
//	struct tm* t;
//#ifdef _WIN32
//	t = localtime(&time);
//#else
//	struct tm tr;
//	localtime_r(&time, &tr);
//	t = &tr;
//#endif
//	t->tm_hour = hour % 24;
//	t->tm_min = 0;
//	t->tm_sec = 0;
//
//	return Seconds(mktime(t));
//}

Time
Util::Time::Hours(size_t t)
{
	return Time(t * 3600 * UTIL_INT64(1000000));
}

Time
Util::Time::Minutes(size_t t)
{
	return Time(t * 60 * UTIL_INT64(1000000));
}

Time
Util::Time::Seconds(Int64 t)
{
    return Time(t * UTIL_INT64(1000000));
}

Time
Util::Time::MilliSeconds(Int64 t)
{
    return Time(t * UTIL_INT64(1000));
}

Time
Util::Time::MicroSeconds(Int64 t)
{
    return Time(t);
}

Time
Util::Time::SecondsDouble(double t)
{
    return Time(Int64(t * 1000000));
}

Time
Util::Time::MilliSecondsDouble(double t)
{
    return Time(Int64(t * 1000));
}

Time
Util::Time::MicroSecondsDouble(double t)
{
    return Time(Int64(t));
}

#ifndef _WIN32
Util::Time::operator timeval() const
{
    timeval tv;
    tv.tv_sec = static_cast<long>(m_microsec / 1000000);
    tv.tv_usec = static_cast<long>(m_microsec % 1000000);
    return tv;
}
#endif

Util::Time::operator tm() const
{
	time_t time = static_cast<long>(m_microsec / 1000000);

	struct tm* t;
#ifdef _WIN32
	t = localtime(&time);
#else
	struct tm tr;
	localtime_r(&time, &tr);
	t = &tr;
#endif

	return *t;
}

Int64
Util::Time::ToSeconds() const
{
    return m_microsec / 1000000;
}

Int64
Util::Time::ToMilliSeconds() const
{
    return m_microsec / 1000;
}

Int64
Util::Time::ToMicroSeconds() const
{
    return m_microsec;
}

double
Util::Time::ToSecondsDouble() const
{
    return m_microsec / 1000000.0;
}

double
Util::Time::ToMilliSecondsDouble() const
{
    return m_microsec / 1000.0;
}

double
Util::Time::ToMicroSecondsDouble() const
{
    return static_cast<double>(m_microsec);
}

std::string
Util::Time::ToDateTime(Clock clock) const
{
    time_t time = static_cast<long>(m_microsec / 1000000);

    struct tm* t;
#ifdef _WIN32
    t = localtime(&time);
#else
    struct tm tr;
    localtime_r(&time, &tr);
    t = &tr;
#endif

    char buf[32];
	if (Realtime == clock)
	{
		strftime(buf, sizeof(buf), "%x %H:%M:%S", t);
	}
	else
	{
		strftime(buf, sizeof(buf), "%H:%M:%S", t);
	}

    std::ostringstream os;
    os << buf << ".";
    os.fill('0');
    os.width(3);
    os << static_cast<long>(m_microsec % 1000000 / 1000);
    return os.str();
}

std::string
Util::Time::ToDuration() const
{
    Int64 usecs = m_microsec % 1000000;
    Int64 secs = m_microsec / 1000000 % 60;
    Int64 mins = m_microsec / 1000000 / 60 % 60;
    Int64 hours = m_microsec / 1000000 / 60 / 60 % 24;
    Int64 days = m_microsec / 1000000 / 60 / 60 / 24;

    using namespace std;

    ostringstream os;
    if (days != 0)
    {
        os << days << "d ";
    }
    os << setfill('0') << setw(2) << hours << ":" << setw(2) << mins << ":" << setw(2) << secs;
    if (usecs != 0)
    {
        os << "." << setw(3) << (usecs / 1000);
    }

    return os.str();
}

struct tm* 
Util::Time::GetGmtTime(struct tm* ptm) const
{
	// Ensure ptm is valid
	ENSURE_THROW(0 != ptm, Util::IllegalArgumentException);

	if (0 != ptm)
	{
		__time64_t time = m_microsec / 1000000;
		struct tm tmTemp;
		errno_t err = _gmtime64_s(&tmTemp, &time);

		// Be sure the call succeeded
		if(0 != err) 
		{
			return 0; 
		}

		*ptm = tmTemp;
		return ptm;
	}

	return 0;
}

struct tm* 
Util::Time::GetLocalTime(struct tm* ptm) const
{
	// Ensure ptm is valid
	ENSURE_THROW(0 != ptm, Util::IllegalArgumentException);

	if (0 != ptm)
	{
		__time64_t time = m_microsec / 1000000;
		struct tm tmTemp;
		errno_t err = _localtime64_s(&tmTemp, &time);

		if(0 != err) 
		{
			return NULL;    // indicates that m_time was not initialized!
		}

		*ptm = tmTemp;
		return ptm;
	}

	return NULL;
}

#ifdef _WIN32

bool 
Util::Time::GetAsSystemTime(SYSTEMTIME& timeDest) const throw()
{
	struct tm ttm;
	struct tm* ptm;

	ptm = GetLocalTime(&ttm);

	if(!ptm) 
	{
		return false; 
	}

	timeDest.wYear = (WORD) (1900 + ptm->tm_year);
	timeDest.wMonth = (WORD) (1 + ptm->tm_mon);
	timeDest.wDayOfWeek = (WORD) ptm->tm_wday;
	timeDest.wDay = (WORD) ptm->tm_mday;
	timeDest.wHour = (WORD) ptm->tm_hour;
	timeDest.wMinute = (WORD) ptm->tm_min;
	timeDest.wSecond = (WORD) ptm->tm_sec;
	timeDest.wMilliseconds = 0;

	return true;
}

#endif

__time64_t 
Util::Time::GetTime() const throw()
{
	return(m_microsec / 1000000);
}

int 
Util::Time::GetYear() const
{ 
	struct tm ttm;
	struct tm * ptm;

	ptm = GetLocalTime(&ttm);
	return ptm ? (ptm->tm_year) + 1900 : 0 ; 
}

int 
Util::Time::GetMonth() const
{ 
	struct tm ttm;
	struct tm * ptm;

	ptm = GetLocalTime(&ttm);
	return ptm ? ptm->tm_mon + 1 : 0;
}

int 
Util::Time::GetDay() const
{
	struct tm ttm;
	struct tm * ptm;

	ptm = GetLocalTime(&ttm);
	return ptm ? ptm->tm_mday : 0 ; 
}

int 
Util::Time::GetHour() const
{
	struct tm ttm;
	struct tm * ptm;

	ptm = GetLocalTime(&ttm);
	return ptm ? ptm->tm_hour : -1 ; 
}

int 
Util::Time::GetMinute() const
{
	struct tm ttm;
	struct tm * ptm;

	ptm = GetLocalTime(&ttm);
	return ptm ? ptm->tm_min : -1 ; 
}

int 
Util::Time::GetSecond() const
{ 
	struct tm ttm;
	struct tm * ptm;

	ptm = GetLocalTime(&ttm);
	return ptm ? ptm->tm_sec : -1 ;
}

int 
Util::Time::GetDayOfWeek() const
{ 
	struct tm ttm;
	struct tm * ptm;

	ptm = GetLocalTime(&ttm);
	return ptm ? ptm->tm_wday + 1 : 0 ;
}

Time::Time(Int64 usec) :
    m_microsec(usec)
{
}

std::ostream&
Util::operator <<(std::ostream& out, const Time& tm)
{
    return out << tm.ToMicroSeconds() / 1000000.0;
}
