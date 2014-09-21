// **********************************************************************
//
// Copyright (c) 2010-2014 Bernard Luo. All rights reserved.
//
// <Email: luo (dot) xiaowei (at) hotmail (dot) com>
//
// **********************************************************************

#if defined(_MSC_VER) && _MSC_VER >= 1700
//
// DbgHelp.dll on Windows XP does not contain Unicode functions, so we 
// "switch on" Unicode only with VS2012 and up
//
#  ifndef UNICODE
#    define UNICODE
#  endif
#  ifndef _UNICODE
#    define _UNICODE
#  endif
#endif

#include <ostream>
#include <iomanip>
#include <cstdlib>
#include <Util/Exception.h>
#include <Util/MutexPtrLock.h>
#include <Util/Mutex.h>
#include <Util/StringUtil.h>

#if defined(__GNUC__) && !defined(__sun) && !defined(__FreeBSD__) && !defined(__MINGW32__)
#  include <execinfo.h>
#  include <cxxabi.h>
#  define UTIL_STACK_TRACES
#  define UTIL_GCC_STACK_TRACES
#endif

#ifdef UTIL_WIN32_STACK_TRACES
#  if defined(_MSC_VER) && _MSC_VER >= 1700
#    define DBGHELP_TRANSLATE_TCHAR
#    include <Util/Unicode.h>
#  endif
#  include <DbgHelp.h>
#  include <tchar.h>
#  define UTIL_STACK_TRACES
#endif


using namespace std;

namespace UtilInternal
{

	bool DECLSPEC_EXPORT printStackTraces = false;
	bool DECLSPEC_EXPORT nullHandleAbort = false;

}

namespace Util
{

namespace Ex
{
	void ThrowMemoryLimitException(const char* file, int line, size_t requested, size_t maximum)
	{
		ostringstream s;
		s << "requested " << requested << " bytes, maximum allowed is " << maximum << " bytes (see Util.MessageSizeMax)";
		throw Util::MemoryLimitException(file, line, s.str());
	}

	void ThrowMarshalException(const char* file, int line, const string& reason)
	{
		throw Util::MarshalException(file, line, reason);
	}

}
}

namespace
{

Util::Mutex* globalMutex = 0;

#ifdef UTIL_WIN32_STACK_TRACES
HANDLE process = 0;
#endif

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
#ifdef UTIL_WIN32_STACK_TRACES
		if (process != 0)
		{
			SymCleanup(process);
			process = 0;
		}
#endif
	}
};

Init init;

