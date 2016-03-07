#if defined(HAVE_QT5)
#include <QtWidgets>
#else
#include <QtGui>
#endif
#include "paintwidget.h"
#include <QFont>
#include <QMessageBox>
#include <QFile>
#include <QList>
#include <QVector>
#include <cmath>
#include <limits>
#include <cstring>
#include <cstdio>

// Вывести msgbox с ошибкой
void paintwidget::print_io_error()
{
    QMessageBox msgBox;
    msgBox.setAttribute(Qt::WA_QuitOnClose);
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.setWindowTitle(trUtf8("Error"));
    msgBox.setText(trUtf8("Error: Corrupted file"));
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setWindowIcon(QIcon(":/resources/icon.ico"));
    msgBox.exec();
}

// Считать из QTextStream число с разделителями ' ', '\t', ',', '\r' или '\n'
float paintwidget::read_number(QTextStream & ifs)
{
    char str[32]; // Должно хватить
    size_t len = 0;
    // Читаем мусор перед числом
    do
        ifs >> str[0];
    while((str[0] == ' ' || str[0] == '\t' || str[0] == ',' || str[0] == '\r' || str[0] == '\n') && ifs.status() == QTextStream::Ok);
    if(ifs.status() != QTextStream::Ok) return 0.0f;
    // Читаем число
    do
        ifs >> str[++len];
    while(str[len] != ' ' && str[len] != '\t' && str[len] != ',' && str[len] != '\r' && str[len] != '\n' && ifs.status() == QTextStream::Ok);
    if(ifs.status() != QTextStream::Ok) return 0.0f;
    str[len] = '\0';
    // Преобразуем во float
    float num = 0.0f;
    sscanf(str, "%f", &num);
    return num;
}

