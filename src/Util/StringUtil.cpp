// **********************************************************************
//
// Copyright (c) 2010-2014 Bernard Luo. All rights reserved.
//
// <Email: luo (dot) xiaowei (at) hotmail (dot) com>
//
// **********************************************************************

#include <cstring>
#include <stdlib.h>
#include <errno.h>
#include <math.h>
#include <iomanip>
#include <errno.h>
#include <float.h>    // FLT_DIG and DBL_DIG
#include <limits>
#include <limits.h>
#include <stdio.h>
#include <iterator>
#include <bitset>

#ifdef __MINGW32__
//#if defined(__MINGW32__) || (defined(_MSC_VER) && (_MSC_VER < 1300))
#    include <limits.h>
#endif

#if defined(__hpux)
#    include <inttypes.h>
#endif

#include <Util/ScopedArray.h>
#include <Util/StringUtil.h>
#include <Util/Exception.h>
#include <Build/UsefulMacros.h>
#include <Build/UndefSysMacros.h>


using namespace std;

UTIL_BEGIN

String::String()
{
}

String::String(const String& other) : std::string(other)
{
}

String::String(const String& src, String::size_type i, String::size_type n) : std::string(src, i, n)
{
}

String::String(const char* src, String::size_type n) : std::string(src, n)
{
}

String::String(const char* src) : std::string(src)
{
}

String::String(String::size_type n, char c) : std::string(n, c)
{
}

String::String(const std::string& src) : std::string(src)
{
}

String::~String()
{
}

#ifdef _WIN32
// Creates a UTF-16 wide string from the given ANSI string, allocating
// memory using new. The caller is responsible for deleting the return
// value using delete[]. Returns the wide string, or NULL if the
// input is NULL.
LPCWSTR String::AnsiToUTF16(const char* ansi)
{
    if (!ansi) 
    {
        return NULL;
    }
    const int length = strlen(ansi);
    const int unicode_length =
        MultiByteToWideChar(CP_ACP, 0, ansi, length,
        NULL, 0);
    WCHAR* unicode = new WCHAR[unicode_length + 1];
    MultiByteToWideChar(CP_ACP, 0, ansi, length,
        unicode, unicode_length);
    unicode[unicode_length] = 0;
    return unicode;
}

// Creates an ANSI string from the given wide string, allocating
// memory using new. The caller is responsible for deleting the return
// value using delete[]. Returns the ANSI string, or NULL if the
// input is NULL.
const char* String::UTF16ToAnsi(LPCWSTR utf16_str)
{
    if (!utf16_str) 
    {
        return NULL;
    }

    const int ansi_length =
        WideCharToMultiByte(CP_ACP, 0, utf16_str, -1,
        NULL, 0, NULL, NULL);
    char* ansi = new char[ansi_length + 1];
    WideCharToMultiByte(CP_ACP, 0, utf16_str, -1,
        ansi, ansi_length, NULL, NULL);
    ansi[ansi_length] = 0;
    return ansi;
}

#endif  // _WIN32

#ifdef HAS_STD_WSTREAM

std::wistream& operator >>(std::wistream& is, Util::String& strret)
{
    std::wstring wstr;
    is >> wstr;

    StringConverterPtr stringConverter = 
#    if defined(_WIN32) && !defined(ICONV_ON_WINDOWS)
        new WindowsStringConverter();
#    else
        new IconvStringConverter<char>();
#    endif

    strret = Util::UTF8ToNative(stringConverter, Util::WstringToString(wstr));

    return is;
}

std::wostream& operator <<(std::wostream& os, const Util::String& strsrc)
{
    StringConverterPtr stringConverter = 
#    if defined(_WIN32) && !defined(ICONV_ON_WINDOWS)
        new WindowsStringConverter();
#    else
        new IconvStringConverter<char>();
#    endif

    // This won't work if the String contains NUL characters.  Unfortunately,
    // std::wostream::write() ignores Format flags, so we cannot use that.
    // The only option would be to create a temporary std::wstring.  However,
    // even then GCC's libstdc++-v3 prints only the characters up to the first
    // NUL.  Given this, there doesn't seem much of a point in allowing NUL in
    // formatted output.  The semantics would be unclear anyway: what's the
    // screen width of a NUL?
    os << Util::StringToWstring(Util::NativeToUTF8(stringConverter, strsrc));

    return os;
}

#endif


namespace
{
//
// Write the byte b as an escape sequence if it isn't a printable ASCII
// character and append the escape sequence to s. Additional characters
// that should be escaped can be passed in special. If b is any of these
// characters, b is preceded by a backslash in s.
//
void
EncodeChar(string::value_type b, string& s, const string& special)
{
    switch(b)
    {
    case '\\': 
        {
            s.append("\\\\");
            break;
        }

    case '\'': 
        {
            s.append("\\'");
            break;
        }

    case '"': 
        {
            s.append("\\\"");
            break;
        }

    case '\b': 
        {
            s.append("\\b");
            break;
        }

    case '\f': 
        {
            s.append("\\f");
            break;
        }

    case '\n': 
        {
            s.append("\\n");
            break;
        }

    case '\r': 
        {
            s.append("\\r");
            break;
        }

    case '\t': 
        {
            s.append("\\t");
            break;
        }

    default: 
        {
            unsigned char i = static_cast<unsigned char>(b);
            if (!(i >= 32 && i <= 126))
            {
                s.push_back('\\');
                string octal = Util::String::ToOctalString(i);
                //
                // Add leading zeroes so that we avoid problems during
                // decoding. For example, consider the escaped string
                // \0013 (i.e., a character with value 1 followed by the
                // character '3'). If the leading zeroes were omitted, the
                // result would be incorrectly interpreted as a single
                // character with value 11.
                //
                for (string::size_type j = octal.size(); j < 3; j++)
                {
                    s.push_back('0');
                }
                s.append(octal);
            }
            else if (special.find(b) != string::npos)
            {
                s.push_back('\\');
                s.push_back(b);
            }
            else
            {
                s.push_back(b);
            }
            break;
        }
    }
}

}

//
// Add escape sequences (such as "\n", or "\007") to make a string
// readable in ASCII. Any characters that appear in special are
// prefixed with a backslash in the returned string.
//
string
String::EscapeString(const string& s, const string& special)
{
    for (string::size_type i = 0; i < special.size(); ++i)
    {
        if (static_cast<unsigned char>(special[i]) < 32 || static_cast<unsigned char>(special[i]) > 126)
        {
            throw IllegalArgumentException(__FILE__, __LINE__, "special characters must be in ASCII range 32-126");
        }
    }

    string result;
    for (string::size_type i = 0; i < s.size(); ++i)
    {
        EncodeChar(s[i], result, special);
    }

    return result;
}

