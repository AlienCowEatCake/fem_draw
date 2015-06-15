#include "paintwidget.h"
#include <fstream>
#include <limits>
#include <cstring>

// Использовать ли фиолетовые оттенки цвета
// Дает большую комбинацию оттенков, но приводит к
// некоторому дисбалансу между красным и синим
//#define USE_PURPLE

// Приглушать ли чрезмерно яркие цвета
// На мониторе они выглядят ничего, но если печатать...
#define USE_LIGHTNESS

// Рисовать ли легенду справа
#define USE_LEGEND

// Вывести msgbox с ошибкой
void paintwidget::print_io_error()
{
    fprintf(stderr, "Error: Corrupted file\n");
}

// Чтение текплотовских значений из файла
void paintwidget::tec_read(const string & filename)
{
    // Чистим старое
    is_loaded = false;
    tec_data.clear();
    triangles.clear();
    for(size_t k = 0; k < isolines.size(); k++)
        isolines[k].clear();
    variables.clear();

    // Пошли читать файл
    ifstream ifs(filename.c_str());
    string tmp;
    // TITLE = "Slice Z = -10"
    getline(ifs, tmp);
    // VARIABLES = "x", "y", "z", "ExR", "EyR", "EzR", "ExI", "EyI", "EzI", "abs(E)"
    getline(ifs, tmp);
    // Разберем переменные
    bool parse_flag = true;
    const char * curr_c = tmp.c_str(), * end_c = curr_c;
    const size_t VAR_MAX_LEN = 10;
    char var_c[3][VAR_MAX_LEN];
    // Первые три могут быть координаты. Также могут быть и первые две, но это мы потом поправим
    for(size_t i = 0; i < 3 && parse_flag; i++)
    {
        if((curr_c = strchr(end_c + 1, '\"')) != NULL)
        {
            if((end_c = strchr(++curr_c, '\"')) != NULL)
            {
                size_t len = end_c - curr_c;
                strncpy(var_c[i], curr_c, len);
                var_c[i][len] = '\0';
            }
            else
                parse_flag = false;
        }
        else
            parse_flag = false;
    }
    if(!parse_flag)
    {
        print_io_error();
        return;
    }
    // Читаем остальные переменные со значениями
    while(parse_flag)
    {
        if((curr_c = strchr(end_c + 1, '\"')) != NULL)
        {
            if((end_c = strchr(++curr_c, '\"')) != NULL)
            {
                char var[VAR_MAX_LEN];
                size_t len = end_c - curr_c;
                strncpy(var, curr_c, len);
                var[len] = '\0';
                variables.push_back(var);
            }
            else
                parse_flag = false;
        }
        else
            parse_flag = false;
    }
    // ZONE I= 101, J= 101, K= 1, F=POINT
    getline(ifs, tmp);
    size_t points_coord = 3; // Число координат у точки
    size_t IJK[3];
    parse_flag = true;
    curr_c = tmp.c_str();
    for(size_t i = 0; i < 3 && parse_flag; i++)
    {
        if((curr_c = strchr(curr_c, '=')) != NULL)
        {
            int bla;
            if(sscanf(++curr_c, "%d", &bla) != 1)
            {
                // Если точка из двух координат, учтем это
                if(i == 2)
                {
                    IJK[2] = 1;
                    points_coord = 2;
                    variables.insert(variables.begin(), var_c[2]);
                    break;
                }
                else
                    parse_flag = false;
            }
            else
                IJK[i] = (size_t)bla;
        }
        else
            parse_flag = false;
    }
    if(!parse_flag)
    {
        print_io_error();
        return;
    }

    // Понимаем в какой плоскости лежит сечение
    size_t ind[2];
    if(IJK[0] == 1) // y-z
    {
        ind[0] = 1;
        ind[1] = 2;
    }
    if(IJK[1] == 1) // x-z
    {
        ind[0] = 0;
        ind[1] = 2;
    }
    if(IJK[2] == 1) // x-y
    {
        ind[0] = 0;
        ind[1] = 1;
    }
    nx = IJK[ind[0]];
    ny = IJK[ind[1]];
    label_x = var_c[ind[0]];
    label_y = var_c[ind[1]];

    // Правим размерность векторов под переменные
    min_u.resize(variables.size());
    max_u.resize(variables.size());
    step_u_big.resize(variables.size());
    step_u_small.resize(variables.size());
    isolines.resize(variables.size());

    // Ищем минимальные и максимальные значения координат
    max_x = max_y = - numeric_limits<double>::max();
    min_x = min_y = numeric_limits<double>::max();
    for(size_t k = 0; k < variables.size(); k++)
    {
        min_u[k] = numeric_limits<double>::max();
        max_u[k] = - numeric_limits<double>::max();
    }

    // Читаем сами данные
    tec_data.resize(IJK[0] * IJK[1] * IJK[2]);
    for(size_t i = 0; i < tec_data.size(); i++)
    {
        double coords[3];
        for(size_t j = 0; j < points_coord; j++)
            ifs >> coords[j];
        tec_data[i].coord.x = coords[ind[0]];
        tec_data[i].coord.y = coords[ind[1]];
        tec_data[i].value.resize(variables.size());
        for(size_t k = 0; k < variables.size(); k++)
            ifs >> tec_data[i].value[k];

        if(tec_data[i].coord.x > max_x) max_x = tec_data[i].coord.x;
        if(tec_data[i].coord.y > max_y) max_y = tec_data[i].coord.y;
        if(tec_data[i].coord.x < min_x) min_x = tec_data[i].coord.x;
        if(tec_data[i].coord.y < min_y) min_y = tec_data[i].coord.y;

        for(size_t k = 0; k < variables.size(); k++)
        {
            if(tec_data[i].value[k] > max_u[k]) max_u[k] = tec_data[i].value[k];
            if(tec_data[i].value[k] < min_u[k]) min_u[k] = tec_data[i].value[k];
        }
    }

    if(!ifs.good())
    {
        print_io_error();
        return;
    }
    ifs.close();

    // Небольшие корректировки на всякий случай
    size_x = max_x - min_x;
    size_y = max_y - min_y;
    min_x -= size_x * 0.01;
    max_x += size_x * 0.01;
    min_y -= size_y * 0.01;
    max_y += size_y * 0.01;

    // Поправляем значения мин / макс чтобы влазило в сетку
    adjustAxis(min_x, max_x, num_ticks_x);
    adjustAxis(min_y, max_y, num_ticks_y);
    size_x = max_x - min_x;
    size_y = max_y - min_y;

    // Шаги для разбиения по цветовым областям
    for(size_t k = 0; k < variables.size(); k++)
    {
#if defined USE_PURPLE
        step_u_big[k] = (max_u[k] - min_u[k]) / 4.0;
#else
        step_u_big[k] = (max_u[k] - min_u[k]) / 3.0;
#endif
        step_u_small[k] = step_u_big[k] / 256.0;
    }

    // Устанавливаем рекомендуемое значение для spinBox_2
    vect_value = (int)(sqrt((double)(IJK[0] * IJK[1] * IJK[2])) / 20);

    // Как-то так
    is_loaded = true;
    set_isolines_num(isolines_num);
    set_div_num(div_num);
}

