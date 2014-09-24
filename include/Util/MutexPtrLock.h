// **********************************************************************
//
// Copyright (c) 2010-2014 Bernard Luo. All rights reserved.
//
// <Email: luo (dot) xiaowei (at) hotmail (dot) com>
//
// **********************************************************************

#ifndef UTIL_MUTEX_PTR_LOCK_H
#define UTIL_MUTEX_PTR_LOCK_H

//#include <Util/Util.h>
#include <Util/ThreadException.h>

namespace Util
{
    
template <typename M>
class MutexPtrLock : public noncopyable
{
public:
    MutexPtrLock<M>(const M* mutex) :
        m_mutex(mutex),
        m_acquired(false)
    {
        if (m_mutex)
        {
            m_mutex->Lock();
            m_acquired = true;
        }
    }

    ~MutexPtrLock()
    {
        if (m_mutex && m_acquired)
        {
            m_mutex->Unlock();
            //m_acquired = false;
        }
    }

    void Acquire() const
    {
        //if (m_acquired)
        //{
        //    throw ThreadLockedException(__FILE__, __LINE__);
        //}

        if (m_mutex)
        {
            m_mutex->Lock();
            m_acquired = true;
        }
    }

    void TryAcquire() const
    {
        //if (m_acquired)
        //{
        //    throw ThreadLockedException(__FILE__, __LINE__);
        //}

        if (m_mutex)
        {
            m_acquired = m_mutex->TryLock();
            return m_acquired;
        }
    }

    void Release() const
    {
        if (m_mutex)
        {
            if (!m_acquired)
            {
                throw ThreadLockedException(__FILE__, __LINE__);
            }

            m_mutex->Unlock();
            m_acquired = false;
        }
    }

    bool Acquired() const
    {
        return m_acquired;
    }

private:
    // Not implemented; prevents accidental use.
    //
    //MutexPtrLock<M>(const MutexPtrLock<M>&);
    //MutexPtrLock<M>& operator=(const MutexPtrLock<M>&);

    const M*    m_mutex;
    mutable bool    m_acquired;
};

}

#endif