namespace
{

char
CheckChar(const string& s, string::size_type pos)
{
    unsigned char c = static_cast<unsigned char>(s[pos]);
    if (!(c >= 32 && c <= 126))
    {
        ostringstream ostr;
        if (pos > 0)
        {
            ostr << "character after `" << s.substr(0, pos) << "'";
        }
        else
        {
            ostr << "first character";
        }
        ostr << " is not a printable ASCII character (ordinal " << (int)c << ")";
        throw IllegalArgumentException(__FILE__, __LINE__, ostr.str());
    }
    return c;
}

//
// Decode the character or escape sequence starting at start and return it.
// end marks the one-past-the-end position of the substring to be scanned.
// nextStart is set to the index of the first character following the decoded
// character or escape sequence.
//
char
DecodeChar(const string& s, string::size_type start, string::size_type end, string::size_type& nextStart)
{
    assert(start < end);
    assert(end <= s.size());

    char c;

    if (s[start] != '\\')
    {
        c = CheckChar(s, start++);
    }
    else
    {
        if (start + 1 == end)
        {
            throw IllegalArgumentException(__FILE__, __LINE__, "trailing backslash");
        }
        switch(s[++start])
        {
        case '\\': 
        case '\'': 
        case '"': 
            {
                c = s[start++];
                break;
            }
        case 'b': 
            {
                ++start;
                c = '\b';
                break;
            }
        case 'f': 
            {
                ++start;
                c = '\f';
                break;
            }
        case 'n': 
            {
                ++start;
                c = '\n';
                break;
            }
        case 'r': 
            {
                ++start;
                c = '\r';
                break;
            }
        case 't': 
            {
                ++start;
                c = '\t';
                break;
            }
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
            {
                int val = 0;
                for (int j = 0; j < 3 && start < end; ++j)
                {
                    int charVal = s[start++] - '0';
                    if (charVal < 0 || charVal > 7)
                    {
                        --start;
                        break;
                    }
                    val = val * 8 + charVal;
                }
                if (val > 255)
                {
                    ostringstream ostr;
                    ostr << "octal value \\" << oct << val << dec << " (" << val << ") is out of range";
                    throw IllegalArgumentException(__FILE__, __LINE__, ostr.str());
                }
                c = (char)val;
                break;
            }
        default:
            {
                c = CheckChar(s, start++);
                break;
            }
        }
    }
    nextStart = start;
    return c;
}

//
// Remove escape sequences from s and append the result to sb.
// Return true if successful, false otherwise.
//
void
DecodeString(const string& s, string::size_type start, string::size_type end, string& sb)
{
    while (start < end)
    {
        sb.push_back(DecodeChar(s, start, end, start));
    }
}

}

//
// Remove escape sequences added by escapeString.
//
string
String::UnescapeString(const string& s, string::size_type start, string::size_type end)
{
    assert(start <= end && end <= s.size());

    string result;
    result.reserve(end - start);
    result.clear();
    DecodeString(s, start, end, result);
    return result;
}

bool
String::SplitString(const string& str, const string& delim, vector<string>& result, bool keepblank)
{
    string::size_type pos = 0;
    string::size_type length = str.length();
    string elt;

    char quoteChar = '\0';
    while (pos < length)
    {
        if (quoteChar == '\0' && (str[pos] == '"' || str[pos] == '\''))
        {
            quoteChar = str[pos++];
            continue; // Skip the quote
        }
        else if (quoteChar == '\0' && str[pos] == '\\' && pos + 1 < length && 
            (str[pos + 1] == '\'' || str[pos + 1] == '"'))
        {
            ++pos;
        }
        else if (quoteChar != '\0' && str[pos] == '\\' && pos + 1 < length && str[pos + 1] == quoteChar)
        {
            ++pos;
        }
        else if (quoteChar != '\0' && str[pos] == quoteChar)
        {
            ++pos;
            quoteChar = '\0';
            continue; // Skip the end quote
        }
        else if (delim.find(str[pos]) != string::npos)
        {
            if (quoteChar == '\0')
            {
                ++pos;
                if (elt.length() > 0 || keepblank)
                {
                    result.push_back(elt);
                    elt = "";
                }
                continue;
            }
        }

        if (pos < length)
        {
            elt += str[pos++];
        }
    }

    if (elt.length() > 0 || keepblank)
    {
        result.push_back(elt);
    }
    if (quoteChar != '\0')
    {
        return false; // Unmatched quote.
    }
    return true;
}

string
String::JoinString(const std::vector<std::string>& values, const std::string& delimiter)
{
    ostringstream out;
    for (unsigned int i = 0; i < values.size(); i++)
    {
        if (i != 0)
        {
            out << delimiter;
        }
        out << values[i];
    }
    return out.str();
}

//
// Trim white space (" \t\r\n")
//
string
String::Trim(const string& s)
{
    static const string delim = " \t\r\n";
    string::size_type beg = s.find_first_not_of(delim);
    if (beg == string::npos)
    {
        return "";
    }
    else
    {
        return s.substr(beg, s.find_last_not_of(delim) - beg + 1);
    }
}

//
// If a single or double quotation mark is found at the start position,
// then the position of the matching closing quote is returned. If no
// quotation mark is found at the start position, then 0 is returned.
// If no matching closing quote is found, then -1 is returned.
//
string::size_type
String::CheckQuote(const string& s, string::size_type start)
{
    string::value_type quoteChar = s[start];
    if (quoteChar == '"' || quoteChar == '\'')
    {
        start++;
        string::size_type pos;
        while (start < s.size() && (pos = s.find(quoteChar, start)) != string::npos)
        {
            if (s[pos - 1] != '\\')
            {
                return pos;
            }
            start = pos + 1;
        }
        return string::npos; // Unmatched quote.
    }
    return 0; // Not quoted.
}

string::size_type
String::ExistQuote(const string& s, string::size_type start)
{
    string::size_type pos;
    while (start < s.size() && (pos = s.find_first_of("\"\'", start)) != string::npos)
    {
        if (s[pos - 1] != '\\')
        {
            return pos;
        }
        start = pos + 1;
    }

    return string::npos; // No quote.
}