// Чтение текплотовских значений из файла
void paintwidget::tec_read(const QString & filename)
{
    // Чистим старое
    is_loaded = false;
    for(size_t i = 0; i < tec_data.size(); i++)
        delete [] tec_data[i].value;
    tec_data.clear();
    for(size_t i = 0; i < triangles.size(); i++)
    {
        delete [] triangles[i].color;
        for(size_t k = 0; k < 3; k++)
            delete [] triangles[i].solution[k];
    }
    triangles.clear();
    for(size_t k = 0; k < isolines.size(); k++)
        isolines[k].clear();
    variables.clear();

    // Пошли читать файл
    QFile qf(filename);
    if (!qf.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        print_io_error();
        return;
    }
    QTextStream ifs(& qf);
    QString tmp;

    // TITLE = "Slice Z = -10"
    // TITLE="Electric Field"
    title = ifs.readLine();
    title = title.remove(0, title.indexOf('=') + 1).trimmed();
    if(title[0] == '\"')
        title = title.remove(0, 1);
    if(title[title.length() - 1] == '\"')
        title.truncate(title.length() - 1);
    // У нас могут попасться заэкранированные символы в тексте, исправим это
    for(int i = title.indexOf('\\', 0); i >= 0 && i < title.length(); i = title.indexOf('\\', i + 1))
        title.remove(i, 1);

    // VARIABLES = "x", "y", "z", "ExR", "EyR", "EzR", "ExI", "EyI", "EzI", "abs(E)"
    // VARIABLES=X,Y,Z
    // VARIABLES = "X" "Y" "ReEx" "ImEx" "ReEy" "ImEy" "ReEz" "ImEz"
    tmp = ifs.readLine();
    // Разберем переменные
    QString::iterator curr_c = tmp.begin(), end_c = curr_c;
    QString var_c[3];
    // Ну мало ли, вдруг кто-нибудь пробелов понаставит в начале строки
    while(* curr_c == ' ' || * curr_c == '\t')
        curr_c++;
    // Строка должна начинаться со слова VARIABLES
    if(QString(&*curr_c, 9) == "VARIABLES")
    {
        // Дальше всегда идет равно, а после него - перечисления переменных
        if((curr_c = std::find(curr_c + 9, tmp.end(), '=')) != tmp.end())
        {
            bool eos_flag = false;
            size_t var_num = 0;
            QString tmpstr;
            // Пока есть данные в строке пригодные для разбора, будем разбирать
            while(!eos_flag)
            {
                curr_c++;
                // Пробелы и табы нас не интересуют
                while(* curr_c == ' ' || * curr_c == '\t')
                    curr_c++;
                // Кавычки тоже, но если начали с кавычки, то ей же и закончим
                bool use_quote = (* curr_c == '\"') ? true : false;
                if(use_quote) curr_c++;

                // Если переменная начинается с кавычки, то и искать следует только кавычку
                // Иначе могут быть пробел, таб или запятая
                if((use_quote && (end_c = std::find(curr_c, tmp.end(), '\"')) != tmp.end()) || (!use_quote &&
                   ((end_c = std::find(curr_c, tmp.end(), ',')) != tmp.end() ||
                    (end_c = std::find(curr_c, tmp.end(), ' ')) != tmp.end() ||
                    (end_c = std::find(curr_c, tmp.end(), '\t')) != tmp.end())))
                {
                    // Никакая это не кавычка, это заэкранированная кавычка в тексте
                    bool maybe_escaped_quote = use_quote;
                    while(maybe_escaped_quote)
                    {
                        size_t counter = 0;
                        for(size_t i = 1; i <= (size_t)(end_c - curr_c); i++)
                            if(* (end_c - i) == '\\')
                                counter++;
                            else
                                break;
                        if(counter % 2 == 1)
                            end_c = std::find(end_c + 1, tmp.end(), '\"');
                        else
                            maybe_escaped_quote = false;
                        if(end_c == NULL)
                        {
                            print_io_error();
                            return;
                        }
                    }
                }
                // Если разделителя нового не было, значит это последняя переменная
                // Однако это не так, если начали с кавычки
                else
                {
                    if(use_quote)
                    {
                        print_io_error();
                        return;
                    }
                    eos_flag = true;
                    end_c = tmp.end();
                }
                QString::iterator end_c_old = end_c;
                if(use_quote)
                {
                    // После кавычки надо найти разделитель и переместиться на него
                    while(end_c_old + 1 != tmp.end() && (* (end_c_old + 1) == ' ' || * (end_c_old + 1) == '\t' || * (end_c_old + 1) == ','))
                        end_c_old++;
                }
                else
                {
                    // Лишнего нам не надо, только само название переменной
                    while(* (end_c - 1) == ' ' || * (end_c - 1) == '\t')
                        end_c--;
                }
                // В нормальном случае длина больше нуля, однако если разделитель - пробел
                // (или таб), а в конце строки есть еще пробел, то мы попадем сюда, когда
                // на самом деле переменных больше нет
                if(end_c > curr_c)
                {
                    tmpstr = QString(&*curr_c, end_c - curr_c);
                    // У нас могут попасться заэкранированные символы в тексте, исправим это
                    for(int i = tmpstr.indexOf('\\'); i > 0; i = tmpstr.indexOf('\\', i + 1))
                        tmpstr.remove(i, 1);
                    // Первые два или три значения будут названиями геометрических переменных
                    // по которым строится график, а остальные - переменные со значениями
                    // Поэтому занесем все, что больше двух в вектор с переменными (потом удалим,
                    // если это нужно), а первые три - в массив геометрических переменных
                    // (потом просто не будем использовать ненужное)
                    if(var_num >= 2)
                        variables.push_back(tmpstr);
                    if(var_num < 3)
                        var_c[var_num++] = tmpstr;
                }
                else
                {
                    eos_flag = true;
                }
                curr_c = end_c_old;
            }
        }
        else
        {
            print_io_error();
            return;
        }
    }
    else
    {
        print_io_error();
        return;
    }

    // ZONE I= 101, J= 101, K= 1, F=POINT
    // ZONE T="BIG ZONE", I=3, J=3, F=POINT
    // ZONE I=100, J=100, DATAPACKING=POINT
    // ZONE I=2, J=2, F=POINT
    tmp = ifs.readLine();
    size_t IJK[3];
    IJK[0] = IJK[1] = IJK[2] = numeric_limits<size_t>::max();
    bool point_accepted = false; // Должен быть указан параметр POINT
    curr_c = tmp.begin();
    // Ну мало ли, вдруг кто-нибудь пробелов понаставит в начале строки
    while(* curr_c == ' ' || * curr_c == '\t')
        curr_c++;
    // Строка должна начинаться со слова ZONE
    if(QString(&*curr_c, 4) == "ZONE")
    {
        curr_c += 4;
        // Пробелы и табы нас не интересуют
        while(* curr_c == ' ' || * curr_c == '\t')
            curr_c++;
        QString param_name, param_value;
        bool flag_eol = false;
        // Пока есть данные в строке пригодные для разбора, будем разбирать
        while(!flag_eol)
        {
            // Пара параметр-значение всегда разделяется знаком равно
            if((end_c = std::find(curr_c, tmp.end(), '=')) != tmp.end())
            {
                // Все, что до равно за вычетов табов-пробелов - название параметра
                QString::iterator end_c_old = end_c;
                while(* (end_c - 1) == ' ' || * (end_c - 1) == '\t')
                    end_c--;
                param_name = QString(&*curr_c, end_c - curr_c);
                curr_c = end_c_old + 1;
                // А после равно - значение
                while(* curr_c == ' ' || * curr_c == '\t')
                    curr_c++;
                bool use_quote = (* curr_c == '\"') ? true : false;
                if(use_quote) curr_c++;

                // Если значение начинается с кавычки, то и искать следует только кавычку
                // Иначе могут быть пробел, таб или запятая
                if((use_quote && (end_c = std::find(curr_c, tmp.end(), '\"')) != tmp.end()) || (!use_quote &&
                   ((end_c = std::find(curr_c, tmp.end(), ',')) != tmp.end() ||
                    (end_c = std::find(curr_c, tmp.end(), ' ')) != tmp.end() ||
                    (end_c = std::find(curr_c, tmp.end(), '\t')) != tmp.end())))
                {
                    // Если тормознулись на кавычке, а это просто заэкранированная кавычка в тексте
                    bool maybe_escaped_quote = use_quote;
                    while(maybe_escaped_quote)
                    {
                        size_t counter = 0;
                        for(size_t i = 1; i <= (size_t)(end_c - curr_c); i++)
                            if(* (end_c - i) == '\\')
                                counter++;
                            else
                                break;
                        if(counter % 2 == 1)
                            end_c = std::find(end_c + 1, tmp.end(), '\"');
                        else
                            maybe_escaped_quote = false;
                        if(end_c == NULL)
                        {
                            print_io_error();
                            return;
                        }
                    }
                    // Если попало сюда, то все идет по плану
                    QString::iterator end_c_old = end_c;
                    if(!use_quote)
                    {
                        // Лишнего нам не надо, только само значение праметра
                        while(* (end_c - 1) == ' ' || * (end_c - 1) == '\t')
                            end_c--;
                    }
                    param_value = QString(&*curr_c, end_c - curr_c);
                    curr_c = end_c_old;
                    // Передвинемся на место после разделителя
                    while(* curr_c == ' ' || * curr_c == '\t' || * curr_c == ',' || * curr_c == '\"')
                        curr_c++;
                }
                else
                {
                    // Если попало сюда, то в значении параметра кавычек нет и пробелов тоже
                    // Однако это не так, если начали с кавычки
                    if(use_quote)
                    {
                        print_io_error();
                        return;
                    }
                    // То есть все до конца строки есть искомое значение
                    flag_eol = true;
                    param_value = QString(curr_c, tmp.end() - curr_c);
                }
                // У нас могут попасться заэкранированные символы в тексте, исправим это
                for(int i = param_value.indexOf('\\'); i > 0; i = param_value.indexOf('\\', i + 1))
                    param_value.remove(i, 1);

                // Для простоты приведем параметр и значение к верхнему регистру
                for(QString::iterator it = param_name.begin(); it != param_name.end(); ++it)
                    *it = it->toUpper();
                for(QString::iterator it = param_value.begin(); it != param_value.end(); ++it)
                    *it = it->toUpper();

                // Теперь разберемся, что за параметр мы считали
                // Это "I"
                if(param_name == "I")
                    IJK[0] = static_cast<size_t>(param_value.toUInt());
                // Это "J"
                else if(param_name == "J")
                    IJK[1] = static_cast<size_t>(param_value.toUInt());
                // Это "K"
                else if(param_name == "K")
                    IJK[2] = static_cast<size_t>(param_value.toUInt());
                // Это что-то с параметром "POINT"
                else if(param_value == "POINT")
                    point_accepted = true;
            }
            else
            {
                flag_eol = true;
            }
        }
    }
    else
    {
        print_io_error();
        return;
    }
    // Мы умеем работать только с точками
    if(!point_accepted)
    {
        print_io_error();
        return;
    }

    // Разберемся, сколько же у нас геометрических переменных
    // За одно понимаем, в какой плоскости лежит сечение
    size_t points_coord;
    size_t ind[2];
    if(IJK[0] == numeric_limits<size_t>::max() ||
       IJK[1] == numeric_limits<size_t>::max() ||
       IJK[2] == numeric_limits<size_t>::max())
    {
        // Таки две
        points_coord = 2;
        for(size_t i = 0; i < 3; i++)
            if(IJK[i] == numeric_limits<size_t>::max()) IJK[i] = 1;
        // Сечение x-y
        ind[0] = 0;
        ind[1] = 1;
    }
    else
    {
        // Таки три
        points_coord = 3;
        variables.erase(variables.begin());
        // Выберем подходящее сечение (по константной переменной всегда будет 1)
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
    }
    nx = IJK[ind[1]];
    ny = IJK[ind[0]];
    label_x = var_c[ind[0]];
    label_y = var_c[ind[1]];

    // Правим размерность векторов под переменные
    min_u.resize(variables.size());
    max_u.resize(variables.size());
    step_u_big.resize(variables.size());
    step_u_small.resize(variables.size());
    isolines.resize(variables.size());

    // Ищем минимальные и максимальные значения координат
    max_x = max_y = - numeric_limits<float>::max();
    min_x = min_y = numeric_limits<float>::max();
    for(size_t k = 0; k < variables.size(); k++)
    {
        min_u[k] = numeric_limits<float>::max();
        max_u[k] = - numeric_limits<float>::max();
    }

    // Читаем сами данные
    tec_data.resize(IJK[0] * IJK[1] * IJK[2]);
    for(size_t i = 0; i < tec_data.size(); i++)
    {
        float coords[3];
        for(size_t j = 0; j < points_coord; j++)
            coords[j] = read_number(ifs);
        tec_data[i].coord.x = coords[ind[0]];
        tec_data[i].coord.y = coords[ind[1]];
        tec_data[i].value = new float [variables.size()];
        for(size_t k = 0; k < variables.size(); k++)
            tec_data[i].value[k] = read_number(ifs);

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

    if(ifs.status() != QTextStream::Ok)
    {
        print_io_error();
        return;
    }
    qf.close();

    // Небольшие корректировки на всякий случай
    size_x = max_x - min_x;
    size_y = max_y - min_y;
    min_x -= size_x * 0.01f;
    max_x += size_x * 0.01f;
    min_y -= size_y * 0.01f;
    max_y += size_y * 0.01f;

    // Поправляем значения мин / макс чтобы влазило в сетку
    adjustAxis(min_x, max_x, num_ticks_x);
    adjustAxis(min_y, max_y, num_ticks_y);
    size_x = max_x - min_x;
    size_y = max_y - min_y;

    // Устанавливаем рекомендуемое значение для spinBox_Vectors
    vect_value = (int)(std::sqrt((float)(IJK[0] * IJK[1] * IJK[2])) / 20.0f);

    // Как-то так
    is_loaded = true;
    set_isolines_num(isolines_num);
    set_div_num(div_num);
}

// Изменение уровня интерполяции
void paintwidget::set_div_num(size_t num)
{
    if(use_memory_limit)
    {
        if(!is_loaded) return;
        // Число прямоугольнков
        size_t planned_size = (nx - 1) * (ny - 1);
        // Каждый прямоугольник бьется на 4 треугольника
        planned_size *= 4;
        // Каждый треугольник бьется на 4 на кадом разбиении
        for(size_t i = 0; i < num; i++)
            planned_size *= 4;
        // Размер самого треугольника
        size_t triangle_size = sizeof(triangle);
        // Размер содержимого векторов в треугольнике
        triangle_size += (sizeof(QColor) + sizeof(float) * 3) * variables.size();
        // Теперь все перемножаем
        planned_size *= triangle_size;

        // Лимит по памяти - 1 GiB
        if(planned_size >= 1024*1024*1024 && num != 0 && num > this->div_num)
            return;
        else
            this->div_num = num;
    }
    else
    {
        this->div_num = num;
        if(!is_loaded) return;
    }

    // Шаги для разбиения по цветовым областям
    for(size_t k = 0; k < variables.size(); k++)
    {
        step_u_big[k] = (max_u[k] - min_u[k]) / (use_purple ? 4.0f : 3.0f);
        step_u_small[k] = step_u_big[k] / 256.0f;
    }

    // Память не резиновая
    for(size_t i = 0; i < triangles.size(); i++)
    {
        delete [] triangles[i].color;
        for(size_t k = 0; k < 3; k++)
            delete [] triangles[i].solution[k];
    }
    triangles.clear();

    size_t tmp1_size = 4;
    for(size_t i = 0; i < num; i++)
        tmp1_size *= 4;
    triangle * tmp1;
    if(num % 2 == 0) tmp1 = new triangle[tmp1_size];
    else             tmp1 = new triangle[tmp1_size / 4];
    // Посчитаем в локальных координатах
    tmp1[0] = triangle( point(0.0f, 0.0f), point(1.0f, 0.0f), point(0.5f, 0.5f) );
    tmp1[1] = triangle( point(1.0f, 0.0f), point(1.0f, 1.0f), point(0.5f, 0.5f) );
    tmp1[2] = triangle( point(1.0f, 1.0f), point(0.0f, 1.0f), point(0.5f, 0.5f) );
    tmp1[3] = triangle( point(0.0f, 1.0f), point(0.0f, 0.0f), point(0.5f, 0.5f) );
    if(num > 0)
    {
        triangle * tmp2;
        if(num % 2 == 0) tmp2 = new triangle[tmp1_size / 4];
        else             tmp2 = new triangle[tmp1_size];
        tmp1_size = 4;
        for(size_t i = 0; i < num; i++)
        {
            size_t curr_index = 0;
            for(size_t j = 0; j < tmp1_size; j++)
            {
                point middles[3] =
                {
                    point((tmp1[j].nodes[0].x + tmp1[j].nodes[1].x) * 0.5f, (tmp1[j].nodes[0].y + tmp1[j].nodes[1].y) * 0.5f),
                    point((tmp1[j].nodes[0].x + tmp1[j].nodes[2].x) * 0.5f, (tmp1[j].nodes[0].y + tmp1[j].nodes[2].y) * 0.5f),
                    point((tmp1[j].nodes[1].x + tmp1[j].nodes[2].x) * 0.5f, (tmp1[j].nodes[1].y + tmp1[j].nodes[2].y) * 0.5f)
                };
                tmp2[curr_index++] = triangle(tmp1[j].nodes[0], middles[0], middles[1]);
                tmp2[curr_index++] = triangle(middles[0], tmp1[j].nodes[1], middles[2]);
                tmp2[curr_index++] = triangle(middles[1], middles[2], tmp1[j].nodes[2]);
                tmp2[curr_index++] = triangle(middles[0], middles[2], middles[1]);
            }
            tmp1_size *= 4;
            swap(tmp1, tmp2);
        }
        delete [] tmp2;
    }

    // Заполняем вектор из треугольников переводя координаты в глобальные и считая цвет
    size_t curr_index = 0;
    triangles.resize(tmp1_size * (nx - 1) * (ny - 1));
    // Обходим все прямоугольники
    for(size_t i = 0; i < nx - 1; i++)
    {
        for(size_t j = 0; j < ny - 1; j++)
        {
            float x0 = tec_data[i * ny + j].coord.x;
            float y0 = tec_data[i * ny + j].coord.y;
            float x1 = tec_data[(i + 1) * ny + j + 1].coord.x;
            float y1 = tec_data[(i + 1) * ny + j + 1].coord.y;
            float hx = x1 - x0;
            float hy = y1 - y0;
            float step_x_i = std::fabs(tec_data[(i + 1) * ny + j].coord.x - x0);
            float step_x_j = std::fabs(tec_data[i * ny + j + 1].coord.x - x0);
            bool native_order = (step_x_i > step_x_j) ? true : false;

            for(size_t tn = 0; tn < tmp1_size; tn++)
            {
                triangle tmp_tr;
                tmp_tr.color = new QColor [variables.size()];
                // Переводим координаты в глобальные
                for(size_t k = 0; k < 3; k++)
                {
                    tmp_tr.nodes[k].x = tmp1[tn].nodes[k].x * hx + x0;
                    tmp_tr.nodes[k].y = tmp1[tn].nodes[k].y * hy + y0;
                    tmp_tr.solution[k] = new float [variables.size()];
                }

                // Занесем значение решения в узлах
                for(size_t k = 0; k < 3; k++)
                {
                    // Строим билинейную интерполяцию
                    for(size_t m = 0; m < variables.size(); m++)
                    {
                        float r1, r2;
                        if(native_order)
                        {
                            r1 = (x1 - tmp_tr.nodes[k].x) / hx * tec_data[i * ny + j].value[m] +
                                    (tmp_tr.nodes[k].x - x0) / hx * tec_data[(i + 1) * ny + j].value[m];
                            r2 = (x1 - tmp_tr.nodes[k].x) / hx * tec_data[i * ny + j + 1].value[m] +
                                    (tmp_tr.nodes[k].x - x0) / hx * tec_data[(i + 1) * ny + j + 1].value[m];
                        }
                        else
                        {
                            r1 = (x1 - tmp_tr.nodes[k].x) / hx * tec_data[i * ny + j].value[m] +
                                    (tmp_tr.nodes[k].x - x0) / hx * tec_data[i * ny + j + 1].value[m];
                            r2 = (x1 - tmp_tr.nodes[k].x) / hx * tec_data[(i + 1) * ny + j].value[m] +
                                    (tmp_tr.nodes[k].x - x0) / hx * tec_data[(i + 1) * ny + j + 1].value[m];
                        }
                        tmp_tr.solution[k][m] = (y1 - tmp_tr.nodes[k].y) / hy * r1 +
                                             (tmp_tr.nodes[k].y - y0) / hy * r2;
                    }
                }

                // Барицентр треугольника
                float cx = 0.0f, cy = 0.0f;
                for(size_t k = 0; k < 3; k++)
                {
                    cx += tmp_tr.nodes[k].x;
                    cy += tmp_tr.nodes[k].y;
                }
                point barycenter(cx / 3.0f, cy / 3.0f);

                // Решение в барицентре
                vector<float> center(variables.size());
                // Строим билинейную интерполяцию
                for(size_t m = 0; m < variables.size(); m++)
                {
                    float r1, r2;
                    if(native_order)
                    {
                        r1 = (x1 - barycenter.x) / hx * tec_data[i * ny + j].value[m] +
                                (barycenter.x - x0) / hx * tec_data[(i + 1) * ny + j].value[m];
                        r2 = (x1 - barycenter.x) / hx * tec_data[i * ny + j + 1].value[m] +
                                (barycenter.x - x0) / hx * tec_data[(i + 1) * ny + j + 1].value[m];
                    }
                    else
                    {
                        r1 = (x1 - barycenter.x) / hx * tec_data[i * ny + j].value[m] +
                                (barycenter.x - x0) / hx * tec_data[i * ny + j + 1].value[m];
                        r2 = (x1 - barycenter.x) / hx * tec_data[(i + 1) * ny + j].value[m] +
                                (barycenter.x - x0) / hx * tec_data[(i + 1) * ny + j + 1].value[m];
                    }
                    center[m] = (y1 - barycenter.y) / hy * r1 +
                                         (barycenter.y - y0) / hy * r2;
                }

                for(size_t v = 0; v < variables.size(); v++)
                {
                    typedef int color_type;
                    // Ищем цвет решения по алгоритму заливки радугой (Rainbow colormap)
                    color_type r_color = 0, g_color = 0, b_color = 0;
                    if(use_purple)
                    {
                        if(center[v] > min_u[v] + step_u_big[v] * 3.0f)
                        {
                            r_color = 255;
                            g_color = 255 - (color_type)((center[v] - (min_u[v] + step_u_big[v] * 3.0f)) / step_u_small[v]);
                            b_color = 0;
                        }
                        else if(center[v] > min_u[v] + step_u_big[v] * 2.0f)
                        {
                            r_color = (color_type)((center[v] - (min_u[v] + step_u_big[v] * 2.0f)) / step_u_small[v]);
                            g_color = 255;
                            b_color = 0;
                        }
                        else if(center[v] > min_u[v] + step_u_big[v])
                        {
                            color_type tmp = (color_type)((center[v] - (min_u[v] + step_u_big[v])) / step_u_small[v]);
                            r_color = 0;
                            g_color = tmp;
                            b_color = 255 - tmp;
                        }
                        else
                        {
                            color_type tmp = 76 - (color_type)((center[v] - min_u[v]) / (step_u_small[v] * (255.0f / 76.0f)));
                            r_color = tmp;
                            g_color = 0;
                            b_color = 255 - tmp;
                        }
                    }
                    else
                    {
                        if(center[v] > min_u[v] + step_u_big[v] * 2.0f)
                        {
                            r_color = 255;
                            g_color = 255 - (color_type)((center[v] - (min_u[v] + step_u_big[v] * 2.0f)) / step_u_small[v]);
                            b_color = 0;
                        }
                        else if(center[v] > min_u[v] + step_u_big[v])
                        {
                            r_color = (color_type)((center[v] - (min_u[v] + step_u_big[v])) / step_u_small[v]);
                            g_color = 255;
                            b_color = 0;
                        }
                        else
                        {
                            color_type tmp = (color_type)((center[v] - min_u[v]) / step_u_small[v]);
                            r_color = 0;
                            g_color = tmp;
                            b_color = 255 - tmp;
                        }
                    }

                    if(use_light_colors)
                    {
                        // Приглушаем кислотные цвета
                        r_color = r_color * 3 / 4 + 64;
                        g_color = g_color * 3 / 4 + 64;
                        b_color = b_color * 3 / 4 + 64;
                    }

                    // Из-за ошибок округления иногда вылетает за границы
                    if(r_color > 255) r_color = 255;
                    if(g_color > 255) g_color = 255;
                    if(b_color > 255) b_color = 255;
                    if(r_color < 0) r_color = 0;
                    if(g_color < 0) g_color = 0;
                    if(b_color < 0) b_color = 0;

                    // Задаем посчитанный цвет
                    tmp_tr.color[v] = QColor(r_color, g_color, b_color);
                }

                // И заносим в вектор
                triangles[curr_index++] = tmp_tr;
            }
        }
    }
    delete [] tmp1;
}

// Конструктор
paintwidget::paintwidget(QWidget * parent) : QWidget(parent)
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

    isolines_config = lines_config(Qt::black, 1.0f);
    vectors_config = lines_config(Qt::black, 1.0f, 10.0f, 5.0f);
    use_purple = false;
    use_legend = true;
    use_light_colors = true;
    use_memory_limit = true;

    buffer_valid = false;
}

