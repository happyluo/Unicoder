// **********************************************************************
//
// Copyright (c) 2012-2013 Bernard Luo. All rights reserved.
//
// <Email: luo (dot) xiaowei (at) hotmail (dot) com>
//
// **********************************************************************

// This file adds defines about the platform we're currently building on.
//  Operating System:
//    OS_WIN / OS_MACOSX / OS_LINUX / OS_POSIX (MACOSX or LINUX) / OS_NACL
//  Compiler:
//    COMPILER_MSVC / COMPILER_GCC
//  Processor:
//    ARCH_CPU_X86 / ARCH_CPU_X86_64 / ARCH_CPU_X86_FAMILY (X86 or X86_64)
//    ARCH_CPU_32_BITS / ARCH_CPU_64_BITS

#ifndef BUILD_CONFIG_H
#define BUILD_CONFIG_H

#include <Build/NonCopyable.h>
//#include <Build/UndefSysMacros.h>
#include <Build/UsefulMacros.h>

#if defined(__APPLE__)
#include <TargetConditionals.h>
#endif

//
// A set of macros to use for platform detection.
//
#if defined(__native_client__)
// __native_client__ must be first, so that other OS_ defines are not set.
#	define OS_NACL 1
#elif defined(ANDROID)
#	define OS_ANDROID 1
#elif defined(__APPLE__)
#	define OS_MACOSX 1
#	if defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE
#		define OS_IOS 1
#	endif  // defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE
#elif defined(__linux__)
#	define OS_LINUX 1
	// include a system header to pull in features.h for glibc/uclibc macros.
#	include <unistd.h>
#	if defined(__GLIBC__) && !defined(__UCLIBC__)
	// we really are using glibc, not uClibc pretending to be glibc
#		define LIBC_GLIBC 1
#	endif
#elif defined(_WIN32)
#	define OS_WIN 1
#	define TOOLKIT_VIEWS 1
#elif defined(__FreeBSD__)
#	define OS_FREEBSD 1
#elif defined(__OpenBSD__)
#	define OS_OPENBSD 1
#elif defined(__sun)
#	define OS_SOLARIS 1
#elif defined(__QNXNTO__)
#	define OS_QNX 1
#else
#	error Please add support for your platform in build/build_config.h
#endif

#if defined(USE_OPENSSL) && defined(USE_NSS)
#	error Cannot use both OpenSSL and NSS
#endif

// For access to standard BSD features, use OS_BSD instead of a
// more specific macro.
#if defined(OS_FREEBSD) || defined(OS_OPENBSD)
#	define OS_BSD 1
#endif

// For access to standard POSIXish features, use OS_POSIX instead of a
// more specific macro.
#if defined(OS_MACOSX) || defined(OS_LINUX) || defined(OS_FREEBSD) ||  \
    defined(OS_OPENBSD) || defined(OS_SOLARIS) || defined(OS_ANDROID) ||  \
    defined(OS_NACL) || defined(OS_QNX)
#	define OS_POSIX 1
#endif

//
// Use tcmalloc
//
#if (defined(OS_WIN) || defined(OS_LINUX) || defined(OS_ANDROID)) && \
    !defined(NO_TCMALLOC)
#	define USE_TCMALLOC 1
#endif

//
// Compiler detection.
//
#if defined(__GNUC__)
#	define COMPILER_GCC 1
#elif defined(_MSC_VER)
#	define COMPILER_MSVC 1
#else
#	error Please add support for your compiler in build/build_config.h
#endif

