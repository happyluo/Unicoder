// **********************************************************************
//
// Copyright (c) 2010-2014 Bernard Luo. All rights reserved.
//
// <Email: luo.xiaowei (at) hotmail.com>
//
// **********************************************************************

#include <Util/Shared.h>

using namespace Util;

#ifdef HAS_ATOMIC_FUNCTIONS

namespace UtilInternal
{

//
// Linux only. Unfortunately, asm/atomic.h builds non-SMP safe code
// with non-SMP kernels. This means that executables compiled with a
// non-SMP kernel would fail randomly due to concurrency errors with
// reference counting on SMP hosts. Therefore the relevent pieces of
// atomic.h are more-or-less duplicated.
//

/*
 * atomicInc - increment ice_atomic variable
 * @v: pointer of type AtomicCounter
 * 
 * Atomically increments @v by 1. Note that the guaranteed useful
 * range of an AtomicCounter is only 24 bits.
 *
 * Inlined because this operation is performance critical.
 */
static inline void atomicInc(volatile int* counter)
{
    __asm__ __volatile__(
        "lock ; incl %0"
        :"=m" (*counter)
        :"m" (*counter));
}

/**
 * atomicDecAndTest - decrement and test
 * @v: pointer of type AtomicCounter
 * 
 * Atomically decrements @v by 1 and returns true if the result is 0,
 * or false for all other cases. Note that the guaranteed useful
 * range of an AtomicCounter is only 24 bits.
 *
 * Inlined because this operation is performance critical.
 */
static inline int atomicDecAndTest(volatile int* counter)
{
    unsigned char c;
    __asm__ __volatile__(
        "lock ; decl %0; sete %1"
        :"=m" (*counter), "=qm" (c)
        :"m" (*counter) : "memory");
    return c != 0;
}

/**
 * atomicExchangeAdd - same as InterlockedExchangeAdd. This
 * didn't come from atomic.h (the code was derived from similar code
 * in /usr/include/asm/rwsem.h)
 *
 * Inlined because this operation is performance critical.
 */
static inline int atomicExchangeAdd(volatile int* counter, int i)
{
    int tmp = i;
    __asm__ __volatile__(
        "lock ; xadd %0,(%2)"
        :"+r"(tmp), "=m"(*counter)
        :"r"(counter), "m"(*counter)
        : "memory");
    return tmp + i;
}

}

#endif


Util::SimpleShared::SimpleShared() :
    m_ref(0),
    m_noDelete(false)
{
}

Util::SimpleShared::SimpleShared(const SimpleShared&) :
    m_ref(0),
    m_noDelete(false)
{
}

Util::Shared::Shared() :
    m_ref(0),
    m_noDelete(false)
{
}

Util::Shared::Shared(const Shared& rhs) :
    m_ref(0),
    m_noDelete(false)
{
}

void 
Util::Shared::IncRef()
{
#if defined(_WIN32)
    assert(InterlockedExchangeAdd(&m_ref, 0) >= 0);
    if (InterlockedExchangeAdd(&m_ref, 0) >= 0)
    {
        InterlockedIncrement(&m_ref);
    }
#elif defined(HAS_GCC_BUILTINS)

#   ifndef NDEBUG
    int c = 
#   endif
        __sync_fetch_and_add(&m_ref, 1);
    assert(c >= 0);
#elif defined(HAS_ATOMIC_FUNCTIONS)
    assert(UtilInternal::atomicExchangeAdd(&m_ref, 0) >= 0);
    UtilInternal::atomicInc(&m_ref);
#else
    m_mutex.Lock();
    assert(m_ref >= 0);
    ++m_ref;
    m_mutex.Unlock();
#endif
}

void 
Util::Shared::DecRef()
{
#if defined(_WIN32)
    assert(InterlockedExchangeAdd(&m_ref, 0) > 0);
    if (InterlockedExchangeAdd(&m_ref, 0) > 0
        && InterlockedDecrement(&m_ref) == 0 
        && !m_noDelete)
    {
        m_noDelete = true;
        delete this;
    }
#elif defined(HAS_GCC_BUILTINS)
    int c = __sync_fetch_and_sub(&m_ref, 1);
    assert(c > 0);
    if (c == 1 && !m_noDelete)
    {
        m_noDelete = true;
        delete this;
    }
#elif defined(HAS_ATOMIC_FUNCTIONS)
    assert(UtilInternal::atomicExchangeAdd(&m_ref, 0) > 0);
    if (UtilInternal::atomicDecAndTest(&m_ref) && !m_noDelete)
    {
        m_noDelete = true;
        delete this;
    }
#else
    m_mutex.Lock();
    bool doDelet = false;
    assert(m_ref > 0);
    if (0 == --m_ref)
    {
        doDelet = !m_noDelete;
        m_noDelete = true;
    }
    m_mutex.Unlock();
    if (doDelet)
    {
        delete this;
    }
#endif
}

int
Util::Shared::GetRef() const
{
#if defined(_WIN32)
    return InterlockedExchangeAdd(const_cast<LONG*>(&m_ref), 0);
#elif defined(HAS_GCC_BUILTINS)
    return __sync_fetch_and_sub(const_cast<volatile int*>(&m_ref), 0);
#elif defined(HAS_ATOMIC_FUNCTIONS)
    return UtilInternal::atomicExchangeAdd(const_cast<volatile int*>(&m_ref), 0);
#else
    m_mutex.Lock();
    int ret = m_ref;
    m_mutex.Unlock();
    return ret;
#endif
}

void
Util::Shared::SetNoDelete(bool b)
{
    m_noDelete = b;
}