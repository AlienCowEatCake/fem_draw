#ifndef PAINTWIDGET_H
#define PAINTWIDGET_H

#include <QWidget>
#include <QString>
#include <QPoint>
#include <QPaintDevice>
#include <QImage>
#include <QTextStream>
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
    point(float x = 0.0f, float y = 0.0f)
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
    QColor * color;
    float * solution[3];
    triangle() {}
    triangle(const point & node1, const point & node2, const point & node3)
    {
        nodes[0] = node1;
        nodes[1] = node2;
        nodes[2] = node3;
    }
};

// Класс виджет для рисования на QPaintDevice
class paintwidget : public QWidget
{
    Q_OBJECT
public:
    // Конструктор
    paintwidget(QWidget * parent = 0);

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
    void tec_read(const QString & filename);

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
    vector<QString> variables;

    // Есть данные
    bool is_loaded;

    // Рекомендуемое значение для spinBox_Vectors
    int vect_value;

    // Уровень интерполяции
    size_t div_num;

    // Отрисовка сцены на QPaintDevice
    void draw(QPaintDevice * device, bool transparency);

    // Перерисовать, сбросив валидность буфера
    void invalidate();

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
    QString title;

protected:
    // Отрисовка сцены
    void paintEvent(QPaintEvent *);

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
    QPoint to_window(float x, float y) const;

    // Подписи осей
    string label_x, label_y;

    // Число отрезков по x и по y
    size_t nx, ny;

    // Вывести msgbox с ошибкой
    void print_io_error();

    // Считать из QTextStream число с разделителями ' ', '\t', ',', '\r' или '\n'
    float read_number(QTextStream & ifs);

    // Буфер изображения
    QImage buffer;

    // Валиден ли буфер изображения
    bool buffer_valid;
};

#endif // PAINTWIDGET_H
