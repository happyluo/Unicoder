// **********************************************************************
//
// Copyright (c) 2010-2014 Bernard Luo. All rights reserved.
//
// <Email: luo (dot) xiaowei (at) hotmail (dot) com>
//
// **********************************************************************

#ifndef UTIL_STRING_UTIL_H
#define UTIL_STRING_UTIL_H

#include <string>
#include <Util/Config.h>
//#include <Util/StaticAssert.h>
#include <Util/ErrorToString.h>

UTIL_BEGIN

class UTIL_API String : public std::string
{
public:
    String();

    ~String();

    String(const String& other);

    String(const std::string& src);

    String(const String& src, size_type i, size_type n=npos);

    String(const char* src, size_type n);

    String(const char* src);

    String(size_type n, char c);

    template <class In> String(In pbegin, In pend);

#ifdef _WIN32
    // Creates a UTF-16 wide string from the given ANSI string, allocating
    // memory using new. The caller is responsible for deleting the return
    // value using delete[]. Returns the wide string, or NULL if the
    // input is NULL.
    static LPCWSTR AnsiToUTF16(const char* ansi);

    // Creates an ANSI string from the given wide string, allocating
    // memory using new. The caller is responsible for deleting the return
    // value using delete[]. Returns the ANSI string, or NULL if the
    // input is NULL.
    static const char* UTF16ToAnsi(LPCWSTR utf16_str);
#endif

    //
    // Add escape sequences (like "\n", or "\0xxx") to make a string
    // readable in ASCII.
    //
    static std::string EscapeString(const std::string&, const std::string&);

    //
    // Remove escape sequences added by escapeString. Throws IllegalArgumentException
    // for an invalid input string.
    //
    static std::string UnescapeString(const std::string&, std::string::size_type, std::string::size_type);

    //
    // Split a string using the given delimiters. Considers single and double quotes;
    // returns false for unbalanced quote, true otherwise.
    //
    static bool SplitString(const std::string& str, const std::string& delim, std::vector<std::string>& result, bool keepblank = false);

    //
    // Join a list of strings using the given delimiter. 
    //
    static std::string JoinString(const std::vector<std::string>& values, const std::string& delimiter);

    //
    // Trim white space
    //
    static std::string Trim(const std::string& src);

    //
    // If a single or double quotation mark is found at the start
    // position, then the position of the matching closing quote is
    // returned. If no quotation mark is found at the start position, then
    // 0 is returned. If no matching closing quote is found, then
    // std::string::npos is returned.
    //
    static std::string::size_type CheckQuote(const std::string&, std::string::size_type = 0);

    static std::string::size_type ExistQuote(const std::string&, std::string::size_type = 0);

    //
    // Match `s' against the pattern `pat'. A * in the pattern acts
    // as a wildcard: it matches any non-empty sequence of characters
    // other than a period (`.'). We match by hand here because
    // it's portable across platforms (whereas regex() isn't).
    //
    static bool Match(const std::string& s, const std::string& pat, bool = false);

    //
    // Translating both the two-character sequence #xD #xA and any #xD that is not followed by #xA to 
    // a single #xA character.
    //    LF  (Line feed, '\n', 0x0A, 10 in decimal)  
    //    CR (Carriage return, '\r', 0x0D, 13 in decimal) 
    //
    static std::string TranslatingCR2LF(const std::string& src);

    //static std::string ToHexString(unsigned long n, bool bupper = false);

    template <typename T>
    static inline
    std::string ToHexString(T n, bool bupper = false);

    //static std::string ToOctalString(unsigned long n)

    template <typename T>
    static inline
    std::string ToOctalString(T n);

    //static std::string ToBinaryString(unsigned long n);

    template <typename T>
    static inline
    std::string ToBinaryString(T n);

    //
    // Functions to convert to lower/upper case. These functions accept
    // UTF8 string/characters but ignore non ASCII characters. Unlike, the
    // C methods, these methods are not local dependent.
    //
    static std::string ToLower(const std::string&);
    static std::string ToUpper(const std::string&);

    static unsigned long Hash(const std::string&);

    //
    // Remove all whitespace from a string
    //
    static std::string RemoveWhitespace(const std::string&);