// Изменение уровня интерполяции
void paintwidget::set_div_num(size_t num)
{
    this->div_num = num;
    if(!is_loaded) return;

    vector<triangle> tmp1;
    vector<triangle> tmp2;
    // Посчитаем в локальных координатах
    tmp1.push_back(triangle(point(0.0, 0.0), point(1.0, 0.0), point(0.0, 1.0)));
    tmp1.push_back(triangle(point(1.0, 0.0), point(1.0, 1.0), point(0.0, 1.0)));
    for(size_t i = 0; i < num; i++)
    {
        tmp2.reserve(tmp1.size() * 4);
        for(size_t j = 0; j < tmp1.size(); j++)
        {
            point middles[3] =
            {
                point((tmp1[j].nodes[0].x + tmp1[j].nodes[1].x) * 0.5, (tmp1[j].nodes[0].y + tmp1[j].nodes[1].y) * 0.5),
                point((tmp1[j].nodes[0].x + tmp1[j].nodes[2].x) * 0.5, (tmp1[j].nodes[0].y + tmp1[j].nodes[2].y) * 0.5),
                point((tmp1[j].nodes[1].x + tmp1[j].nodes[2].x) * 0.5, (tmp1[j].nodes[1].y + tmp1[j].nodes[2].y) * 0.5)
            };
            tmp2.push_back(triangle(tmp1[j].nodes[0], middles[0], middles[1]));
            tmp2.push_back(triangle(middles[0], tmp1[j].nodes[1], middles[2]));
            tmp2.push_back(triangle(middles[1], middles[2], tmp1[j].nodes[2]));
            tmp2.push_back(triangle(middles[0], middles[2], middles[1]));
        }
        tmp2.swap(tmp1);
        tmp2.clear();
    }

    // Заполняем вектор из треугольников переводя координаты в глобальные и считая цвет
    triangles.clear();
    // Обходим все прямоугольники
    for(size_t i = 0; i < nx - 1; i++)
    {
        for(size_t j = 0; j < ny - 1; j++)
        {
            double x0 = tec_data[i * nx + j].coord.x;
            double y0 = tec_data[i * nx + j].coord.y;
            double x1 = tec_data[(i + 1) * nx + j + 1].coord.x;
            double y1 = tec_data[(i + 1) * nx + j + 1].coord.y;
            double hx = x1 - x0;
            double hy = y1 - y0;

            for(size_t tn = 0; tn < tmp1.size(); tn++)
            {
                triangle tmp_tr;
                tmp_tr.color.resize(variables.size());
                // Переводим координаты в глобальные
                for(size_t k = 0; k < 3; k++)
                {
                    tmp_tr.nodes[k].x = tmp1[tn].nodes[k].x * hx + x0;
                    tmp_tr.nodes[k].y = tmp1[tn].nodes[k].y * hy + y0;
                    tmp_tr.solution[k].resize(variables.size());
                }

                // Занесем значение решения в узлах
                for(size_t k = 0; k < 3; k++)
                {
                    // Строим билинейную интерполяцию
                    for(size_t m = 0; m < variables.size(); m++)
                    {
                        double r1 = (x1 - tmp_tr.nodes[k].x) / hx * tec_data[i * nx + j].value[m] +
                                    (tmp_tr.nodes[k].x - x0) / hx * tec_data[i * nx + j + 1].value[m];
                        double r2 = (x1 - tmp_tr.nodes[k].x) / hx * tec_data[(i + 1) * nx + j].value[m] +
                                    (tmp_tr.nodes[k].x - x0) / hx * tec_data[(i + 1) * nx + j + 1].value[m];
                        tmp_tr.solution[k][m] = (y1 - tmp_tr.nodes[k].y) / hy * r1 +
                                             (tmp_tr.nodes[k].y - y0) / hy * r2;
                    }
                }

                // Барицентр треугольника
                double cx = 0.0, cy = 0.0;
                for(size_t k = 0; k < 3; k++)
                {
                    cx += tmp_tr.nodes[k].x;
                    cy += tmp_tr.nodes[k].y;
                }
                point barycenter(cx / 3.0, cy / 3.0);

                // Решение в барицентре
                vector<double> center(variables.size());
                // Строим билинейную интерполяцию
                for(size_t m = 0; m < variables.size(); m++)
                {
                    double r1 = (x1 - barycenter.x) / hx * tec_data[i * nx + j].value[m] +
                                (barycenter.x - x0) / hx * tec_data[i * nx + j + 1].value[m];
                    double r2 = (x1 - barycenter.x) / hx * tec_data[(i + 1) * nx + j].value[m] +
                                (barycenter.x - x0) / hx * tec_data[(i + 1) * nx + j + 1].value[m];
                    center[m] = (y1 - barycenter.y) / hy * r1 +
                                         (barycenter.y - y0) / hy * r2;
                }

                for(size_t v = 0; v < variables.size(); v++)
                {
#if defined USE_PURPLE
                    // Ищем цвет решения по алгоритму заливки радугой (Rainbow colormap)
                    unsigned short r_color = 0, g_color = 0, b_color = 0;
                    if(center[v] > min_u[v] + step_u_big[v] * 3.0)
                    {
                        r_color = 255;
                        g_color = 255 - (unsigned short)((center[v] - (min_u[v] + step_u_big[v] * 3.0)) / step_u_small[v]);
                        b_color = 0;
                    }
                    else if(center[v] > min_u[v] + step_u_big[v] * 2.0)
                    {
                        r_color = (unsigned short)((center[v] - (min_u[v] + step_u_big[v] * 2.0)) / step_u_small[v]);
                        g_color = 255;
                        b_color = 0;
                    }
                    else if(center[v] > min_u[v] + step_u_big[v])
                    {
                        unsigned short tmp = (unsigned short)((center[v] - (min_u[v] + step_u_big[v])) / step_u_small[v]);
                        r_color = 0;
                        g_color = tmp;
                        b_color = 255 - tmp;
                    }
                    else
                    {
                        unsigned short tmp = 76 - (unsigned short)((center[v] - min_u[v]) / (step_u_small[v] * (255.0 / 76.0)));
                        r_color = tmp;
                        g_color = 0;
                        b_color = 255 - tmp;
                    }
#else
                    // Ищем цвет решения по алгоритму заливки радугой (Rainbow colormap)
                    unsigned short r_color = 0, g_color = 0, b_color = 0;
                    if(center[v] > min_u[v] + step_u_big[v] * 2.0)
                    {
                        r_color = 255;
                        g_color = 255 - (unsigned short)((center[v] - (min_u[v] + step_u_big[v] * 2.0)) / step_u_small[v]);
                        b_color = 0;
                    }
                    else if(center[v] > min_u[v] + step_u_big[v])
                    {
                        r_color = (unsigned short)((center[v] - (min_u[v] + step_u_big[v])) / step_u_small[v]);
                        g_color = 255;
                        b_color = 0;
                    }
                    else
                    {
                        unsigned short tmp = (unsigned short)((center[v] - min_u[v]) / step_u_small[v]);
                        r_color = 0;
                        g_color = tmp;
                        b_color = 255 - tmp;
                    }
#endif

#if defined USE_LIGHTNESS
                    // Приглушаем кислотные цвета
                    r_color = r_color * 3 / 4 + 64;
                    g_color = g_color * 3 / 4 + 64;
                    b_color = b_color * 3 / 4 + 64;
#endif

                    // Задаем посчитанный цвет
                    tmp_tr.color[v].rgbtRed = r_color;
                    tmp_tr.color[v].rgbtGreen = g_color;
                    tmp_tr.color[v].rgbtBlue = b_color;
                }

                // И заносим в вектор
                triangles.push_back(tmp_tr);
            }
        }
    }
}

