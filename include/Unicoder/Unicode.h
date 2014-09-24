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

#ifndef UTIL_UNICODE_H
#define UTIL_UNICODE_H

#include <Util/Config.h>
#include <Util/Exception.h>

namespace Util
{

enum ConversionFlags
{
    strictConversion = 0,
    lenientConversion
};

UTIL_API std::string WstringToString(const std::wstring&, ConversionFlags = lenientConversion);
UTIL_API std::wstring StringToWstring(const std::string&, ConversionFlags = lenientConversion);

#define WideToUTF8 Util::WstringToString
#define UTF8ToWide Util::StringToWstring

typedef unsigned char Byte;

UTIL_API bool
IsLegalUTF8Sequence(const Byte* source, const Byte* end);

enum ConversionErrorType
{
    partialCharacter,
    badEncoding
};

//
// UTFConversionException is raised by WstringToString() or StringToWstring()
// to report a conversion error 
//
class UTIL_API UTFConversionException : public Exception
{
public:
    
    UTFConversionException(const char*, int, ConversionErrorType);
    virtual std::string Name() const;
    virtual void Print(std::ostream&) const;
    virtual UTFConversionException* Clone() const;
    virtual void Throw() const;

    ConversionErrorType ConversionError() const;
private:

    const ConversionErrorType m_conversionError;
    static const char* m_name;    
};

}

namespace UtilInternal
{

//
// Converts UTF-8 byte-sequences to and from UTF-16 or UTF-32 (with native
// endianness) depending on sizeof(wchar_t).
//
// These are thin wrappers over the UTF8/16/32 converters provided by 
// unicode.org
//

enum ConversionResult
{
    conversionOK,           /* conversion successful */
    sourceExhausted,        /* partial character in source, but hit end */
    targetExhausted,        /* insuff. room in target for conversion */
    sourceIllegal           /* source sequence is illegal/malformed */
};

UTIL_API ConversionResult 
ConvertUTFWstringToUTF8(const wchar_t*& sourceStart, const wchar_t* sourceEnd, 
                        Util::Byte*& targetStart, Util::Byte* targetEnd, Util::ConversionFlags flags);

UTIL_API ConversionResult
ConvertUTF8ToUTFWstring(const Util::Byte*& sourceStart, const Util::Byte* sourceEnd, 
                        wchar_t*& targetStart, wchar_t* targetEnd, Util::ConversionFlags flags);

UTIL_API ConversionResult 
ConvertUTF8ToUTFWstring(const Util::Byte*& sourceStart, const Util::Byte* sourceEnd, 
                        std::wstring& target, Util::ConversionFlags flags);

}

#endif
