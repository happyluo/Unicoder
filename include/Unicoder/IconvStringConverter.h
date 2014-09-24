// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

// **********************************************************************
//
// Copyright (c) 2010-2014 Bernard Luo. All rights reserved.
//
// <Email: luo (dot) xiaowei (at) hotmail (dot) com>
//
// **********************************************************************


#ifndef UTIL_ICONV_STRING_CONVERTER
#define UTIL_ICONV_STRING_CONVERTER

#include <Unicoder/StringConverter.h>
#include <Build/UndefSysMacros.h>

#include <algorithm>
#include <iconv.h>

#ifndef _WIN32
#include <langinfo.h>
#endif

#if (defined(__APPLE__) && _LIBICONV_VERSION < 0x010B) || defined(__FreeBSD__)
    //
    // See http://sourceware.org/bugzilla/show_bug.cgi?id=2962
    //
#   define Util_CONST_ICONV_INBUF 1
#endif

//
// On Windows, we need to be very careful with errno: if we use different C 
// runtime libraries for the main program and the libiconv DLL, we end up with
// two different errnos ... a not-so-good work-around is to ignore errno 
// altogether, by defining UTIL_NO_ERRNO
//

namespace Util
{

//
// Converts charT encoded with internalCode to and from UTF-8 byte sequences
//
// The implementation allocates a pair of iconv_t on each thread, to avoid
// opening / closing iconv_t objects all the time.
//
//
template<typename charT>
class IconvStringConverter : public Util::BasicStringConverter<charT>
{
public:

#ifdef _WIN32
    IconvStringConverter(const char* internalCode);
#else
    IconvStringConverter(const char* internalCode = nl_langinfo(CODESET));
#endif

    virtual ~IconvStringConverter();

    virtual Util::Byte* ToUTF8(const charT*, const charT*, Util::UTF8Buffer&) const;
    
    virtual void FromUTF8(const Util::Byte*, const Util::Byte*, std::basic_string<charT>&) const;
    
private:

    std::pair<iconv_t, iconv_t> CreateDescriptors() const;
    std::pair<iconv_t, iconv_t> GetDescriptors() const;

    static void CleanupKey(void*);
    static void Close(std::pair<iconv_t, iconv_t>);

#ifdef _WIN32
    DWORD m_key;
#else    
    mutable pthread_key_t m_key;
#endif
    const std::string m_internalCode;
};

//
// Implementation
//

#ifdef __SUNPRO_CC
extern "C"
{
    typedef void (*PthreadKeyDestructor)(void*);
}
#endif

template<typename charT>
IconvStringConverter<charT>::IconvStringConverter(const char* internalCode) :
    m_internalCode(internalCode)
{
    try
    {
        Close(CreateDescriptors());
    }
    catch(const Util::StringConversionException& sce)
    {
        throw Util::InitializationException(__FILE__, __LINE__, sce.m_reason);
    }

#ifdef _WIN32
    m_key = TlsAlloc();
    if (m_key == TLS_OUT_OF_INDEXES)
    {
        throw Util::ThreadSyscallException(__FILE__, __LINE__, GetLastError());
    }
#else
#    ifdef __SUNPRO_CC
    int rs = pthread_key_create(&m_key, reinterpret_cast<PthreadKeyDestructor>(&CleanupKey));
#    else
    int rs = pthread_key_create(&m_key, &CleanupKey);
#    endif

    if (rs != 0)
    {
        throw Util::ThreadSyscallException(__FILE__, __LINE__, rs);
    }
#endif
}

template<typename charT>
IconvStringConverter<charT>::~IconvStringConverter()
{
#ifdef _WIN32
    void* val = TlsGetValue(m_key);
    if (val != 0)
    {
        CleanupKey(val);
    }
    if (TlsFree(m_key) == 0)
    {
        assert(0);
    }
#else
    void* val = pthread_getspecific(m_key);
    if (val != 0)
    {
        CleanupKey(val);
    }
    if (pthread_key_delete(m_key) != 0)
    {
        assert(0);
    }
#endif
}

template<typename charT> std::pair<iconv_t, iconv_t>
IconvStringConverter<charT>::CreateDescriptors() const
{
    std::pair<iconv_t, iconv_t> cdp;

    const char* externalCode = "UTF-8";

    cdp.first = iconv_open(m_internalCode.c_str(), externalCode);
    if (cdp.first == iconv_t(-1))
    {
        throw Util::StringConversionException(
            __FILE__, __LINE__,
            std::string("iconv cannot convert from ") 
            + externalCode + " to " + m_internalCode);                      
    }
    
    cdp.second = iconv_open(externalCode, m_internalCode.c_str());
    if (cdp.second == iconv_t(-1))
    {
        iconv_close(cdp.first);

        throw Util::StringConversionException(
            __FILE__, __LINE__,
            std::string("iconv cannot convert from ") + m_internalCode + " to " + externalCode);                    
    }
    return cdp;
}

template<typename charT> std::pair<iconv_t, iconv_t>
IconvStringConverter<charT>::GetDescriptors() const
{
#ifdef _WIN32
    void* val = TlsGetValue(m_key);
#else
    void* val = pthread_getspecific(m_key);
#endif
    if (val != 0)
    {
        return *static_cast<std::pair<iconv_t, iconv_t>*>(val);
    }
    else
    {
        std::pair<iconv_t, iconv_t> cdp = CreateDescriptors();
        
#ifdef _WIN32
        if (TlsSetValue(m_key, new std::pair<iconv_t, iconv_t>(cdp)) == 0)
        {
            throw Util::ThreadSyscallException(__FILE__, __LINE__, GetLastError());
        }
#else
        int rs = pthread_setspecific(m_key, new std::pair<iconv_t, iconv_t>(cdp));
        if (rs != 0)
        {
            throw Util::ThreadSyscallException(__FILE__, __LINE__, rs);
        }
#endif
        return cdp;
    }
}

template<typename charT> /*static*/ void
IconvStringConverter<charT>::CleanupKey(void* val)
{
    std::pair<iconv_t, iconv_t>* cdp = static_cast<std::pair<iconv_t, iconv_t>*>(val);

    Close(*cdp);
    delete cdp;
}

template<typename charT> /*static*/ void
IconvStringConverter<charT>::Close(std::pair<iconv_t, iconv_t> cdp)
{
    int rs = iconv_close(cdp.first);
    assert(rs == 0);

    rs = iconv_close(cdp.second);
    assert(rs == 0);
}
 
template<typename charT> Util::Byte* 
IconvStringConverter<charT>::ToUTF8(const charT* sourceStart, const charT* sourceEnd, Util::UTF8Buffer& buf) const
{ 
    iconv_t cd = GetDescriptors().second;
    
#ifdef NDEBUG
    iconv(cd, 0, 0, 0, 0);
#else
    size_t rs = iconv(cd, 0, 0, 0, 0);
    assert(rs == 0);
#endif

#ifdef Util_CONST_ICONV_INBUF
    const char* inbuf = reinterpret_cast<const char*>(sourceStart);
#else
    const char* inbuf = reinterpret_cast<char*>(const_cast<charT*>(sourceStart));
#endif
    size_t inbytesleft = (sourceEnd - sourceStart) * sizeof(charT);
    char* outbuf  = 0;
  
    size_t count = 0; 
    do
    {
        size_t howMany = std::max(inbytesleft, size_t(4));
        outbuf = reinterpret_cast<char*>(buf.GetMoreBytes(howMany, reinterpret_cast<Util::Byte*>(outbuf)));
        count = iconv(cd, &inbuf, &inbytesleft, &outbuf, &howMany);
#ifdef UTIL_NO_ERRNO
    } while (count == size_t(-1));
#else
    } while (count == size_t(-1) && errno == E2BIG);
#endif