//
// Match `s' against the pattern `pat'. A * in the pattern acts
// as a wildcard: it matches any non-empty sequence of characters.
// We match by hand here because it's portable across platforms 
// (whereas regex() isn't). Only one * per pattern is supported.
//
bool
String::Match(const string& s, const string& pat, bool emptyMatch)
{
    assert(!s.empty());
    assert(!pat.empty());

    //
    // If pattern does not contain a wildcard just compare strings.
    //
    string::size_type beginIndex = pat.find('*');
    if (beginIndex == string::npos)
    {
        return s == pat;
    }

    //
    // Make sure start of the strings match
    //
    if (beginIndex > s.length() || s.substr(0, beginIndex) != pat.substr(0, beginIndex))
    {
        return false;
    }

    //
    // Make sure there is something present in the middle to match the
    // wildcard. If emptyMatch is true, allow a match of "".
    //
    string::size_type endLength = pat.length() - beginIndex - 1;
    if (endLength > s.length())
    {
        return false;
    }
    string::size_type endIndex = s.length() - endLength;
    if (endIndex < beginIndex || (!emptyMatch && endIndex == beginIndex))
    {
        return false;
    }

    //
    // Make sure end of the strings match
    //
    if (s.substr(endIndex, s.length()) != pat.substr(beginIndex + 1, pat.length()))
    {
        return false;
    }

    return true;
}

std::string 
String::TranslatingCR2LF(const std::string& src)
{
    // Translating both the two-character sequence #xD #xA and any #xD that is not followed by #xA to 
    // a single #xA character.

    // Process the buffer in place to normalize new lines. (See comment above.)
    // Copies from the 'pread' to 'pwrite' pointer, where pread can advance faster if
    // a newline-carriage return is hit.
    //
    // Wikipedia:
    // Systems based on ASCII or a compatible character set use either LF  (Line feed, '\n', 0x0A, 10 in decimal) or 
    // CR (Carriage return, '\r', 0x0D, 13 in decimal) individually, or CR followed by LF (CR+LF, 0x0D 0x0A)...
    //        * LF:    Multics, Unix and Unix-like systems (GNU/Linux, AIX, Xenix, Mac OS X, FreeBSD, etc.), BeOS, Amiga, RISC OS, and others
    //        * CR+LF: DEC RT-11 and most other early non-Unix, non-IBM OSes, CP/M, MP/M, DOS, OS/2, Microsoft Windows, Symbian OS
    //        * CR:    Commodore 8-bit machines, Apple II family, Mac OS up to version 9 and OS-9

    std::string result(src);

    const char *pread = &result[0];
    char *pwrite = &result[0];
    const char *pend = &result[0] + result.size();

    const char CR = 0x0d;
    const char LF = 0x0a;

    while (pread != pend)
    {
        assert(pwrite <= pread);

        if (CR == *pread)
        {
            *pwrite++ = LF;
            ++pread;
            if (LF == *pread)
            {
                ++pread;
            }
        }
        else
        {
            *pwrite++ = *pread++;
        }
    }

    result.resize(pwrite - &result[0]);

    return result;
}

std::string
String::ToLower(const std::string& s)
{
    string result;
    result.reserve(s.size());
    for (unsigned int i = 0; i < s.length(); ++i)
    {
        if (isascii(s[i]))
        {
            result += tolower(static_cast<unsigned char>(s[i]));
        }
        else
        {
            result += s[i];
        }
    }
    return result;
}

std::string
String::ToUpper(const std::string& s)
{
    string result;
    result.reserve(s.size());
    for (unsigned int i = 0; i < s.length(); ++i)
    {
        if (isascii(s[i]))
        {
            result += toupper(static_cast<unsigned char>(s[i]));
        }
        else
        {
            result += s[i];
        }
    }
    return result;
}

unsigned long 
String::Hash(const std::string& s)
{
    unsigned long hashCode = s.length();
    size_t step = (s.length() >> 5) + 1;
    for (size_t i = s.length(); i >= step; i -= step)
    {
        hashCode = hashCode ^ ((hashCode << 5) + ( hashCode >> 2) + (unsigned long)s[i-1]);
    }

    return hashCode;
}

string
String::RemoveWhitespace(const std::string& s)
{
    string result;
    for (unsigned int i = 0; i < s.length(); ++ i)
    {
        if (!isspace(static_cast<unsigned char>(s[i])))
        {
            result += s[i];
        }
    }
    return result;
}

//////////////////////////////////////////////////////////////////////////
/// String To Int64
static const string allDigits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

//
// Table to convert ASCII digits/letters into their value (100 for unused slots)
//
static const char digitVal[] =
{
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9,                // '0' - '9'
    100, 100, 100, 100, 100, 100, 100,            // punctuation
    10, 11, 12, 13, 14, 15, 16, 17, 18, 19,        // 'A' - 'J'
    20, 21, 22, 23, 24, 25, 26, 27, 28, 29,     // 'K' - 'T'
    30, 31, 32, 33, 34, 35                        // 'U' - 'Z'
};

#ifdef __MINGW32__
//#if defined(__MINGW32__) || (defined(_MSC_VER) && (_MSC_VER < 1300))

namespace UtilInternal
{
//
// The MINGW runtime does not include _strtoi64, so we provide our own implementation
//

static Util::Int64 strToInt64Impl(const char* s, char** endptr, int base)
{
    //
    // Assume nothing will be there to convert for now
    //
    if (endptr)
    {
        *endptr = const_cast<char *>(s);
    }

    //
    // Skip leading whitespace
    //
    while (*s && isspace(static_cast<unsigned char>(*s)))
    {
        ++s;
    }

    //
    // Check for sign
    //
    int sign = 1;
    if (*s == '+')
    {
        ++s;
    }
    else if (*s == '-')
    {
        sign = -1;
        ++s;
    }

    //
    // Check that base is valid
    //
    if (base == 0)
    {
        if (*s == '0')
        {
            base = 8;
            ++s;

            //
            // We have at least this zero
            //
            if (endptr)
            {
                *endptr = const_cast<char *>(s);
            }

            if (*s == 'x' || *s == 'X')
            {
                base = 16;
                ++s;
            }
        }
        else
        {
            base = 10;
        }
    }
    else if (base < 2 || base > 36)
    {
        errno = EINVAL;
        return 0;
    }

    //
    // Check that we have something left to parse
    //
    if (*s == '\0')
    {
        //
        // We did not read any new digit so we don't update endptr
        //
        return 0;
    }

    Int64 result = 0;
    bool overflow = false;
    bool digitFound = false;
    const string validDigits(allDigits.begin(), allDigits.begin() + base);
    while (*s && validDigits.find_first_of(toupper(static_cast<unsigned char>(*s))) != validDigits.npos)
    {   
        digitFound = true;
        if (!overflow)
        {
            int digit = digitVal[toupper(static_cast<unsigned char>(*s)) - '0'];
            assert(digit != 100);
            if (result < _I64_MAX / base)
            {
                result *= base;
                result += digit;
            }
            else if ((digit <= _I64_MAX % base) || (sign == -1 && digit == _I64_MAX % base + 1))
            {
                result *= base;
                result += digit;
            }
            else
            {
                overflow = true;
                result = sign == -1 ? _I64_MIN : _I64_MAX;
            }
        }
        ++s;
    }

    if (overflow)
    {
        errno = ERANGE;
    }
    else
    {
        result *= sign;
    }

    if (digitFound && endptr != 0)
    {
        *endptr = const_cast<char *>(s);
    }

    return result;
}
}

