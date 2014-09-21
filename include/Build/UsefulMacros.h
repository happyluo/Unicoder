// **********************************************************************
//
// Copyright (c) 2012-2013 Bernard Luo. All rights reserved.
//
// <Email: luo (dot) xiaowei (at) hotmail (dot) com>
//
// **********************************************************************

#ifndef BUILD_USEFUL_MACRO_H
#define BUILD_USEFUL_MACRO_H

//
// Defines some utility macros.
//

// The GNU compiler emits a warning if nested "if" statements are followed by
// an "else" statement and braces are not used to explicitly disambiguate the
// "else" binding.  This leads to problems with code like:
//
//   if (gate)
//     ASSERT_*(condition) << "Some message";
//
// The "switch (0) case 0:" idiom is used to suppress this.
#ifdef __INTEL_COMPILER
# define AMBIGUOUS_ELSE_BLOCKER
#else
# define AMBIGUOUS_ELSE_BLOCKER switch (0) case 0: default:  // NOLINT
#endif

// Use this annotation at the end of a struct/class definition to
// prevent the compiler from optimizing away instances that are never
// used.  This is useful when all interesting logic happens inside the
// c'tor and / or d'tor.  Example:
//
//   struct Foo {
//     Foo() { ... }
//   } ATTRIBUTE_UNUSED;
//
// Also use it after a variable or parameter declaration to tell the
// compiler the variable/parameter does not have to be used.
#if defined(__GNUC__) && !defined(COMPILER_ICC)
# define ATTRIBUTE_UNUSED __attribute__ ((unused))
#else
# define ATTRIBUTE_UNUSED
#endif

// Tell the compiler to warn about unused return values for functions declared
// with this macro.  The macro should be used on function declarations
// following the argument list:
//
//   Sprocket* AllocateSprocket() MUST_USE_RESULT;
#if defined(__GNUC__) && (GTEST_GCC_VER_ >= 30400) && !defined(COMPILER_ICC)
# define MUST_USE_RESULT __attribute__ ((warn_unused_result))
#else
# define MUST_USE_RESULT
#endif 

#ifdef __GNUC__
// Ask the compiler to never inline a given function.
# define NO_INLINE __attribute__((noinline))
#else
# define NO_INLINE
#endif

// Put this in the private: declarations for a class to be uncopyable.
#define DISALLOW_COPY(TypeName) \
	TypeName(const TypeName&)

// A macro to disallow operator=
// This should be used in the private: declarations for a class.
#define DISALLOW_ASSIGN(TypeName)\
	TypeName& operator=(TypeName const &)

// A macro to disallow copy constructor and operator= functions
// This should be used in the private: declarations for a class.
#define DISALLOW_COPY_AND_ASSIGN(TypeName)\
	TypeName(TypeName const &);\
	DISALLOW_ASSIGN(TypeName)

// A macro to disallow all the implicit constructors, namely the
// default constructor, copy constructor and operator= functions.
//
// This should be used in the private: declarations for a class
// that wants to prevent anyone from instantiating it. This is
// especially useful for classes containing only static methods.
#define DISALLOW_IMPLICIT_CONSTRUCTORS(TypeName) \
	TypeName();                                    \
	DISALLOW_COPY_AND_ASSIGN(TypeName)

//
// calculate data member's offset of struct or class.
// 
#define OffsetOf(Type, member)					\
	reinterpret_cast<size_t>(&reinterpret_cast<Type*>(0)->member)
//
// calculate struct or class's address by data member's address.
// 
#define ContainerOf(memberptr, Type, member)	\
	reinterpret_cast<Type*>(reinterpret_cast<char*>(memberptr) - OffsetOf(Type, member))

#define min_t(Type, x, y) ({			\
	Type __min1 = (x);					\
	Type __min2 = (y);					\
	__min1 < __min2 ? __min1 : __min2; })

#define max_t(Type, x, y) ({			\
	Type __max1 = (x);					\
	Type __max2 = (y);					\
	__max1 > __max2 ? __max1 : __max2; })

//
//  Workaround for the unfortunate min/max macros defined by some platform headers
//
#define PREVENT_MACRO_SUBSTITUTION

#ifndef USING_STD_MIN
#  define USING_STD_MIN() using std::min
#endif

#ifndef USING_STD_MAX
#  define USING_STD_MAX() using std::max
#endif

//
//  NO_STD_MIN_MAX workaround  -----------------------------------------------//
//
#  if defined(NO_STD_MIN_MAX) && defined(__cplusplus)