// Processor architecture detection.  For more info on what's defined, see:
//   http://msdn.microsoft.com/en-us/library/b0084kay.aspx
//   http://www.agner.org/optimize/calling_conventions.pdf
//   or with gcc, run: "echo | gcc -E -dM -"
#if defined(_M_X64) || defined(__x86_64__)
#	define ARCH_CPU_X86_FAMILY 1
#	define ARCH_CPU_X86_64 1
#	define ARCH_CPU_64_BITS 1
#	define ARCH_CPU_LITTLE_ENDIAN 1
#elif defined(_M_IX86) || defined(__i386__)
#	define ARCH_CPU_X86_FAMILY 1
#	define ARCH_CPU_X86 1
#	define ARCH_CPU_32_BITS 1
#	define ARCH_CPU_LITTLE_ENDIAN 1
#elif defined(__ARMEL__)
#	define ARCH_CPU_ARM_FAMILY 1
#	define ARCH_CPU_ARMEL 1
#	define ARCH_CPU_32_BITS 1
#	define ARCH_CPU_LITTLE_ENDIAN 1
#elif defined(__aarch64__)
#	define ARCH_CPU_ARM_FAMILY 1
#	define ARCH_CPU_ARM64 1
#	define ARCH_CPU_64_BITS 1
#	define ARCH_CPU_LITTLE_ENDIAN 1
#elif defined(__pnacl__)
#	define ARCH_CPU_32_BITS 1
#	define ARCH_CPU_LITTLE_ENDIAN 1
#elif defined(__MIPSEL__)
#	define ARCH_CPU_MIPS_FAMILY 1
#	define ARCH_CPU_MIPSEL 1
#	define ARCH_CPU_32_BITS 1
#	define ARCH_CPU_LITTLE_ENDIAN 1
#else
#error Please add support for your architecture in build/build_config.h
#endif

//
// Type detection for wchar_t.
//
#if defined(OS_WIN)
#	define WCHAR_T_IS_UTF16
#elif defined(OS_POSIX) && defined(COMPILER_GCC) && \
    defined(__WCHAR_MAX__) && \
    (__WCHAR_MAX__ == 0x7fffffff || __WCHAR_MAX__ == 0xffffffff)
#	define WCHAR_T_IS_UTF32
#elif defined(OS_POSIX) && defined(COMPILER_GCC) && \
    defined(__WCHAR_MAX__) && \
    (__WCHAR_MAX__ == 0x7fff || __WCHAR_MAX__ == 0xffff)
	// On Posix, we'll detect short wchar_t, but projects aren't guaranteed to
	// compile in this mode (in particular, Chrome doesn't). This is intended for
	// other projects using base who manage their own dependencies and make sure
	// short wchar works for them.
#	define WCHAR_T_IS_UTF16
#else
#	error Please add support for your compiler in build/build_config.h
#endif

#if defined(OS_ANDROID)
	// The compiler thinks std::string::const_iterator and "const char*" are
	// equivalent types.
#	define STD_STRING_ITERATOR_IS_CHAR_POINTER
	// The compiler thinks base::string16::const_iterator and "char16*" are
	// equivalent types.
#	define BASE_STRING16_ITERATOR_IS_CHAR16_POINTER
#endif

//////////////////////////////////////////////////////////////////////////

#if defined(_WIN32)
#   ifndef _WIN32_WINNT
		//
		// Necessary for TryEnterCriticalSection (see Concurrency/Mutex.h).
		//
#       if defined(_MSC_VER) && _MSC_VER < 1500
#           define _WIN32_WINNT 0x0400
#       endif
#   elif _WIN32_WINNT < 0x0400
#       error "TryEnterCricalSection requires _WIN32_WINNT >= 0x0400"
#   endif

#   if !defined(STATIC_LIBS) && defined(_MSC_VER) && (!defined(_DLL) || !defined(_MT))
#       error "Only multi-threaded DLL libraries can be used with Concurrency!"
#   endif

#	include <windows.h>
#	include <process.h>
#	include <errno.h>