    //////////////////////////////////////////////////////////////////////////
    /// string & data convert
    //
    // Portable strtoll/_strtoi64
    //
    static Util::Int64 ToInt64(const char* s, char** endptr, int base);

    //
    // ToInt64 converts a string into a signed 64-bit integer.
    // It's a simple wrapper around ToInt64.
    //
    // Semantics:
    //
    // - Ignore leading whitespace
    //
    // - If the string starts with '0', parse as octal
    //
    // - If the string starts with "0x" or "0X", parse as hexadecimal
    //
    // - Otherwise, parse as decimal
    //
    // - return value == true indicates a successful conversion and result contains the converted value
    // - return value == false indicates an unsuccessful conversion:
    //      - result == 0 indicates that no digits were available for conversion
    //      - result == "Int64 Min" or result == "Int64 Max" indicate underflow or overflow.
    //
    static  bool ToInt64(const std::string& s,  Util::Int64& result);

    static unsigned long ToULong(const std::string& strval, size_t* endindex = 0, unsigned int base = 10);

    static long ToLong(const std::string& strval, size_t* endindex = 0, unsigned int base = 10);

    static double ToDouble(const std::string& strval, size_t* endindex = 0, int precision = 6);

    template <typename T>
    static inline
    T ToData(const std::string& strval, size_t* endindex = 0, unsigned int base = 10);

    static std::string ToString(unsigned long n);

    static std::string ToString(long n);

    //
    // Determines if a string is a number of not.
    //
    static  bool IsNumber(const std::string& s, bool* isdecimal);

    //
    // Skip leading none digit character, get the first number in string.
    //
    static  int GetIntInString(const char* s, char** endptr, int base);

    static const Util::Byte* FindStringInBuffer(Util::Byte* pBuff, size_t iBuffSize, const std::string& strSearch);

    //
    // Swap lhs and rhs's value
    //
    static inline void Swap(unsigned char* lhs, unsigned char* rhs);

    //
    // Reverse elements in [begin, end)
    //
    static inline void ReverseBuffer(unsigned char* begin, unsigned char* end);

    static std::string BytesToString(const Util::Byte* src, size_t size);
    static std::string BytesToString(const Util::ByteSeq& bytes);
    static Util::ByteSeq StringToBytes(const std::string&);

    //
    // Hex-dump at most 16 bytes starting at offset from a memory area of size
    // bytes.  Return the number of bytes actually dumped.
    //
    static size_t HexDumpLine(const void* ptr, size_t offset, size_t size, std::string& line, size_t linelength = 16);

    template <typename OutIt>
    static inline
    void HexDump(const void* ptr, size_t size, OutIt out, size_t linelength/* = 16*/);

    static std::string HexDump(const void* ptr, size_t size, size_t linelength = 16);

    static std::string HexStringToBuffer(const std::string &hexString, std::string &buffer, const std::string& delimiter = ",");

    static size_t BinDumpLine(const void* ptr, size_t offset, size_t size, std::string& line, size_t linelength = 8);

    template <typename OutIt>
    static inline
    void BinDump(const void* ptr, size_t size, OutIt out, size_t linelength/* = 8*/);

    static std::string BinDump(const void* ptr, size_t size, size_t linelength = 8);

    // If *pstr starts with the given prefix, modifies *pstr to be right
    // past the prefix and returns true; otherwise leaves *pstr unchanged
    // and returns false.  None of pstr, *pstr, and prefix can be NULL.
    static bool SkipPrefix(const char* prefix, const char** pstr);
};

template <class In>
String::String(In pbegin, In pend) : std::string(pbegin, pend)
{
}

#ifdef HAS_STD_WSTREAM

//
// Wide stream input operator.
//
std::wistream& operator >>(std::wistream& is, Util::String& strret);

//
// Wide stream output operator.
//
std::wostream& operator <<(std::wostream& os, const Util::String& strsrc);

#endif // HAS_STD_WSTREAM

//////////////////////////////////////////////////////////////////////////
// data to string conversion
template <typename T>
inline // static
std::string String::ToHexString(T n, bool bupper/* = false*/)
{
    std::string s;
    size_t size = sizeof(T) * 2;
    s.resize(size);
    std::string::size_type charPos = size;

    const int radix = 1 << 4;
    int mask = radix - 1;
    char base = bupper ? 'A' : 'a';

    do
    {
        int d = n & mask;
        s[--charPos] = d < 10 ? '0' + d : base + (d - 10);
        n >>= 4;
    }while (0 != n);

    return std::string(s, charPos, (size - charPos));
}