// Деструктор
paintwidget::~paintwidget()
{
    for(size_t i = 0; i < tec_data.size(); i++)
        delete [] tec_data[i].value;
    for(size_t i = 0; i < triangles.size(); i++)
    {
        delete [] triangles[i].color;
        for(size_t k = 0; k < 3; k++)
            delete [] triangles[i].solution[k];
    }
}

// Пересчет значений изолиний
void paintwidget::set_isolines_num(size_t isolines_num)
{
    this->isolines_num = isolines_num;
    if(!is_loaded) return;
    for(size_t j = 0; j < variables.size(); j++)
    {
        isolines[j].clear();
        float isolines_step = (max_u[j] - min_u[j]) / (float)(isolines_num + 1);
        for(size_t i = 0; i < isolines_num; i++)
            isolines[j].insert(min_u[j] + isolines_step * (float)(i + 1));
    }
}

// Подгонка осей под реальность и вычисление шагов координатной сетки
void paintwidget::adjustAxis(float & min, float & max, size_t & numTicks) const
{
    static const float axis_epsilon = 1.0f / 10000.0f;
    if(max - min < axis_epsilon)
    {
        min -= 2.0f * axis_epsilon;
        max += 2.0f * axis_epsilon;
    }

    static const size_t MinTicks = 10;
    float grossStep = (max - min) / MinTicks;
    float step = std::pow(10.0f, std::floor(std::log10(grossStep)));

    if (5.0f * step < grossStep)
        step *= 5.0f;
    else if (2.0f * step < grossStep)
        step *= 2.0f;

    numTicks = (size_t)(std::ceil(max / step) - std::floor(min / step));
    min = std::floor(min / step) * step;
    max = std::ceil(max / step) * step;
}