#	ifdef _MSC_VER
//		'...' : forcing value to bool 'true' or 'false' (performance warning)
#		pragma warning( disable : 4800 )
//     ... identifier was truncated to '255' characters in the debug information
#		pragma warning( disable : 4786 )
//		'this' : used in base member initializer list
#		pragma warning( disable : 4355 )
//		class ... needs to have dll-interface to be used by clients of class ...
//#		pragma warning( disable : 4251 )
//     ... : inherits ... via dominance
//#		pragma warning( disable : 4250 )
//     non dll-interface class ... used as base for dll-interface class ...
#		pragma warning( disable : 4275 )
//		...: decorated name length exceeded, name was truncated
#		pragma warning( disable : 4503 )  
//		 This function or variable may be unsafe. 
#		pragma warning( disable : 4996 )  //??
//
// Move some warnings to level 4
//
#		pragma warning( 4 : 4250 ) // ... : inherits ... via dominance
#		pragma warning( 4 : 4251 ) // class ... needs to have dll-interface to be used by clients of class ..

#		define UTIL_TOSTRING2(x) #x
#		define UTIL_TOSTRING(x) UTIL_TOSTRING2(x)
#		define UTIL_WARNING(x) __pragma(message(__FILE__ "(" UTIL_TOSTRING(__LINE__) ") : warning note: " x))

#	endif

#	define __alignof__ __alignof

#else
#	include <pthread.h>
#	include <errno.h>

#	if defined(__sun__) || defined(__linux__) || defined(_AIX)
#		include <unistd.h>
#	else
#		include <sys/sysctl.h>
#	endif

#	if defined(__NetBSD__)
#		pragma weak pthread_create // Do not create libpthread dependency
#	endif

#endif

//
// Visual Studio 2012 or later, without Windows XP/2003 support
//
#if defined(_MSC_VER) && (_MSC_VER >= 1700) && !defined(_USING_V110_SDK71_)

//
// Check if building for WinRT
//
#   include <winapifamily.h>
#   if defined(WINAPI_FAMILY) && (WINAPI_FAMILY == WINAPI_FAMILY_APP)
#      define OS_WINRT
#      define STATIC_LIBS
#   endif
//
// Windows provides native condition variables on Vista and later,
// and Visual Studio 2012 with the default Platform Toolset (vc100) no 
// longer supports Windows XP or Windows Server 2003.
//
// You can "switch-on" this macro to use native condition variables with
// other C++ compilers on Windows.
//
#   define HAS_WIN32_CONDVAR 
#endif

//
// Endianness
//
// Most CPUs support only one endianness, with the notable exceptions
// of Itanium (IA64) and MIPS.
//
#ifdef __GLIBC__
# include <endian.h>
#endif

#if defined(__i386)     || defined(_M_IX86) || defined(__x86_64)  || \
	defined(_M_X64)     || defined(_M_IA64) || defined(__alpha__) || \
	defined(__ARMEL__) || defined(_M_ARM_FP) || \
	defined(__MIPSEL__) || (defined(__BYTE_ORDER) && (__BYTE_ORDER == __LITTLE_ENDIAN))
#   define LITTLE_ENDIAN
#elif defined(__sparc) || defined(__sparc__) || defined(__hppa)      || \
	defined(__ppc__) || defined(__powerpc) || defined(_ARCH_COM) || \
	defined(__MIPSEB__) || (defined(__BYTE_ORDER) && (__BYTE_ORDER == __BIG_ENDIAN))
#   define BIG_ENDIAN
#else
#   error "Unknown architecture"
#endif

//
// 32 or 64 bit mode?
//
#if defined(__linux) && defined(__sparc__)
//
// We are a linux sparc, which forces 32 bit usr land, no matter 
// the architecture
//
#   define  UTIL_32
#elif defined(__sun) && (defined(__sparcv9) || defined(__x86_64))  || \
	defined(__linux) && defined(__x86_64)				|| \
	defined(__hppa) && defined(__LP64__)				|| \
	defined(_ARCH_COM) && defined(__64BIT__)			|| \
	defined(__alpha__)									|| \
	defined(_WIN64)
#   define UTIL_64
#else
#   define UTIL_32
#endif


