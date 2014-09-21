// **********************************************************************
//
// Copyright (c) 2010-2014 Bernard Luo. All rights reserved.
//
// <Email: luo (dot) xiaowei (at) hotmail (dot) com>
//
// **********************************************************************

#ifndef UTIL_EXCEPTION_H
#define UTIL_EXCEPTION_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <string>
#include <exception>
#include <sstream>
#include <Util/Config.h>
#include <Util/Exception.h>

namespace Util
{

namespace Ex
{
	UTIL_API void ThrowMemoryLimitException(const char*, int, size_t, size_t);
	UTIL_API void ThrowMarshalException(const char*, int, const std::string&);
}

/// �쳣�����
class UTIL_API Exception : public std::exception
{
public:
	Exception();

	///���캯��
	///@param[in]		file		�쳣���������ļ�
	///@param[in]		line	�쳣����������
	Exception(const char* file, int line);

	///��������
	virtual ~Exception() throw();

	///��ȡ�쳣������
	///return �쳣������
	virtual std::string Name() const;

	///����쳣��Ϣ
	///@param[out]	out	�����
	virtual void Print(std::ostream& out) const;

	// return pointer to message string
	///��ȡ�쳣��Ϣ
	///return �쳣������ԭ��
	virtual const char* what() const throw();

	///���ڴ��д�����ǰ������µĸ���
	///return		�´����ĸ�������Ӧ��ָ�룬ע�⣺�����߸����ͷ��ڴ�
	virtual Exception* Clone() const;

	///���쳣�׳�
	virtual void Throw() const;

	///��ȡ�쳣���������ļ�
	const char* File() const;

	///��ȡ�쳣����������
	int Line() const;

	const std::string& StackTrace() const;

private:

	///�쳣���������ļ�
	const char			*m_fileName;

	///�쳣����������
	int					m_line;

	///�쳣������
	static const char	*ms_pcName;

	const std::string	m_stackTrace;

	///�쳣��Ϣ
	mutable std::string	m_strWhat;		// Initialized lazily in what().
};

///������������������ڽ��쳣��Ϣ���
UTIL_API std::ostream& operator << (std::ostream &out, const Util::Exception &ex);

/// �չ���ָ���쳣��
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

	std::string			m_reason;

	static const char	*ms_pcName;
};

/// ϵͳ�����쳣��
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

	const int			m_errorCode;
	static const char	*ms_pcName;
};

//////////////////////////////////////////////////////////////////////////
/// FileException
class UTIL_API FileException : public SyscallException
{
public:
	FileException(const char* file, int line, int syscallError, const std::string& path);

	virtual ~FileException() throw();

	virtual std::string Name() const;

	virtual void Print(std::ostream& out) const;

	virtual FileException* Clone() const;

	virtual void Throw() const;

	const std::string& Path() const;

private:

	static const char *ms_pcName;
	std::string m_path;
};


//////////////////////////////////////////////////////////////////////////
/// FileLockException
class UTIL_API FileLockException : public Exception
{
public:
	FileLockException(const char* file, int line, int syscallError, const std::string& path);

	virtual ~FileLockException() throw();

	virtual std::string Name() const;

	virtual void Print(std::ostream& out) const;

	virtual FileLockException* Clone() const;

	virtual void Throw() const;

	const std::string& Path() const;

	int Error() const;

private:

	const int m_errorCode;
	static const char *ms_pcName;
	std::string m_path;
};

//////////////////////////////////////////////////////////////////////////
/// OptionalNotSetException
class UTIL_API OptionalNotSetException : public Exception
{
public:
    
    OptionalNotSetException(const char*, int);
    virtual ~OptionalNotSetException() throw();
    virtual std::string Name() const;
    virtual OptionalNotSetException* Clone() const;
    virtual void Throw() const;

private:

    static const char* ms_pcName;
};

//////////////////////////////////////////////////////////////////////////
/// InitializationException
class UTIL_API InitializationException : public Exception
{
	friend class Properties;

public:

	InitializationException(const char* file, int line);
	InitializationException(const char* file, int line, const std::string& reason);
	virtual std::string Name() const;
	virtual void Print(std::ostream& out) const;
	virtual InitializationException* Clone() const;
	virtual void Throw() const;

	const std::string& Reason() const;

private:

	std::string m_reason;

	static const char* ms_pcName;    
};

//////////////////////////////////////////////////////////////////////////
/// VersionMismatchException
class UTIL_API VersionMismatchException : public Exception
{
public:

	VersionMismatchException(const char* file, int line);
	virtual ~VersionMismatchException() throw();
	virtual std::string Name() const;
	virtual VersionMismatchException* Clone() const;
	virtual void Throw() const;

private:

	static const char *ms_pcName;
};

//////////////////////////////////////////////////////////////////////////
/// class MarshalException
class MarshalException : public Exception
{
public:

	MarshalException(const char*, int);
	MarshalException(const char*, int, const ::std::string&);
	virtual ~MarshalException() throw();

	virtual ::std::string Name() const;
	virtual void Print(::std::ostream&) const;
	virtual Exception* Clone() const;
	virtual void Throw() const;

protected:

	std::string m_reason;

	static const char *ms_pcName;
};

//////////////////////////////////////////////////////////////////////////
///  class UnmarshalOutOfBoundsException
class UnmarshalOutOfBoundsException : public MarshalException
{
public:

	UnmarshalOutOfBoundsException(const char*, int);
	UnmarshalOutOfBoundsException(const char*, int, const ::std::string&);
	virtual ~UnmarshalOutOfBoundsException() throw();

	virtual ::std::string Name() const;
	virtual void Print(::std::ostream&) const;
	virtual Exception* Clone() const;
	virtual void Throw() const;

private:

	static const char *ms_pcName;
};

//////////////////////////////////////////////////////////////////////////
///  class MemoryLimitException
class MemoryLimitException : public MarshalException
{
public:

	MemoryLimitException(const char*, int);
	MemoryLimitException(const char*, int, const ::std::string&);
	virtual ~MemoryLimitException() throw();

	virtual ::std::string Name() const;
	virtual void Print(::std::ostream&) const;
	virtual Exception* Clone() const;
	virtual void Throw() const;

private:

	static const char *ms_pcName;
};

//////////////////////////////////////////////////////////////////////////
///  class TrackableCallbackListClearedException
class TrackableCallbackListClearedException : public Exception
{
public:

	TrackableCallbackListClearedException(const char*, int);
	virtual ~TrackableCallbackListClearedException() throw();

	virtual ::std::string Name() const;
	virtual Exception* Clone() const;
	virtual void Throw() const;

private:

	static const char *ms_pcName;
};

//////////////////////////////////////////////////////////////////////////
/// RegexException
class UTIL_API RegexException : public Exception
{
public:

	RegexException(const char* file, int line);
	RegexException(const char* file, int line, const std::string& reason);
	virtual std::string Name() const;
	virtual void Print(std::ostream& out) const;
	virtual RegexException* Clone() const;
	virtual void Throw() const;

	const std::string& Reason() const;

private:
	std::string m_reason;

	static const char* ms_pcName;    
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