// Геометрия окна
QPoint paintwidget::to_window(float x, float y) const
{
    // В OpenGL это был бы glOrtho
    const float gl_x0 = -0.06f;
    const float gl_y0 = -0.06f;
    const float gl_x1 = use_legend ? 1.132f : 1.015f;
    const float gl_y1 = 1.02f;
    const float gl_hx = gl_x1 - gl_x0;
    const float gl_hy = gl_y1 - gl_y0;
    // Перевод
    int xl = (int)((x - gl_x0) / gl_hx * (float)width());
    int yl = height() - (int)((y - gl_y0) / gl_hy * (float)height());
    return QPoint(xl, yl);
}

// Перерисовать, сбросив валидность буфера
void paintwidget::invalidate()
{
    buffer_valid = false;
    repaint();
}

// Отрисовка сцены
void paintwidget::paintEvent(QPaintEvent *)
{
    if(!buffer_valid || buffer.width() != width() || buffer.height() != height())
    {
        buffer = QImage(width(), height(), QImage::Format_RGB888);
        draw(& buffer, false);
        buffer_valid = true;
    }
    QPainter painter;
    painter.begin(this);
    painter.setViewport(0, 0, width(), height());
    painter.drawImage(0, 0, buffer);
    painter.end();
}

// Матрицы цветов для рисования легенды
// Для спектра с фиолетовым цветом
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
const QColor purple_light_legend_colors[14] =
{
    QColor(  76 * 3 / 4 + 64,   0 * 3 / 4 + 64, 179 * 3 / 4 + 64 ),
    QColor(  15 * 3 / 4 + 64,   0 * 3 / 4 + 64, 240 * 3 / 4 + 64 ),
    QColor(   0 * 3 / 4 + 64,  50 * 3 / 4 + 64, 205 * 3 / 4 + 64 ),
    QColor(   0 * 3 / 4 + 64, 115 * 3 / 4 + 64, 140 * 3 / 4 + 64 ),
    QColor(   0 * 3 / 4 + 64, 180 * 3 / 4 + 64,  75 * 3 / 4 + 64 ),
    QColor(   0 * 3 / 4 + 64, 245 * 3 / 4 + 64,  10 * 3 / 4 + 64 ),
    QColor(  55 * 3 / 4 + 64, 255 * 3 / 4 + 64,   0 * 3 / 4 + 64 ),
    QColor( 120 * 3 / 4 + 64, 255 * 3 / 4 + 64,   0 * 3 / 4 + 64 ),
    QColor( 185 * 3 / 4 + 64, 255 * 3 / 4 + 64,   0 * 3 / 4 + 64 ),
    QColor( 250 * 3 / 4 + 64, 255 * 3 / 4 + 64,   0 * 3 / 4 + 64 ),
    QColor( 255 * 3 / 4 + 64, 195 * 3 / 4 + 64,   0 * 3 / 4 + 64 ),
    QColor( 255 * 3 / 4 + 64, 130 * 3 / 4 + 64,   0 * 3 / 4 + 64 ),
    QColor( 255 * 3 / 4 + 64,  65 * 3 / 4 + 64,   0 * 3 / 4 + 64 ),
    QColor( 255 * 3 / 4 + 64,   0 * 3 / 4 + 64,   0 * 3 / 4 + 64 )
};
const QColor purple_normal_legend_colors[14] =
{
    QColor(  76,   0, 179 ),
    QColor(  15,   0, 240 ),
    QColor(   0,  50, 205 ),
    QColor(   0, 115, 140 ),
    QColor(   0, 180,  75 ),
    QColor(   0, 245,  10 ),
    QColor(  55, 255,   0 ),
    QColor( 120, 255,   0 ),
    QColor( 185, 255,   0 ),
    QColor( 250, 255,   0 ),
    QColor( 255, 195,   0 ),
    QColor( 255, 130,   0 ),
    QColor( 255,  65,   0 ),
    QColor( 255,   0,   0 )
};
// Для спектра без фиолетового цвета
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
const QColor default_light_legend_colors[14] =
{
    QColor(   0 * 3 / 4 + 64,   0 * 3 / 4 + 64, 255 * 3 / 4 + 64 ),
    QColor(   0 * 3 / 4 + 64,  59 * 3 / 4 + 64, 196 * 3 / 4 + 64 ),
    QColor(   0 * 3 / 4 + 64, 118 * 3 / 4 + 64, 137 * 3 / 4 + 64 ),
    QColor(   0 * 3 / 4 + 64, 177 * 3 / 4 + 64,  78 * 3 / 4 + 64 ),
    QColor(   0 * 3 / 4 + 64, 236 * 3 / 4 + 64,  19 * 3 / 4 + 64 ),
    QColor(  50 * 3 / 4 + 64, 245 * 3 / 4 + 64,   0 * 3 / 4 + 64 ),
    QColor(  99 * 3 / 4 + 64, 255 * 3 / 4 + 64,   0 * 3 / 4 + 64 ),
    QColor( 157 * 3 / 4 + 64, 255 * 3 / 4 + 64,   0 * 3 / 4 + 64 ),
    QColor( 215 * 3 / 4 + 64, 255 * 3 / 4 + 64,   0 * 3 / 4 + 64 ),
    QColor( 255 * 3 / 4 + 64, 236 * 3 / 4 + 64,   0 * 3 / 4 + 64 ),
    QColor( 255 * 3 / 4 + 64, 177 * 3 / 4 + 64,   0 * 3 / 4 + 64 ),
    QColor( 255 * 3 / 4 + 64, 118 * 3 / 4 + 64,   0 * 3 / 4 + 64 ),
    QColor( 255 * 3 / 4 + 64,  59 * 3 / 4 + 64,   0 * 3 / 4 + 64 ),
    QColor( 255 * 3 / 4 + 64,   0 * 3 / 4 + 64,   0 * 3 / 4 + 64 )
};
const QColor default_normal_legend_colors[14] =
{
    QColor(   0,   0, 255 ),
    QColor(   0,  59, 196 ),
    QColor(   0, 118, 137 ),
    QColor(   0, 177,  78 ),
    QColor(   0, 236,  19 ),
    QColor(  50, 245,   0 ),
    QColor(  99, 255,   0 ),
    QColor( 157, 255,   0 ),
    QColor( 215, 255,   0 ),
    QColor( 255, 236,   0 ),
    QColor( 255, 177,   0 ),
    QColor( 255, 118,   0 ),
    QColor( 255,  59,   0 ),
    QColor( 255,   0,   0 )
};

