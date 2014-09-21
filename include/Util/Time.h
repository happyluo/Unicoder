// **********************************************************************
//
// Copyright (c) 2010-2014 Bernard Luo. All rights reserved.
//
// <Email: luo (dot) xiaowei (at) hotmail (dot) com>
//
// **********************************************************************

#ifndef UTIL_TIME_H
#define UTIL_TIME_H

#include <Util/Config.h>

#ifndef _WIN32
#   include <sys/time.h>
//#else
//#	include <winsock2.h>
#endif

namespace Util
{

class UTIL_API Time
{
public:
	Time(void) throw();
	Time(int year, int month, int day, int hour, int min, int sec, int DST = -1);
	//~Time(void);

    // No copy constructor and assignment operator necessary. The
    // automatically generated copy constructor and assignment
    // operator do the right thing.
    
	enum Clock { Realtime, Monotonic };
	//static Time GetCurrentTime() throw();
	static Time Now(Clock = Realtime);

	static Time TimeOfToday(size_t, size_t=0, size_t=0);
	//static Time HourOfDay(size_t hour);
	static Time Hours(size_t);
	static Time Minutes(size_t);
	static Time Seconds(Int64);
	static Time MilliSeconds(Int64);
	static Time MicroSeconds(Int64);

	static Time SecondsDouble(double);
	static Time MilliSecondsDouble(double);
	static Time MicroSecondsDouble(double);

#ifndef _WIN32
	operator timeval() const;
#endif

	operator tm() const;

	Int64 ToSeconds() const;
	Int64 ToMilliSeconds() const;
	Int64 ToMicroSeconds() const;

	double ToSecondsDouble() const;
	double ToMilliSecondsDouble() const;
	double ToMicroSecondsDouble() const;

	std::string ToDateTime(Clock = Realtime) const;
	std::string ToDuration() const;

	struct tm* GetGmtTime(struct tm* ptm) const;
	struct tm* GetLocalTime(struct tm* ptm) const;

#ifdef _WIN32
	bool GetAsSystemTime(SYSTEMTIME& st) const throw();
#endif
	__time64_t GetTime() const throw();

	int GetYear() const throw();
	int GetMonth() const throw();
	int GetDay() const throw();
	int GetHour() const throw();
	int GetMinute() const throw();
	int GetSecond() const throw();
	int GetDayOfWeek() const throw();

	Time operator -() const
	{
		return Time(-m_microsec);
	}

	Time operator -(const Time& rhs) const
	{
		return Time(m_microsec - rhs.m_microsec);
	}

	Time operator +(const Time& rhs) const
	{
		return Time(m_microsec + rhs.m_microsec);
	}

	Time& operator +=(const Time& rhs)
	{
		m_microsec += rhs.m_microsec;
		return *this;
	}

	Time& operator -=(const Time& rhs)
	{
		m_microsec -= rhs.m_microsec;
		return *this;
	}

	bool operator <(const Time& rhs) const
	{
		return m_microsec < rhs.m_microsec;
	}

	bool operator <=(const Time& rhs) const
	{
		return m_microsec <= rhs.m_microsec;
	}

	bool operator >(const Time& rhs) const
	{
		return m_microsec > rhs.m_microsec;
	}

	bool operator >=(const Time& rhs) const
	{
		return m_microsec >= rhs.m_microsec;
	}

	bool operator ==(const Time& rhs) const
	{
		return m_microsec == rhs.m_microsec;
	}

	bool operator !=(const Time& rhs) const
	{
		return m_microsec != rhs.m_microsec;
	}

	double operator /(const Time& rhs) const
	{
		return (double)m_microsec / (double)rhs.m_microsec;
	}

	Time& operator *=(int rhs)
	{
		m_microsec *= rhs;
		return *this;
	}

	Time operator *(int rhs) const
	{
		Time t;
		t.m_microsec = m_microsec * rhs;
		return t;
	}

