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

#include <Unicoder/Unicode.h>
#include <Unicoder/ConvertUTF.h>

using namespace std;
using namespace Util;
using namespace UtilInternal;

namespace
{
//
// Helper class, base never defined
// Usage: WstringHelper<sizeof(wchar_t)>::ToUTF8 and FromUTF8.
//
template<size_t wcharSize> 
struct WstringHelper
{
    static ConversionResult ToUTF8(
        const wchar_t*& sourceStart, const wchar_t* sourceEnd, 
        Byte*& targetStart, Byte* targetEnd, ConversionFlags flags);

    static ConversionResult FromUTF8(
        const Byte*& sourceStart, const Byte* sourceEnd, 
        wchar_t*& targetStart, wchar_t* targetEnd, ConversionFlags flags);
};

template<>
struct WstringHelper<2>
{
    static ConversionResult ToUTF8(
        const wchar_t*& sourceStart, const wchar_t* sourceEnd, 
        Byte*& targetStart, Byte* targetEnd, ConversionFlags flags)
    {
        return ConvertUTF16toUTF8(
            reinterpret_cast<const UTF16**>(&sourceStart),
            reinterpret_cast<const UTF16*>(sourceEnd),
            &targetStart, targetEnd, flags);
    }
    
    static ConversionResult FromUTF8(
        const Byte*& sourceStart, const Byte* sourceEnd, 
        wchar_t*& targetStart, wchar_t* targetEnd, ConversionFlags flags)
    {
        return ConvertUTF8toUTF16(
            &sourceStart, sourceEnd,
            reinterpret_cast<UTF16**>(&targetStart),
            reinterpret_cast<UTF16*>(targetEnd), flags);
    }
};

template<>
struct WstringHelper<4>
{
    static ConversionResult ToUTF8(
        const wchar_t*& sourceStart, const wchar_t* sourceEnd, 
        Byte*& targetStart, Byte* targetEnd, ConversionFlags flags)
    {
        return ConvertUTF32toUTF8(
            reinterpret_cast<const UTF32**>(&sourceStart),
            reinterpret_cast<const UTF32*>(sourceEnd),
            &targetStart, targetEnd, flags);
    }
    
    static ConversionResult FromUTF8(
        const Byte*& sourceStart, const Byte* sourceEnd, 
        wchar_t*& targetStart, wchar_t* targetEnd, ConversionFlags flags)
    {
        return ConvertUTF8toUTF32(
            &sourceStart, sourceEnd,
            reinterpret_cast<UTF32**>(&targetStart),
            reinterpret_cast<UTF32*>(targetEnd), flags);
    }
};
}

//
// convertXXX functions
//

ConversionResult 
UtilInternal::ConvertUTFWstringToUTF8(
    const wchar_t*& sourceStart, const wchar_t* sourceEnd, 
    Byte*& targetStart, Byte* targetEnd, ConversionFlags flags)
{
    return WstringHelper<sizeof(wchar_t)>::ToUTF8(
        sourceStart, sourceEnd, targetStart, targetEnd, flags);
}

ConversionResult
UtilInternal::ConvertUTF8ToUTFWstring(
    const Byte*& sourceStart, const Byte* sourceEnd, 
    wchar_t*& targetStart, wchar_t* targetEnd, ConversionFlags flags)
{
    return WstringHelper<sizeof(wchar_t)>::FromUTF8(
        sourceStart, sourceEnd, targetStart, targetEnd, flags);
}

ConversionResult 
UtilInternal::ConvertUTF8ToUTFWstring(const Byte*& sourceStart, const Byte* sourceEnd, 
                                 std::wstring& target, ConversionFlags flags)
{
    //
    // Could be reimplemented without this temporary wchar_t buffer
    //
    size_t size = static_cast<size_t>(sourceEnd - sourceStart);
    wchar_t* outBuf = new wchar_t[size];
    wchar_t* targetStart = outBuf; 
    wchar_t* targetEnd = targetStart + size;

    ConversionResult result =  
        ConvertUTF8ToUTFWstring(sourceStart, sourceEnd, targetStart,
                                targetEnd, flags);

    if (result == conversionOK)
    {
        std::wstring s(outBuf, static_cast<size_t>(targetStart - outBuf));
        s.swap(target);
    }
    delete[] outBuf;
    return result;
}


//
// WstringToString and StringToWstring
//

const char* Util::UTFConversionException::m_name = "Util::UTFConversionException";

Util::UTFConversionException::UTFConversionException(const char* file, int line, 
                                                        ConversionErrorType ce): 
    Exception(file, line),
    m_conversionError(ce)
{}

string
Util::UTFConversionException::Name() const
{
    return m_name;
}

void
Util::UTFConversionException::Print(ostream& os) const
{
    Exception::Print(os);
    switch(m_conversionError)
    {
        case partialCharacter:
            os << ": partial character";
            break;
        case badEncoding:
            os << ": bad encoding";
            break;
        default:
            assert(0);
            break;
    };
}

Util::UTFConversionException*
Util::UTFConversionException::Clone() const
{
    return new UTFConversionException(*this);
}

void
Util::UTFConversionException::Throw() const
{
    throw *this;
}

Util::ConversionErrorType
Util::UTFConversionException::ConversionError() const
{
    return m_conversionError;
}


string
Util::WstringToString(const wstring& wstr, ConversionFlags flags)
{
    string target;
    
    size_t size = wstr.size() * 3 * (sizeof(wchar_t) / 2);

    Byte* outBuf = new Byte[size];
    Byte* targetStart = outBuf; 
    Byte* targetEnd = outBuf + size;

    const wchar_t* sourceStart = wstr.data();
  
    ConversionResult cr = 
        ConvertUTFWstringToUTF8(
            sourceStart, sourceStart + wstr.size(), 
            targetStart, targetEnd, flags);
        
    if (cr != conversionOK)
    {
        delete[] outBuf;
        assert(cr == sourceExhausted || cr == sourceIllegal);
        throw UTFConversionException(__FILE__, __LINE__, 
                                     cr == sourceExhausted ? partialCharacter : badEncoding);
    }
    
    string s(reinterpret_cast<char*>(outBuf),
             static_cast<size_t>(targetStart - outBuf));
    s.swap(target);
    delete[] outBuf;
    return target;
}

wstring
Util::StringToWstring(const string& str, ConversionFlags flags)
{
    wstring result;
    const Byte* sourceStart = reinterpret_cast<const Byte*>(str.data());
    
    ConversionResult cr 
        = ConvertUTF8ToUTFWstring(sourceStart, sourceStart + str.size(),
                                  result, flags);

    if (cr != conversionOK)
    {
        assert(cr == sourceExhausted || cr == sourceIllegal);

        throw UTFConversionException(__FILE__, __LINE__,
                                     cr == sourceExhausted ? partialCharacter : badEncoding);
    }
    return result;
}
