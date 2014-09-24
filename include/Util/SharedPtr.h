// **********************************************************************
//
// Copyright (c) 2010-2014 Bernard Luo. All rights reserved.
//
// <Email: luo (dot) xiaowei (at) hotmail (dot) com>
//
// **********************************************************************

#ifndef UTIL_SHAREDPTR_H
#define UTIL_SHAREDPTR_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <algorithm>
#include <Util/Exception.h>

namespace Util
{
// class SharedPtrBase
///侵入式引用计数智能指针基类，实现对被管理指针的基本操作
template<typename T>
class SharedPtrBase
{
public:
    
    typedef T element_type;
    
    T* Get() const
    {
        return m_ptr;
    }
    
    T* operator->() const
    {
        if (!m_ptr)
        {
            ThrowNullSharedPtrException(__FILE__, __LINE__);           
        }
        
        return m_ptr;
    }
    
    T& operator*() const
    {
        if (!m_ptr)
        {
            ThrowNullSharedPtrException(__FILE__, __LINE__);           
        }
        
        return *m_ptr;
    }
    
    operator bool() const
    {
        return m_ptr ? true : false;
    }
    
    void Swap(SharedPtrBase& other)
    {
        std::swap(m_ptr, other.m_ptr);
    }
    
    T *m_ptr;                

private:
    void ThrowNullSharedPtrException(const char *file, int line) const;
};
    
template<typename T> inline void 
SharedPtrBase<T>::ThrowNullSharedPtrException(const char* file, int line) const
{
    throw NullSharedPtrException(file, line);
}

template<typename T, typename U>
inline bool operator ==(const SharedPtrBase<T>& lhs, const SharedPtrBase<U>& rhs)
{
    T* l = lhs.Get();
    U* r = rhs.Get();
    if (l && r)
    {
        return *l == *r;
    }
    
    return !l && !r;
}

template<typename T, typename U>
inline bool operator !=(const SharedPtrBase<T>& lhs, const SharedPtrBase<U>& rhs)
{
    return !operator ==(lhs, rhs);
}

template<typename T, typename U>
inline bool operator <(const SharedPtrBase<T>& lhs, const SharedPtrBase<U>& rhs)
{
    T* l = lhs.Get();
    U* r = rhs.Get();
    if (l && r)
    {
        return *l < *r;
    }
    
    return !l && r;
}

template<typename T, typename U>
inline bool operator <=(const SharedPtrBase<T>& lhs, const SharedPtrBase<U>& rhs)
{
    return lhs < rhs || lhs == rhs;
}

template<typename T, typename U>
inline bool operator >(const SharedPtrBase<T>& lhs, const SharedPtrBase<U>& rhs)
{
    return !(lhs < rhs || lhs == rhs);
}

template<typename T, typename U>
inline bool operator >=(const SharedPtrBase<T>& lhs, const SharedPtrBase<U>& rhs)
{
    return !(lhs < rhs);
}
    
// class SharedPtr
///侵入式引用计数智能指针类，实现对指针本身及其所对应内存的管理
template<typename T>
class SharedPtr : public SharedPtrBase<T>
{
public:   
    SharedPtr(T* p = 0)
    {
        this->m_ptr = p;
        
        if (this->m_ptr)
        {
            this->m_ptr->IncRef();
        }
    }

    template<typename Y>
    SharedPtr(const SharedPtr<Y>& rhs)
    {
        this->m_ptr = rhs.m_ptr;
        
        if (this->m_ptr)
        {
            this->m_ptr->IncRef();
        }
    }
    
    SharedPtr(const SharedPtr& rhs)
    {
        this->m_ptr = rhs.m_ptr;
        
        if (this->m_ptr)
        {
            this->m_ptr->IncRef();
        }
    }
    
    ~SharedPtr()
    {
        if (this->m_ptr)
        {            
            this->m_ptr->DecRef();
        }
        
        this->m_ptr = 0;
    }

    SharedPtr& operator=(T* p)
    {
        if (this->m_ptr != p)
        {
            if (p)
            {
                p->IncRef();
            }
            
            T* ptr = this->m_ptr;
            this->m_ptr = p;
            
            if (ptr)
            {
                long lRefCount = ptr->GetRef();
                
                ptr->DecRef();
                
                if (1 == lRefCount)
                {
                    ptr = 0;
                }
            }
        }
        return *this;
    }
    
    template<typename Y>
    SharedPtr& operator=(const SharedPtr<Y>& rhs)
    {
        if (this->m_ptr != rhs.m_ptr)
        {
            if (rhs.m_ptr)
            {
                rhs.m_ptr->IncRef();
            }
            
            T* ptr = this->m_ptr;
            this->m_ptr = rhs.m_ptr;
            
            if (ptr)
            {
                long lRefCount = ptr->GetRef();
                
                ptr->DecRef();
                
                if (1 == lRefCount)
                {
                    ptr = 0;
                }
            }
        }
        return *this;
    }
    
    SharedPtr& operator=(const SharedPtr& rhs)
    {
        if (this->m_ptr != rhs.m_ptr)
        {
            if (rhs.m_ptr)
            {
                rhs.m_ptr->IncRef();
            }
            
            T* ptr = this->m_ptr;
            this->m_ptr = rhs.m_ptr;
            
            if (ptr)
            {
                long lRefCount = ptr->GetRef();
                
                ptr->DecRef();
                
                if (1 == lRefCount)
                {
                    ptr = 0;
                }
            }
        }
        return *this;
    }
    
    template<class Y>
    static SharedPtr DynamicCast(const SharedPtrBase<Y>& rhs)
    {
        //return SharedPtr<T>(dynamic_cast<T*>(rhs.m_ptr));
        return SharedPtr(dynamic_cast<T*>(rhs.m_ptr));
    }
    
    template<class Y>
    static SharedPtr DynamicCast(Y* p)
    {
        return SharedPtr(dynamic_cast<T*>(p));
    }
};

}

#endif
