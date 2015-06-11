#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QWidget>
#include <QString>
#include <QPoint>
#include <QPaintDevice>
#include <set>
#include <vector>

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

// Класс элемент сечения текплота
typedef vector<double> tecplot_value;

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
    point nodes[3];
    vector<QColor> color;
    tecplot_value solution[3];
    triangle() {}
    triangle(const point & node1, const point & node2, const point & node3)
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

    // Изменение уровня интерполяции
    void set_div_num(size_t num);

    // Подписи к переменным
    vector<QString> variables;

    // Есть данные
    bool is_loaded;

    // Рекомендуемое значение для spinBox_2
    int vect_value;

    // Уровень интерполяции
    size_t div_num;

    // Отрисовка сцены на QPaintDevice
    void draw(QPaintDevice * device, QPaintEvent *event);

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
    vector<set<double> > isolines;

    // Треугольники, которые будем рисовать
    vector<triangle> triangles;

    // Геометрия окна
    QPoint to_window(double x, double y) const;

    // Подписи осей
    string label_x, label_y;

    // Число отрезков по x и по y
    size_t nx, ny;

    // Вывести msgbox с ошибкой
    void print_io_error();
};

#endif // GLWIDGET_H
