// **********************************************************************
//
// Copyright (c) 2010-2014 Bernard Luo. All rights reserved.
//
// <Email: luo (dot) xiaowei (at) hotmail (dot) com>
//
// **********************************************************************

#ifndef UTIL_CONFIG_H
#define UTIL_CONFIG_H

#ifndef NOMINMAX
#	define NOMINMAX
#endif

#include <Build/BuildConfig.h>
#include <Build/UsefulMacros.h>
//#include <Util/Port.h>

//
// Some include files we need almost everywhere.
//
#include <ctype.h>   // for isspace, etc
#include <stddef.h>  // for ptrdiff_t
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <cassert>
#include <iostream>
#include <sstream>
#include <string> 
#include <vector>

#if defined(LANG_CPP11)
#	include <functional>
#else
#	if defined(_WIN32)
#		include <functional>
#	else
#		include <tr1/functional>
#	endif

namespace std {
	using tr1::function;
	using tr1::bind;
	using tr1::ref;
	namespace placeholders = tr1::placeholders;
	using tr1::hash;
}

#endif

//
// Let's use these extensions with Util:
//
#ifdef  UTIL_API_EXPORTS
#	define UTIL_API	DECLSPEC_EXPORT
#else
#	define UTIL_API	DECLSPEC_IMPORT
#endif

namespace Util
{

enum MutexProtocol
{
	PrioInherit,
	PrioNone
};

//
// Calculate current CPU's endianness
//
/// 字节序枚举
enum EndianType
{
	BigEndian = 0,		//BigEndian
	LittleEndian,		//LittleEndian
};

/// 判断当前主机是否是Big Endian
/*static */inline bool IsBigEndian() 
{
#ifdef BIG_ENDIAN
	return true;
#else
	return false;
#endif

	int iValue = 1;
	unsigned char cValue = *((unsigned char*)&iValue);
	return 0 == cValue;
}

/// 判断当前主机(Machine)是否是Big Endian
/*static */inline EndianType CurrentEndian()
{
#ifdef BIG_ENDIAN
	return BigEndian;
#else
	return LittleEndian;
#endif

#if 0
	union
	{
		int m_int;
		char m_cahr[sizeof(int)];
	}endian;

	endian.m_int = 1;
	if (1 == endian.m_cahr[sizeof(long) - 1])
	{
		return LittleEndian;
	}

	return BigEndian;

//#else

	int iValue = 1;
	unsigned char cValue = *((unsigned char*)&iValue);
	if (0 == cValue)
	{
		return BigEndian;
	}

	return LittleEndian;
#endif
}

// 
// Calculate stack grow direction.
// 
enum StackDirection
{
	GrowUpward,
	GrowDownward
};

inline StackDirection GetStackDirection()
{
	static char *addr = NULL;	// Address of first `dummy', once known. 
	auto char dummy;			// To get stack address. (all the temporary variables are stored in stack.)

	if (NULL == addr)
	{ 
		// Initial entry. 
		addr = &dummy;

		return GetStackDirection();	// Recurse once. 
	}
	else
	{
		// Second entry. 
		if (&dummy > addr)
			return GrowUpward;		// Stack grew upward. 
		else
			return GrowDownward;	// Stack grew downward. 
	}
}

//
// By deriving from this class, other classes are made non-copyable.
//
class UTIL_API noncopyable
{
protected:

    noncopyable() { }
    ~noncopyable() { } // May not be virtual! Classes without virtual 
                       // operations also derive from noncopyable.

private:

    noncopyable(const noncopyable&);
    const noncopyable& operator=(const noncopyable&);
};

//
// Int64 typedef
//
#if defined(_MSC_VER)
	//
	// With Visual C++,, long is always 32-bit
	//
	typedef __int64				Int64;
	typedef unsigned __int64		UInt64;
#elif defined(UTIL_64)
	typedef long					Int64;
	typedef unsigned long			UInt64;
#else
	typedef long long				Int64;
	typedef unsigned long long	UInt64;  // NOLINT
#endif

typedef unsigned char			Byte;
typedef short					Short;
typedef int					Int;
typedef Util::Int64				Long;
typedef float					Float;
typedef double					Double;

// A sequence of bools. 
typedef std::vector<bool>			BoolSeq;
// A sequence of bytes. 
typedef std::vector<Byte>			ByteSeq;
// A sequence of shorts. 
typedef std::vector<Short>			ShortSeq;
// A sequence of ints. 
typedef std::vector<Int>			IntSeq;
// A sequence of longs. 
typedef std::vector<Long>			LongSeq;
// A sequence of floats. 
typedef std::vector<Float>			FloatSeq;
// A sequence of doubles. 
typedef std::vector<Double>			DoubleSeq;
// A sequence of strings. 
typedef std::vector<std::string>	StringSeq;

// A sequence of objects. 
//typedef std::vector<Object>		ObjectSeq;

// A sequence of object proxies. 
//typedef std::vector<Object*>		ObjectProxySeq;

}

namespace UtilInternal
{
// TODO: Should not be inline, this is not performance critical.
#ifdef _WIN32
	inline int GetSystemErrno() { return GetLastError(); }
#else
	inline int GetSystemErrno() { return errno; }
#endif
}

//
// NAMESPACE 
//
#if defined(__cplusplus)
#	define UTIL_BEGIN			namespace Util {
#	define UTIL_END				}
#	define USING_UTIL			using namespace Util;
#	define UTIL					::Util::

#	define UTILINTERNAL_BEGIN	namespace UtilInternal {
#	define UTILINTERNAL_END		}
#	define USING_UTILINTERNAL	using namespace UtilInternal;
#	define UTILINTERNAL			::UtilInternal::

#else // __cplusplus

#	define UTIL_BEGIN
#	define UTIL_END
#	define UTIL

#	define UTILINTERNAL_BEGIN
#	define UTILINTERNAL_END
#	define UTILINTERNAL

#endif // __cplusplus

//  NO_STDC_NAMESPACE workaround  --------------------------------------------//
//  Because std::size_t usage is so common, even in headers which do not
//  otherwise use the C library, the <cstddef> workaround is included here so
//  that ugly workaround code need not appear in many other boost headers.
//  NOTE WELL: This is a workaround for non-conforming compilers; <cstddef>
//  must still be #included in the usual places so that <cstddef> inclusion
//  works as expected with standard conforming compilers.  The resulting
//  double inclusion of <cstddef> is harmless.

#if defined(NO_STDC_NAMESPACE) && defined(__cplusplus)
#	include <cstddef>
	namespace std { using ::ptrdiff_t; using ::size_t; }
#	define ADD_TO_STD(Symbol)	namespace std { using ::Symbol; }
#endif

// A secret type that users don't know about.  It has no
// definition on purpose.  Therefore it's impossible to create a
// Secret object, which is what we want.
class Secret;

//
// The version information.
//
#define STRING_VERSION "1.0"	// "A.B.C", with A=major, B=minor, C=patch
#define INT_VERSION 10000		// AABBCC, with AA=major, BB=minor, CC=patch

#endif