#ifdef UTIL_STACK_TRACES
string GetStackTrace()
{
	if (!UtilInternal::printStackTraces)
	{
		return "";
	}

	string stackTrace;

#  ifdef UTIL_WIN32_STACK_TRACES
	//
	// Note: the Sym functions are not thread-safe
	//
	Util::MutexPtrLock<Util::Mutex> lock(globalMutex);
	if (process == 0)
	{

		//
		// Compute Search path (best effort)
		// consists of the current working directory, this DLL (or exe) directory and %_NT_SYMBOL_PATH% 
		//
		basic_string<TCHAR> searchPath;
		const TCHAR pathSeparator = _T('\\');
		const TCHAR searchPathSeparator = _T(';');

		TCHAR cwd[MAX_PATH];
		if (GetCurrentDirectory(MAX_PATH, cwd) != 0)
		{
			searchPath = cwd;
		}

		HMODULE myModule = 0;
		GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
			"startHook", &myModule);
		//
		// If GetModuleHandleEx fails, myModule is NULL, i.e. we'll locate the current exe's directory.
		//

		TCHAR myFilename[MAX_PATH];
		DWORD len = GetModuleFileName(myModule, myFilename, MAX_PATH);
		if (len != 0 && len < MAX_PATH)
		{
			assert(myFilename[len] == 0);

			basic_string<TCHAR> myPath = myFilename;
			size_t pos = myPath.find_last_of(pathSeparator);
			if (pos != basic_string<TCHAR>::npos)
			{
				myPath = myPath.substr(0, pos);

				if (!searchPath.empty())
				{
					searchPath += searchPathSeparator;
				}
				searchPath += myPath;
			}
		}

		const DWORD size = 1024;
		TCHAR symbolPath[size];
		len = GetEnvironmentVariable(_T("_NT_SYMBOL_PATH"), symbolPath, size);
		if (len > 0 && len < size)
		{
			if (!searchPath.empty())
			{
				searchPath += searchPathSeparator;
			}
			searchPath += symbolPath;
		}

		process = GetCurrentProcess();

		SymSetOptions(SYMOPT_LOAD_LINES | SYMOPT_DEFERRED_LOADS | SYMOPT_EXACT_SYMBOLS | SYMOPT_UNDNAME);
		if (SymInitialize(process, searchPath.c_str(), TRUE) == 0)
		{
			process = 0;
			return "No stack trace: SymInitialize failed with " + UtilInternal::ErrorToString(GetLastError());
		}
	}
	lock.Release();

	const int stackSize = 61;
	void* stack[stackSize];

	//
	// 1: skip the first frame (the call to getStackTrace)
	// 1 + stackSize < 63 on XP according to the documentation for CaptureStackBackTrace
	//
	USHORT frames = CaptureStackBackTrace(1, stackSize, stack, 0);

	if (frames > 0)
	{
#if defined(_MSC_VER) && (_MSC_VER >= 1600)
#   if defined(DBGHELP_TRANSLATE_TCHAR) 
		static_assert(sizeof(TCHAR) == sizeof(wchar_t), "Bad TCHAR - should be wchar_t");
#   else
		static_assert(sizeof(TCHAR) == sizeof(char), "Bad TCHAR - should be char");
#  endif
#endif

		char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];

		SYMBOL_INFO* symbol = reinterpret_cast<SYMBOL_INFO*>(buffer);
		symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
		symbol->MaxNameLen = MAX_SYM_NAME;

		IMAGEHLP_LINE64 line = {};
		line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
		DWORD displacement = 0;

		lock.Acquire();

		// TODO: call SymRefreshModuleList here? (not available on XP)

		for (int i = 0; i < frames; i++)
		{
			if (!stackTrace.empty())
			{
				stackTrace += "\n";
			}

			stringstream s;
			s << setw(3) << i << " ";

			DWORD64 address = reinterpret_cast<DWORD64>(stack[i]);

			BOOL ok = SymFromAddr(process, address, 0, symbol);
			if (ok)
			{
#ifdef DBGHELP_TRANSLATE_TCHAR
				s << Util::WstringToString(symbol->Name);
#else
				s << symbol->Name;
#endif
				ok = SymGetLineFromAddr64(process, address, &displacement, &line);
				if (ok)
				{
					s << " at line " << line.LineNumber << " in " 
#ifdef DBGHELP_TRANSLATE_TCHAR                 
						<< Util::WstringToString(line.FileName);
#else
						<< line.FileName;
#endif
				}
			}
			else
			{
				s << hex << "0x" << address;
			}
			stackTrace += s.str();
		}
		lock.Release();
	}