    if (count == size_t(-1))
    {
       std::string msg = "Unknown error";
#ifndef UTIL_NO_ERRNO
        if (errno != 0)
        {
            msg = strerror(errno);
        }
#endif
        throw Util::StringConversionException(__FILE__, __LINE__, msg);
    }
    return reinterpret_cast<Util::Byte*>(outbuf);
}
  
template<typename charT> void
IconvStringConverter<charT>::FromUTF8(const Util::Byte* sourceStart, const Util::Byte* sourceEnd,
                                      std::basic_string<charT>& target) const
{
    iconv_t cd = GetDescriptors().first;
    
#ifdef NDEBUG
    iconv(cd, 0, 0, 0, 0);
#else
    size_t rs = iconv(cd, 0, 0, 0, 0);
    assert(rs == 0);
#endif

#ifdef Util_CONST_ICONV_INBUF
    const char* inbuf = reinterpret_cast<const char*>(sourceStart);
#else
    const char* inbuf = reinterpret_cast<char*>(const_cast<Util::Byte*>(sourceStart));
#endif
    size_t inbytesleft = sourceEnd - sourceStart;

    char* buf = 0;
    size_t bufsize = 0;

    char* outbuf = 0;
    size_t outbytesleft = 0;

    size_t count = 0;

    do
    {
        size_t increment = std::max(inbytesleft * sizeof(wchar_t), size_t(8));
        bufsize += increment;   
                                    
        char* newbuf = static_cast<char*>(realloc(buf, bufsize));

        if (newbuf == 0)
        {
            free(buf);
            throw Util::StringConversionException(
                __FILE__, __LINE__, "Out of memory");
        }

        outbuf = newbuf + (outbuf - buf);
        outbytesleft += increment;

        buf = newbuf;
        
        count = iconv(cd, &inbuf, &inbytesleft, &outbuf, &outbytesleft);
#ifdef UTIL_NO_ERRNO
    } while (count == size_t(-1));
#else
    } while (count == size_t(-1) && errno == E2BIG);
#endif

    if (count == size_t(-1))
    {
        std::string msg = "Unknown error";
#ifndef UTIL_NO_ERRNO
        if (errno != 0)
        {
            msg = strerror(errno);
        }
#endif
        free(buf);
        throw Util::StringConversionException(__FILE__, __LINE__, msg);
    }
    
    size_t length = (bufsize - outbytesleft) / sizeof(charT);
    
    std::basic_string<charT> result(reinterpret_cast<charT*>(buf), length);
    target.swap(result);
    free(buf);
}

}

#endif