// Конструктор
paintwidget::paintwidget()
{
    isolines_num = 10;
    draw_index = 0;
    draw_color = true;
    draw_isolines = true;
    is_loaded = false;
    ind_vec_1 = 0;
    ind_vec_2 = 1;
    draw_vectors = false;
    skip_vec = 1;
    div_num = 2;
    vect_value = 1;

    // WinAPI, мать его
//    hwnd = GetConsoleWindow();
//    hdc = GetDC(hwnd);
//    SetBkColor(hdc, RGB(255, 255, 255));
    hwnd = NULL;
    hdc = NULL;
}

// Пересчет значений изолиний
void paintwidget::set_isolines_num(size_t isolines_num)
{
    this->isolines_num = isolines_num;
    if(!is_loaded) return;
    for(size_t j = 0; j < variables.size(); j++)
    {
        isolines[j].clear();
        double isolines_step = (max_u[j] - min_u[j]) / (double)(isolines_num + 1);
        for(size_t i = 0; i < isolines_num; i++)
            isolines[j].insert(min_u[j] + isolines_step * (double)(i + 1));
    }
}

// Подгонка осей под реальность и вычисление шагов координатной сетки
void paintwidget::adjustAxis(double & min, double & max, size_t & numTicks) const
{
    static const double axis_epsilon = 1.0 / 10000.0;
    if(max - min < axis_epsilon)
    {
        min -= 2.0 * axis_epsilon;
        max += 2.0 * axis_epsilon;
    }

    static const size_t MinTicks = 10;
    double grossStep = (max - min) / MinTicks;
    double step = pow(10, floor(log10(grossStep)));

    if (5 * step < grossStep)
        step *= 5;
    else if (2 * step < grossStep)
        step *= 2;

    numTicks = (size_t)(ceil(max / step) - floor(min / step));
    min = floor(min / step) * step;
    max = ceil(max / step) * step;
}

