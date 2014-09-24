// **********************************************************************
//
// Copyright (c) 2010-2014 Bernard Luo. All rights reserved.
//
// <Email: luo (dot) xiaowei (at) hotmail (dot) com>
//
// **********************************************************************


#include <ostream>
#include <iomanip>
#include <cstdlib>
#include <Util/Exception.h>
#include <Util/MutexPtrLock.h>
#include <Util/Mutex.h>
#include <Util/StringUtil.h>


using namespace std;

namespace UtilInternal
{
    bool DECLSPEC_EXPORT nullHandleAbort = false;
}

namespace
{

Util::Mutex* globalMutex = 0;


class Init
{
public:

    Init()
    {
        globalMutex = new Util::Mutex;
    }

    ~Init()
    {
        delete globalMutex;
        globalMutex = 0;
    }
};

Init init;

}

Util::Exception::Exception() :
    m_fileName(0),
    m_line(0)
{
}

Util::Exception::Exception(const char* file, int line) :
    m_fileName(file),
    m_line(line)
{
}

Util::Exception::~Exception() throw()
{
}

const char* Util::Exception::ms_pcName = "Util::Exception";

string
Util::Exception::Name() const
{
    return ms_pcName;
}

void
Util::Exception::Print(ostream &out) const
{
    if (m_fileName && m_line > 0)
    {
        out << m_fileName << ':' << m_line << ": ";
    }
    out << Name();
}

const char*
Util::Exception::what() const throw()
{
    try
    {
        Util::MutexPtrLock<Util::Mutex> lock(globalMutex);
        {
            if (m_strWhat.empty())
            {
                stringstream strStream;
                Print(strStream);
                m_strWhat = strStream.str();        // Lazy initialization.
            }
        }

        return m_strWhat.c_str();
    }
    catch(...)
    {
    }
    return "";
}

Util::Exception*
Util::Exception::Clone() const
{
    return new Exception(*this);
}

void
Util::Exception::Throw() const
{
    throw *this;
}

const char*
Util::Exception::File() const
{
    return m_fileName;
}

int
Util::Exception::Line() const
{
    return m_line;
}

std::ostream&
Util::operator<<(std::ostream &out, const Util::Exception &ex)
{
    ex.Print(out);
    return out;
}

//////////////////////////////////////////////////////////////////////////
/// NullSharedPtrException
Util::NullSharedPtrException::NullSharedPtrException(const char *file, int line) :
    Exception(file, line)
{
    if(UtilInternal::nullHandleAbort)
    {
        abort();
    }
}

Util::NullSharedPtrException::~NullSharedPtrException() throw()
{
}

const char* Util::NullSharedPtrException::ms_pcName = "Util::NullSharedPtrException";

string
Util::NullSharedPtrException::Name() const
{
    return ms_pcName;
}

Util::Exception*
Util::NullSharedPtrException::Clone() const
{
    return new NullSharedPtrException(*this);
}

void
Util::NullSharedPtrException::Throw() const
{
    throw *this;
}

//////////////////////////////////////////////////////////////////////////
/// IllegalArgumentException
Util::IllegalArgumentException::IllegalArgumentException(const char *file, int line) :
    Exception(file, line)
{
}

Util::IllegalArgumentException::IllegalArgumentException(
    const char *file, int line, const std::string& reason) :
    Exception(file, line), m_reason(reason)
{
}

Util::IllegalArgumentException::~IllegalArgumentException() throw()
{
}

const char* Util::IllegalArgumentException::ms_pcName = "Util::IllegalArgumentException";

std::string
Util::IllegalArgumentException::Name() const
{
    return ms_pcName;
}

void 
Util::IllegalArgumentException::Print(std::ostream& out) const
{
    Exception::Print(out);
    out << ": " << m_reason;
}

Util::Exception*
Util::IllegalArgumentException::Clone() const
{
    return new IllegalArgumentException(*this);
}

void
Util::IllegalArgumentException::Throw() const
{
    throw *this;
}

const std::string&
Util::IllegalArgumentException::Reason() const
{
    return m_reason;
}

//////////////////////////////////////////////////////////////////////////
/// SyscallException
Util::SyscallException::SyscallException(const char *file, int line, int syscallError) :
    Exception(file, line), m_errorCode(syscallError)
{
}

Util::SyscallException::~SyscallException() throw()
{
}

const char* Util::SyscallException::ms_pcName = "Util::SyscallException";

std::string
Util::SyscallException::Name() const
{
    return ms_pcName;
}

void 
Util::SyscallException::Print(std::ostream& out) const
{
    Exception::Print(out);
    if (0 != m_errorCode)
    {
        out << "\nsystem call exception: " << Util::ErrorToString(m_errorCode);
    }
}

Util::Exception*
Util::SyscallException::Clone() const
{
    return new SyscallException(*this);
}

void
Util::SyscallException::Throw() const
{
    throw *this;
}

int 
Util::SyscallException::Error() const
{
    return m_errorCode;
}

//////////////////////////////////////////////////////////////////////////
/// FatalException
Util::FatalException::FatalException(
    const char *file, int line, const std::string& reason) :
    Exception(file, line), m_reason(reason)
{
}

Util::FatalException::~FatalException() throw()
{
}

const char* Util::FatalException::ms_pcName = "Util::FatalException";

string
Util::FatalException::Name() const
{
    return ms_pcName;
}

void
Util::FatalException::Print(ostream& out) const
{
    Exception::Print(out);
    out << ": " << m_reason;
}

Util::FatalException*
Util::FatalException::Clone() const
{
    return new FatalException(*this);
}

void
Util::FatalException::Throw() const
{
    throw *this;
}

const std::string&
Util::FatalException::Reason() const
{
    return m_reason;
}

