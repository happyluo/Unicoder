// **********************************************************************
//
// Copyright (c) 2010-2014 Bernard Luo. All rights reserved.
//
// <Email: luo (dot) xiaowei (at) hotmail (dot) com>
//
// **********************************************************************

#ifndef UTIL_THREAD_EXCEPTION_H
#define UTIL_THREAD_EXCEPTION_H

#include <Util/Config.h>
#include <Util/Exception.h>
#include <Util/Time.h>

namespace Util
{

class UTIL_API ThreadSyscallException : public SyscallException
{
public:
	ThreadSyscallException(const char *file, int line, int syscallError);
	virtual ~ThreadSyscallException(void) throw();

	virtual std::string Name() const;

	virtual Exception* Clone() const;

	virtual void Throw() const;

private:

	///异常的名称
	static const char *ms_pcName;
};

class UTIL_API ThreadLockedException : public Exception
{
public:
	ThreadLockedException(const char *file, int line);
	virtual ~ThreadLockedException(void) throw();

	virtual std::string Name() const;

	virtual Exception* Clone() const;

	virtual void Throw() const;

private:

	///异常的名称
	static const char *ms_pcName;
};

class UTIL_API ThreadStartedException : public Exception
{
public:
	ThreadStartedException(const char *file, int line);
	virtual ~ThreadStartedException(void) throw();

	virtual std::string Name() const;

	virtual Exception* Clone() const;

	virtual void Throw() const;

private:

	///异常的名称
	static const char *ms_pcName;
};

class UTIL_API ThreadNotStartedException : public Exception
{
public:
	ThreadNotStartedException(const char *file, int line);
	virtual ~ThreadNotStartedException(void) throw();

	virtual std::string Name() const;

	virtual Exception* Clone() const;

	virtual void Throw() const;

private:

	///异常的名称
	static const char *ms_pcName;
};

class UTIL_API BadThreadControlException : public Exception
{
public:
	BadThreadControlException(const char *file, int line);
	virtual ~BadThreadControlException(void) throw();

	virtual std::string Name() const;

	virtual Exception* Clone() const;

	virtual void Throw() const;

private:

	///异常的名称
	static const char *ms_pcName;
};

class UTIL_API InvalidTimeoutException : public Exception
{
public:
	InvalidTimeoutException(const char *file, int line, const Time& timeout);
	virtual ~InvalidTimeoutException(void) throw();

	virtual std::string Name() const;

	virtual void Print(std::ostream& out) const;

	virtual Exception* Clone() const;

	virtual void Throw() const;

private:

	Time						m_uSec;
	///异常的名称
	static const char *ms_pcName;
};

class UTIL_API DeadlockException : public Exception
{
public:
	DeadlockException(const char *file, int line);
	virtual ~DeadlockException(void) throw();

	virtual std::string Name() const;

	virtual Exception* Clone() const;

	virtual void Throw() const;

private:

	///异常的名称
	static const char *ms_pcName;
};

class UTIL_API ThreadPoolDestroyedException : public Exception
{
public:
	ThreadPoolDestroyedException(const char *file, int line);
	virtual ~ThreadPoolDestroyedException(void) throw();

	virtual std::string Name() const;

	virtual Exception* Clone() const;

	virtual void Throw() const;

private:

	///异常的名称
	static const char *ms_pcName;
};

}
#endif