#ifndef COMPILERSUPPORT_H
#define COMPILERSUPPORT_H

// ===== ALL =============================================================================

// UNICODE для windows.h, _UNICODE для C runtime
// TEXT для windows.h, _TEXT для C runtime
// И прочее. Понятое дело, что это стоит использовать совместно.

#if defined _UNICODE
#if !defined UNICODE
#define UNICODE
#endif
#endif

#if defined UNICODE
#if !defined _UNICODE
#define _UNICODE
#endif
#endif

// ===== MINGW ===========================================================================

#if defined __MINGW32__ || defined __MINGW64__ || defined __GNUC__

// Не умеет wifstream
#if defined UNICODE || defined _UNICODE
#define wifstream     wifstream_unused
#include <cwchar>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <iostream>
#include <windows.h>
#undef wifstream

namespace std
{
    // Обертка над обычным ifstream
    class wifstream : public ifstream
    {
    public:
        wifstream(const wchar_t * filename, ios_base::openmode mode = ios_base::in)
        {
            open(filename, mode);
        }
        void open(const wchar_t * filename, ios_base::openmode mode = ios_base::in)
        {
            const int bufsize = FILENAME_MAX;
            char * tmp = (char *)malloc(sizeof(char) * bufsize);
            WideCharToMultiByte(CP_ACP, 0, filename, -1, tmp, bufsize, 0, 0);
            tmp[bufsize - 1] = 0;
            ifstream::open(tmp, mode);
            free(tmp);
        }
        wifstream & getline(wchar_t * s, streamsize n)
        {
            char * s_c = (char *)malloc(sizeof(char) * n);
            ifstream::getline(s_c, n);
            MultiByteToWideChar(CP_ACP, 0, s_c, n, s, n);
            free(s_c);
            return * this;
        }
        wifstream & get(wchar_t & c)
        {
            char c_c;
            if(ifstream::get(c_c))
                MultiByteToWideChar(CP_ACP, 0, &c_c, 1, &c, 1);
            return * this;
        }
        template<typename T>
        wifstream & operator >> (T & val)
        {
            * (dynamic_cast<ifstream *>(this)) >> val;
            return * this;
        }
    };
}

// Не умеет юникодный wWinMain
// Поэтому грязный чит :D
#define wWinMain \
    WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) \
    { \
        const int bufsize = 8192; \
        wchar_t * tmp = (wchar_t *)malloc(sizeof(wchar_t) * bufsize); \
        MultiByteToWideChar(CP_ACP, 0, lpCmdLine, -1, tmp, bufsize); \
        tmp[bufsize - 1] = 0; \
        int status = wWinMain(hInstance, hPrevInstance, tmp, nCmdShow); \
        free(tmp); \
        return status; \
    } \
    int WINAPI wWinMain

#endif

#endif

// ===== MSVC 2003 =======================================================================

#if defined _MSC_VER && _MSC_VER <= 1310 && _MSC_VER > 1200

// Не умеет wifstream
#if defined UNICODE || defined _UNICODE
#define wifstream     wifstream_unused
#include <cwchar>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <windows.h>
#undef wifstream

namespace std
{
    // Обертка над обычным ifstream
    class wifstream : public ifstream
    {
    public:
        wifstream(const wchar_t * filename, ios_base::openmode mode = ios_base::in)
        {
            open(filename, mode);
        }
        void open(const wchar_t * filename, ios_base::openmode mode = ios_base::in)
        {
            const int bufsize = FILENAME_MAX;
            char * tmp = (char *)malloc(sizeof(char) * bufsize);
            WideCharToMultiByte(CP_ACP, 0, filename, -1, tmp, bufsize, 0, 0);
            tmp[bufsize - 1] = 0;
            ifstream::open(tmp, mode);
            free(tmp);
        }
        wifstream & getline(wchar_t * s, streamsize n)
        {
            char * s_c = (char *)malloc(sizeof(char) * n);
            ifstream::getline(s_c, n);
            MultiByteToWideChar(CP_ACP, 0, s_c, n, s, n);
            free(s_c);
            return * this;
        }
        wifstream & get(wchar_t & c)
        {
            char c_c;
            if(ifstream::get(c_c))
                MultiByteToWideChar(CP_ACP, 0, &c_c, 1, &c, 1);
            return * this;
        }
        template<typename T>
        wifstream & operator >> (T & val)
        {
            * (dynamic_cast<ifstream *>(this)) >> val;
            return * this;
        }
    };
}

#endif

#endif

// ===== MSVC 6.0 ========================================================================

#if defined _MSC_VER && _MSC_VER <= 1200

#if defined UNICODE || defined _UNICODE
#error No unicode support for old MSVC!
#endif

// Область видимости for сделана очень странно
#define for if (false) ; else for

// Нет стандартных математических функций в std::
#include <math.h>
namespace std
{
    inline float _hack_sqrt(float arg) {return (float)sqrt((double)arg);}
    inline float _hack_fabs(float arg) {return (float)fabs((double)arg);}
    inline float _hack_floor(float arg) {return (float)floor((double)arg);}
    inline float _hack_ceil(float arg) {return (float)ceil((double)arg);}
    inline float _hack_log10(float arg) {return (float)log10((double)arg);}
    inline float _hack_pow(float arg1, float arg2) {return (float)pow((double)arg1, (double)arg2);}
    inline float sqrt(float arg) {return _hack_sqrt(arg);}
    inline float fabs(float arg) {return _hack_fabs(arg);}
    inline float floor(float arg) {return _hack_floor(arg);}
    inline float ceil(float arg) {return _hack_ceil(arg);}
    inline float log10(float arg) {return _hack_log10(arg);}
    inline float pow(float arg1, float arg2) {return _hack_pow(arg1, arg2);}
}

#endif

// ===== Open WATCOM 1.9 =================================================================

#if defined __WATCOMC__

#if defined UNICODE || defined _UNICODE
#error No unicode support for WATCOMC!
#endif

// Нужно заинклудить раньше всех, ибо см ниже
#include <algorithm>

// В cstdlib забиты макросы min и max, они ломают numeric_limits
// Это отменит передефайнивание, но сохранит работоспособность лимитов
#define max max
#define min min

// Нет getline
#include <string>
#include <fstream>
#include <iostream>
#include <string.h>
namespace std
{
    // Определим getline
    template<typename T_string> // Шаблон только чтобы в хедере описать нормально
    istream & getline(istream & is, T_string & str)
    {
        str.clear();
        char c = '\0';
        while(is.good() && is.get(c) && c != '\n')
            str.push_back(c);
        return is;
    }
}

// Нет стандартных математических функций
#include <math.h>
inline float floorf(float arg) {return floor(arg);}
inline float ceilf(float arg) {return ceil(arg);}

#endif


#endif // COMPILERSUPPORT_H