template <typename T>
inline // static
std::string String::ToOctalString(T n)
{
    std::string s;
    size_t size = sizeof(T) * 8;
    s.resize(size);
    std::string::size_type charPos = size;

    const int radix = 1 << 3;
    int mask = radix - 1;

    do
    {
        s[--charPos] = '0' + static_cast<int>(n & mask);
        n >>= 3;
    }while (0 != n);

    return std::string(s, charPos, (size - charPos));
}

template <typename T>
inline // static
std::string String::ToBinaryString(T n)
{
    std::string s;
    size_t size = sizeof(T) * 8;
    s.resize(size);
    std::string::size_type charPos = size;

    do
    {
        s[--charPos] = (n & 1) + '0';
        n >>= 1;
    }while (0 != n);

    return std::string(s, charPos, (size - charPos));
}

//
// Swap lhs and rhs's value
//
inline // static
void String::Swap(unsigned char* lhs, unsigned char* rhs)
{
    //return std::swap(*lhs, *rhs);

    unsigned char tmp = *lhs;
    *lhs = *rhs;
    *rhs = tmp;
}

//
// Reverse elements in [begin, end)
//
inline // static
void String::ReverseBuffer(unsigned char* begin, unsigned char* end)
{
    //return std::reverse(begin, end);
#if 0
    for (; begin != end && begin != --end; ++begin)
    {
        //Swap(begin, end);
        std::swap(*begin, *end);
    }

#else

    while (begin < --end)
    {
        //Swap(begin++, end);
        std::swap(*begin, *end);
    }
#endif
}

template <typename OutIt>
inline // static
void String::HexDump(const void* ptr, size_t size, OutIt out, size_t linelength/* = 16*/)
{
    size_t offset = 0;
    std::string line;
    while (offset < size) 
    {
        offset += HexDumpLine(ptr, offset, size, line, linelength);
        *out++ = line.c_str();
        //*out++ = line;
    }
}

template <typename OutIt>
inline // static
void String::BinDump(const void* ptr, size_t size, OutIt out, size_t linelength/* = 8*/)
{
    size_t offset = 0;
    std::string line;
    while (offset < size) 
    {
        offset += BinDumpLine(ptr, offset, size, line, linelength);
        *out++ = line.c_str();
        //*out++ = line;
    }
}

//
// Global Format utility
// 
UTIL_API std::string Format( const char* format, ...);

// Formats an int value as "%02d".
UTIL_API std::string FormatIntWidth2(int value);

// Formats an int value as "%X".
UTIL_API std::string FormatHexInt(int value);

// Formats a byte as "%02X".
UTIL_API std::string FormatByte(unsigned char value);

// Converts the buffer in a stringstream to an std::string, converting NUL
// bytes to "\\0" along the way.
UTIL_API std::string StringStreamToString(::std::stringstream* ss);

// Converts a Unicode code point to a narrow string in UTF-8 encoding.
// code_point parameter is of type unsigned int because wchar_t may not be
// wide enough to contain a code point.
// If the code_point is not a valid Unicode code point
// (i.e. outside of Unicode range U+0 to U+10FFFF) it will be converted
// to "(Invalid Unicode 0xXXXXXXXX)".
UTIL_API std::string CodePointToUtf8(unsigned int code_point);

// Converts a wide string to a narrow string in UTF-8 encoding.
// The wide string is assumed to have the following encoding:
//   UTF-16 if sizeof(wchar_t) == 2 (on Windows, Cygwin, Symbian OS)
//   UTF-32 if sizeof(wchar_t) == 4 (on Linux)
// Parameter str points to a null-terminated wide string.
// Parameter num_chars may additionally limit the number
// of wchar_t characters processed. -1 is used when the entire string
// should be processed.
// If the string contains code points that are not valid Unicode code points
// (i.e. outside of Unicode range U+0 to U+10FFFF) they will be output
// as '(Invalid Unicode 0xXXXXXXXX)'. If the string is in UTF16 encoding
// and contains invalid UTF-16 surrogate pairs, values in those pairs
// will be encoded as individual Unicode characters from Basic Normal Plane.
UTIL_API std::string WideStringToUtf8(const wchar_t* str, int num_chars);

