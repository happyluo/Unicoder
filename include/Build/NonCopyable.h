// **********************************************************************
//
// Copyright (c) 2012-2013 Bernard Luo. All rights reserved.
//
// <Email: luo (dot) xiaowei (at) hotmail (dot) com>
//
// **********************************************************************

#ifndef BUILD_NONCOPYABLE_H
#define BUILD_NONCOPYABLE_H

namespace Base
{
//
// By deriving from this class, other classes are made non-copyable.
//
class noncopyable  
{
public:
    noncopyable()
    {
    }

    // May not be virtual! Classes without virtual 
    // operations also derive from noncopyable.
    ~noncopyable()
    {
    }

private:
    noncopyable(const noncopyable&);
    const noncopyable& operator =(const noncopyable&);
};
}

#endif 
