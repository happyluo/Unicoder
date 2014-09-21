// **********************************************************************
//
// Copyright (c) 2010-2014 Bernard Luo. All rights reserved.
//
// <Email: luo (dot) xiaowei (at) hotmail (dot) com>
//
// **********************************************************************

#ifndef UTIL_LOCK_H
#define UTIL_LOCK_H

#include <Util/Config.h>

namespace Util
{

// LockT 和 TryLockT 是构造 simple 和 recursive mutexes(Mutex/RecMutex)的首选.
// 其会在构造时自动获取互斥量的锁,析构时自动释放锁, 避免用户手动获取/释放锁, 减少错误发生率.
//
// LockT 和 TryLockT 是非递归的, 不可以对同一个Lock 或 TryLock对象重复获取(acquire)多次.
// 
#	ifdef LANG_CPP11
#	include <Concurrency/Mutex.h>

template <typename T>		//	T= std::mutex || std::recursive_mutex
class LockT : public noncopyable
{
	friend class Cond;

public:
	LockT(const T& mutex) 
	try : m_mutex(mutex)
	{
	}  
	catch(const std::system_error& ex)
	{
		throw ThreadSyscallException(__FILE__, __LINE__, ex.code().value());
	}

	~LockT(void)
	{
	}

	void Acquire() const 
	{
		if (m_mutex)
		{
			throw ThreadLockedException(__FILE__, __LINE__);
		}

		try
		{
			m_mutex.lock();
		}
		catch(const std::system_error& ex)
		{
			//If there is no associated mutex, std::system_error with an error code of std::errc::operation_not_permitted.
			//If the mutex is already locked, std::system_error with an error code of std::errc::resource_deadlock_would_occur.
			throw ThreadSyscallException(__FILE__, __LINE__, ex.code().value());
		}
	}

	bool TryAcquire() const
	{
		if (m_mutex)
		{
			throw ThreadLockedException(__FILE__, __LINE__);
		}

		try
		{
			return m_mutex.try_lock();
		}
		catch(const std::system_error& ex)
		{
			throw ThreadSyscallException(__FILE__, __LINE__, ex.code().value());
		}
	}

	void Release() const
	{
		if (!m_mutex)
		{
			throw ThreadLockedException(__FILE__, __LINE__);
		}

		try
		{
			m_mutex.unlock();
		}
		catch(const std::system_error& ex)
		{
			throw ThreadSyscallException(__FILE__, __LINE__, ex.code().value());
		}
	}

	bool Acquired() const
	{
		return m_mutex.owns_lock();
	}

public:

	// TryLockT's contructor
	LockT(const T& mutex, bool)
	try : m_mutex(mutex, std::try_to_lock)
	{
	}  
	catch(const std::system_error& ex)
	{
		throw ThreadSyscallException(__FILE__, __LINE__, ex.code().value());
	}

private:
	const std::unique_lock<T::mutex_type>&	m_mutex;		//  Mutex::mutex_type || RecMutex::mutex_type
};

#	else

template <typename T>
class LockT : public noncopyable
{
	friend class Cond;

public:
	LockT(const T& mutex) : m_mutex(mutex)
	{
		m_mutex.Lock();
		m_acquired = true;
	}

	~LockT(void)
	{
		if (m_acquired)
		{
			m_mutex.Unlock();
		}
	}

	void Acquire() const 
	{
		if (m_acquired)
		{
			throw ThreadLockedException(__FILE__, __LINE__);
		}

		m_mutex.Lock();
		m_acquired = true;
	}

	bool TryAcquire() const
	{
		if (m_acquired)
		{
			throw ThreadLockedException(__FILE__, __LINE__);
		}

		m_acquired = m_mutex.TryLock();
		return m_acquired;
	}

	void Release() const
	{
		if (!m_acquired)
		{
			throw ThreadLockedException(__FILE__, __LINE__);
		}

		m_mutex.Unlock();
		m_acquired = false;
	}

	bool Acquired() const
	{
		return m_acquired;
	}

protected:

	// TryLockT's contructor
	LockT(const T& mutex, bool) : m_mutex(mutex)
	{
		m_acquired = m_mutex.TryLock();
	}

private:
	const T&	m_mutex;
	mutable bool	m_acquired;
};

#	endif

template <typename T>
class TryLockT : public LockT<T>
{
public:
	TryLockT(const T& mutex) : LockT<T>(mutex, true)
	{
	}
};

}

#endif