namespace std {
	template <class Type>
	inline const Type& min PREVENT_MACRO_SUBSTITUTION (const Type& __a, const Type& __b) {
		return __b < __a ? __b : __a;
	}
	template <class Type>
	inline const Type& max PREVENT_MACRO_SUBSTITUTION (const Type& __a, const Type& __b) {
		return  __a < __b ? __b : __a;
	}
}

#  endif

//
// char workaround  -----------------------------------------------//
//
#define IsSpaceChar(X)  isspace(static_cast<unsigned char>(X))
#define IsDigitChar(X)  isdigit(static_cast<unsigned char>(X))
#define IsXDigitChar(X)  isxdigit(static_cast<unsigned char>(X))
#define ToLowerChar(X)  (char)tolower((unsigned char)X)
#define ToUpperChar(X)  (char)toupper((unsigned char)X)

//#define DigitValue(X)  ({								\
//	unsigned char uc(static_cast<unsigned char>(X));	\
//	isdigit(uc) ? uc - '0' : -1;						\
//})
//
//#define XDigitValue(X)  ({							\
//	unsigned char uc(static_cast<unsigned char>(X));	\
//	isxdigit(uc)										\
//	? (isdigit(uc) ? uc - '0' : toupper(uc) - 'A' + 10)	\
//	: -1;												\
//})

// The arraysize(arr) macro returns the # of elements in an array arr.
// The expression is a compile-time constant, and therefore can be
// used in defining new arrays, for example.  If you use arraysize on
// a pointer by mistake, you will get a compile-time error.
//
// One caveat is that arraysize() doesn't accept any array of an
// anonymous type or a type defined inside a function.  In these rare
// cases, you have to use the unsafe ARRAYSIZE_UNSAFE() macro below.  This is
// due to a limitation in C++'s template system.  The limitation might
// eventually be removed, but it hasn't happened yet.

// This template function declaration is used in defining arraysize.
// Note that the function doesn't need an implementation, as we only
// use its type.
template <typename T, size_t N>
char (&ArraySizeHelper(T (&array)[N]))[N];

// That gcc wants both of these prototypes seems mysterious. VC, for
// its part, can't decide which to use (another mystery). Matching of
// template overloads: the final frontier.
#ifndef _MSC_VER
template <typename T, size_t N>
char (&ArraySizeHelper(const T (&array)[N]))[N];
#endif

#define arraysize(array) (sizeof(ArraySizeHelper(array)))

//
// Number of elements in an array
//
// ARRAYSIZE_UNSAFE performs essentially the same calculation as arraysize,
// but can be used on anonymous types or types defined inside
// functions.  It's less safe than arraysize as it accepts some
// (although not all) pointers.  Therefore, you should use arraysize
// whenever possible.
//
// The expression ARRAYSIZE_UNSAFE(a) is a compile-time constant of type
// size_t.
//
// The ARRAYSIZE_UNSAFE(arr) macro returns the # of elements in an array arr.
// The expression is a compile-time constant, and therefore can be
// used in defining new arrays, for example.
//
// ARRAYSIZE_UNSAFE catches a few type errors.  If you see a compiler error
//
//   "warning: division by zero in ..."
//
// when using ARRAYSIZE_UNSAFE, you are (wrongfully) giving it a pointer.
// You should only use ARRAYSIZE_UNSAFE on statically allocated arrays.
//
// The following comments are on the implementation details, and can
// be ignored by the users.
//
// ARRAYSIZE_UNSAFE(arr) works by inspecting sizeof(arr) (the # of bytes in
// the array) and sizeof(*(arr)) (the # of bytes in one array
// element).  If the former is divisible by the latter, perhaps arr is
// indeed an array, in which case the division result is the # of
// elements in the array.  Otherwise, arr cannot possibly be an array,
// and we generate a compiler error to prevent the code from
// compiling.
//
// Since the size of bool is implementation-defined, we need to cast
// !(sizeof(a) & sizeof(*(a))) to size_t in order to ensure the final
// result has type size_t.
//
// This macro is not perfect as it wrongfully accepts certain
// pointers, namely where the pointer size is divisible by the pointee
// size.  Since all our code has to go through a 32-bit compiler,
// where a pointer is 4 bytes, this means all pointers to a type whose
// size is 3 or greater than 4 will be (righteously) rejected.
//
// Kudos to Jorg Brown for this simple and elegant implementation.

