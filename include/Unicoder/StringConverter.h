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

#ifndef UTIL_STRING_CONVERTER_H
#define UTIL_STRING_CONVERTER_H

#include <Util/Config.h>
#include <Util/Exception.h>
#include <Util/Shared.h>
#include <Util/SharedPtr.h>
#include <Unicoder/Unicode.h>

#include <string>

namespace Util
{

//
// Provides bytes to ToUTF8. Raises MemoryLimitException when too many
// bytes are requested.
//
class UTIL_API UTF8Buffer
{
public:
    virtual Byte* GetMoreBytes(size_t howMany, Byte* firstUnused) = 0;
    
    virtual ~UTF8Buffer() {}
};

//
// A StringConverter converts narrow or wide-strings to and from UTF-8 byte sequences.
// It's used by the communicator during marshaling (ToUTF8) and unmarshaling (FromUTF8).
// It report errors by raising StringConversionFailed or MemoryLimitException.
//
template<typename charT>
class BasicStringConverter : public Util::Shared
{
public:
    
    //
    // Returns a pointer to byte after the last written byte (which may be
    // past the last byte returned by getMoreBytes).
    //
    virtual Byte* ToUTF8(const charT* sourceStart, const charT* sourceEnd,
                         UTF8Buffer&) const = 0;

    //
    // Unmarshals a UTF-8 sequence into a basic_string
    //
    virtual void FromUTF8(const Byte* sourceStart, const Byte* sourceEnd,
                          std::basic_string<charT>& target) const = 0;
};

typedef BasicStringConverter<char> StringConverter;
typedef Util::SharedPtr<StringConverter> StringConverterPtr;

typedef BasicStringConverter<wchar_t> WstringConverter;
typedef Util::SharedPtr<WstringConverter> WstringConverterPtr;


//
// Converts to and from UTF-16 or UTF-32 depending on sizeof(wchar_t)
//
class UTIL_API UnicodeWstringConverter : public WstringConverter
{
public:

    UnicodeWstringConverter(Util::ConversionFlags = Util::lenientConversion);

    virtual Byte* ToUTF8(const wchar_t*, const wchar_t*, UTF8Buffer&) const;

    virtual void FromUTF8(const Byte*, const Byte*, std::wstring&) const;

private:
    const Util::ConversionFlags m_conversionFlags;
};

#ifdef _WIN32

//
// Converts to/from UTF-8 using MultiByteToWideChar and WideCharToMultiByte
//

class UTIL_API WindowsStringConverter : public StringConverter
{
public:

    explicit WindowsStringConverter(const std::string& internalCode);
    explicit WindowsStringConverter(unsigned int codepage = GetACP());

    virtual Byte* ToUTF8(const char*, const char*, UTF8Buffer&) const;

    virtual void FromUTF8(const Byte*, const Byte*, std::string& target) const;

private:
    int getCodePage(const std::string& internalCode);

    unsigned int m_codePage;
    UnicodeWstringConverter m_unicodeWstringConverter;
};
#endif

//
// Converts the given string from the native narrow string encoding to
// UTF8 using the given converter. If the converter is null, returns
// the given string.
//
UTIL_API std::string
NativeToUTF8(const Util::StringConverterPtr&, const std::string&);

//
// Converts the given string from UTF8 to the native narrow string
// encoding using the given converter. If the converter is null,
// returns the given string.
//
UTIL_API std::string
UTF8ToNative(const Util::StringConverterPtr&, const std::string&);

//////////////////////////////////////////////////////////////////////////
/// StringConversionException
class UTIL_API StringConversionException : public Exception
{
public:

    StringConversionException(const char* file, int line);
    StringConversionException(const char* file, int line, const std::string& reason);
    virtual std::string Name() const;
    virtual void Print(std::ostream& out) const;
    virtual StringConversionException* Clone() const;
    virtual void Throw() const;

    const std::string& Reason() const;

    std::string m_reason;

private:

    static const char* m_name;    
};

}

#endif
