#ifndef PAINTWIDGET_H
#define PAINTWIDGET_H

#if defined _MSC_VER
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "comdlg32.lib")
#pragma comment(lib, "shell32.lib")
#endif

#include "common.h"
#include <windows.h>
#include <windowsx.h>
#include <set>
#include <vector>
#include <algorithm>

using namespace std;

// Класс динамический массив (сокращенный)
template<typename value_type, typename size_type = size_t>
class dynarray_t
{
public:
    dynarray_t(size_type n = 0)         { _data = NULL; resize(n); }
    dynarray_t(const dynarray_t & x)    { * this = x; }
    ~dynarray_t()                       { delete [] _data; }
    inline size_type size() const       { return _size; }
    inline void clear()                 { resize(0); }
    dynarray_t & operator = (const dynarray_t & x)
    {
        if(this != & x) {
            resize(x._size);
            for(size_type i = 0; i < _size; ++i)
                _data[i] = x._data[i];
        }
        return * this;
    }
    void resize(size_type n)
    {
        _size = n;
        delete [] _data;
        _data = _size <= 0 ? NULL : new value_type [_size];
    }
    inline const value_type & operator [] (size_type n) const   { return _data[n]; }
    inline value_type & operator [] (size_type n)               { return _data[n]; }
protected:
    value_type * _data;
    size_type _size;
};

// Класс точка
class point
{
public:
    float x, y;
    point(float x = 0.0, float y = 0.0)
    {
        this->x = x;
        this->y = y;
    }
};

// Класс элемент сечения текплота
class tecplot_node
{
public:
    point coord;
    float * value;
};

// Класс треугольник
class triangle
{
public:
    point nodes[3];
    COLORREF * color;
    float * solution[3];
    triangle() {}
    triangle(const point & node1, const point & node2, const point & node3)
    {
        nodes[0] = node1;
        nodes[1] = node2;
        nodes[2] = node3;
    }
};

// Класс конфигурация для объектов-линий (изолиний, векторов)
class lines_config
{
public:
    COLORREF color;
    int width;
    float length;
    float arrow_size;
    lines_config(const COLORREF & n_color = RGB(0, 0, 0), int n_width = 1, float n_length = 10.0f, float n_arrow_size = 5.0f)
        : color(n_color), width(n_width), length(n_length), arrow_size(n_arrow_size) {}
};

// Класс виджет для рисования через WinAPI
class paintwidget
{
public:
    // Конструктор
    paintwidget();

    // Деструктор
    ~paintwidget();

    // Флаг отрисовки изолиний
    bool draw_isolines;

    // Флаг закраски цветом
    bool draw_color;

    // Пересчет значений изолиний
    void set_isolines_num(size_t isolines_num);

    // Индекс что сейчас рисуем
    size_t draw_index;

    // Чтение текплотовских значений из файла
    void tec_read(LPCTSTR filename);

    // Количество изолиний
    size_t isolines_num;

    // Переменные по которым рисуем вектора
    size_t ind_vec_1, ind_vec_2;

    // Флаг надобности векторов
    bool draw_vectors;

    // Число пропускаемых векторов
    size_t skip_vec;

    // Изменение уровня интерполяции
    void set_div_num(size_t num);

    // Подписи к переменным
    vector<string> variables;

    // Есть данные
    bool is_loaded;

    // Рекомендуемое значение для spinBox_Vectors
    int vect_value;

    // Уровень интерполяции
    size_t div_num;

    // Отрисовка сцены
    void paintEvent();

    // Флаг валидности изображения
    bool hbmp_is_valid;

    // WinAPI, мать его
    HWND hwnd;
    PAINTSTRUCT ps;
    HBITMAP hbmp;

    // Конфигурация изолиний
    lines_config isolines_config;
    // Конфигурация векторов
    lines_config vectors_config;
    // Использовать ли фиолетовые оттенки цвета
    // Дает большую комбинацию оттенков, но приводит к
    // некоторому дисбалансу между красным и синим
    bool use_purple;
    // Рисовать ли легенду справа
    bool use_legend;
    // Приглушать ли чрезмерно яркие цвета
    // На мониторе они выглядят ничего, но если печатать...
    bool use_light_colors;
    // Лимит по памяти
    bool use_memory_limit;

    // Заголовок
    u_string title;

private:
    // Текплотовские значения
    dynarray_t<tecplot_node> tec_data;

    // Минимальные и максимальные значения геометрии + размер
    float min_x, max_x, size_x;
    float min_y, max_y, size_y;

    // Количество шагов координатной сетки
    size_t num_ticks_x, num_ticks_y;
    // Подгонка осей под реальность и вычисление шагов координатной сетки
    void adjustAxis(float & min, float & max, size_t & numTicks) const;

    // Минимальное и максимальное значения решения
    dynarray_t<float> min_u, max_u;

    // Вспомогательные шаги по цвету для закраски
    dynarray_t<float> step_u_big, step_u_small;

    // Значения изолиний
    dynarray_t<set<float> > isolines;

    // Треугольники, которые будем рисовать
    dynarray_t<triangle> triangles;

    // Геометрия окна
    int height, width;
    void to_window(float x, float y, int & xl, int & yl) const;

    // Подписи осей
    string label_x, label_y;

    // Число отрезков по x и по y
    size_t nx, ny;

    // Вывести msgbox с ошибкой
    void print_io_error();

    // Считать из u_ifstream число с разделителями ' ', '\t', ',', '\r' или '\n'
    float read_number(u_ifstream & ifs);

    // Отрисовка сцены на HDC
    void draw(HDC hdc_local);

    // Печать текста с заданным интервалом между символами
    void TextOutSpacingA(HDC hdc, int x, int y, const char * str, size_t len, int spacing);
};

#endif // PAINTWIDGET_H
