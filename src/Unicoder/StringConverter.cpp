// **********************************************************************
//
// Copyright (c) 2010-2014 Bernard Luo. All rights reserved.
//
// <Email: luo.xiaowei (at) hotmail.com>
//
// **********************************************************************

#include <Unicoder/StringConverter.h>
#include <Util/StringUtil.h>
#include <Util/ScopedArray.h>

#ifndef _WIN32
#include <Util/IconvStringConverter.h>
#endif

#ifdef __MINGW32__
#  include <limits.h>
#endif

using namespace Util;
using namespace UtilInternal;
using namespace std;

namespace
{

class UTF8BufferI : public Util::UTF8Buffer
{
public:

   UTF8BufferI() :
        m_buffer(0),
        m_offset(0)
    {
    }
    
   ~UTF8BufferI()
    {
        free(m_buffer);
    }

    Util::Byte* GetMoreBytes(size_t howMany, Byte* firstUnused)
    {
        if (m_buffer == 0)
        {
            m_buffer = (Byte*)malloc(howMany);
        }
        else
        {
            assert(firstUnused != 0);
            m_offset = firstUnused - m_buffer;
            m_buffer = (Byte*)realloc(m_buffer, m_offset + howMany);
        }
        
        if (!m_buffer)
        {
            throw std::bad_alloc();
        }
        return m_buffer + m_offset;
    }
    
    Util::Byte* GetBuffer()
    {
        return m_buffer;
    }
    
    void Reset()
    {
        free(m_buffer);
        m_buffer = 0;
        m_offset = 0;
    }

private:

    Util::Byte* m_buffer;
    size_t m_offset;
};
}



