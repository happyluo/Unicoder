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
	///¿½±´º¯ÊýË½ÓÐ»¯£¬½ûÖ¹¿½±´
	noncopyable(const noncopyable&);
	///¸³Öµº¯ÊýË½ÓÐ»¯£¬½ûÖ¹¸´ÖÆ
	const noncopyable& operator =(const noncopyable&);
};
}

#endif 
