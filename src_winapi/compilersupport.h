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

// Не умеет wstringstream и wifstream
#if defined UNICODE || defined _UNICODE
#define wstringstream wstringstream_unused
#define wifstream     wifstream_unused
#include <cwchar>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <iostream>
#include <windows.h>
#undef wstringstream
#undef wifstream

namespace std
{
/*
    // Полная по C++98 реализация char_traits для wchar_t
    struct char_traits_wchar_t
    {
        typedef wchar_t char_type;
        typedef wint_t int_type;
        typedef streamoff off_type;
        typedef wstreampos pos_type;
        typedef mbstate_t state_type;
        static size_t length(const char_type * s)
        {
            return wcslen(s);
        }
        static void assign(char_type & r, const char_type & c)
        {
            r = c;
        }
        static char_type assign(char_type * p, size_t n, char_type c)
        {
            for(size_t i = 0; i < n; i++)
                p[i] = c;
            return c;
        }
        static bool eq(const char_type & c, const char_type & d)
        {
            return c == d;
        }
        static bool lt(const char_type & c, const char_type & d)
        {
            return c < d;
        }
        static int compare(const char_type * p, const char_type * q, size_t n)
        {
            while(n--)
            {
                if(!eq(*p, *q))
                    return lt(*p, *q);
                ++p;
                ++q;
            }
            return 0;
        }
        static const char_type * find(const char_type * p, size_t n, const char_type & c)
        {
            for(size_t i = 0; i < n; i++)
                if(eq(p[i], c)) return p + i;
            return NULL;
        }
        static char_type * copy(char_type * dest, const char_type * src, size_t n)
        {
            return (char_type *)memcpy(dest, src, sizeof(char_type) * n);
        }
        static char_type * move(char_type * dest, const char_type * src, size_t n)
        {
            char_type * tmp = (char_type *)malloc(sizeof(char_type) * n);
            copy(tmp, src, n);
            copy(dest, tmp, n);
            free(tmp);
            return dest;
        }
        static bool eq_int_type(const int_type & x, const int_type & y)
        {
            return x == y;
        }
        static int_type to_int_type(const char_type & c)
        {
            return (int_type)c;
        }
        static char_type to_char_type(const int_type & c)
        {
            return (char_type)c;
        }
        static int_type eof()
        {
            return WEOF;
        }
        static int_type not_eof(const int_type & c)
        {
            return !eq_int_type(eof(), c);
        }
    };

    // А вот это уже правильный wstringstream
    typedef basic_stringstream<wchar_t, char_traits_wchar_t> wstringstream;
*/

    // Обертка над обычным stringstream
    class wstringstream : public stringstream
    {
    public:
        wstring str() const
        {
            string s_c = stringstream::str();
            const int bufsize = s_c.length() + 1;
            wchar_t * tmp = (wchar_t *)malloc(sizeof(wchar_t) * bufsize);
            MultiByteToWideChar(CP_ACP, 0, s_c.c_str(), -1, tmp, bufsize);
            tmp[bufsize - 1] = 0;
            wstring s_w(tmp);
            free(tmp);
            return s_w;
        }
        wstringstream & operator << (const wchar_t * val)
        {
            const int bufsize = wcslen(val) + 1;
            char * tmp = (char *)malloc(sizeof(char) * bufsize);
            WideCharToMultiByte(CP_ACP, 0, val, -1, tmp, bufsize, 0, 0);
            tmp[bufsize - 1] = 0;
            * (dynamic_cast<stringstream *>(this)) << tmp;
            free(tmp);
            return * this;
        }
        template<typename T>
        wstringstream & operator << (const T & val)
        {
            * (dynamic_cast<stringstream *>(this)) << val;
            return * this;
        }
    };

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

// В cstdlib забиты макросы min и max, они ломают numeric_limits
// Это отменит передефайнивание, но сохранит работоспособность лимитов
#define max max
#define min min

// Нет stringstream и нет getline
#include <strstream>
#include <string>
#include <fstream>
#include <iostream>
#include <string.h>
namespace std
{
    // Определим класс stringstream из strstream
    // Из используемого нужна только операция str(), которая должна возвращать std::string
    class stringstream : public strstream
    {
    public:
        string str()
        {
            (*this) << '\0' << '\0' ; // Иногда такое впечатление, что строка внутри нетерминирована нулем
            flush();
            string s = strstream::str();
            rdbuf()->freeze(0);
            return s;
        }
    };

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