namespace Util
{

UnicodeWstringConverter::UnicodeWstringConverter(ConversionFlags flags) :
    m_conversionFlags(flags)
{
}

Byte* 
UnicodeWstringConverter::ToUTF8(const wchar_t* sourceStart, 
                                const wchar_t* sourceEnd,
                                UTF8Buffer& buffer) const
{
    //
    // The "chunk size" is the maximum of the number of characters in the
    // source and 6 (== max bytes necessary to encode one Unicode character).
    //
    size_t chunkSize = std::max<size_t>(static_cast<size_t>(sourceEnd - sourceStart), 6);

    Byte* targetStart = buffer.GetMoreBytes(chunkSize, 0);
    Byte* targetEnd = targetStart + chunkSize;

    ConversionResult result;

    while ((result =
          ConvertUTFWstringToUTF8(sourceStart, sourceEnd, 
                                  targetStart, targetEnd, m_conversionFlags))
          == targetExhausted)
    {
        targetStart = buffer.GetMoreBytes(chunkSize, targetStart);
        targetEnd = targetStart + chunkSize;
    }
        
    switch(result)
    {
        case conversionOK:
            break;
        case sourceExhausted:
            throw StringConversionException(__FILE__, __LINE__, "wide string source exhausted");
        case sourceIllegal:
            throw StringConversionException(__FILE__, __LINE__, "wide string source illegal");
        default:
        {
            assert(0);
            throw StringConversionException(__FILE__, __LINE__);
        }
    }
    return targetStart;
}


void 
UnicodeWstringConverter::FromUTF8(const Byte* sourceStart, 
								  const Byte* sourceEnd,
                                  wstring& target) const
{
    if (sourceStart == sourceEnd)
    {
        target = L"";
        return;
    }

    ConversionResult result = 
        ConvertUTF8ToUTFWstring(sourceStart, sourceEnd, target, m_conversionFlags);

    switch(result)
    {    
        case conversionOK:
            break;
        case sourceExhausted:
            throw StringConversionException(__FILE__, __LINE__, "UTF-8 string source exhausted");
        case sourceIllegal:
            throw StringConversionException(__FILE__, __LINE__, "UTF-8 string source illegal");
        default:
        {
            assert(0);
            throw StringConversionException(__FILE__, __LINE__);
        }
    }
}

#ifdef _WIN32

//////////////////////////////////////////////////////////////////////////
/// WindowsStringConverter
int WindowsStringConverter::getCodePage(const std::string& internalCode)
{
	std::string codeName = Util::String::ToUpper(internalCode);
	if (Util::String::Match(codeName, "GB*", true))
	{
		codeName = "WINDOWS-936";
	}
	
	static const string prefix("WINDOWS-");
	if (!Util::String::Match(codeName, "WINDOWS-*", false))
	{
		return -1;
	}

	int index = prefix.size();
	int codePage = 0;

	for (; codeName[index]; index++) 
	{
		static const char digits[] = "0123456789";
		const char *s = strchr(digits, codeName[index]);
		if (!s)
		{
			return -1;
		}
		codePage *= 10;
		codePage += (int)(s - digits);
		if (codePage >= 0x10000)
		{
			return -1;
		}
	}

	return codePage;
}

WindowsStringConverter::WindowsStringConverter(const std::string& internalCode)
{
	if (-1 == (m_codePage = getCodePage(internalCode)))
	{
		throw StringConversionException(__FILE__, __LINE__, "Unknow Code Page: " + internalCode);
	}
}

WindowsStringConverter::WindowsStringConverter(unsigned int codepage) :
    m_codePage(codepage)
{
}

Byte*
WindowsStringConverter::ToUTF8(const char* sourceStart,
                               const char* sourceEnd,
                               UTF8Buffer& buffer) const
{
    //
    // First convert to UTF-16
    //
    int sourceSize = static_cast<int>(sourceEnd - sourceStart);
    if (sourceSize == 0)
    {
        return buffer.GetMoreBytes(1, 0);
    }

    int size = 0;
    int writtenWchar = 0;
    ScopedArray<wchar_t> wbuffer;
    do
    {
        size = size == 0 ? sourceSize + 2 : 2 * size;
        wbuffer.Reset(new wchar_t[size]);

        writtenWchar = MultiByteToWideChar(m_codePage, MB_ERR_INVALID_CHARS, sourceStart,
                                           sourceSize, wbuffer.Get(), size);
    } while (writtenWchar == 0 && GetLastError() == ERROR_INSUFFICIENT_BUFFER);

    if (writtenWchar == 0)
    {
        throw StringConversionException(__FILE__, __LINE__, Util::LastErrorToString());
    }

    //
    // Then convert this UTF-16 wbuffer into UTF-8
    //
    return m_unicodeWstringConverter.ToUTF8(wbuffer.Get(), wbuffer.Get() + writtenWchar, buffer);
}

void
WindowsStringConverter::FromUTF8(const Byte* sourceStart, const Byte* sourceEnd,
                                 string& target) const
{
    if (sourceStart == sourceEnd)
    {
        target = "";
        return;
    }

    //
    // First convert to wstring (UTF-16)
    //
    wstring wtarget;
    m_unicodeWstringConverter.FromUTF8(sourceStart, sourceEnd, wtarget);

    //
    // And then to a multi-byte narrow string
    //
    int size = 0;
    int writtenChar = 0;
    ScopedArray<char> buffer;
    do
    {
        size = size == 0 ? static_cast<int>(sourceEnd - sourceStart) + 2 : 2 * size;
        buffer.Reset(new char[size]);
        writtenChar = WideCharToMultiByte(m_codePage, 0, wtarget.data(), static_cast<int>(wtarget.size()),
                                          buffer.Get(), size, 0, 0);
    } while (writtenChar == 0 && GetLastError() == ERROR_INSUFFICIENT_BUFFER);

    if (writtenChar == 0)
    {
        throw StringConversionException(__FILE__, __LINE__, Util::LastErrorToString());
    }

    target.assign(buffer.Get(), writtenChar);
}

#endif

string
Util::NativeToUTF8(const Util::StringConverterPtr& converter, const string& str)
{
    if (!converter)
    {
        return str;
    }
    if (str.empty())
    {
        return str;
    }
    UTF8BufferI buffer;
    Util::Byte* last = converter->ToUTF8(str.data(), str.data() + str.size(), buffer);
    return string(reinterpret_cast<const char*>(buffer.GetBuffer()), last - buffer.GetBuffer());
}

string
Util::UTF8ToNative(const Util::StringConverterPtr& converter, const string& str)
{
    if (!converter)
    {
        return str;
    }
    if (str.empty())
    {
        return str;
    }
    string tmp;
    converter->FromUTF8(reinterpret_cast<const Util::Byte*>(str.data()),
                        reinterpret_cast<const Util::Byte*>(str.data() + str.size()), tmp);
    return tmp;
}

//////////////////////////////////////////////////////////////////////////
/// StringConversionException
Util::StringConversionException::StringConversionException(const char *file, int line) :
	Exception(file, line)
{
}

Util::StringConversionException::StringConversionException(
	const char *file, int line, const std::string& reason) :
	Exception(file, line), m_reason(reason)
{
}
const char* Util::StringConversionException::m_name = "Util::StringConversionException";

string
Util::StringConversionException::Name() const
{
	return m_name;
}

void
Util::StringConversionException::Print(ostream& out) const
{
	Exception::Print(out);
	out << ": " << m_reason;
}

Util::StringConversionException*
Util::StringConversionException::Clone() const
{
	return new StringConversionException(*this);
}

void
Util::StringConversionException::Throw() const
{
	throw *this;
}

const std::string&
Util::StringConversionException::Reason() const
{
	return m_reason;
}

}
