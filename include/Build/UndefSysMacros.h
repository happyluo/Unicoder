// **********************************************************************
//
// Copyright (c) 2012-2013 Bernard Luo. All rights reserved.
//
// <Email: luo (dot) xiaowei (at) hotmail (dot) com>
//
// **********************************************************************

#ifndef BUILD_UNDEF_SYS_MACROS_H
#define BUILD_UNDEF_SYS_MACROS_H

//
// This header includes macros that can end up being dragged into
// the generated code from system headers, such as major() or NDEBUG.
// If a Slice symbol has the same name as a macro, the generated
// code most likely won't compile (but, depending how the macro is
// defined, may even compile).
//
// Here, we undefine symbols that cause such problems.
//
// The #ifdef ... #endif protection is necessary to prevent
// warnings on some platforms.
//

#ifdef major
#    undef major
#endif

#ifdef minor
#    undef minor
#endif

//#ifdef min
//#    undef min
//#endif
//
//#ifdef max
//#    undef max
//#endif

#ifdef min
#    if defined(_MSC_VER) && ! defined(__clang__)
    UTIL_WARNING("macro min is incompatible with C++.  Try #define NOMINMAX "
                "before any Windows header. #undefing min")
#    else
#    warning: macro min is incompatible with C++.  #undefing min
#    endif

#    undef min
#endif

#ifdef max
#    if defined(_MSC_VER) && ! defined(__clang__)
    UTIL_WARNING("macro max is incompatible with C++.  Try #define NOMINMAX "
                "before any Windows header. #undefing max")
#    else
#    warning: macro max is incompatible with C++.  #undefing max
#    endif

#    undef max
#endif

#ifdef Yield
#    undef Yield
#endif

//#ifdef ERROR
//#    undef ERROR
//#endif

#endif
