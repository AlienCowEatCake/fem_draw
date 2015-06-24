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
typedef std::wstringstream  u_stringstream;
typedef std::wifstream      u_ifstream;

inline size_t u_strlen(const wchar_t * str) { return wcslen(str); }

#else

typedef std::string        u_string;
typedef std::stringstream  u_stringstream;
typedef std::ifstream      u_ifstream;

inline size_t u_strlen(const char * str) { return strlen(str); }

#endif

#endif // COMMON_H
