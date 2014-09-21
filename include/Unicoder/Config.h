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
#include <Build/UndefSysMacros.h>

//
// Let's use these extensions with Util:
//
#ifdef  UTIL_API_EXPORTS
#	define UTIL_API	DECLSPEC_EXPORT
#else
#	define UTIL_API	DECLSPEC_IMPORT
#endif

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

#endif