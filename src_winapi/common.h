#ifndef COMMON_H
#define COMMON_H

#include "compilersupport.h"
#include <string>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cassert>

#if defined UNICODE || defined _UNICODE

#include <cwchar>

typedef std::wstring        u_string;
typedef std::wifstream      u_ifstream;

inline size_t u_strlen(const wchar_t * str) { return wcslen(str); }

#else

typedef std::string        u_string;
typedef std::ifstream      u_ifstream;

inline size_t u_strlen(const char * str) { return strlen(str); }

#endif

// Основано на коде из OpenBSD
// http://cvsweb.openbsd.org/cgi-bin/cvsweb/src/lib/libc/string/strlcpy.c

// Copy string src to buffer dst of size dsize.  At most dsize-1
// chars will be copied.  Always NUL terminates (unless dsize == 0).
// Returns strlen(src); if retval >= dsize, truncation occurred.
template<typename T>
size_t u_strlcpy(T * dst, const T * src, size_t dsize)
{
    const T * osrc = src;
    size_t nleft = dsize;
    // Copy as many bytes as will fit.
    if(nleft != 0)
    {
        while(--nleft != 0)
        {
            if((*dst++ = *src++) == 0)
                break;
        }
    }
    // Not enough room in dst, add NUL and traverse rest of src.
    if(nleft == 0)
    {
        if(dsize != 0)
            *dst = 0;       // NUL-terminate dst
        while(*src++);
    }
    return(src - osrc - 1); // count does not include NUL
}

#endif // COMMON_H
