// **********************************************************************
//
// Copyright (c) 2010-2014 Bernard Luo. All rights reserved.
//
// <Email: luo (dot) xiaowei (at) hotmail (dot) com>
//
// **********************************************************************

#include <Util/ErrorToString.h>
#include <Unicoder/Unicode.h>

UTIL_BEGIN

using namespace std;

#ifdef _WIN32

string
ErrorToString(int error, LPCVOID source)
{
    if (error < WSABASEERR)
    {
#ifdef OS_WINRT

        int size = 256;
        Util::ScopedArray<wchar_t> lpMsgBuf(new wchar_t[size]);

        DWORD stored = 0;

        while (stored == 0)
        {
            stored = FormatMessageW(
                FORMAT_MESSAGE_FROM_SYSTEM |
                FORMAT_MESSAGE_IGNORE_INSERTS |
                (source != NULL ? FORMAT_MESSAGE_FROM_HMODULE : 0),
                source,
                error,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
                lpMsgBuf.Get(),
                size,
                NULL);

            if (stored == 0)
            {
                DWORD err = GetLastError();
                if (err == ERROR_INSUFFICIENT_BUFFER)
                {
                    if (size == 65536)
                    {
                        break; // already at the max size
                    }
                    else
                    {
                        size *= 4;
                        size = max(size, 65536);
                        lpMsgBuf.reset(new wchar_t[size]);
                    }
                }
                else
                {
                    break;
                }   
            }
        }

        LPWSTR msg = lpMsgBuf.Get();

#else
        LPWSTR msg = 0;

        DWORD stored = FormatMessageW(
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS |
            (source != NULL ? FORMAT_MESSAGE_FROM_HMODULE : 0),
            source,
            error,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
            reinterpret_cast<LPWSTR>(&msg),
            0,
            NULL);
#endif

        if (stored > 0)
        {
            assert(msg && wcslen(msg) > 0);
            wstring result = msg;
            if (result[result.length() - 1] == L'\n')
            {
                result = result.substr(0, result.length() - 2);
            }
#ifndef OS_WINRT
            if (msg)
            {
                LocalFree(msg);
            }
#endif
            return Util::WstringToString(result);
        }
        else
        {
#ifndef OS_WINRT
            if (msg)
            {
                LocalFree(msg);
            }
#endif
            ostringstream os;
            os << "unknown error: " << error;
            return os.str();
        }
    }

    switch(error)
    {
    case WSAEINTR:
        return "WSAEINTR";

    case WSAEBADF:
        return "WSAEBADF";

    case WSAEACCES:
        return "WSAEACCES";

    case WSAEFAULT:
        return "WSAEFAULT";

    case WSAEINVAL:
        return "WSAEINVAL";

    case WSAEMFILE:
        return "WSAEMFILE";

    case WSAEWOULDBLOCK:
        return "WSAEWOULDBLOCK";

    case WSAEINPROGRESS:
        return "WSAEINPROGRESS";

    case WSAEALREADY:
        return "WSAEALREADY";

    case WSAENOTSOCK:
        return "WSAENOTSOCK";

    case WSAEDESTADDRREQ:
        return "WSAEDESTADDRREQ";

    case WSAEMSGSIZE:
        return "WSAEMSGSIZE";

    case WSAEPROTOTYPE:
        return "WSAEPROTOTYPE";

    case WSAENOPROTOOPT:
        return "WSAENOPROTOOPT";

    case WSAEPROTONOSUPPORT:
        return "WSAEPROTONOSUPPORT";

    case WSAESOCKTNOSUPPORT:
        return "WSAESOCKTNOSUPPORT";

    case WSAEOPNOTSUPP:
        return "WSAEOPNOTSUPP";

    case WSAEPFNOSUPPORT:
        return "WSAEPFNOSUPPORT";

    case WSAEAFNOSUPPORT:
        return "WSAEAFNOSUPPORT";

    case WSAEADDRINUSE:
        return "WSAEADDRINUSE";

    case WSAEADDRNOTAVAIL:
        return "WSAEADDRNOTAVAIL";

    case WSAENETDOWN:
        return "WSAENETDOWN";

    case WSAENETUNREACH:
        return "WSAENETUNREACH";

    case WSAENETRESET:
        return "WSAENETRESET";

    case WSAECONNABORTED:
        return "WSAECONNABORTED";

    case WSAECONNRESET:
        return "WSAECONNRESET";

    case WSAENOBUFS:
        return "WSAENOBUFS";

    case WSAEISCONN:
        return "WSAEISCONN";

    case WSAENOTCONN:
        return "WSAENOTCONN";

    case WSAESHUTDOWN:
        return "WSAESHUTDOWN";

    case WSAETOOMANYREFS:
        return "WSAETOOMANYREFS";

    case WSAETIMEDOUT:
        return "WSAETIMEDOUT";

    case WSAECONNREFUSED:
        return "WSAECONNREFUSED";

    case WSAELOOP:
        return "WSAELOOP";

    case WSAENAMETOOLONG:
        return "WSAENAMETOOLONG";

    case WSAEHOSTDOWN:
        return "WSAEHOSTDOWN";

    case WSAEHOSTUNREACH:
        return "WSAEHOSTUNREACH";

    case WSAENOTEMPTY:
        return "WSAENOTEMPTY";

    case WSAEPROCLIM:
        return "WSAEPROCLIM";

    case WSAEUSERS:
        return "WSAEUSERS";

    case WSAEDQUOT:
        return "WSAEDQUOT";

    case WSAESTALE:
        return "WSAESTALE";

    case WSAEREMOTE:
        return "WSAEREMOTE";

    case WSAEDISCON:
        return "WSAEDISCON";

    case WSASYSNOTREADY:
        return "WSASYSNOTREADY";

    case WSAVERNOTSUPPORTED:
        return "WSAVERNOTSUPPORTED";

    case WSANOTINITIALISED:
        return "WSANOTINITIALISED";

    case WSAHOST_NOT_FOUND:
        return "WSAHOST_NOT_FOUND";

    case WSATRY_AGAIN:
        return "WSATRY_AGAIN";

    case WSANO_RECOVERY:
        return "WSANO_RECOVERY";

    case WSANO_DATA:
        return "WSANO_DATA";

    default:
        {
            ostringstream os;
            os << "unknown socket error: " << error;
            return os.str();
        }
    }
}

string
LastErrorToString()
{
    return ErrorToString(GetLastError());
}

#else

string
ErrorToString(int error)
{
    return strerror(error);
}

string
LastErrorToString()
{
    return ErrorToString(errno);
}

#endif

UTIL_END