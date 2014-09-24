// **********************************************************************
//
// Copyright (c) 2010-2014 Bernard Luo. All rights reserved.
//
// <Email: luo (dot) xiaowei (at) hotmail (dot) com>
//
// **********************************************************************

#ifndef UTIL_EXCEPTION_H
#define UTIL_EXCEPTION_H

#include <string>
#include <exception>
#include <sstream>
#include <Util/Config.h>
#include <Util/Exception.h>

namespace Util
{

class UTIL_API Exception : public std::exception
{
public:
    Exception();
    Exception(const char* file, int line);

    virtual ~Exception() throw();

    virtual std::string Name() const;

    virtual void Print(std::ostream& out) const;

    virtual const char* what() const throw();

    virtual Exception* Clone() const;

    virtual void Throw() const;

    const char* File() const;

    int Line() const;

private:

    const char            *m_fileName;
    int                    m_line;
    static const char    *ms_pcName;
    mutable std::string    m_strWhat;        // Initialized lazily in what().
};

UTIL_API std::ostream& operator << (std::ostream &out, const Util::Exception &ex);

/// 空共享指针异常类
class UTIL_API NullSharedPtrException : public Exception
{
public:
    NullSharedPtrException(const char* file, int line);

    virtual ~NullSharedPtrException() throw();

    virtual std::string Name() const;

    virtual Exception* Clone() const;

    virtual void Throw() const;
    
private:
    
    static const char *ms_pcName;
};
    
class UTIL_API IllegalArgumentException : public Exception
{
public:
    IllegalArgumentException(const char* file, int line);

    IllegalArgumentException(const char* file, int line, const std::string& reason);

    virtual ~IllegalArgumentException() throw();

    virtual std::string Name() const;

    virtual void Print(std::ostream& out) const;

    virtual Exception* Clone() const;

    virtual void Throw() const;

    const std::string& Reason() const;

private:

    std::string            m_reason;

    static const char    *ms_pcName;
};

/// 系统调用异常类
class UTIL_API SyscallException : public Exception
{
public:
    SyscallException(const char* file, int line, int syscallError);

    virtual ~SyscallException() throw();

    virtual std::string Name() const;

    virtual void Print(std::ostream& out) const;

    virtual Exception* Clone() const;

    virtual void Throw() const;

    int Error() const;

private:

    const int            m_errorCode;
    static const char    *ms_pcName;
};

//////////////////////////////////////////////////////////////////////////
/// FatalException
class UTIL_API FatalException : public Exception
{
public:
    FatalException(const char* file, int line, const std::string& message);
    virtual ~FatalException() throw();

    virtual std::string Name() const;
    virtual void Print(std::ostream& out) const;
    virtual FatalException* Clone() const;
    virtual void Throw() const;

    const std::string& Reason() const;

private:
    std::string m_reason;

    static const char* ms_pcName;    
};

}

#endif
