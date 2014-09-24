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
#    include <Concurrency/Mutex.h>
#endif

namespace Util
{
// class SimpleShared
/// A non thread-safe base class for reference-counted types.
class UTIL_API SimpleShared
{
public:
    SimpleShared();
    SimpleShared(const SimpleShared&);
    
    virtual ~SimpleShared()
    {
        m_ref = 0;
        m_noDelete = false;
    }
    
    SimpleShared& operator=(const SimpleShared&)
    {
        return *this;
    }
    
    void IncRef()
    {
        assert(m_ref >= 0);
        if (m_ref >= 0)
        {
            ++m_ref;
        }
    }
    
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
    
    int GetRef() const
    {
        return m_ref;
    }
    
    void SetNoDelete(bool b)
    {
        m_noDelete = b;
    }
    
private:
    
    int m_ref;            
    bool m_noDelete;        
};

// class Shared
/// A thread-safe base class for reference-counted types.
class UTIL_API Shared
{
public:
    Shared();
    Shared(const Shared&);
    
    virtual ~Shared()
    {
        m_ref = 0;
        m_noDelete = false;
    }
    
    Shared& operator =(const Shared&)
    {
        return *this;
    }
    
    virtual void IncRef();
    virtual void DecRef();
    virtual int GetRef() const;
    virtual void SetNoDelete(bool);
    
protected:
#if defined(_WIN32)
    LONG    m_ref;        
#elif defined(HAS_ATOMIC_FUNCTIONS) || defined(HAS_GCC_BUILTINS)
    volatile int m_ref;

#else
    long    m_ref;
    Util::Mutex    m_mutex;    
#endif
    bool m_noDelete;    
};

}

#endif
