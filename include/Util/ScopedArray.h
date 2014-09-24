// **********************************************************************
//
// Copyright (c) 2010-2014 Bernard Luo. All rights reserved.
//
// <Email: luo (dot) xiaowei (at) hotmail (dot) com>
//
// **********************************************************************

#ifndef UTIL_SCOPED_ARRAY_H
#define UTIL_SCOPED_ARRAY_H

#include <Util/Config.h>

namespace Util
{

template <class T> class ScopedPtr;
template <class T> class ScopedArray;

// The size of a ScopedPtr is small:
// sizeof(ScopedPtr<T>) == sizeof(T*)
template <class T>
class ScopedPtr 
{
public:

    // The element type
    typedef T element_type;

    // Constructor.  Defaults to intializing with NULL.
    // There is no way to create an uninitialized ScopedPtr.
    // The input parameter must be allocated with new.
    explicit ScopedPtr(T* p = NULL) :
        m_ptr(p) 
    {
    }

    // Destructor.  If there is a T object, delete it.
    // We don't need to test m_ptr == NULL because C++ does that for us.
    ~ScopedPtr()
    {
        enum { type_must_be_complete = sizeof(T) };
        delete m_ptr;
    }

    // Reset.  Deletes the current owned object, if any.
    // Then takes ownership of a new object, if given.
    // this->reset(this->get()) works.
    void Reset(T* p = NULL) 
    {
        if (p != m_ptr) 
        {
            enum { type_must_be_complete = sizeof(T) };
            delete m_ptr;
            m_ptr = p;
        }
    }

    // Accessors to get the owned object.
    // operator* and operator-> will assert() if there is no current object.
    T& operator*() const
    {
        assert(m_ptr != NULL);
        return *m_ptr;
    }
    T* operator->() const
    {
        assert(m_ptr != NULL);
        return m_ptr;
    }

    T* Get() const
    {
        return m_ptr;
    }

    // Comparison operators.
    // These return whether two ScopedPtr refer to the same object, not just to
    // two different but equal objects.
    bool operator==(T* p) const { return m_ptr == p; }
    bool operator!=(T* p) const { return m_ptr != p; }

    // Swap two scoped pointers.
    void Swap(ScopedPtr& rhs)
    {
        T* tmp = m_ptr;
        m_ptr = rhs.m_ptr;
        rhs.m_ptr = tmp;
    }

    // Release a pointer.
    // The return value is the current pointer held by this object.
    // If this object holds a NULL pointer, the return value is NULL.
    // After this operation, this object will hold a NULL pointer,
    // and will not own the object any more.
    T* Release()
    {
        T* retVal = m_ptr;
        m_ptr = NULL;
        return retVal;
    }

private:
    T* m_ptr;

    // Forbid comparison of ScopedPtr types.  If T2 != T, it totally doesn't
    // make sense, and if T2 == T, it still doesn't make sense because you should
    // never have the same object owned by two different scoped_ptrs.
    template <class T2> bool operator==(ScopedPtr<T2> const& rhs) const;
    template <class T2> bool operator!=(ScopedPtr<T2> const& rhs) const;

    // Disallow evil constructors
    ScopedPtr(const ScopedPtr&);
    void operator=(const ScopedPtr&);
};

// ScopedArray<T> is like ScopedPtr<T>, except that the caller must allocate
// with new [] and the destructor deletes objects with delete [].
//
// As with ScopedPtr<T>, a ScopedArray<T> either points to an object
// or is NULL.  A ScopedArray<T> owns the object that it points to.
//
// Size: sizeof(ScopedArray<T>) == sizeof(T*)
template<typename T>
class ScopedArray
{
public:

    // The element type
    typedef T element_type;

    // Constructor.  Defaults to intializing with NULL.
    // There is no way to create an uninitialized scoped_array.
    // The input parameter must be allocated with new [].
    explicit ScopedArray(T* array = 0) :
        m_array(array)
    {
    }

    // Destructor.  If there is a T object, delete it.
    // We don't need to test ptr_ == NULL because C++ does that for us.
    ~ScopedArray()
    {
        if (m_array != 0)
        {
            enum { type_must_be_complete = sizeof(T) };
            delete[] m_array;
        }
    }

    // Reset.  Deletes the current owned object, if any.
    // Then takes ownership of a new object, if given.
    // this->Reset(this->Get()) works.
    void Reset(T* ptr = 0)
    {
#if 1
        if (ptr != m_array)
        {
            enum { type_must_be_complete = sizeof(T) };
            delete[] m_array;
            m_array = ptr;
        }
#else
        assert(ptr == 0 || ptr != m_array);
        if (m_array != 0)
        {
            delete[] m_array;
        }
        m_array = ptr;
#endif
    }

    // Get one element of the current object.
    // Will assert() if there is no current object, or index i is negative.
    T& operator[](std::ptrdiff_t i) const
    {
        assert(m_array != 0);
        assert(i >= 0);
        return m_array[i];
    }

    // Get a pointer to the zeroth element of the current object.
    // If there is no current object, return NULL.
    T* Get() const
    {
        return m_array;
    }

    // Comparison operators.
    // These return whether two ScopedArray refer to the same object, not just to
    // two different but equal objects.
    bool operator ==(T* p) const
    {
        return m_array == p; 
    }
    bool operator !=(T* p) const 
    {
        return m_array != p; 
    }

    // Swap two scoped arrays.
    void Swap(ScopedArray& a)
    {
        T* tmp = a.m_array;
        a.m_array = m_array;
        m_array = tmp;
    }

    // Release an array.
    // The return value is the current pointer held by this object.
    // If this object holds a NULL pointer, the return value is NULL.
    // After this operation, this object will hold a NULL pointer,
    // and will not own the object any more.
    T* Release()
    {
        T* tmp = m_array;
        m_array = 0;
        return tmp;
    }

private:

    T* m_array;

    // Forbid comparison of different ScopedArray types.
    template <class T2> bool operator==(ScopedArray<T2> const& rhs) const;
    template <class T2> bool operator!=(ScopedArray<T2> const& rhs) const;

    // Disallow evil constructors
    ScopedArray(const ScopedArray&);
    void operator=(const ScopedArray&);
};

} // End of namespace Util

#endif