#   elif defined(UTIL_GCC_STACK_TRACES)    

	const size_t maxDepth = 100;
	void *stackAddrs[maxDepth];

	size_t stackDepth = backtrace(stackAddrs, maxDepth);
	char **stackStrings = backtrace_symbols(stackAddrs, stackDepth);

	//
	// Start at 1 to skip the top frame (== call to this function)
	//
	for (size_t i = 1; i < stackDepth; i++)
	{
		string line(stackStrings[i]);

		if (i > 1)
		{
			stackTrace += "\n";
		}

		stringstream s;
		s << setw(3) << i - 1 << " ";

		//
		// For each line attempt to parse the mangled function name as well
		// as the source library/executable.
		//
		string mangled;
		string source;
		string::size_type openParen = line.find_first_of('(');
		if (openParen != string::npos)
		{
			//
			// Format: "/opt/Ice/lib/libUtil.so.33(_ZN7Util9ExceptionC2EPKci+0x51) [0x73b267]"
			//
			string::size_type closeParen = line.find_first_of(')', openParen);
			if (closeParen != string::npos)
			{
				string tmp = line.substr(openParen + 1, closeParen - openParen - 1);
				string::size_type plus = tmp.find_last_of('+');
				if (plus != string::npos)
				{
					mangled = tmp.substr(0 , plus);

					source = line.substr(0, openParen); 
				}
			}
		}
		else
		{
			//
			// Format: "1    libIce.3.3.1.dylib   0x000933a1 _ZN7Util9ExceptionC2EPKci + 71"
			//
			string::size_type plus = line.find_last_of('+');
			if (plus != string::npos)
			{
				string tmp = line.substr(0, plus - 1);
				string::size_type space = tmp.find_last_of(" \t");
				if (space != string::npos)
				{
					tmp = tmp.substr(space + 1, tmp.size() - space);

					string::size_type start = line.find_first_not_of(" \t", 3);
					if (start != string::npos)
					{
						string::size_type finish = line.find_first_of(" \t", start);
						if (finish != string::npos)
						{
							mangled = tmp;

							source = line.substr(start, finish - start);
						}
					}
				}
			}
		}
		if (mangled.size() != 0)
		{
			//
			// Unmangle the function name
			//
			int status;
			char* unmangled = abi::__cxa_demangle(mangled.c_str(), 0, 0, &status);
			if (unmangled)
			{
				s << unmangled;
				free(unmangled);
			}
			else
			{
				s << mangled << "()";
			}

			if (!source.empty())
			{
				s << " in " << source;
			}
		}
		else
		{
			s << line;
		}

		stackTrace += s.str();

	}
	free(stackStrings);

#   endif

	return stackTrace;
}
#endif

}

Util::Exception::Exception() :
	m_fileName(0),
	m_line(0)
#ifdef UTIL_STACK_TRACES
	, m_stackTrace(GetStackTrace())
#endif
{
}

Util::Exception::Exception(const char* file, int line) :
	m_fileName(file),
	m_line(line)
#ifdef UTIL_STACK_TRACES
	, m_stackTrace(GetStackTrace())
#endif
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
				m_strWhat = strStream.str();		// Lazy initialization.
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

