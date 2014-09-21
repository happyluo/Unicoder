// **********************************************************************
//
// Copyright (c) 2010-2014 Bernard Luo. All rights reserved.
//
// <Email: luo (dot) xiaowei (at) hotmail (dot) com>
//
// **********************************************************************

#ifndef UTIL_ERROR_TO_STRING_H
#define UTIL_ERROR_TO_STRING_H

#include <Util/Config.h>
#include <string>

UTIL_BEGIN

//
// Get the error message for the last error code or given error code.
//
UTIL_API std::string LastErrorToString();

#ifdef _WIN32
UTIL_API std::string ErrorToString(int, LPCVOID = NULL);
#else
UTIL_API std::string ErrorToString(int);
#endif

UTIL_END

#endif