// **********************************************************************
//
// Copyright (c) 2010-2014 Bernard Luo. All rights reserved.
//
// <Email: luo (dot) xiaowei (at) hotmail (dot) com>
//
// **********************************************************************

#ifndef UTIL_SHARED_H
#define UTIL_SHARED_H


#include <Util/Config.h>

#if defined(USE_MUTEX_SHARED)

#   include <Util/Mutex.h>

// Using the gcc builtins requires gcc 4.1 or better. For Linux, i386
// doesn't work. Apple is supported for all architectures. Sun only
// supports sparc (32 and 64 bit).

#elif (__GNUC__ >= 4 && __GNUC_MINOR__ >= 1 || __clang__)  &&            \
        ((defined(__sun) && (defined(__sparc) || defined(__sparcv9))) || \
         defined(__APPLE__) || \
        (defined(__linux) && \
                (defined(__i486) || defined(__i586) || \
                 defined(__i686) || defined(__x86_64))))

#   define HAS_GCC_BUILTINS

#elif (defined(__APPLE__) || defined(__linux) || defined(__FreeBSD__)) && (defined(__i386) || defined(__x86_64)) && !defined(__ICC)

#   define HAS_ATOMIC_FUNCTIONS

#elif defined(_WIN32)
// Nothing to include
#else
// Use a simple mutex
#	include <Concurrency/Mutex.h>
#endif

//
// Base classes for reference counted types. The IceUtil::Handle
// template can be used for smart pointers to types derived from these
// bases.
//
// Util::SimpleShared
// =====================
//
// A non thread-safe base class for reference-counted types.
//
// Util::Shared
// ===============
//
// A thread-safe base class for reference-counted types.
//
namespace Util
{
// class SimpleShared
/// A non thread-safe base class for reference-counted types.
class UTIL_API SimpleShared
{
public:
	///默认构造函数
	SimpleShared();
	///拷贝构造函数
	SimpleShared(const SimpleShared&);
	
	///析构函数
	virtual ~SimpleShared()
	{
		m_ref = 0;
		m_noDelete = false;
	}
	
	///复制操作
	SimpleShared& operator=(const SimpleShared&)
	{
		return *this;
	}
	
	///增加引用计数
	void IncRef()
	{
		assert(m_ref >= 0);
		if (m_ref >= 0)
		{
			++m_ref;
		}
	}
	
	///减小引用计数，如果引用计数减小到0，则释放被管理的指针所对应的内存
	void DecRef()
	{
		assert(m_ref > 0);
		if (m_ref > 0 && --m_ref == 0)
		{
			if (!m_noDelete)
			{
				m_noDelete = true;
				delete this;
			}
		}
	}
	
	///获取指针当前被引用的次数
	int GetRef() const
	{
		return m_ref;
	}
	
	///设置是否允许释放被管理的指针所对应的内存
	void SetNoDelete(bool b)
	{
		m_noDelete = b;
	}
	
private:
	
	///引用计数
	int m_ref;			
	///释放使能标识
	bool m_noDelete;		
};

// class Shared
/// A thread-safe base class for reference-counted types.
class UTIL_API Shared
{
public:
	///默认构造函数
	Shared();
	///拷贝构造函数
	Shared(const Shared&);
	
	///析构函数
	virtual ~Shared()
	{
		m_ref = 0;
		m_noDelete = false;
	}
	
	///赋值操作符
	Shared& operator =(const Shared&)
	{
		return *this;
	}
	
	///增加引用计数
	virtual void IncRef();
	///减小引用计数，如果引用计数减小到0，则释放被管理的指针所对应的内存
    virtual void DecRef();
	///获取指针当前被引用的次数
    virtual int GetRef() const;
	///设置是否允许释放被管理的指针所对应的内存
    virtual void SetNoDelete(bool);
	
protected:
#if defined(_WIN32)
	///引用计数
    LONG	m_ref;		
#elif defined(HAS_ATOMIC_FUNCTIONS) || defined(HAS_GCC_BUILTINS)
    volatile int m_ref;

#else
	long	m_ref;
	Util::Mutex	m_mutex;	
#endif
	///释放使能标识
    bool m_noDelete;	
};

}

#endif