//
// Check for C++ 11 support
// 
// We cannot just check for C++11 mode as some features were not 
// implemented in first versions of the compilers.
//
// The require compiler version should be equal or greater than
// VC100, G++ 4.5, Clang Apple 4.2 or Clang 3.2 (Unsupported).
//
#if (defined(__GNUC__) && (((__GNUC__* 100) + __GNUC_MINOR__) >= 405) && defined(__GXX_EXPERIMENTAL_CXX0X__)) || \
	(defined(__clang__) && \
	((defined(__apple_build_version__) && (((__clang_major__ * 100) + __clang_minor__) >= 402)) || \
	(!defined(__apple_build_version__) && (((__clang_major__ * 100) + __clang_minor__) >= 302))) && \
	__cplusplus >= 201103) || \
	(defined(_MSC_VER) && (_MSC_VER >= 1600))
#   define LANG_CPP11
#elif __cplusplus >= 201103 || defined(__GXX_EXPERIMENTAL_CXX0X__)
#   error Unsupported C++11 compiler
#endif

#ifndef CPP_VER
#  if  __cplusplus <= 201103L
#    define CPP_VER 11
#  else
#    define CPP_VER 14  // current year, or date of c++14 ratification
#  endif
#endif  // CPP_VER

#if defined(LANG_CPP11)// && !defined(_MSC_VER)
// Visual Studio does not support noexcept yet
#   define UTIL_NOEXCEPT			noexcept
#   define UTIL_NOEXCEPT_EXPR(expr) noexcept(expr)
#   define UTIL_NOEXCEPT_FALSE		noexcept(false)

#	define UTIL_CONSTEXPR constexpr

// cxx11 strong enums and their qualification
#	define UTIL_ENUM_CLASS enum class
#	define UTIL_ENUM_QUAL( e ) e::
#else
#   define UTIL_NOEXCEPT throw()
#   define UTIL_NOEXCEPT_EXPR(expr) /**/
#   define UTIL_NOEXCEPT_FALSE		 /**/

#	define UTIL_CONSTEXPR

// cxx11 strong enums and their qualification
#	define UTIL_ENUM_CLASS enum
#	define UTIL_ENUM_QUAL( e )

#	define nullptr_t

//#	define decltype(x) __typeof__(x)
#endif

#if CPP_VER > 11
#	define UTIL_CONSTEXPR_AFTER_CXX11
#	define UTIL_EXPLICIT_AFTER_CXX11
#	define UTIL_DEPRECATED_AFTER_CXX11
#else
#	define UTIL_CONSTEXPR_AFTER_CXX11 constexpr
#	define UTIL_EXPLICIT_AFTER_CXX11 explicit
#	define UTIL_DEPRECATED_AFTER_CXX11 [[deprecated]]
#endif

//
// Compiler extensions to export and import symbols: see the documentation 
// for Visual C++, Sun ONE Studio 8/Solaris Studio and HP aC++.
//
#if /*defined(__BCPLUSPLUS__) || */(defined(_MSC_VER) && !defined(STATIC_LIBS)) || \
	(defined(__HP_aCC) && defined(__HP_WINDLL))
#   define DECLSPEC_EXPORT __declspec(dllexport)
#   define DECLSPEC_IMPORT __declspec(dllimport)
#   define HAS_DECLSPEC_IMPORT_EXPORT
#elif defined(__SUNPRO_CC) && (__SUNPRO_CC >= 0x550)
#   define DECLSPEC_EXPORT __global
#   define DECLSPEC_IMPORT /**/
#else
#   define DECLSPEC_EXPORT /**/
#   define DECLSPEC_IMPORT /**/
#endif

#if defined(_MSC_VER)
#   define DEPRECATED_API __declspec(deprecated)
#elif defined(__GNUC__)
#   define DEPRECATED_API __attribute__((deprecated))
#else
#   define DEPRECATED_API /**/
#endif

//
// UTIL_INT64: macro for Int64 literal values
// long long macros to be used because gcc and vc++ use different suffixes,
// and different size specifiers in format strings
//
#undef UTIL_INT64
#undef UTIL_UINT64
#undef UTIL_64_FORMAT