#endif


Util::Int64 
String::ToInt64(const char* s, char** endptr, int base)
{
#if defined(_WIN32)
#   ifdef __MINGW32__
    return strToInt64Impl(s, endptr, base);
#   else
    return _strtoi64(s, endptr, base);
#   endif
#elif defined(UTIL_64)
    return strtol(s, endptr, base);
#elif defined(__hpux)
    return __strtoll(s, endptr, base);
#else
    return strtoll(s, endptr, base);
#endif
}

bool 
String::ToInt64(const string& s,  Util::Int64& result)
{
    const char* start = s.c_str();
    char* end = 0;
    errno = 0;
    result = ToInt64(start, &end, 0);
    return (errno == 0 && start != end);
}

unsigned long 
String::ToULong(const std::string& strval, size_t* endindex, unsigned int base)
{
    //
    // Assume nothing will be there to convert for now
    //
    if (endindex)
    {
        *endindex = 0;
    }

    std::string::const_pointer iter = strval.c_str();

    //
    // Skip leading whitespace
    //
    while (*iter && isspace(static_cast<unsigned char>(*iter)))
    {
        ++iter;
    }

    //
    // Check for sign
    //
    int sign = 1;
    if ('+' == *iter)
    {
        ++iter;
    }
    else if ('-' == *iter)
    {
        sign = -1;
        ++iter;
    }

    //
    // Check that base is valid
    //
    if (base == 0)
    {
        if ('0' == *iter)
        {
            base = 8;
            ++iter;

            //
            // We have at least this zero
            //
            if (endindex)
            {
                *endindex = iter - strval.c_str();
            }

            if ('x' == *iter || 'X' == *iter)
            {
                base = 16;
                ++iter;
            }
        }
        else
        {
            base = 10;
        }
    }
    else if (base < 2 || base > 16)    //else if (2 != base && 8 != base && 10 != base && 16 != base)
    {
        errno = EINVAL;
        return 0;
    }

    int exp = 0;
    unsigned long result = 0;
    unsigned value;
    while (isxdigit(*iter) 
        && (value = isdigit(*iter) ? *iter - '0' : toupper(*iter) - 'A' + 10) < base)
    {
        result = result * base + value;
        ++iter;
    }

    if (10 == base)
    {
        if ('.' == *iter)
        {
            ++iter;
            for (int i = -1; isdigit(*iter); --i, ++iter)
            {
                result = result *base + (*iter - '0');
                exp = i;
            }
        }

        if ('E' == toupper(*iter))
        {
            ++iter;
            string strexp;
            strexp.reserve(strval.size() - (iter - strval.c_str()));
            if ('+' == *iter || '-' == *iter)
            {
                strexp += *iter;
                ++iter;
            }

            while (isdigit(*iter))
            {
                strexp += *iter;
                ++iter;
            }

            exp += ToLong(strexp, 0, 10);
        }
    }

    if (endindex)
    {
        *endindex = iter - strval.c_str();
    }

    result = static_cast<unsigned long>(result * ::pow(1.0 * base, exp) + 0.5);

    return sign * result;
}

long 
String::ToLong(const std::string& strval, size_t* endindex, unsigned int base)
{
    return static_cast<long>(ToULong(strval, endindex, base));
}

double 
String::ToDouble(const std::string& strval, size_t* endindex, int precision)
{
    //
    // Assume nothing will be there to convert for now
    //
    if (endindex)
    {
        *endindex = 0;
    }

    std::string::const_pointer iter = strval.c_str();

    //
    // Skip leading whitespace
    //
    while (*iter && isspace(static_cast<unsigned char>(*iter)))
    {
        ++iter;
    }

    //
    // Check for sign
    //
    int sign = 1;
    if ('+' == *iter)
    {
        ++iter;
    }
    else if ('-' == *iter)
    {
        sign = -1;
        ++iter;
    }

    int exp = 0;
    double result = 0;    
    while (isdigit(*iter))
    {
        result = result * 10 + *iter - '0';
        ++iter;
    }

    if ('.' == *iter)
    {
        ++iter;
        for (int i = -1; precision-- && isdigit(*iter); --i, ++iter)
        {
            result = result * 10 + (*iter - '0');
            exp = i;
        }
    }

    if ('E' == toupper(*iter))
    {
        ++iter;
        string strexp;
        strexp.reserve(strval.size() - (iter - strval.c_str()));
        if ('+' == *iter || '-' == *iter)
        {
            strexp += *iter;
            ++iter;
        }

        while (isdigit(*iter))
        {
            strexp += *iter;
            ++iter;
        }

        exp += ToLong(strexp, 0, 10);
    }

    if (endindex)
    {
        *endindex = iter - strval.c_str();
    }

    result *= ::pow(10.0, exp);

    return sign * result;
}

std::string 
String::ToString(unsigned long n)
{
    string s;
    size_t size = sizeof(unsigned long) * 8;
    s.resize(size);
    string::size_type charPos = size;

    do
    {
        s[--charPos] = n % 10 + '0';
        n /= 10;
    }while (0 != n);

    return string(s, charPos, (size - charPos));
}

std::string 
String::ToString(long n)
{
    string s;
    size_t size = sizeof(unsigned long) * 8;
    s.resize(size);
    string::size_type charPos = size;

    bool negative = false;
    if (n < 0)
    {
        n = -n;
        negative = true;
    }

    do
    {
        s[--charPos] = n % 10 + '0';
        n /= 10;
    }while (0 != n);

    if (negative)
    {
        s[--charPos] = '-';
    }

    return string(s, charPos, (size - charPos));
}

