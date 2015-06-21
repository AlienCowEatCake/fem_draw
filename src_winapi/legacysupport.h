#ifndef LEGACYSUPPORT_H
#define LEGACYSUPPORT_H

// ===== MSVC 6.0 ========================================================================

#if defined _MSC_VER && _MSC_VER <= 1200

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

// В cstdlib забиты макросы min и max, они ломают numeric_limits
// Это отменит передефайнивание, но сохранит работоспособность лимитов
#define max max
#define min min

// Нет stringstream и нет getline
#include <strstream>
#include <string>
#include <fstream>
#include <iostream>
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
            return string(strstream::str());
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


#endif // LEGACYSUPPORT_H