#undef ARRAYSIZE_UNSAFE
#define ARRAYSIZE_UNSAFE(array) \
	((sizeof(array) / sizeof(*(array))) / \
	static_cast<size_t>(!(sizeof(array) % sizeof(*(array)))))
//#define ARRAYSIZE_UNSAFE(array)  (size_t)(sizeof(array) / sizeof(array[0]))


// STATIC_CONSTANT workaround --------------------------------------------- //
// On compilers which don't allow in-class initialization of static integral
// constant members, we must use enums as a workaround if we want the constants
// to be available at compile-time. This macro gives us a convenient way to
// declare such constants.

#ifdef NO_INCLASS_MEMBER_INITIALIZATION
#	define STATIC_CONSTANT(type, assignment) enum { assignment }
#else
#	define STATIC_CONSTANT(type, assignment) static const type assignment
#endif

// MSVC "deprecates" snprintf and issues warnings wherever it is used.  In
// order to avoid these warnings, we need to use _snprintf or _snprintf_s on
// MSVC-based platforms.  We map the SNPRINTF macro to the appropriate
// function in order to achieve that.  We use macro definition here because
// snprintf is a variadic function.
#if _MSC_VER >= 1400 && !OS_WINDOWS_MOBILE
//	MSVC 2005 and above support variadic macros.
#	define snprintf(buffer, size, format, ...) \
	_snprintf_s(buffer, size, size, format, __VA_ARGS__)
#elif defined(_MSC_VER)
//	Windows CE does not define _snprintf_s and MSVC prior to 2005 doesn't
//	complain about _snprintf.
#	define snprintf _snprintf
#else
#	define snprintf snprintf
#endif

#ifdef _MSC_VER
#	define strtoll  _strtoi64
#	define strtoull _strtoui64
#elif defined(__DECCXX) && defined(__osf__)
// HP C++ on Tru64 does not have strtoll, but strtol is already 64-bit.
#	define strtoll strtol
#	define strtoull strtoul
#endif

#define ENSURE_THROW(expr, exception) do     \
{                                            \
	if (!(expr))                             \
	{                                        \
		throw exception(__FILE__, __LINE__); \
	}                                        \
} while (0);

#define CONCAT1(X, Y) X##Y
#define CONCAT(X, Y) _LIBCPP_CONCAT1(X, Y)

// It's possible for functions that use a va_list, such as StringPrintf, to
// invalidate the data in it upon use.  The fix is to make a copy of the
// structure before using it and use that copy instead.  va_copy is provided
// for this purpose.  MSVC does not provide va_copy, so define an
// implementation here.  It is not guaranteed that assignment is a copy, so the
// StringUtil.VariableArgsFunc unit test tests this capability.
#if defined(_MSC_VER)
#	define VA_COPY(dest, src) ((dest) = (src))
#else
#	define VA_COPY(dest, src) (va_copy(dest, src))
#endif

// Define an OS-neutral wrapper for shared library entry points
#if defined(_WIN32)
#	define API_CALL __stdcall
#else
#	define API_CALL
#endif

#ifndef __has_feature
#	define __has_feature(expr) 0
#endif

#ifdef _MSC_VER
#	define UTIL_INLINE_VISIBILITY __forceinline
#else // MinGW GCC and Clang
#	define UTIL_INLINE_VISIBILITY __attribute__ ((__always_inline__))
#endif

// The following enum should be used only as a constructor argument to indicate
// that the variable has static storage class, and that the constructor should
// do nothing to its state.  It indicates to the reader that it is legal to
// declare a static instance of the class, provided the constructor is given
// the base::LINKER_INITIALIZED argument.  Normally, it is unsafe to declare a
// static variable that has a constructor or a destructor because invocation
// order is undefined.  However, IF the type can be initialized by filling with
// zeroes (which the loader does for static variables), AND the destructor also
// does nothing to the storage, AND there are no virtual methods, then a
// constructor declared as
//       explicit MyClass(base::LinkerInitialized x) {}
// and invoked as
//       static MyClass my_variable_name(base::LINKER_INITIALIZED);
namespace Util
{
enum LinkerInitialized { LINKER_INITIALIZED };
}  // Util

// Use these to declare and define a static local variable (static T;) so that
// it is leaked so that its destructors are not called at exit. If you need
// thread-safe initialization, use base/lazy_instance.h instead.
#define CR_DEFINE_STATIC_LOCAL(type, name, arguments) \
	static type& name = *new type arguments

#endif