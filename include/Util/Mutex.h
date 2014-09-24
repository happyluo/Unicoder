// **********************************************************************
//
// Copyright (c) 2010-2014 Bernard Luo. All rights reserved.
//
// <Email: luo (dot) xiaowei (at) hotmail (dot) com>
//
// **********************************************************************

#ifndef UTIL_MUTEX_H
#define UTIL_MUTEX_H

#include <Util/Config.h>
#include <Util/Lock.h>

#ifdef LANG_CPP11
#   include <mutex>
#endif

namespace Util
{

//
// Simple non-recursive Util::Mutex implementation.
//
class UTIL_API Mutex : public noncopyable
{
    friend class Cond;

public:
    //
    // Lock & TryLock typedefs.
    //
    typedef LockT<Mutex> LockGuard;            
    typedef TryLockT<Mutex> TryLockGuard;

    Mutex(void);
    Mutex(MutexProtocol);
    ~Mutex(void);

    void Lock()    const;

    bool TryLock() const;

    void Unlock() const;

    //
    // Returns true if the mutex will unlock when calling unlock()
    // (false otherwise). For non-recursive mutexes, this will always
    // return true. 
    // This function is used by the Monitor implementation to know whether 
    // the Mutex has been locked for the first time, or unlocked for the 
    // last time (that is another thread is able to acquire the mutex).
    //
    // Pre-condition: the mutex must be locked.
    //
    bool WillUnlock() const;

#ifdef LANG_CPP11
    operator const std::mutex&() const
    {
        return m_mutex;
    }
#endif

private:

    void init(MutexProtocol);

    // noncopyable
    //Mutex(const Mutex&);
    //void operator=(const Mutex&);

    //
    // LockState and the lock/unlock variations are for use by the
    // Condition variable implementation.
    //
#ifdef LANG_CPP11
    struct LockState
    {
        std::mutex* m_pmutex;
    };
#elif defined(_WIN32)
    struct LockState
    {
#   ifdef HAS_WIN32_CONDVAR
        CRITICAL_SECTION* m_pmutex;
#   endif 
    };
#else
    struct LockState
    {
        pthread_mutex_t* m_pmutex;
    };
#endif

    void unlock(LockState&) const;
    void lock(LockState&) const;

#ifdef LANG_CPP11
    typedef std::mutex mutex_type;
    mutable std::mutex m_mutex;
#elif defined(_WIN32)
    mutable CRITICAL_SECTION m_mutex;
#else
    mutable pthread_mutex_t m_mutex;
#endif
};


//
// For performance reasons the following functions are inlined.
//
inline Mutex::Mutex()
{
    init(PrioNone);
}

inline Mutex::Mutex(MutexProtocol protocol)
{
#ifdef _WIN32
    init(PrioNone);
#else
    init(protocol);
#endif
}

}

#endif