bool 
String::IsNumber(const std::string& s, bool* isdecimal)
{
    const char* src(s.c_str());

    if ('-' == *src || '+' == *src)
    {
        ++src;
    }

    if (!IsDigit(*src))
    {
        return false;
    }

    ++src;

    while (IsDigit(*src))
    {
        ++src;
    }

    isdecimal ? *isdecimal = false : 0/*do nothing*/;

    if ('.' == *src)
    {
        ++src;
        if (!IsDigit(*src))
        {
            return false;
        }

        while (IsDigit(*src))
        {
            ++src;
        }
        isdecimal ? *isdecimal = true : 0/*do nothing*/;
    }
    if ('e' == *src || 'E' == *src)
    {
        ++src;
        if ('+' == *src || '-' == *src) 
        {
            ++src;
        }

        if (!IsDigit(*src))
        {
            return false;
        }

        while (IsDigit(*src))
        {
            ++src;
        }
        isdecimal ? *isdecimal = true : 0/*do nothing*/;
    }

    return 0 == *src;
}

int 
String::GetIntInString(const char* s, char** endptr, int base)
{
    //
    // Assume nothing will be there to convert for now
    //
    if (endptr)
    {
        *endptr = const_cast<char *>(s);
    }

    //
    // Skip leading none digit character
    //
    while (*s && '+' != *s && '-' != *s
        && !isdigit(static_cast<unsigned char>(*s)))
    {
        ++s;
    }

    //
    // Check for sign
    //
    int sign = 1;
    if (*s == '+')
    {
        ++s;
    }
    else if (*s == '-')
    {
        sign = -1;
        ++s;
    }

    //
    // Check that base is valid
    //
    if (base == 0)
    {
        if (*s == '0')
        {
            base = 8;
            ++s;

            //
            // We have at least this zero
            //
            if (endptr)
            {
                *endptr = const_cast<char *>(s);
            }

            if (*s == 'x' || *s == 'X')
            {
                base = 16;
                ++s;
            }
        }
        else
        {
            base = 10;
        }
    }
    else if (base < 2 || base > 36)
    {
        errno = EINVAL;
        return 0;
    }

    //
    // Check that we have something left to parse
    //
    if (*s == '\0')
    {
        //
        // We did not read any new digit so we don't update endptr
        //
        return 0;
    }

    int result = 0;
    bool overflow = false;
    bool digitFound = false;
    const string validDigits(allDigits.begin(), allDigits.begin() + base);
    while (*s && validDigits.find_first_of(toupper(static_cast<unsigned char>(*s))) != validDigits.npos)
    {   
        digitFound = true;
        if (!overflow)
        {
            int digit = digitVal[toupper(static_cast<unsigned char>(*s)) - '0'];
            assert(digit != 100);
            if (result < _I32_MAX / base)
            {
                result *= base;
                result += digit;
            }
            else if ((digit <= _I32_MAX % base) || (sign == -1 && digit == _I32_MAX % base + 1))
            {
                result *= base;
                result += digit;
            }
            else
            {
                overflow = true;
                result = sign == -1 ? _I32_MIN : _I32_MAX;
            }
        }
        ++s;
    }

    if (overflow)
    {
        errno = ERANGE;
    }
    else
    {
        result *= sign;
    }

    if (digitFound && endptr != 0)
    {
        *endptr = const_cast<char *>(s);
    }

    return result;
}

const Util::Byte* 
String::FindStringInBuffer(Util::Byte* buffer, size_t buffsize, const std::string& strtosearch)
{
    if (buffsize < strtosearch.length()
        || "" == strtosearch)
    {
        return NULL;
    }

    for (size_t i = 0; i < buffsize; ++i)
    {
        size_t curindex = i;
        const char *search = strtosearch.c_str();

        while (*search && curindex < buffsize && *search == buffer[curindex])
        {
            ++curindex;
            ++search;
        }

        if (!*search)
        {
            return buffer + i;
        }
    }

    return NULL;
}

string 
String::BytesToString(const Byte* src, size_t size)
{
    const Byte* end(src + size);
#if 0
    ostringstream s;
    for (; src != end; ++src)
    {
        s << setw(2) << setfill('0') << hex << static_cast<int>(*src);
    }

    return s.str();

#else

    static const char* toHex = "0123456789abcdef";

    string s;
    s.resize(size * 2);

    for (unsigned int i = 0 ; src != end; ++src, ++i)
    {
        s[i * 2] = toHex[(*src >> 4) & 0xf];
        s[i * 2 + 1] = toHex[*src & 0xf];
    }

    return s;
#endif
}

string
String::BytesToString(const ByteSeq& bytes)
{
    return BytesToString(&*bytes.begin(), bytes.size());

#if 0
    ostringstream s;
    for (ByteSeq::const_iterator p = bytes.begin(); p != bytes.end(); ++p)
    {
        s << setw(2) << setfill('0') << hex << static_cast<int>(*p);
    }

    return s.str();

    //#else

    static const char* toHex = "0123456789abcdef";

    string s;
    s.resize(bytes.size() * 2);

    for (unsigned int i = 0; i < bytes.size(); ++i)
    {
        s[i * 2] = toHex[(bytes[i] >> 4) & 0xf];
        s[i * 2 + 1] = toHex[bytes[i] & 0xf];
    }

    return s;
#endif

}

ByteSeq
String::StringToBytes(const string& str)
{
    ByteSeq bytes;
    bytes.reserve((str.size() + 1) / 2);

    for(unsigned int i = 0; i + 1 < str.size(); i += 2)
    {
        int byte = 0;
#if 0

        istringstream is(str.substr(i, 2));
        is >> hex >> byte;

#else
        for(int j = 0; j < 2; ++j)
        {
            char c = str[i + j];

            if(c >= '0' && c <= '9')
            {
                byte |= c - '0';
            }
            else if(c >= 'a' && c <= 'f')
            {
                byte |= 10 + c - 'a';
            }
            else if(c >= 'A' && c <= 'F')
            {
                byte |= 10 + c - 'A';
            }

            if(j == 0)
            {
                byte <<= 4;
            }
        }
#endif
        bytes.push_back(static_cast<Byte>(byte));
    }

    return bytes;
}