// Геометрия окна
void paintwidget::to_window(double x, double y, int & xl, int & yl) const
{
    // В OpenGL это был бы glOrtho
    static const double gl_x0 = -0.06;
    static const double gl_y0 = -0.06;
#if defined USE_LEGEND
    static const double gl_x1 = 1.125;
#else
    static const double gl_x1 = 1.015;
#endif
    static const double gl_y1 = 1.02;
    static const double gl_hx = gl_x1 - gl_x0;
    static const double gl_hy = gl_y1 - gl_y0;
    xl = (int)((x - gl_x0) / gl_hx * (double)width);
    yl = height - (int)((y - gl_y0) / gl_hy * (double)height);
}


// Отрисовка сцены
void paintwidget::paintEvent()
{
    draw(/*this, false*/);
}

// Отрисовка сцены на QPaintDevice
void paintwidget::draw(/*QPaintDevice * device, bool transparency*/)
{
    hdc = BeginPaint(hwnd, &ps);

    // Геометрия окна
    RECT r;
    GetClientRect(hwnd, &r);
    height = r.bottom - r.top;
    width = r.right - r.left;

    POINT pt;
    int x, y;
    HPEN hOldPen;
    HBRUSH hOldBrush;
    HFONT hOldFont;
    POINT tr[3];

    // Узнаем размеры шрифта
    HFONT hTestFont = (HFONT)GetStockObject(ANSI_VAR_FONT);
    hOldFont = (HFONT)SelectObject(hdc, hTestFont);
    TEXTMETRIC tm;
    GetTextMetrics(hdc, &tm);
    SelectObject(hdc, hOldFont);
    double font_correct = (double)tm.tmHeight / (double)height * 0.9;

    // Заливка области белым цветом
    HPEN hAreaPen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
    hOldPen = SelectPen(hdc, hAreaPen);
    HBRUSH hAreaBrush = CreateSolidBrush(RGB(255, 255, 255));
    hOldBrush = SelectBrush(hdc, hAreaBrush);
    Rectangle(hdc, 0, 0, width, height);
    SelectPen(hdc, hOldPen);
    SelectBrush(hdc, hOldBrush);
    DeletePen(hAreaPen);
    DeleteBrush(hAreaBrush);

    if(!is_loaded) return;

    // Координатная сетка
    HPEN hGridPen = CreatePen(PS_SOLID, 1, RGB(217, 217, 217));
    hOldPen = SelectPen(hdc, hGridPen);
    for(size_t i = 0; i <= num_ticks_x; i++)
    {
        double xd = (double)i / (double)num_ticks_x;
        to_window(xd, -0.01, x, y);
        MoveToEx(hdc, x, y, &pt);
        to_window(xd, 1.0, x, y);
        LineTo(hdc, x, y);
    }
    for(size_t i = 0; i <= num_ticks_y; i++)
    {
        double yd = (double)i / (double)num_ticks_y;
        to_window(-0.01, yd, x, y);
        MoveToEx(hdc, x, y, &pt);
        to_window(1.0, yd, x, y);
        LineTo(hdc, x, y);
    }
    SelectPen(hdc, hOldPen);
    DeletePen(hGridPen);

    // Координатные оси
    HPEN hAxisPen = CreatePen(PS_SOLID, 2, RGB(0, 0, 0));
    hOldPen = SelectPen(hdc, hAxisPen);
    to_window(0.0, -0.005, x, y);
    MoveToEx(hdc, x, y, &pt);
    to_window(0.0, 1.005, x, y);
    LineTo(hdc, x, y);
    to_window(-0.005, 0.0, x, y);
    MoveToEx(hdc, x, y, &pt);
    to_window(1.005, 0.0, x, y);
    LineTo(hdc, x, y);
    SelectPen(hdc, hOldPen);
    DeletePen(hAxisPen);

    // Подписи осей
    HFONT hAxisFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    hOldFont = (HFONT)SelectObject(hdc, hAxisFont);
    to_window(0.99, -0.04 + font_correct, x, y);
    TextOutA(hdc, x, y, label_x.c_str(), 1);
    to_window(-0.05, 0.99 + font_correct, x, y);
    TextOutA(hdc, x, y, label_y.c_str(), 1);
    SelectObject(hdc, hOldFont);

    // Отрисовка шкалы
    HFONT hGridFont = (HFONT)GetStockObject(ANSI_VAR_FONT);
    hOldFont = (HFONT)SelectObject(hdc, hGridFont);
    for(size_t i = 0; i < num_ticks_x; i++)
    {
        double xd = (double)i / (double)num_ticks_x;
        double x_real = (double)(floor((xd * size_x + min_x) * 10000.0 + 0.5)) / 10000.0;
        char st[17];
        sprintf(st, "%.2f", x_real);
        to_window(xd - 0.01, -0.04 + font_correct, x, y);
        TextOutA(hdc, x, y, st, (int)strlen(st));
    }
    for(size_t i = 0; i < num_ticks_y; i++)
    {
        double yd = (double)i / (double)num_ticks_y;
        double y_real = (double)(floor((yd * size_y + min_y) * 10000.0 + 0.5)) / 10000.0;
        char st[17];
        sprintf(st, "%.2f", y_real);
        to_window(-0.052, yd - 0.01 + font_correct, x, y);
        TextOutA(hdc, x, y, st, (int)strlen(st));
    }
    SelectObject(hdc, hOldFont);

    // Раскрашивать будем если запрошено сие
    if(draw_color)
    {
        // Отрисовка всех треугольников
        for(size_t i = 0; i < triangles.size(); i++)
        {
            HPEN hTrPen = GetStockPen(NULL_PEN);
            hOldPen = SelectPen(hdc, hTrPen);
            HBRUSH hTrBrush;

            // Задаем посчитанный цвет
            hTrBrush = CreateSolidBrush(RGB(triangles[i].color[draw_index].rgbtRed, triangles[i].color[draw_index].rgbtGreen, triangles[i].color[draw_index].rgbtBlue));
            hOldBrush = SelectBrush(hdc, hTrBrush);

            // Рисуем
            for(size_t k = 0; k < 3; k++)
            {
                to_window((triangles[i].nodes[k].x - min_x) / size_x, (triangles[i].nodes[k].y - min_y) / size_y, x, y);
                tr[k].x = (LONG)x;
                tr[k].y = (LONG)y;
            }
            Polygon(hdc, tr, 3);

            SelectPen(hdc, hOldPen);
            SelectBrush(hdc, hOldBrush);
            DeleteBrush(hTrBrush);
        }
    }

    HPEN hIsolPen = GetStockPen(BLACK_PEN);
    hOldPen = SelectPen(hdc, hIsolPen);
    // Изолинии рисуем только если оно нам надо
    if(draw_isolines)
    {
        for(size_t i = 0; i < triangles.size(); i++)
        {
            // Теперь рисуем изолинии
            // Будем искать наименьшее значение, большее или равное решению
            // Если значения на разных концах ребра будут разными, значит изолиния проходит через это ребро
            set<double>::const_iterator segment_isol[3];
            for(size_t k = 0; k < 3; k++)
                segment_isol[k] = isolines[draw_index].lower_bound(triangles[i].solution[k][draw_index]);

            // Немного странной фигни
            vector<pair<double, double> > vct;
            if(segment_isol[0] != segment_isol[1])
                vct.push_back(make_pair(((triangles[i].nodes[1].x + triangles[i].nodes[0].x) * 0.5 - min_x) / size_x, ((triangles[i].nodes[1].y + triangles[i].nodes[0].y) * 0.5 - min_y) / size_y));
            if(segment_isol[0] != segment_isol[2])
                vct.push_back(make_pair(((triangles[i].nodes[2].x + triangles[i].nodes[0].x) * 0.5 - min_x) / size_x, ((triangles[i].nodes[2].y + triangles[i].nodes[0].y) * 0.5 - min_y) / size_y));
            if(segment_isol[1] != segment_isol[2])
                vct.push_back(make_pair(((triangles[i].nodes[2].x + triangles[i].nodes[1].x) * 0.5 - min_x) / size_x, ((triangles[i].nodes[2].y + triangles[i].nodes[1].y) * 0.5 - min_y) / size_y));

            // А теперь нарисуем, согласно вышеприведенному условию
            if(vct.size() > 1)
            {
                to_window(vct[0].first, vct[0].second, x, y);
                MoveToEx(hdc, x, y, &pt);
                to_window(vct[1].first, vct[1].second, x, y);
                LineTo(hdc, x, y);
            }
        }
    }
    SelectPen(hdc, hOldPen);

#if defined USE_LEGEND
    // Легенда
#if defined USE_PURPLE
/*
 * Матрица цветов:
 * 255,   0,   0        glColor3d(1.0, 0.0, 0.0);
 * 255,  65,   0        glColor3d(1.0, 0.254901961, 0.0);
 * 255, 130,   0        glColor3d(1.0, 0.509803922, 0.0);
 * 255, 195,   0        glColor3d(1.0, 0.764705882, 0.0);
 * 250, 255,   0        glColor3d(0.980392157, 1.0, 0.0);
 * 185, 255,   0        glColor3d(0.725490196, 1.0, 0.0);
 * 120, 255,   0        glColor3d(0.470588235, 1.0, 0.0);
 *  55, 255,   0        glColor3d(0.215686275, 1.0, 0.0);
 *   0, 245,  10        glColor3d(0.0, 0.960784314, 0.039215686);
 *   0, 180,  75        glColor3d(0.0, 0.705882353, 0.294117647);
 *   0, 115, 140        glColor3d(0.0, 0.450980392, 0.549019608);
 *   0,  50, 205        glColor3d(0.0, 0.196078431, 0.803921569);
 *  15,   0, 240        glColor3d(0.058823529, 0.0, 0.941176471);
 *  76,   0, 179        glColor3d(0.298039216, 0.0, 0.701960784);
 *
 * 1). G = 255 - (center_u - (min_u + step_u_big * 3.0)) / step_u_small
 * 2). R = (center_u - (min_u + step_u_big * 2.0)) / step_u_small
 * 3). G = (center_u - (min_u + step_u_big)) / step_u_small
 * 4). R = 76 - (center_u - min_u) / (step_u_small * (255.0 / 76.0))
 *
 * 1). U = (255 - G) * step_u_small + min_u + step_u_big * 3.0
 * 2). U = R * step_u_small + min_u + step_u_big * 2.0
 * 3). U = G * step_u_small + min_u + step_u_big
 * 4). U = (76 - R) * (step_u_small * (255.0 / 76.0)) + min_u
 */
#if defined USE_LIGHTNESS
    static const COLORREF legend_colors[14] =
    {
        RGB(  76 * 3 / 4 + 64,   0 * 3 / 4 + 64, 179 * 3 / 4 + 64 ),
        RGB(  15 * 3 / 4 + 64,   0 * 3 / 4 + 64, 240 * 3 / 4 + 64 ),
        RGB(   0 * 3 / 4 + 64,  50 * 3 / 4 + 64, 205 * 3 / 4 + 64 ),
        RGB(   0 * 3 / 4 + 64, 115 * 3 / 4 + 64, 140 * 3 / 4 + 64 ),
        RGB(   0 * 3 / 4 + 64, 180 * 3 / 4 + 64,  75 * 3 / 4 + 64 ),
        RGB(   0 * 3 / 4 + 64, 245 * 3 / 4 + 64,  10 * 3 / 4 + 64 ),
        RGB(  55 * 3 / 4 + 64, 255 * 3 / 4 + 64,   0 * 3 / 4 + 64 ),
        RGB( 120 * 3 / 4 + 64, 255 * 3 / 4 + 64,   0 * 3 / 4 + 64 ),
        RGB( 185 * 3 / 4 + 64, 255 * 3 / 4 + 64,   0 * 3 / 4 + 64 ),
        RGB( 250 * 3 / 4 + 64, 255 * 3 / 4 + 64,   0 * 3 / 4 + 64 ),
        RGB( 255 * 3 / 4 + 64, 195 * 3 / 4 + 64,   0 * 3 / 4 + 64 ),
        RGB( 255 * 3 / 4 + 64, 130 * 3 / 4 + 64,   0 * 3 / 4 + 64 ),
        RGB( 255 * 3 / 4 + 64,  65 * 3 / 4 + 64,   0 * 3 / 4 + 64 ),
        RGB( 255 * 3 / 4 + 64,   0 * 3 / 4 + 64,   0 * 3 / 4 + 64 )
    };
#else
    static const COLORREF legend_colors[14] =
    {
        RGB(  76,   0, 179 ),
        RGB(  15,   0, 240 ),
        RGB(   0,  50, 205 ),
        RGB(   0, 115, 140 ),
        RGB(   0, 180,  75 ),
        RGB(   0, 245,  10 ),
        RGB(  55, 255,   0 ),
        RGB( 120, 255,   0 ),
        RGB( 185, 255,   0 ),
        RGB( 250, 255,   0 ),
        RGB( 255, 195,   0 ),
        RGB( 255, 130,   0 ),
        RGB( 255,  65,   0 ),
        RGB( 255,   0,   0 )
    };
#endif
    double legend_values[14] =
    {
        min_u[draw_index],
        61.0 * (step_u_small[draw_index] * (255.0 / 76.0)) + min_u[draw_index],
        50.0 * step_u_small[draw_index] + min_u[draw_index] + step_u_big[draw_index],
        115.0 * step_u_small[draw_index] + min_u[draw_index] + step_u_big[draw_index],
        180.0 * step_u_small[draw_index] + min_u[draw_index] + step_u_big[draw_index],
        245.0 * step_u_small[draw_index] + min_u[draw_index] + step_u_big[draw_index],
        55.0 * step_u_small[draw_index] + min_u[draw_index] + step_u_big[draw_index] * 2.0,
        120.0 * step_u_small[draw_index] + min_u[draw_index] + step_u_big[draw_index] * 2.0,
        185.0 * step_u_small[draw_index] + min_u[draw_index] + step_u_big[draw_index] * 2.0,
        250.0 * step_u_small[draw_index] + min_u[draw_index] + step_u_big[draw_index] * 2.0,
        60.0 * step_u_small[draw_index] + min_u[draw_index] + step_u_big[draw_index] * 3.0,
        125.0 * step_u_small[draw_index] + min_u[draw_index] + step_u_big[draw_index] * 3.0,
        190.0 * step_u_small[draw_index] + min_u[draw_index] + step_u_big[draw_index] * 3.0,
        max_u[draw_index]
    };
#else
    /*
     * Матрица цветов:
     * 255,   0,   0
     * 255,  59,   0
     * 255, 118,   0
     * 255, 177,   0
     * 255, 236,   0
     * 215, 255,   0
     * 157, 255,   0
     *  99, 255,   0
     *  50, 255,   0
     *   0, 236,  19
     *   0, 177,  78
     *   0, 118, 137
     *   0,  59, 196
     *   0,   0, 255
     *
     * 1). U = (255 - G) * step_u_small + min_u + step_u_big * 2.0
     * 2). U = R * step_u_small + min_u + step_u_big
     * 3). U = G * step_u_small + min_u
     */
#if defined USE_LIGHTNESS
    static const COLORREF legend_colors[14] =
    {
        RGB(   0 * 3 / 4 + 64,   0 * 3 / 4 + 64, 255 * 3 / 4 + 64 ),
        RGB(   0 * 3 / 4 + 64,  59 * 3 / 4 + 64, 196 * 3 / 4 + 64 ),
        RGB(   0 * 3 / 4 + 64, 118 * 3 / 4 + 64, 137 * 3 / 4 + 64 ),
        RGB(   0 * 3 / 4 + 64, 177 * 3 / 4 + 64,  78 * 3 / 4 + 64 ),
        RGB(   0 * 3 / 4 + 64, 236 * 3 / 4 + 64,  19 * 3 / 4 + 64 ),
        RGB(  50 * 3 / 4 + 64, 245 * 3 / 4 + 64,   0 * 3 / 4 + 64 ),
        RGB(  99 * 3 / 4 + 64, 255 * 3 / 4 + 64,   0 * 3 / 4 + 64 ),
        RGB( 157 * 3 / 4 + 64, 255 * 3 / 4 + 64,   0 * 3 / 4 + 64 ),
        RGB( 215 * 3 / 4 + 64, 255 * 3 / 4 + 64,   0 * 3 / 4 + 64 ),
        RGB( 255 * 3 / 4 + 64, 236 * 3 / 4 + 64,   0 * 3 / 4 + 64 ),
        RGB( 255 * 3 / 4 + 64, 177 * 3 / 4 + 64,   0 * 3 / 4 + 64 ),
        RGB( 255 * 3 / 4 + 64, 118 * 3 / 4 + 64,   0 * 3 / 4 + 64 ),
        RGB( 255 * 3 / 4 + 64,  59 * 3 / 4 + 64,   0 * 3 / 4 + 64 ),
        RGB( 255 * 3 / 4 + 64,   0 * 3 / 4 + 64,   0 * 3 / 4 + 64 )
    };
#else
    static const COLORREF legend_colors[14] =
    {
        RGB(   0,   0, 255 ),
        RGB(   0,  59, 196 ),
        RGB(   0, 118, 137 ),
        RGB(   0, 177,  78 ),
        RGB(   0, 236,  19 ),
        RGB(  50, 245,   0 ),
        RGB(  99, 255,   0 ),
        RGB( 157, 255,   0 ),
        RGB( 215, 255,   0 ),
        RGB( 255, 236,   0 ),
        RGB( 255, 177,   0 ),
        RGB( 255, 118,   0 ),
        RGB( 255,  59,   0 ),
        RGB( 255,   0,   0 )
    };
#endif
    double legend_values[14] =
    {
        min_u[draw_index],
        59.0 * step_u_small[draw_index] + min_u[draw_index],
        118.0 * step_u_small[draw_index] + min_u[draw_index],
        177.0 * step_u_small[draw_index] + min_u[draw_index],
        236.0 * step_u_small[draw_index] + min_u[draw_index],
        50.0 * step_u_small[draw_index] + step_u_big[draw_index] + min_u[draw_index],
        99.0 * step_u_small[draw_index] + step_u_big[draw_index] + min_u[draw_index],
        157.0 * step_u_small[draw_index] + step_u_big[draw_index] + min_u[draw_index],
        215.0 * step_u_small[draw_index] + step_u_big[draw_index] + min_u[draw_index],
        19.0 * step_u_small[draw_index] + step_u_big[draw_index] * 2.0 + min_u[draw_index],
        78.0 * step_u_small[draw_index] + step_u_big[draw_index] * 2.0 + min_u[draw_index],
        137.0 * step_u_small[draw_index] + step_u_big[draw_index] * 2.0 + min_u[draw_index],
        196.0 * step_u_small[draw_index] + step_u_big[draw_index] * 2.0 + min_u[draw_index],
        max_u[draw_index]
    };
#endif
    for(size_t i = 0; i < 14; i++)
    {
        static const double x0 = 1.0175;
        static const double y0 = 0;
        static const double dx = 0.0;
        static const double dy = 0.07;
        static const double hx = 0.103;
        static const double hy = 0.073;
        HPEN hLegPen = GetStockPen(NULL_PEN);
        hOldPen = SelectPen(hdc, hLegPen);
        HBRUSH hLegBrush;
        hLegBrush = CreateSolidBrush(legend_colors[i]);
        hOldBrush = SelectBrush(hdc, hLegBrush);
        int coords[4];
        to_window(x0, y0 + dy * i, coords[0], coords[3]);
        to_window(x0 + hx, y0 + dy * i + hy, coords[2], coords[1]);
        Rectangle(hdc, coords[0], coords[1], coords[2], coords[3]);
        SelectPen(hdc, hOldPen);
        SelectBrush(hdc, hOldBrush);
        DeleteBrush(hLegBrush);

        SetBkColor(hdc, legend_colors[i]);
        HFONT hLegFont = (HFONT)GetStockObject(ANSI_VAR_FONT);
        hOldFont = (HFONT)SelectObject(hdc, hLegFont);
        char st[17];
        int exponent  = (int)floor(log10(fabs(legend_values[i])));
        if(abs(exponent) < 0) exponent = 0;
        double base   = legend_values[i] * pow(10.0, -1.0 * exponent);
#if defined _MSC_VER && _MSC_VER >= 1400
        sprintf_s(st, 17, "%.2fE%+03d", base, exponent);
#else
        sprintf(st, "%.2fE%+03d", base, exponent);
#endif
        to_window(x0 + dx * i + 0.004, y0 + dy * i + hy / 2.0 - 0.01 + font_correct, x, y);
        TextOutA(hdc, x, y, st, (int)strlen(st));
        SelectObject(hdc, hOldFont);
    }
    SetBkColor(hdc, RGB(255, 255, 255));
#endif
/*
    if(draw_vectors)
    {
        painter.setPen(QPen(Qt::black, 1.2));
        painter.setBrush(QBrush(Qt::black));

        double vec_len = 10, arrow_len = 1.5;

        for(size_t j = 0; j < ny; j += skip_vec)
        {
            for(size_t i = 0; i < nx; i += skip_vec)
            {
                tecplot_node * v = & tec_data[j * nx + i];
                QPoint begin = to_window((v->coord.x - min_x) / size_x, (v->coord.y - min_y) / size_y);
                double len_x = v->value[ind_vec_1];
                double len_y = v->value[ind_vec_2];
                double norm = sqrt(len_x * len_x + len_y * len_y);
                len_x /= norm;
                len_y /= norm;
                if(len_x == len_x && len_y == len_y)
                {
                    QPoint end((int)(len_x * vec_len), -(int)(len_y * vec_len));
                    end.setX(end.x() + begin.x());
                    end.setY(end.y() + begin.y());

                    painter.drawLine(begin, end);
                    painter.drawEllipse(QPointF(end), (qreal)arrow_len, (qreal)arrow_len);
                }
            }
        }
    }
*/
    EndPaint(hwnd, &ps);
}