const std::string& 
Util::Exception::StackTrace() const
{
	return m_stackTrace;
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
		//out << ":\nsystem call exception: " << strerror(m_errorCode) << "(error code: " << m_errorCode << ")";
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
/// FileException
Util::FileException::FileException(const char *file, int line, int err, const string& path) :
	SyscallException(file, line, err), m_path(path)
{
}

Util::FileException::~FileException() throw()
{
}

const char* Util::FileException::ms_pcName = "Util::FileException";

std::string
Util::FileException::Name() const
{
	return ms_pcName;
}

void 
Util::FileException::Print(std::ostream& out) const
{
	SyscallException::Print(out);
	out << ":\ncould not open file: `" << m_path << "'"; 
}

Util::FileException*
Util::FileException::Clone() const
{
	return new FileException(*this);
}

void
Util::FileException::Throw() const
{
	throw *this;
}

const std::string& 
Util::FileException::Path() const
{
	return m_path;
}

//////////////////////////////////////////////////////////////////////////
/// FileLockException
Util::FileLockException::FileLockException(const char *file, int line, int err, const string& path) :
	Exception(file, line), m_errorCode(err), m_path(path)
{
}

Util::FileLockException::~FileLockException() throw()
{
}

const char* Util::FileLockException::ms_pcName = "Util::FileLockException";

std::string
Util::FileLockException::Name() const
{
	return ms_pcName;
}

void 
Util::FileLockException::Print(std::ostream& out) const
{
	Exception::Print(out);
	out << ":\ncould not lock file: `" << m_path << "'"; 
	if (0 != m_errorCode)
	{
		out << "\nsystem call exception: " << Util::ErrorToString(m_errorCode);
		//out << ":\nsystem call exception: " << strerror(m_errorCode) << "(error code: " << m_errorCode << ")";
	}
}

Util::FileLockException*
Util::FileLockException::Clone() const
{
	return new FileLockException(*this);
}

void
Util::FileLockException::Throw() const
{
	throw *this;
}

const std::string& 
Util::FileLockException::Path() const
{
	return m_path;
}

int 
Util::FileLockException::Error() const
{
	return m_errorCode;
}

//////////////////////////////////////////////////////////////////////////
/// OptionalNotSetException
Util::OptionalNotSetException::OptionalNotSetException(const char* file, int line) :
    Exception(file, line)
{
    if(UtilInternal::nullHandleAbort)
    {
        abort();
    }
}

Util::OptionalNotSetException::~OptionalNotSetException() throw()
{
}

const char* Util::OptionalNotSetException::ms_pcName = "Util::OptionalNotSetException";

string
Util::OptionalNotSetException::Name() const
{
    return ms_pcName;
}

Util::OptionalNotSetException*
Util::OptionalNotSetException::Clone() const
{
    return new OptionalNotSetException(*this);
}

void
Util::OptionalNotSetException::Throw() const
{
    throw *this;
}

//////////////////////////////////////////////////////////////////////////
/// InitializationException
Util::InitializationException::InitializationException(const char *file, int line) :
Exception(file, line)
{
}

Util::InitializationException::InitializationException(
	const char *file, int line, const std::string& reason) :
Exception(file, line), m_reason(reason)
{
}
const char* Util::InitializationException::ms_pcName = "Util::InitializationException";

string
Util::InitializationException::Name() const
{
	return ms_pcName;
}

void
Util::InitializationException::Print(ostream& out) const
{
	Exception::Print(out);
	out << ": " << m_reason;
}

Util::InitializationException*
Util::InitializationException::Clone() const
{
	return new InitializationException(*this);
}

void
Util::InitializationException::Throw() const
{
	throw *this;
}

const std::string&
Util::InitializationException::Reason() const
{
	return m_reason;
}

//////////////////////////////////////////////////////////////////////////
/// VersionMismatchException
Util::VersionMismatchException::VersionMismatchException(const char *file, int line) :
	Exception(file, line)
{
}

Util::VersionMismatchException::~VersionMismatchException() throw()
{
}

const char* Util::VersionMismatchException::ms_pcName = "Util::VersionMismatchException";

string
Util::VersionMismatchException::Name() const
{
	return ms_pcName;
}

Util::VersionMismatchException*
Util::VersionMismatchException::Clone() const
{
	return new VersionMismatchException(*this);
}

void
Util::VersionMismatchException::Throw() const
{
	throw *this;
}

//////////////////////////////////////////////////////////////////////////
/// class MarshalException
Util::MarshalException::MarshalException(const char *file, int line) :
	Exception(file, line)
{
}

Util::MarshalException::MarshalException(const char* file, int line, const ::std::string& reason) :
	Exception(file, line)
	, m_reason(reason)
{
}

Util::MarshalException::~MarshalException() throw()
{
}

const char* Util::MarshalException::ms_pcName = "Util::MarshalException";

::std::string
Util::MarshalException::Name() const
{
	return ms_pcName;
}

void
Util::MarshalException::Print(ostream& out) const
{
	Exception::Print(out);
	out << ":\nprotocol error: error during marshaling or unmarshaling";
	if(!m_reason.empty())
	{
		out << ":\n" << m_reason;
	}
}

::Util::Exception*
Util::MarshalException::Clone() const
{
	return new MarshalException(*this);
}

void
Util::MarshalException::Throw() const
{
	throw *this;
}

//////////////////////////////////////////////////////////////////////////
/// UnmarshalOutOfBoundsException
Util::UnmarshalOutOfBoundsException::UnmarshalOutOfBoundsException(const char *file, int line) :
	::Util::MarshalException(file, line)
{
}

Util::UnmarshalOutOfBoundsException::UnmarshalOutOfBoundsException(const char *file, int line, const ::std::string& reason) :
	::Util::MarshalException(file, line, reason)
{
}

Util::UnmarshalOutOfBoundsException::~UnmarshalOutOfBoundsException() throw()
{
}

const char* Util::UnmarshalOutOfBoundsException::ms_pcName = "Util::UnmarshalOutOfBoundsException";

::std::string
Util::UnmarshalOutOfBoundsException::Name() const
{
	return ms_pcName;
}

void
Util::UnmarshalOutOfBoundsException::Print(ostream& out) const
{
	Exception::Print(out);
	out << ":\nprotocol error: out of bounds during unmarshaling";
	if(!m_reason.empty())
	{
		out << ":\n" << m_reason;
	}
}

::Util::Exception*
Util::UnmarshalOutOfBoundsException::Clone() const
{
	return new UnmarshalOutOfBoundsException(*this);
}

void
Util::UnmarshalOutOfBoundsException::Throw() const
{
	throw *this;
}

//////////////////////////////////////////////////////////////////////////
/// class MemoryLimitException
Util::MemoryLimitException::MemoryLimitException(const char *file, int line) :
	::Util::MarshalException(file, line)
{
}

Util::MemoryLimitException::MemoryLimitException(const char *file, int line, const ::std::string& reason) :
	::Util::MarshalException(file, line, reason)
{
}

Util::MemoryLimitException::~MemoryLimitException() throw()
{
}

const char* Util::MemoryLimitException::ms_pcName = "Util::MemoryLimitException";

::std::string
Util::MemoryLimitException::Name() const
{
	return ms_pcName;
}

void
Util::MemoryLimitException::Print(ostream& out) const
{
	Exception::Print(out);
	out << ":\nprotocol error: memory limit exceeded";
	if(!m_reason.empty())
	{
		out << ":\n" << m_reason;
	}
}

::Util::Exception*
Util::MemoryLimitException::Clone() const
{
	return new MemoryLimitException(*this);
}

void
Util::MemoryLimitException::Throw() const
{
	throw *this;
}

//////////////////////////////////////////////////////////////////////////
/// class TrackableCallbackListClearedException
Util::TrackableCallbackListClearedException::TrackableCallbackListClearedException(const char *file, int line) :
	Exception(file, line)
{
}

Util::TrackableCallbackListClearedException::~TrackableCallbackListClearedException() throw()
{
}

const char* Util::TrackableCallbackListClearedException::ms_pcName = "Util::TrackableCallbackListClearedException";

::std::string
Util::TrackableCallbackListClearedException::Name() const
{
	return ms_pcName;
}

::Util::Exception*
Util::TrackableCallbackListClearedException::Clone() const
{
	return new TrackableCallbackListClearedException(*this);
}

void
Util::TrackableCallbackListClearedException::Throw() const
{
	throw *this;
}

//////////////////////////////////////////////////////////////////////////
/// RegexException
Util::RegexException::RegexException(const char *file, int line) :
Exception(file, line)
{
}

Util::RegexException::RegexException(
	const char *file, int line, const std::string& reason) :
	Exception(file, line), m_reason(reason)
{
}

const char* Util::RegexException::ms_pcName = "Util::RegexException";

string
Util::RegexException::Name() const
{
	return ms_pcName;
}

void
Util::RegexException::Print(ostream& out) const
{
	Exception::Print(out);
	out << ": " << m_reason;
}

Util::RegexException*
Util::RegexException::Clone() const
{
	return new RegexException(*this);
}

void
Util::RegexException::Throw() const
{
	throw *this;
}

const std::string&
Util::RegexException::Reason() const
{
	return m_reason;
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