size_t 
String::HexDumpLine(const void* ptr, size_t offset, size_t size, std::string& line, size_t linelength)
{
    // Line layout:
    // 12: address
    // 1: space
    // (1+2)*linelength: hex bytes, each preceded by a space
    // 1: space separating the two halves
    // 4: "   |"
    // 1*linelength: characters
    // 1: "|"
    // Total: 19 + linelength * (1 + 2 + 1)
    line.clear();
    line.reserve(19 + linelength * (1 + 2 + 1));
    const Util::Byte* srcstr = reinterpret_cast<const Util::Byte*>(ptr) + offset;
    size_t length = std::min(size - offset, linelength);

    std::ostringstream out;
    out << std::hex << std::uppercase;
    out << '[';
    out.width(8);
    out.fill('0');
    out << offset << "h]: ";

    for (size_t i = 0; i < length; ++i)
    {
        if (i == linelength / 2)
        {
            out << ' ';
        }

        out.width(2);
        out.fill('0');
        out << (srcstr[i] & 0x00ff) << ' ';
    }

    // 3 spaces for each byte we're not printing, one separating the halves
    // if necessary
    out << string(3 * (linelength - length) + (length <= linelength / 2), ' ');
    out << "   |";
    line.append(out.str());

    for (size_t i = 0; i < length; i++) 
    {
        char c = (srcstr[i] >= 32 && srcstr[i] <= 126 ? static_cast<char>(srcstr[i]) : '.');
        line.append(1, c);
    }
    line.append(linelength - length, ' ');
    line.push_back('|');

    assert(line.size() == 19 + linelength * 4); 

    return length;
}

std::string 
String::HexDump(const void* ptr, size_t size, size_t linelength) 
{
    std::ostringstream os;
    HexDump(ptr, size, std::ostream_iterator<const char*>(os, "\n"), linelength);
    return os.str();
}

std::string 
String::HexStringToBuffer(const string &hexString, string &buffer, const std::string& delimiter)
{
    //
    //For the "C" locale, white-space characters are any of:
    //    ' '        (0x20)    space (SPC)
    //    '\t'    (0x09)    horizontal tab (TAB)
    //    '\n'    (0x0a)    newline (LF)
    //    '\v'    (0x0b)    vertical tab (VT)
    //    '\f'    (0x0c)    feed (FF)
    //    '\r'    (0x0d)    carriage return (CR)
    //

    const string IFS(delimiter + " \t\r\n\v\f"); // Internal Field Separator(space is default separator).

    buffer.clear();
    string::const_pointer data = hexString.c_str();
    size_t pos(hexString.find("h]:"));    // skip address info if exist.
    if (string::npos != pos)
    {
        data += pos + 3;
    }

    while ('\0' != data)
    {
        if (string::npos != IFS.find(*data))
        {
            ++data;
            continue;
        }
        else if (!isxdigit(*data) && string::npos == string("+-").find(*data))
        {
            break;
        }

        int sign = 1;
        if ('+' == *data)
        {
            ++data;
        }
        else if ('-' == *data)
        {
            sign = -1;
            ++data;
        }

        if ('\0' == data)
        {
            break;
        }

        if ('0' == *data && 'x' == tolower(*(data + 1)))
        {
            data += 2;
        }

        string item;
        item.reserve(128);        // for performance
        while (*data && isxdigit(*data))
        {
            item.push_back(*data);
            ++data;
        }

        if (0 != item.size() % 2)
        {
            item = '0' + item;
        }

        string::const_iterator citer = item.begin();

        while (item.end() != citer)
        {
            unsigned short ucdata;
            std::stringstream is(string(citer, citer + 2));
            is >> std::hex >> ucdata;
            ucdata = (unsigned short)ToULong(string(citer, citer + 2), 0, 16);
            buffer += (char)(sign * ucdata);

            citer += 2;
        }
    }

    return buffer;
}

size_t
String::BinDumpLine(const void* ptr, size_t offset, size_t size, std::string& line, size_t linelength)
{
    // Line layout:
    // 12: address
    // 1: space
    // (1+8)*linelength: binary bytes, each preceded by a space
    // 1: space separating the two halves
    // 4: "   |"
    // 1*linelength: characters
    // 1: "|"
    // Total: 19 + linelength * (1 + 8 + 1)
    line.clear();
    line.reserve(19 + linelength * (1 + 8 + 1));
    const Util::Byte* srcstr = reinterpret_cast<const Util::Byte*>(ptr) + offset;
    size_t length = std::min(size - offset, linelength);

    std::ostringstream out;
    out << std::hex << std::uppercase;
    out << '[';
    out.width(8);
    out.fill('0');
    out << offset << "h]: ";

    for (size_t i = 0; i < length; ++i)
    {
        if (i == linelength / 2)
        {
            out << ' ';
        }

        bitset<8> bits(srcstr[i]);
        out << bits.to_string() << ' ';
    }

    // 9 spaces for each byte we're not printing, one separating the halves
    // if necessary
    out << string(9 * (linelength - length) + (length <= linelength / 2), ' ');
    out << "   |";
    line.append(out.str());

    for (size_t i = 0; i < length; i++) 
    {
        char c = (srcstr[i] >= 32 && srcstr[i] <= 126 ? static_cast<char>(srcstr[i]) : '.');
        line.append(1, c);
    }
    line.append(linelength - length, ' ');
    line.push_back('|');

    assert(line.size() == 19 + linelength * 10); 

    return length;
}

std::string 
String::BinDump(const void* ptr, size_t size, size_t linelength) 
{
    std::ostringstream os;
    BinDump(ptr, size, std::ostream_iterator<const char*>(os, "\n"), linelength);
    return os.str();
}

// If *pstr starts with the given prefix, modifies *pstr to be right
// past the prefix and returns true; otherwise leaves *pstr unchanged
// and returns false.  None of pstr, *pstr, and prefix can be NULL.
bool 
String::SkipPrefix(const char* prefix, const char** pstr) 
{
    const size_t prefix_len = strlen(prefix);
    if (strncmp(*pstr, prefix, prefix_len) == 0) 
    {
        *pstr += prefix_len;
        return true;
    }
    return false;
}

// Formats an int value as "%02d".
std::string FormatIntWidth2(int value) 
{
    std::stringstream ss;
    ss << std::setfill('0') << std::setw(2) << value;
    return ss.str();
}

// Formats an int value as "%X".
std::string FormatHexInt(int value) 
{
    std::stringstream ss;
    ss << std::hex << std::uppercase << value;
    return ss.str();
}