#if defined(_MSC_VER)
#   define UTIL_INT64(n)  n##i64		// I64
#   define UTIL_UINT64(n) n##ui64		// UI64
#   define UTIL_64_FORMAT "i64"		// As in printf("%I64d", ...)
#elif defined(UTIL_64)
#   define UTIL_INT64(n)   n##L
#   define UTIL_UINT64(n)  n##UL		
#else
#   define UTIL_INT64(n)   n##LL		// LL
#   define UTIL_UINT64(n)  n##ULL		// ULL
#   define UTIL_64_FORMAT  "ll"			// As in "%lld". Note that "q" is poor form also.
#endif

typedef unsigned int uint;

#ifdef _MSC_VER
	typedef __int8  int8;
	typedef __int16 int16;
	typedef __int32 int32;
	typedef __int64 int64;

	typedef unsigned __int8  uint8;
	typedef unsigned __int16 uint16;
	typedef unsigned __int32 uint32;
	typedef unsigned __int64 uint64;
#else
	typedef int8_t  int8;
	typedef int16_t int16;
	typedef int32_t int32;
	typedef int64_t int64;

	typedef uint8_t  uint8;
	typedef uint16_t uint16;
	typedef uint32_t uint32;
	typedef uint64_t uint64;
#endif

// Per C99 7.8.14, define __STDC_CONSTANT_MACROS before including <stdint.h>  
// to get the INTn_C and UINTn_C macros for integer constants.  It's difficult  
// to guarantee any specific ordering of header includes, so it's difficult to  
// guarantee that the INTn_C macros can be defined by including <stdint.h> at  
// any specific point.  Provide UTIL_INTn_C macros instead.  
#define UTIL_INT8_C(x)    (x)  
#define UTIL_INT16_C(x)   (x)  
#define UTIL_INT32_C(x)   (x)  
#define UTIL_INT64_C(x)   UTIL_INT64(x)  

#define UTIL_UINT8_C(x)   (x ## U)  
#define UTIL_UINT16_C(x)  (x ## U)  
#define UTIL_UINT32_C(x)  (x ## U)  
#define UTIL_UINT64_C(x)  UTIL_UINT64(x)  

static const uint8  kuint8max  = (( uint8) 0xFF);  
static const uint16 kuint16max = ((uint16) 0xFFFF);  
static const uint32 kuint32max = ((uint32) 0xFFFFFFFFu);  
static const uint64 kuint64max = ((uint64) UTIL_UINT64(0xFFFFFFFFFFFFFFFF));  
static const int8   kint8min   = ((  int8) 0x80);  
static const int8   kint8max   = ((  int8) 0x7F);  
static const int16  kint16min  = (( int16) 0x8000);  
static const int16  kint16max  = (( int16) 0x7FFF);  
static const int32  kint32min  = (( int32) 0x80000000);  
static const int32  kint32max  = (( int32) 0x7FFFFFFF);  
//static const int32  kint32min  = -kint32max - 1;
static const int64  kint64min  = (( int64) UTIL_UINT64(0x8000000000000000));  
static const int64  kint64max  = (( int64) UTIL_UINT64(0x7FFFFFFFFFFFFFFF));  
//static const int64  kint64min  = -kint64max - 1;


#if __cplusplus < 201103L	// 
#	if defined(__linux__) || defined(_WIN32)
#		define UTIL_HAS_NO_UNICODE_CHARS
#	else
	typedef __char16_t char16_t;
	typedef __char32_t char32_t;
#	endif
#endif

#ifndef __SIZEOF_INT128__
#	define UTIL_HAS_NO_INT128
#endif

	//
	// NAMESPACE 
	//
#if defined(__cplusplus)
#	define USING_STD(type)		using std::type;

#	define C_LIB_DECL			extern "C" {	// C has extern "C" linkage
#	define END_C_LIB_DECL		}
//#	define EXTERN_C				extern "C" {
//#	define END_EXTERN_C			}

#else // __cplusplus

#	define C_LIB_DECL
#	define END_C_LIB_DECL
//#	define EXTERN_C
//#	define END_EXTERN_C

#endif // __cplusplus

#endif  // BUILD_CONFIG_H