// Converts a wide C string to an std::string using the UTF-8 encoding.
// NULL will be converted to "(null)".
UTIL_API std::string ShowWideCString(const wchar_t * wide_c_str);

// Compares two wide C strings.  Returns true iff they have the same
// content.
//
// Unlike wcscmp(), this function can handle NULL argument(s).  A NULL
// C string is considered different to any non-NULL C string,
// including the empty string.
UTIL_API bool WideCStringEquals(const wchar_t * lhs, const wchar_t * rhs);

// Compares two C strings.  Returns true iff they have the same content.
//
// Unlike strcmp(), this function can handle NULL argument(s).  A NULL
// C string is considered different to any non-NULL C string,
// including the empty string.
UTIL_API bool CStringEquals(const char * lhs, const char * rhs);

// Compares two C strings, ignoring case.  Returns true iff they have
// the same content.
//
// Unlike strcasecmp(), this function can handle NULL argument(s).  A
// NULL C string is considered different to any non-NULL C string,
// including the empty string.
UTIL_API bool CaseInsensitiveCStringEquals(const char * lhs, const char * rhs);

// Compares two wide C strings, ignoring case.  Returns true iff they
// have the same content.
//
// Unlike wcscasecmp(), this function can handle NULL argument(s).
// A NULL C string is considered different to any non-NULL wide C string,
// including the empty string.
// NB: The implementations on different platforms slightly differ.
// On windows, this method uses _wcsicmp which compares according to LC_CTYPE
// environment variable. On GNU platform this method uses wcscasecmp
// which compares according to LC_CTYPE category of the current locale.
// On MacOS X, it uses towlower, which also uses LC_CTYPE category of the
// current locale.
UTIL_API bool CaseInsensitiveWideCStringEquals(const wchar_t* lhs, const wchar_t* rhs);

// Returns true iff str ends with the given suffix, ignoring case.
// Any string is considered to end with an empty suffix.
UTIL_API bool EndsWithCaseInsensitive(const std::string& str, const std::string& suffix);

UTIL_API std::string Double2String(double value, int precision);
UTIL_API double String2Double(const std::string& str);


//
// Utilities for char.
//
// isspace(int ch) and friends accept an unsigned char or EOF.  char
// may be signed, depending on the compiler (or compiler flags).
// Therefore we need to cast a char to unsigned char before calling
// isspace(), etc.

UTIL_API bool IsAlpha(char);
UTIL_API bool IsDigit(char);

//inline bool IsAlpha(char ch) 
//{
//    return 0 != isalpha(static_cast<unsigned char>(ch));
//}

inline bool IsAlNum(char ch)
{
    return 0 != isalnum(static_cast<unsigned char>(ch));
}

//inline bool IsDigit(char ch) 
//{
//    return 0 != isdigit(static_cast<unsigned char>(ch));
//}

inline bool IsLower(char ch) 
{
    return 0 != islower(static_cast<unsigned char>(ch));
}

inline bool IsSpace(char ch)
{
    return 0 != isspace(static_cast<unsigned char>(ch));
}

inline bool IsUpper(char ch)
{
    return 0 != isupper(static_cast<unsigned char>(ch));
}

inline bool IsXDigit(char ch)
{
    return 0 != isxdigit(static_cast<unsigned char>(ch));
}

inline bool IsXDigit(wchar_t ch)
{
    const unsigned char low_byte = static_cast<unsigned char>(ch);
    return ch == low_byte && 0 != isxdigit(low_byte);
}

inline char ToLower(char ch)
{
    return static_cast<char>(tolower(static_cast<unsigned char>(ch)));
}

inline char ToUpper(char ch) 
{
    return static_cast<char>(toupper(static_cast<unsigned char>(ch)));
}

inline int DigitValue(char ch)
{
    unsigned char uc(static_cast<unsigned char>(ch));
    return isdigit(uc) ? uc - '0' : -1;
}

inline int XDigitValue(char ch)
{
    unsigned char uc(static_cast<unsigned char>(ch));
    return isxdigit(uc)
        ? (isdigit(uc) ? uc - '0' : toupper(uc) - 'A' + 10) 
        : -1;
}

UTIL_END

#endif