// Formats a byte as "%02X".
std::string FormatByte(unsigned char value) 
{
    std::stringstream ss;
    ss << std::setfill('0') << std::setw(2) << std::hex << std::uppercase
        << static_cast<unsigned int>(value);
    return ss.str();
}

// Converts the buffer in a stringstream to an std::string, converting NUL
// bytes to "\\0" along the way.
std::string StringStreamToString(::std::stringstream* ss) 
{
    const ::std::string& str = ss->str();
    const char* const start = str.c_str();
    const char* const end = start + str.length();

    std::string result;
    result.reserve(2 * (end - start));
    for (const char* ch = start; ch != end; ++ch) 
    {
        if (*ch == '\0') 
        {
            result += "\\0";  // Replaces NUL with "\\0";
        } 
        else 
        {
            result += *ch;
        }
    }

    return result;
}

// Utility functions for encoding Unicode text (wide strings) in
// UTF-8.

// A Unicode code-point can have upto 21 bits, and is encoded in UTF-8
// like this:
//
// Code-point length   Encoding
//   0 -  7 bits       0xxxxxxx
//   8 - 11 bits       110xxxxx 10xxxxxx
//  12 - 16 bits       1110xxxx 10xxxxxx 10xxxxxx
//  17 - 21 bits       11110xxx 10xxxxxx 10xxxxxx 10xxxxxx

// The maximum code-point a one-byte UTF-8 sequence can represent.
const unsigned int kMaxCodePoint1 = (static_cast<unsigned int>(1) <<  7) - 1;

// The maximum code-point a two-byte UTF-8 sequence can represent.
const unsigned int kMaxCodePoint2 = (static_cast<unsigned int>(1) << (5 + 6)) - 1;

// The maximum code-point a three-byte UTF-8 sequence can represent.
const unsigned int kMaxCodePoint3 = (static_cast<unsigned int>(1) << (4 + 2*6)) - 1;

// The maximum code-point a four-byte UTF-8 sequence can represent.
const unsigned int kMaxCodePoint4 = (static_cast<unsigned int>(1) << (3 + 3*6)) - 1;

// Chops off the n lowest bits from a bit pattern.  Returns the n
// lowest bits.  As a side effect, the original bit pattern will be
// shifted to the right by n bits.
inline unsigned int ChopLowBits(unsigned int* bits, int n) 
{
    const unsigned int low_bits = *bits & ((static_cast<unsigned int>(1) << n) - 1);
    *bits >>= n;
    return low_bits;
}

// Converts a Unicode code point to a narrow string in UTF-8 encoding.
// code_point parameter is of type unsigned int because wchar_t may not be
// wide enough to contain a code point.
// If the code_point is not a valid Unicode code point
// (i.e. outside of Unicode range U+0 to U+10FFFF) it will be converted
// to "(Invalid Unicode 0xXXXXXXXX)".
std::string CodePointToUtf8(unsigned int code_point)
{
    if (code_point > kMaxCodePoint4) 
    {
        return "(Invalid Unicode 0x" + FormatHexInt(code_point) + ")";
    }

    char str[5];  // Big enough for the largest valid code point.
    if (code_point <= kMaxCodePoint1) 
    {
        str[1] = '\0';
        str[0] = static_cast<char>(code_point);                          // 0xxxxxxx
    }
    else if (code_point <= kMaxCodePoint2) 
    {
        str[2] = '\0';
        str[1] = static_cast<char>(0x80 | ChopLowBits(&code_point, 6));  // 10xxxxxx
        str[0] = static_cast<char>(0xC0 | code_point);                   // 110xxxxx
    }
    else if (code_point <= kMaxCodePoint3)
    {
        str[3] = '\0';
        str[2] = static_cast<char>(0x80 | ChopLowBits(&code_point, 6));  // 10xxxxxx
        str[1] = static_cast<char>(0x80 | ChopLowBits(&code_point, 6));  // 10xxxxxx
        str[0] = static_cast<char>(0xE0 | code_point);                   // 1110xxxx
    }
    else // code_point <= kMaxCodePoint4
    {  
        str[4] = '\0';
        str[3] = static_cast<char>(0x80 | ChopLowBits(&code_point, 6));  // 10xxxxxx
        str[2] = static_cast<char>(0x80 | ChopLowBits(&code_point, 6));  // 10xxxxxx
        str[1] = static_cast<char>(0x80 | ChopLowBits(&code_point, 6));  // 10xxxxxx
        str[0] = static_cast<char>(0xF0 | code_point);                   // 11110xxx
    }
    return str;
}

// The following two functions only make sense if the the system
// uses UTF-16 for wide string encoding. All supported systems
// with 16 bit wchar_t (Windows, Cygwin, Symbian OS) do use UTF-16.

// Determines if the arguments constitute UTF-16 surrogate pair
// and thus should be combined into a single Unicode code point
// using CreateCodePointFromUtf16SurrogatePair.
inline bool IsUtf16SurrogatePair(wchar_t first, wchar_t second) 
{
    return sizeof(wchar_t) == 2 &&
        (first & 0xFC00) == 0xD800 && (second & 0xFC00) == 0xDC00;
}

// Creates a Unicode code point from UTF16 surrogate pair.
inline unsigned int CreateCodePointFromUtf16SurrogatePair(wchar_t first,
                                                          wchar_t second) 
{
    const unsigned int mask = (1 << 10) - 1;

    return (sizeof(wchar_t) == 2) ?
        (((first & mask) << 10) | (second & mask)) + 0x10000 :
    // This function should not be called when the condition is
    // false, but we provide a sensible default in case it is.
    static_cast<unsigned int>(first);
}

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
std::string WideStringToUtf8(const wchar_t* str, int num_chars)
{
    if (-1 == num_chars)
    {
        num_chars = static_cast<int>(wcslen(str));
    }

    ::std::stringstream stream;
    for (int i = 0; i < num_chars; ++i)
    {
        unsigned int unicode_code_point;

        if (str[i] == L'\0') 
        {
            break;
        } 
        else if (i + 1 < num_chars && IsUtf16SurrogatePair(str[i], str[i + 1]))
        {
            unicode_code_point = 
                CreateCodePointFromUtf16SurrogatePair(str[i], str[i + 1]);
            i++;
        } 
        else 
        {
            unicode_code_point = static_cast<unsigned int>(str[i]);
        }

        stream << CodePointToUtf8(unicode_code_point);
    }
    return StringStreamToString(&stream);
}

