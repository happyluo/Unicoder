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
	///Ĭ�Ϲ��캯��
	SimpleShared();
	///�������캯��
	SimpleShared(const SimpleShared&);
	
	///��������
	virtual ~SimpleShared()
	{
		m_ref = 0;
		m_noDelete = false;
	}
	
	///���Ʋ���
	SimpleShared& operator=(const SimpleShared&)
	{
		return *this;
	}
	
	///�������ü���
	void IncRef()
	{
		assert(m_ref >= 0);
		if (m_ref >= 0)
		{
			++m_ref;
		}
	}
	
	///��С���ü�����������ü�����С��0�����ͷű������ָ������Ӧ���ڴ�
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
	
	///��ȡָ�뵱ǰ�����õĴ���
	int GetRef() const
	{
		return m_ref;
	}
	
	///�����Ƿ������ͷű������ָ������Ӧ���ڴ�
	void SetNoDelete(bool b)
	{
		m_noDelete = b;
	}
	
private:
	
	///���ü���
	int m_ref;			
	///�ͷ�ʹ�ܱ�ʶ
	bool m_noDelete;		
};

// class Shared
/// A thread-safe base class for reference-counted types.
class UTIL_API Shared
{
public:
	///Ĭ�Ϲ��캯��
	Shared();
	///�������캯��
	Shared(const Shared&);
	
	///��������
	virtual ~Shared()
	{
		m_ref = 0;
		m_noDelete = false;
	}
	
	///��ֵ������
	Shared& operator =(const Shared&)
	{
		return *this;
	}
	
	///�������ü���
	virtual void IncRef();
	///��С���ü�����������ü�����С��0�����ͷű������ָ������Ӧ���ڴ�
    virtual void DecRef();
	///��ȡָ�뵱ǰ�����õĴ���
    virtual int GetRef() const;
	///�����Ƿ������ͷű������ָ������Ӧ���ڴ�
    virtual void SetNoDelete(bool);
	
protected:
#if defined(_WIN32)
	///���ü���
    LONG	m_ref;		
#elif defined(HAS_ATOMIC_FUNCTIONS) || defined(HAS_GCC_BUILTINS)
    volatile int m_ref;

#else
	long	m_ref;
	Util::Mutex	m_mutex;	
#endif
	///�ͷ�ʹ�ܱ�ʶ
    bool m_noDelete;	
};

}

#endif
