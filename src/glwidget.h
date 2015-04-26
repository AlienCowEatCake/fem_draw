#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QWidget>
#include <QString>
#include <QPoint>
#include <set>

using namespace std;

// Класс точка
class point
{
public:
    double x, y;
    point(double x = 0.0, double y = 0.0)
    {
        this->x = x;
        this->y = y;
    }
};

static const size_t TEC_SIZE = 7;

// Класс элемент сечения текплота
class tecplot_value
{
public:
    double ExR, EyR, EzR;
    double ExI, EyI, EzI;
    double absE;
    double & operator [] (size_t i)
    {
        if(i == 0) return ExR;
        if(i == 1) return EyR;
        if(i == 2) return EzR;
        if(i == 3) return ExI;
        if(i == 4) return EyI;
        if(i == 5) return EzI;
        if(i == 6) return absE;
        return absE;
    }
};

// Класс элемент сечения текплота
class tecplot_node
{
public:
    point coord;
    tecplot_value value;
};

// Класс треугольник
class triangle
{
public:
    const point * nodes[3];
    QColor color[TEC_SIZE];
    tecplot_value * solution[3];
    triangle() {}
    triangle(const point * node1, const point * node2, const point * node3)
    {
        nodes[0] = node1;
        nodes[1] = node2;
        nodes[2] = node3;
    }
};

// Класс OpenGL виджет
class glwidget : public QWidget
{
    Q_OBJECT
public:
    // Конструктор
    glwidget(QWidget * parent = 0);

    // Флаг отрисовки изолиний
    bool draw_isolines;

    // Флаг закраски цветом
    bool draw_color;

    // Пересчет значений изолиний
    void set_isolines_num(size_t isolines_num);

    // Индекс что сейчас рисуем
    size_t draw_index;

    // Чтение текплотовских значений из файла
    void tec_read(const string & filename);

    // Количество изолиний
    size_t isolines_num;

    // Переменные по которым рисуем вектора
    size_t ind_vec_1, ind_vec_2;

    // Флаг надобности векторов
    bool draw_vectors;

    // Число пропускаемых векторов
    size_t skip_vec;

protected:
    // Отрисовка сцены
    void paintEvent(QPaintEvent *event);

private:
    // Текплотовские значения
    vector<tecplot_node> tec_data;

    // Минимальные и максимальные значения геометрии + размер
    double min_x, max_x, size_x;
    double min_y, max_y, size_y;

    // Количество шагов координатной сетки
    size_t num_ticks_x, num_ticks_y;
    // Подгонка осей под реальность и вычисление шагов координатной сетки
    void adjustAxis(double & min, double & max, size_t & numTicks);

    // Минимальное и максимальное значения решения
    tecplot_value min_u, max_u;

    // Вспомогательные шаги по цвету для закраски
    tecplot_value step_u_big, step_u_small;

    // Значения изолиний
    set<double> isolines[TEC_SIZE];

    // Треугольники, которые будем рисовать
    vector<triangle> triangles;

    // Геометрия окна
    QPoint to_window(double x, double y) const;

    // Подписи осей
    string label_x, label_y;

    // Есть данные
    bool is_loaded;

    // Число отрезков по x и по y
    size_t nx, ny;
};

#endif // GLWIDGET_H