// Converts a wide C string to an std::string using the UTF-8 encoding.
// NULL will be converted to "(null)".
std::string ShowWideCString(const wchar_t * wide_c_str)
{
    if (wide_c_str == NULL) 
    {
        return "(null)";
    }

    return WideStringToUtf8(wide_c_str, -1);
}

// Compares two wide C strings.  Returns true iff they have the same
// content.
//
// Unlike wcscmp(), this function can handle NULL argument(s).  A NULL
// C string is considered different to any non-NULL C string,
// including the empty string.
bool WideCStringEquals(const wchar_t * lhs, const wchar_t * rhs) 
{
    if (NULL == lhs) 
    {
        return rhs == NULL;
    }

    if (NULL == rhs) 
    {
        return false;
    }

    return 0 == wcscmp(lhs, rhs);
}

// Compares two C strings.  Returns true iff they have the same content.
//
// Unlike strcmp(), this function can handle NULL argument(s).  A NULL
// C string is considered different to any non-NULL C string,
// including the empty string.
bool CStringEquals(const char * lhs, const char * rhs)
{
    if (NULL == lhs)
    {
        return rhs == NULL;
    }

    if (NULL == rhs) 
    {
        return false;
    }

    return 0 == strcmp(lhs, rhs);
}

// Compares two C strings, ignoring case.  Returns true iff they have
// the same content.
//
// Unlike strcasecmp(), this function can handle NULL argument(s).  A
// NULL C string is considered different to any non-NULL C string,
// including the empty string.
bool CaseInsensitiveCStringEquals(const char * lhs, const char * rhs) 
{
    if (NULL == lhs)
    {
        return rhs == NULL;
    }
    if (NULL == rhs)
    {
        return false;
    }
    return 0 == stricmp(lhs, rhs);
}

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
bool CaseInsensitiveWideCStringEquals(const wchar_t* lhs, const wchar_t* rhs)
{
    if (NULL == lhs)
    {
        return rhs == NULL;
    }

    if (NULL == rhs) 
    {
        return false;
    }

#ifdef _WIN32
    return _wcsicmp(lhs, rhs) == 0;
#elif defined (__linux__) && !defined(__ANDROID__)
    return wcscasecmp(lhs, rhs) == 0;
#else
    // Android, Mac OS X and Cygwin don't define wcscasecmp.
    // Other unknown OSes may not define it either.
    wint_t left, right;
    do
    {
        left = towlower(*lhs++);
        right = towlower(*rhs++);
    } while (left && left == right);
    return left == right;
#endif  // OS selector
}

// Returns true iff str ends with the given suffix, ignoring case.
// Any string is considered to end with an empty suffix.
bool EndsWithCaseInsensitive(const std::string& str, const std::string& suffix)
{
    const size_t str_len = str.length();
    const size_t suffix_len = suffix.length();
    return (str_len >= suffix_len) &&
        CaseInsensitiveCStringEquals(str.c_str() + str_len - suffix_len,
        suffix.c_str());
}

//////////////////////////////////////////////////////////////////////////
/// string format
static string 
formatImpl( const char* format, va_list arglist)
{
    va_list tmpvarlist = arglist;

    if (NULL == format)
    {
        return "";
    }

    // MSVC 8 deprecates vsnprintf(), so we want to suppress warning
    // 4996 (deprecated function) there.
#ifdef _MSC_VER  // We are using MSVC.
# pragma warning(push)          // Saves the current warning state.
# pragma warning(disable:4996)  // Temporarily disables warning 4996.

    const int size = vsnprintf(NULL, 0, format, tmpvarlist);

# pragma warning(pop)           // Restores the warning state.
#else  // We are not using MSVC.
    const int size = vsnprintf(NULL, 0, format, tmpvarlist);
#endif  // _MSC_VER

    if(size < 0)
    {
        return "";
    }

    string str;
    str.resize(size);
    vsprintf((char*)str.c_str(), format, arglist);

    return str;
}

string 
Format( const char* format, ...)
{
    if (NULL == format)
    {
        return "";
    }

    va_list varList;
    va_start(varList, format);
    string str = formatImpl(format, varList);
    va_end(varList);

    return str;
}

double 
String2Double(const string& str)
{
    string value(str);
    double result = 0;
    double sign = 1;
    int scale = 0;
    int exponent = 0;
    int expsign = 1;
    int j = 0;
    int jMax = (int) value.length();
    if (jMax > 0)
    {
        if ('+' == value[j])
        {
            j++;
        }
        else if ('-' == value[j])
        {
            sign = -1;
            j++;
        }
        while (j < jMax && IsDigit(value[j]))
        {
            result = result * 10 + (value[j] - '0');
            j++;
        }
        if (j < jMax && value[j] == '.')
        {
            j++;
            while (j < jMax && IsDigit(value[j]))
            {
                result = result*10 + (value[j] - '0');
                scale++;
                j++;
            }
        }
        if (j < jMax && (value[j] == 'E' || value[j] == 'e'))
        {
            j++;
            if ('+' == value[j])
            {
                j++;
            }
            else if ('-' == value[j])
            {
                expsign = -1;
                j++;
            }
            while (j < jMax && IsDigit(value[j]))
            {
                exponent = exponent*10 + (value[j] - '0');
                j++;
            }
            exponent *= expsign;
        }
        result = sign * result * pow(10.0, exponent-scale);
    }
    return result;
}

string 
Double2String(double value, int precision)
{
    string number;
    if (precision < 0)
    {
        precision = 0;
    }
    else if (precision > 16)
    {
        precision = 16;
    }

    // Use absolute value locally
    double localValue = fabs(value);
    double localFraction = (localValue - floor(localValue)) +(5. * pow(10.0, -precision-1));
    if (localFraction >= 1)
    {
        localValue += 1.0;
        localFraction -= 1.0;
    }
    localFraction *= pow(10.0, precision);

    if (value < 0)
    {
        number += "-";
    }

    number += Format("%.0f", floor(localValue));

    // generate fraction, padding with zero if necessary.
    if (precision > 0)
    {
        number += ".";
        string fraction = Format("%.0f", floor(localFraction));
        if (fraction.length() < ((size_t) precision))
        {
            number += string(precision - fraction.length(), '0');
        }
        number += fraction;
    }

    return number;
}

bool
IsAlpha(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool
IsDigit(char c)
{
    return c >= '0' && c <= '9';
}

UTIL_END