// Отрисовка сцены на QPaintDevice
void paintwidget::draw(QPaintDevice * device, bool transparency, bool is_svg)
{
    QPainter painter;
    painter.begin(device);
    painter.setViewport(0, 0, device->width(), device->height());

    // Заливка области белым цветом
    if(!transparency)
        painter.fillRect(0, 0, device->width(), device->height(), QBrush(Qt::white));
    else
        painter.fillRect(0, 0, device->width(), device->height(), QBrush(Qt::transparent));

    if(!is_loaded) return;

    // Координатная сетка
    painter.setPen(QPen(Qt::lightGray));
    for(size_t i = 0; i <= num_ticks_x; i++)
    {
        float x = (float)i / (float)num_ticks_x;
        painter.drawLine(to_window(x, -0.01f), to_window(x, 1.0f));
    }
    for(size_t i = 0; i <= num_ticks_y; i++)
    {
        float y = (float)i / (float)num_ticks_y;
        painter.drawLine(to_window(-0.01f, y), to_window(1.0f, y));
    }

    // Координатные оси
    painter.setPen(QPen(Qt::black, 2.0f));
    painter.drawLine(to_window(0.0f, -0.005f), to_window(0.0f, 1.005f));
    painter.drawLine(to_window(-0.005f, 0.0f), to_window(1.005f, 0.0f));

    // Шрифты
    QFont fnt_mono("Courier");
    fnt_mono.setLetterSpacing(QFont::PercentageSpacing, 82);
    int fnt_mono_h = height() / 45;
    int fnt_mono_w = width() / 55;
    fnt_mono.setPixelSize(fnt_mono_h < fnt_mono_w ? fnt_mono_h : fnt_mono_w);
    QFont fnt_serif("Times");
    int fnt_serif_h = height() / 40;
    int fnt_serif_w = width() / 45;
    fnt_serif.setPixelSize(fnt_serif_h < fnt_serif_w ? fnt_serif_h : fnt_serif_w);
    fnt_serif.setBold(true);

#if defined HAVE_LESS_THAN_QT45
    fnt_mono.setPointSizeF(qreal(fnt_mono.pixelSize()) * qreal(0.75));
    fnt_serif.setPointSizeF(qreal(fnt_serif.pixelSize()) * qreal(0.75));
#endif

    // Подписи осей
    painter.setFont(fnt_serif);
    painter.setPen(QPen(Qt::black, 1.0f));
    painter.drawText(to_window(0.99f, -0.04f), label_x);
    painter.drawText(to_window(-0.05f, 0.99f), label_y);

    // Отрисовка шкалы
    painter.setFont(fnt_mono);
    for(size_t i = 0; i < num_ticks_x; i++)
    {
        float x = (float)i / (float)num_ticks_x;
        float x_real = (float)(std::floor((x * size_x + min_x) * 10000.0f + 0.5f)) / 10000.0f;
        QString st = QString::number(x_real);
        painter.drawText(to_window(x - 0.01f, -0.04f), st);
    }
    for(size_t i = 0; i < num_ticks_y; i++)
    {
        float y = (float)i / (float)num_ticks_y;
        float y_real = (float)(std::floor((y * size_y + min_y) * 10000.0f + 0.5f)) / 10000.0f;
        QString st = QString::number(y_real);
        painter.drawText(to_window(-0.05f, y - 0.01f), st);
    }

    // Раскрашивать будем если запрошено сие
    if(draw_color)
    {
        // Если не SVG, то рисуем как обычно
        if(!is_svg)
        {
            // Отрисовка всех треугольников
            for(size_t i = 0; i < triangles.size(); i++)
            {
                // Задаем посчитанный цвет
                painter.setBrush(QBrush(triangles[i].color[draw_index]));
                painter.setPen(QPen(triangles[i].color[draw_index], 1));

                // Рисуем
                QPoint tr[3];
                for(size_t k = 0; k < 3; k++)
                    tr[k] = to_window((triangles[i].nodes[k].x - min_x) / size_x, (triangles[i].nodes[k].y - min_y) / size_y);
                painter.drawPolygon(tr, 3);
            }
        }
        // Для SVG будем использовать растровую заливку цветом (иначе дюже жирный файл выходит)
        else
        {
            QImage image(device->width(), device->height(), QImage::Format_ARGB32_Premultiplied);
            QPainter painter_img;
            painter_img.begin(& image);
            painter_img.setViewport(0, 0, device->width(), device->height());
            painter_img.fillRect(0, 0, device->width(), device->height(), QBrush(Qt::transparent));
            // Изображение потом обрежем по реальному полезному размеру
            int max_x_real = -numeric_limits<int>::max(), max_y_real = -numeric_limits<int>::max();
            int min_x_real = numeric_limits<int>::max(), min_y_real = numeric_limits<int>::max();

            // Отрисовка всех треугольников
            for(size_t i = 0; i < triangles.size(); i++)
            {
                // Задаем посчитанный цвет
                painter_img.setBrush(QBrush(triangles[i].color[draw_index]));
                painter_img.setPen(QPen(triangles[i].color[draw_index], 1));

                // Рисуем
                QPoint tr[3];
                for(size_t k = 0; k < 3; k++)
                {
                    tr[k] = to_window((triangles[i].nodes[k].x - min_x) / size_x, (triangles[i].nodes[k].y - min_y) / size_y);
                    if(tr[k].x() < min_x_real) min_x_real = tr[k].x();
                    if(tr[k].x() > max_x_real) max_x_real = tr[k].x();
                    if(tr[k].y() < min_y_real) min_y_real = tr[k].y();
                    if(tr[k].y() > max_y_real) max_y_real = tr[k].y();
                }
                painter_img.drawPolygon(tr, 3);
            }

            painter_img.end();
            image = image.copy(min_x_real, min_y_real, max_x_real - min_x_real + 1, max_y_real - min_y_real + 1);
            painter.drawImage(min_x_real, min_y_real, image);
        }
    }

    // Изолинии рисуем только если оно нам надо
    if(draw_isolines)
    {
        painter.setPen(QPen(isolines_config.color, isolines_config.width));
        // Узлы изолиний с номерами (нужно для графов для SVG)
        QList<QPointN> isol_nodes_l;
        // Списки смежности узлов изолиний (нужно для графов для SVG)
        QVector<QList<int> > isol_connected;

        for(size_t i = 0; i < triangles.size(); i++)
        {
            // Теперь рисуем изолинии
            // Будем искать наименьшее значение, большее или равное решению
            // Если значения на разных концах ребра будут разными, значит изолиния проходит через это ребро
            set<float>::const_iterator segment_isol[3];
            for(size_t k = 0; k < 3; k++)
                segment_isol[k] = isolines[draw_index].lower_bound(triangles[i].solution[k][draw_index]);

            // А теперь нарисуем, согласно вышеприведенному условию
            QPoint isol_points[3];
            size_t counter = 0;
            if(segment_isol[0] != segment_isol[1])
            {
                isol_points[counter] = to_window(((triangles[i].nodes[1].x + triangles[i].nodes[0].x) * 0.5f - min_x) / size_x, ((triangles[i].nodes[1].y + triangles[i].nodes[0].y) * 0.5f - min_y) / size_y);
                counter++;
            }
            if(segment_isol[0] != segment_isol[2])
            {
                isol_points[counter] = to_window(((triangles[i].nodes[2].x + triangles[i].nodes[0].x) * 0.5f - min_x) / size_x, ((triangles[i].nodes[2].y + triangles[i].nodes[0].y) * 0.5f - min_y) / size_y);
                counter++;
            }
            if(segment_isol[1] != segment_isol[2])
            {
                isol_points[counter] = to_window(((triangles[i].nodes[2].x + triangles[i].nodes[1].x) * 0.5f - min_x) / size_x, ((triangles[i].nodes[2].y + triangles[i].nodes[1].y) * 0.5f - min_y) / size_y);
                counter++;
            }

            // Если есть что рисовать - нарисуем
            if(counter > 1)
            {
                // Если не SVG, то рисуем как обычно
                if(!is_svg)
                {
                    painter.drawLine(isol_points[0], isol_points[1]);
                }
                // Для SVG нужно объединить все связные сегменты изолиний в одну полилинию
                // Поэтому занумеруем все точки и построим списки смежности
                else
                {
                    // Занумеруем точки
                    int nums[2] = {-1, -1};
                    for(size_t k = 0; k < 2; k++)
                    {
                        QPointN new_node(isol_points[k]);
                        QList<QPointN>::iterator r = std::find(isol_nodes_l.begin(), isol_nodes_l.end(), new_node);
                        if(r != isol_nodes_l.end())
                            nums[k] = r->num;
                        else
                        {
                            nums[k] = new_node.num = isol_nodes_l.size();
                            isol_nodes_l.push_front(new_node);
                        }
                    }
                    // Если есть непустой путь, то добавим его в списки смежности
                    if(nums[0] != nums[1])
                    {
                        int max_num = (nums[0] > nums[1] ? nums[0] : nums[1]);
                        if(isol_connected.size() <= max_num) isol_connected.resize(max_num + 1);
                        isol_connected[nums[0]].push_back(nums[1]);
                        isol_connected[nums[1]].push_back(nums[0]);
                    }
                }
            }
        }

        // Для SVG по спискам смежности сделаем объединение сегментов в полилинию
        if(is_svg)
        {
            // Перенесем все в вектор для удобства индексации
            QVector<QPoint> isol_nodes_v(isol_nodes_l.size());
            for(QList<QPointN>::iterator it = isol_nodes_l.begin(); it != isol_nodes_l.end(); ++it)
                isol_nodes_v[it->num] = it->toQPoint();
            isol_nodes_l.clear();

            // Алгоритм в основан на поиске в глубину
            while(true)
            {
                QList<int> path;

                // Ищем вершину, с которой начнем свой путь
                int beg_index;
                for(beg_index = 0; beg_index < isol_connected.size(); beg_index++)
                    if(isol_connected[beg_index].size() > 0)
                        break;
                if(beg_index == isol_connected.size())
                    break;
                path.push_back(beg_index);

                // Идем обходом в глубину от этой вершины
                int curr_index = beg_index;
                while(true)
                {
                    QList<int>::iterator dest_iter = isol_connected[curr_index].begin();
                    if(dest_iter == isol_connected[curr_index].end())
                        break;
                    int dest_index = * dest_iter;
                    isol_connected[curr_index].erase(dest_iter);
                    isol_connected[dest_index].erase(std::find(isol_connected[dest_index].begin(), isol_connected[dest_index].end(), curr_index));
                    path.push_back(dest_index);
                    curr_index = dest_index;
                }

                // И в обратную сторону тоже
                curr_index = beg_index;
                while(true)
                {
                    QList<int>::iterator dest_iter = isol_connected[curr_index].begin();
                    if(dest_iter == isol_connected[curr_index].end())
                        break;
                    int dest_index = * dest_iter;
                    isol_connected[curr_index].erase(dest_iter);
                    isol_connected[dest_index].erase(std::find(isol_connected[dest_index].begin(), isol_connected[dest_index].end(), curr_index));
                    path.push_front(dest_index);
                    curr_index = dest_index;
                }

                // Подумаем, а стоит ли рисовать этот путь?
                int critical_number = 5;        // Число сегментов, больше которого будем рисовать без вопросов
                float critical_distance = 2.5f; // Максимальное расстояние между двумя точками в пути, == || ==
                bool is_long_path = (path.size() > critical_number ? true : false);
                if(!is_long_path)
                {
                    float max_distance = 0.0f;
                    for(QList<int>::iterator it = path.begin(); it != path.end(); ++it)
                    {
                        for(QList<int>::iterator jt = path.begin(); jt != path.end(); ++jt)
                        {
                            float dx = (float)isol_nodes_v[* it].x() - (float)isol_nodes_v[* jt].x();
                            float dy = (float)isol_nodes_v[* it].y() - (float)isol_nodes_v[* jt].y();
                            float curr_distance = (float)sqrt(dx * dx + dy * dy);
                            if(curr_distance > max_distance)
                                max_distance = curr_distance;
                        }
                    }
                    if(max_distance > critical_distance)
                        is_long_path = true;
                }
                if(is_long_path)
                {
                    // Рисуем полученный путь
                    QVector<QPoint> path_vec(path.size());
                    int index = 0;
                    for(QList<int>::iterator it = path.begin(); it != path.end(); ++it)
                        path_vec[index++] = isol_nodes_v[* it];
                    painter.drawPolyline(path_vec.data(), index);
                    //painter.drawEllipse(isol_nodes_v[path.front()], 2, 2);
                    //painter.drawEllipse(isol_nodes_v[path.back()], 2, 2);
                }
            }

        }
    }

    // Легенда
    if(use_legend)
    {
        const QColor * legend_colors;
        if(use_purple)
            if(use_light_colors)
                legend_colors = purple_light_legend_colors;
            else
                legend_colors = purple_normal_legend_colors;
        else
            if(use_light_colors)
                legend_colors = default_light_legend_colors;
            else
                legend_colors = default_normal_legend_colors;

        float legend_values[14];
        if(use_purple)
        {
            legend_values [0]  = min_u[draw_index];
            legend_values [1]  = 61.0f * (step_u_small[draw_index] * (255.0f / 76.0f)) + min_u[draw_index];
            legend_values [2]  = 50.0f * step_u_small[draw_index] + min_u[draw_index] + step_u_big[draw_index];
            legend_values [3]  = 115.0f * step_u_small[draw_index] + min_u[draw_index] + step_u_big[draw_index];
            legend_values [4]  = 180.0f * step_u_small[draw_index] + min_u[draw_index] + step_u_big[draw_index];
            legend_values [5]  = 245.0f * step_u_small[draw_index] + min_u[draw_index] + step_u_big[draw_index];
            legend_values [6]  = 55.0f * step_u_small[draw_index] + min_u[draw_index] + step_u_big[draw_index] * 2.0f;
            legend_values [7]  = 120.0f * step_u_small[draw_index] + min_u[draw_index] + step_u_big[draw_index] * 2.0f;
            legend_values [8]  = 185.0f * step_u_small[draw_index] + min_u[draw_index] + step_u_big[draw_index] * 2.0f;
            legend_values [9]  = 250.0f * step_u_small[draw_index] + min_u[draw_index] + step_u_big[draw_index] * 2.0f;
            legend_values [10] = 60.0f * step_u_small[draw_index] + min_u[draw_index] + step_u_big[draw_index] * 3.0f;
            legend_values [11] = 125.0f * step_u_small[draw_index] + min_u[draw_index] + step_u_big[draw_index] * 3.0f;
            legend_values [12] = 190.0f * step_u_small[draw_index] + min_u[draw_index] + step_u_big[draw_index] * 3.0f;
            legend_values [13] = max_u[draw_index];
        }
        else
        {
            legend_values [0]  = min_u[draw_index];
            legend_values [1]  = 59.0f * step_u_small[draw_index] + min_u[draw_index];
            legend_values [2]  = 118.0f * step_u_small[draw_index] + min_u[draw_index];
            legend_values [3]  = 177.0f * step_u_small[draw_index] + min_u[draw_index];
            legend_values [4]  = 236.0f * step_u_small[draw_index] + min_u[draw_index];
            legend_values [5]  = 50.0f * step_u_small[draw_index] + step_u_big[draw_index] + min_u[draw_index];
            legend_values [6]  = 99.0f * step_u_small[draw_index] + step_u_big[draw_index] + min_u[draw_index];
            legend_values [7]  = 157.0f * step_u_small[draw_index] + step_u_big[draw_index] + min_u[draw_index];
            legend_values [8]  = 215.0f * step_u_small[draw_index] + step_u_big[draw_index] + min_u[draw_index];
            legend_values [9]  = 19.0f * step_u_small[draw_index] + step_u_big[draw_index] * 2.0f + min_u[draw_index];
            legend_values [10] = 78.0f * step_u_small[draw_index] + step_u_big[draw_index] * 2.0f + min_u[draw_index];
            legend_values [11] = 137.0f * step_u_small[draw_index] + step_u_big[draw_index] * 2.0f + min_u[draw_index];
            legend_values [12] = 196.0f * step_u_small[draw_index] + step_u_big[draw_index] * 2.0f + min_u[draw_index];
            legend_values [13] = max_u[draw_index];
        }

        for(size_t i = 0; i < 14; i++)
        {
            static const float x0 = 1.0175f;
            static const float y0 = 0.0f;
            static const float dx = 0.0f;
            static const float dy = 0.07f;
            static const float hx = 0.107f;
            static const float hy = 0.073f;
            painter.setBrush(QBrush(legend_colors[i]));
            painter.setPen(QPen(legend_colors[i], 1));
            painter.drawRect(QRect(to_window(x0, y0 + dy * i + hy), to_window(x0 + hx, y0 + dy * i)));
            painter.setFont(fnt_mono);
            painter.setPen(QPen(Qt::black));
            QString st = QString::number(legend_values[i], 'E', 2);
            painter.drawText(to_window(x0 + dx * i + 0.005f, y0 + dy * i + hy / 2.0f - 0.01f), st);
        }
    }

    if(draw_vectors)
    {
        painter.setPen(QPen(vectors_config.color, vectors_config.width));

        float vec_len = vectors_config.length, arrow_len = vectors_config.arrow_size;
        float angle = 38.0f * 3.14159265358979323846f / 180.0f;
        float sin_angle = std::sin(angle), cos_angle = std::cos(angle);

        for(size_t j = 0; j < ny; j += skip_vec)
        {
            for(size_t i = 0; i < nx; i += skip_vec)
            {
                tecplot_node * v = & tec_data[i * ny + j];
                QPoint begin = to_window((v->coord.x - min_x) / size_x, (v->coord.y - min_y) / size_y);
                float len_x = v->value[ind_vec_1];
                float len_y = v->value[ind_vec_2];
                float norm = std::sqrt(len_x * len_x + len_y * len_y);
                len_x /= norm;
                len_y /= norm;
                if(len_x == len_x && len_y == len_y)
                {
                    QPoint end((int)(len_x * vec_len), -(int)(len_y * vec_len));
                    end.setX(end.x() + begin.x());
                    end.setY(end.y() + begin.y());
                    painter.drawLine(begin, end);

                    // x1=x*cos(angle)-y*sin(angle);
                    // y1=y*cos(angle)+x*sin(angle);

                    // Угол Pi + angle
                    float x1 = - len_x * cos_angle + len_y * sin_angle;
                    float y1 = - len_y * cos_angle - len_x * sin_angle;
                    norm = std::sqrt(x1 * x1 + y1 * y1);
                    x1 *= arrow_len / norm;
                    y1 *= arrow_len / norm;
                    painter.drawLine(end, QPoint((int)x1 + end.x(), -(int)y1 + end.y()));

                    // Угол Pi - angle
                    float x2 = - len_x * cos_angle - len_y * sin_angle;
                    float y2 = - len_y * cos_angle + len_x * sin_angle;
                    norm = std::sqrt(x2 * x2 + y2 * y2);
                    x2 *= arrow_len / norm;
                    y2 *= arrow_len / norm;
                    painter.drawLine(end, QPoint((int)x2 + end.x(), -(int)y2 + end.y()));
                }
            }
        }
    }

    painter.end();
}
