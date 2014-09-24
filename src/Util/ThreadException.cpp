// **********************************************************************
//
// Copyright (c) 2010-2014 Bernard Luo. All rights reserved.
//
// <Email: luo (dot) xiaowei (at) hotmail (dot) com>
//
// **********************************************************************

#include <Util/ThreadException.h>


//////////////////////////////////////////////////////////////////////////
/// ThreadSyscallException
Util::ThreadSyscallException::ThreadSyscallException(const char *file, int line, int syscallError) :
    SyscallException(file, line, syscallError)
{
}

Util::ThreadSyscallException::~ThreadSyscallException() throw()
{
}

const char* Util::ThreadSyscallException::ms_pcName = "Util::ThreadSyscallException";

std::string
Util::ThreadSyscallException::Name() const
{
    return ms_pcName;
}

Util::Exception*
Util::ThreadSyscallException::Clone() const
{
    return new ThreadSyscallException(*this);
}

void
Util::ThreadSyscallException::Throw() const
{
    throw *this;
}

//////////////////////////////////////////////////////////////////////////
/// ThreadLockedException
Util::ThreadLockedException::ThreadLockedException(const char *file, int line) :
    Exception(file, line)
{
}

Util::ThreadLockedException::~ThreadLockedException() throw()
{
}

const char* Util::ThreadLockedException::ms_pcName = "Util::ThreadLockedException";

std::string
Util::ThreadLockedException::Name() const
{
    return ms_pcName;
}

Util::Exception*
Util::ThreadLockedException::Clone() const
{
    return new ThreadLockedException(*this);
}

void
Util::ThreadLockedException::Throw() const
{
    throw *this;
}

//////////////////////////////////////////////////////////////////////////
/// class ThreadStartedException
Util::ThreadStartedException::ThreadStartedException(const char *file, int line) :
    Exception(file, line)
{
}

Util::ThreadStartedException::~ThreadStartedException() throw()
{
}

const char* Util::ThreadStartedException::ms_pcName = "Util::ThreadStartedException";

std::string
Util::ThreadStartedException::Name() const
{
    return ms_pcName;
}

Util::Exception*
Util::ThreadStartedException::Clone() const
{
    return new ThreadStartedException(*this);
}

void
Util::ThreadStartedException::Throw() const
{
    throw *this;
}

//////////////////////////////////////////////////////////////////////////
/// class ThreadNotStartedException
Util::ThreadNotStartedException::ThreadNotStartedException(const char *file, int line) :
    Exception(file, line)
{
}

Util::ThreadNotStartedException::~ThreadNotStartedException() throw()
{
}

const char* Util::ThreadNotStartedException::ms_pcName = "Util::ThreadNotStartedException";

std::string
Util::ThreadNotStartedException::Name() const
{
    return ms_pcName;
}

Util::Exception*
Util::ThreadNotStartedException::Clone() const
{
    return new ThreadNotStartedException(*this);
}

void
Util::ThreadNotStartedException::Throw() const
{
    throw *this;
}

//////////////////////////////////////////////////////////////////////////
/// BadThreadControlException
Util::BadThreadControlException::BadThreadControlException(const char *file, int line) :
    Exception(file, line)
{
}

Util::BadThreadControlException::~BadThreadControlException() throw()
{
}

const char* Util::BadThreadControlException::ms_pcName = "Util::BadThreadControlException";

std::string
Util::BadThreadControlException::Name() const
{
    return ms_pcName;
}

Util::Exception*
Util::BadThreadControlException::Clone() const
{
    return new BadThreadControlException(*this);
}

void
Util::BadThreadControlException::Throw() const
{
    throw *this;
}

//////////////////////////////////////////////////////////////////////////
/// class InvalidTimeoutException
Util::InvalidTimeoutException::InvalidTimeoutException(
    const char *file, int line, const Time& timeout) :
    Exception(file, line), m_uSec(timeout)
{
}

Util::InvalidTimeoutException::~InvalidTimeoutException() throw()
{
}

const char* Util::InvalidTimeoutException::ms_pcName = "Util::InvalidTimeoutException";

std::string
Util::InvalidTimeoutException::Name() const
{
    return ms_pcName;
}

void 
Util::InvalidTimeoutException::Print(std::ostream& out) const
{
    Exception::Print(out);
    out << ":\ninvalid timeout: " << m_uSec << " seconds";
}

Util::Exception*
Util::InvalidTimeoutException::Clone() const
{
    return new InvalidTimeoutException(*this);
}

void
Util::InvalidTimeoutException::Throw() const
{
    throw *this;
}

//////////////////////////////////////////////////////////////////////////
/// class DeadlockException
Util::DeadlockException::DeadlockException(
    const char *file, int line) :
    Exception(file, line)
{
}

Util::DeadlockException::~DeadlockException() throw()
{
}

const char* Util::DeadlockException::ms_pcName = "Util::DeadlockException";

std::string
Util::DeadlockException::Name() const
{
    return ms_pcName;
}

Util::Exception*
Util::DeadlockException::Clone() const
{
    return new DeadlockException(*this);
}

void
Util::DeadlockException::Throw() const
{
    throw *this;
}

//////////////////////////////////////////////////////////////////////////
/// class ThreadPoolDestroyedException
Util::ThreadPoolDestroyedException::ThreadPoolDestroyedException(
    const char *file, int line) :
    Exception(file, line)
{
}

Util::ThreadPoolDestroyedException::~ThreadPoolDestroyedException() throw()
{
}

const char* Util::ThreadPoolDestroyedException::ms_pcName = "Util::ThreadPoolDestroyedException";

std::string
Util::ThreadPoolDestroyedException::Name() const
{
    return ms_pcName;
}

Util::Exception*
Util::ThreadPoolDestroyedException::Clone() const
{
    return new ThreadPoolDestroyedException(*this);
}

void
Util::ThreadPoolDestroyedException::Throw() const
{
    throw *this;
}