	Time& operator /=(int rhs)
	{
		m_microsec /= rhs;
		return *this;
	}

	Time operator /(int rhs) const
	{
		Time t;
		t.m_microsec = m_microsec / rhs;
		return t;
	}

	Time& operator *=(Int64 rhs)
	{
		m_microsec *= rhs;
		return *this;
	}

	Time operator *(Int64 rhs) const
	{
		Time t;
		t.m_microsec = m_microsec * rhs;
		return t;
	}

	Time& operator /=(Int64 rhs)
	{
		m_microsec /= rhs;
		return *this;
	}

	Time operator /(Int64 rhs) const
	{
		Time t;
		t.m_microsec = m_microsec / rhs;
		return t;
	}

	Time& operator *=(double rhs)
	{
		m_microsec = static_cast<Int64>(static_cast<double>(m_microsec) * rhs);
		return *this;
	}

	Time operator *(double rhs) const
	{
		Time t;
		t.m_microsec = static_cast<Int64>(static_cast<double>(m_microsec) * rhs);
		return t;
	}

	Time& operator /=(double rhs)
	{
		m_microsec = static_cast<Int64>(static_cast<double>(m_microsec) / rhs);
		return *this;
	}

	Time operator /(double rhs) const
	{
		Time t;
		t.m_microsec = static_cast<Int64>(static_cast<double>(m_microsec) / rhs);
		return t;
	}

private:

	Time(Int64);

	Int64 m_microsec;
	
};

UTIL_API std::ostream& operator <<(std::ostream&, const Time&);

#ifndef _WIN32
//
// compare two timevals and return -1 if a is < b, 0 if a == b,
// or 1 if b > a 
//
static inline int TimevalCompare(struct timeval a, struct timeval b)
{
  if (a.tv_sec < b.tv_sec) 
  {
    return -1;
  }
  if (a.tv_sec > b.tv_sec) 
  {
    return 1;
  }
  if (a.tv_usec < b.tv_usec) 
  {
    return -1;
  }
  if (a.tv_usec > b.tv_usec)
  {
    return 1;
  }
  return 0;
}


#define USEC_IN_SEC 1000000
#define NSEC_IN_USEC 1000
#define NSEC_IN_SEC (1000 * 1000 * 1000)
#define NSEC_IN_MSEC 1000000

#if defined(__APPLE__) || defined(__FreeBSD__)
// BSD-style subsecond timespec
#	define ST_TIMESPEC(type) st_##type##timespec
#else
// POSIX standard timespec
#	define ST_TIMESPEC(type) st_##type##tim
#endif

static inline void TimevalAdd(const struct timeval a,
								 const struct timeval b, struct timeval *result)
{
	result->tv_sec = a.tv_sec + b.tv_sec;
	result->tv_usec = a.tv_usec + b.tv_usec;

	if (result->tv_usec > USEC_IN_SEC) 
	{
		result->tv_sec++;
		result->tv_usec -= USEC_IN_SEC;
	}
}

static inline void TimevalSub(const struct timeval a,
								 const struct timeval b, struct timeval *result)
{
	result->tv_sec = a.tv_sec - b.tv_sec;
	result->tv_usec = a.tv_usec - b.tv_usec;

	if (result->tv_usec < 0) 
	{
		result->tv_sec--;
		result->tv_usec += USEC_IN_SEC;
	}
}

#ifndef _WIN32
static inline void TimevalToTimespec(
	const struct timeval a, struct timespec *ts)
{
	ts->tv_sec = a.tv_sec;
	ts->tv_nsec = a.tv_usec * NSEC_IN_USEC;
}
#endif

static inline double TimevalDiff(struct timeval *start, struct timeval *end)
{
	double s = start->tv_sec + ((double)start->tv_usec) / USEC_IN_SEC;
	double e = end->tv_sec + ((double)end->tv_usec) / USEC_IN_SEC;

	return e - s;
}

#endif

}

#endif