#define NOMINMAX

#include "paintwidget.h"
#include <limits>
#include <cstdio>
#include <cmath>

// getline с поддержкой юникода
#if defined UNICODE || defined _UNICODE
wifstream & getline(wifstream & ifs, string & str)
{
    str.clear();
    wchar_t c_w = 0;
    while(ifs.good() && ifs.get(c_w) && c_w != L'\n')
    {
        char c_c;
        WideCharToMultiByte(CP_ACP, 0, &c_w, 1, &c_c, 1, 0, 0);
        str.push_back(c_c);
    }
    return ifs;
}
#endif

// Вывести msgbox с ошибкой
void paintwidget::print_io_error()
{
    MessageBox(hwnd, TEXT("Error: Corrupted file"), TEXT("Error"), MB_OK | MB_ICONERROR);
}

// Считать из u_ifstream число с разделителями ' ', '\t', ',', '\r' или '\n'
float paintwidget::read_number(u_ifstream & ifs)
{
    TCHAR str[32]; // Должно хватить
    size_t len = 0;
    // Читаем мусор перед числом
    do
        ifs.get(str[0]);
    while((str[0] == TEXT(' ') || str[0] == TEXT('\t') || str[0] == TEXT(',') || str[0] == TEXT('\r') || str[0] == TEXT('\n')) && ifs.good());
    if(!ifs.good()) return 0.0f;
    // Читаем число
    do
        ifs.get(str[++len]);
    while(str[len] != TEXT(' ') && str[len] != TEXT('\t') && str[len] != TEXT(',') && str[len] != TEXT('\r') && str[len] != TEXT('\n') && ifs.good());
    if(!ifs.good()) return 0.0f;
    str[len] = TEXT('\0');
    // Преобразуем во float
    float num = 0.0f;
#if defined _MSC_VER && _MSC_VER >= 1400
#if defined UNICODE || defined _UNICODE
    swscanf_s(str, TEXT("%f"), &num);
#else
    sscanf_s(str, TEXT("%f"), &num);
#endif
#else
#if defined UNICODE || defined _UNICODE
    swscanf(str, TEXT("%f"), &num);
#else
    sscanf(str, TEXT("%f"), &num);
#endif
#endif
    return num;
}

// Чтение текплотовских значений из файла
void paintwidget::tec_read(LPCTSTR filename)
{
    // Чистим старое
    is_loaded = false;
    for(size_t i = 0; i < tec_data.size(); i++)
        free(tec_data[i].value);
    tec_data.clear();
    for(size_t i = 0; i < triangles.size(); i++)
    {
        free(triangles[i].color);
        for(size_t k = 0; k < 3; k++)
            free(triangles[i].solution[k]);
    }
    triangles.clear();
    for(size_t k = 0; k < isolines.size(); k++)
        isolines[k].clear();
    variables.clear();

    // Пошли читать файл
    u_ifstream ifs(filename);
    string tmp;

    // TITLE = "Slice Z = -10"
    // TITLE="Electric Field"
    getline(ifs, tmp);

    // VARIABLES = "x", "y", "z", "ExR", "EyR", "EzR", "ExI", "EyI", "EzI", "abs(E)"
    // VARIABLES=X,Y,Z
    // VARIABLES = "X" "Y" "ReEx" "ImEx" "ReEy" "ImEy" "ReEz" "ImEz"
    getline(ifs, tmp);
    // Разберем переменные
    const char * curr_c = tmp.c_str(), * end_c = curr_c;
    const size_t VAR_MAX_LEN = 32;
    char var_c[3][VAR_MAX_LEN];
    // Ну мало ли, вдруг кто-нибудь пробелов понаставит в начале строки
    while(* curr_c == ' ' || * curr_c == '\t')
        curr_c++;
    // Строка должна начинаться со слова VARIABLES
    if(strncmp(curr_c, "VARIABLES", 9) == 0)
    {
        // Дальше всегда идет равно, а после него - перечисления переменных
        if((curr_c = strchr(curr_c + 9, '=')) != NULL)
        {
            bool eos_flag = false;
            size_t var_num = 0;
            char tmpstr[VAR_MAX_LEN];
            // Пока есть данные в строке пригодные для разбора, будем разбирать
            while(!eos_flag)
            {
                curr_c++;
                // Пробелы и табы нас не интересуют
                while(* curr_c == ' ' || * curr_c == '\t')
                    curr_c++;
                // Кавычки тоже
                if(* curr_c == '\"')
                    curr_c++;
                // Переменные разделяются пробелом, табом или запятой
                // Если разделителя нового не было, значит это последняя переменная
                if((end_c = strchr(curr_c, ',')) == NULL &&
                   (end_c = strchr(curr_c, ' ')) == NULL &&
                   (end_c = strchr(curr_c, '\t')) == NULL)
                {
                    eos_flag = true;
                    end_c = curr_c + strlen(curr_c);
                }
                const char * end_c_old = end_c;
                // Лишнего нам не надо, только само название переменной
                while(* (end_c - 1) == ' ' || * (end_c - 1) == '\t' || * (end_c - 1) == '\"')
                    end_c--;
                size_t len = end_c - curr_c;
                // В нормальном случае длина больше нуля, однако если разделитель - пробел
                // (или таб), а в конце строки есть еще пробел, то мы попадем сюда, когда
                // на самом деле переменных больше нет
                if(len > 0)
                {
#if defined _MSC_VER && _MSC_VER >= 1400
                    strncpy_s(tmpstr, VAR_MAX_LEN, curr_c, len);
#else
                    strncpy(tmpstr, curr_c, len);
#endif
                    tmpstr[len] = '\0';
                    // Первые два или три значения будут названиями геометрических переменных
                    // по которым строится график, а остальные - переменные со значениями
                    // Поэтому занесем все, что больше двух в вектор с переменными (потом удалим,
                    // если это нужно), а первые три - в массив геометрических переменных
                    // (потом просто не будем использовать ненужное)
                    if(var_num >= 2)
                        variables.push_back(tmpstr);
                    if(var_num < 3)
#if defined _MSC_VER && _MSC_VER >= 1400
                        strncpy_s(var_c[var_num++], VAR_MAX_LEN, tmpstr, VAR_MAX_LEN);
#else
                        strncpy(var_c[var_num++], tmpstr, VAR_MAX_LEN);
#endif
                    //printf("[VAR]\t%s\n", tmpstr);
                    //fflush(stdout);
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
    getline(ifs, tmp);
    size_t IJK[3];
    IJK[0] = IJK[1] = IJK[2] = numeric_limits<size_t>::max();
    bool point_accepted = false; // Должен быть указан параметр POINT
    curr_c = tmp.c_str();
    // Ну мало ли, вдруг кто-нибудь пробелов понаставит в начале строки
    while(* curr_c == ' ' || * curr_c == '\t')
        curr_c++;
    // Строка должна начинаться со слова ZONE
    if(strncmp(curr_c, "ZONE", 4) == 0)
    {
        curr_c += 4;
        // Пробелы и табы нас не интересуют
        while(* curr_c == ' ' || * curr_c == '\t')
            curr_c++;
        char param_name[VAR_MAX_LEN], param_value[255];
        bool flag_eol = false;
        // Пока есть данные в строке пригодные для разбора, будем разбирать
        while(!flag_eol)
        {
            // Пара параметр-значение всегда разделяется знаком равно
            if((end_c = strchr(curr_c, '=')) != NULL)
            {
                // Все, что до равно за вычетов табов-пробелов - название параметра
                const char * end_c_old = end_c;
                while(* (end_c - 1) == ' ' || * (end_c - 1) == '\t')
                    end_c--;
                size_t len = end_c - curr_c;
#if defined _MSC_VER && _MSC_VER >= 1400
                strncpy_s(param_name, VAR_MAX_LEN, curr_c, len);
#else
                strncpy(param_name, curr_c, len);
#endif
                param_name[len] = '\0';
                curr_c = end_c_old + 1;
                // А после равно - значение
                while(* curr_c == ' ' || * curr_c == '\t')
                    curr_c++;
                bool use_quote = (* curr_c == '\"') ? true : false;
                if(use_quote) curr_c++;

                // Если значение начинается с кавычки, то и искать следует только кавычку
                // Иначе могут быть пробел, таб или запятая
                // TODO: Тут может быть заэкранированная кавычка в параметре, пока это не работает
                if((use_quote && (end_c = strchr(curr_c, '\"')) != NULL) ||
                   (end_c = strchr(curr_c, ',')) != NULL ||
                   (!use_quote && (end_c = strchr(curr_c, ' ')) != NULL) ||
                   (!use_quote && (end_c = strchr(curr_c, '\t')) != NULL))
                {
                    // Если попало сюда, то все идет по плану
                    const char * end_c_old = end_c;
                    while(* (end_c - 1) == ' ' || * (end_c - 1) == '\t')
                        end_c--;
                    size_t len = end_c - curr_c;
#if defined _MSC_VER && _MSC_VER >= 1400
                    strncpy_s(param_value, sizeof(param_value) / sizeof(char), curr_c, len);
#else
                    strncpy(param_value, curr_c, len);
#endif
                    param_value[len] = '\0';
                    curr_c = end_c_old;
                    // Передвинемся на место после разделителя
                    while(* curr_c == ' ' || * curr_c == '\t' || * curr_c == ',' || * curr_c == '\"')
                        curr_c++;
                }
                else
                {
                    // Если попало сюда, то в значении параметра кавычек нет и пробелов тоже
                    // То есть все до конца строки есть искомое значение
                    flag_eol = true;
                    end_c = curr_c + strlen(curr_c);
                    size_t len = end_c - curr_c;
#if defined _MSC_VER && _MSC_VER >= 1400
                    strncpy_s(param_value, sizeof(param_value) / sizeof(char), curr_c, len);
#else
                    strncpy(param_value, curr_c, len);
#endif
                    param_value[len] = '\0';
                }
                //printf("[PARAM]\t%s = %s\n", param_name, param_value);
                //fflush(stdout);

                // Для простоты приведем параметр и значение к верхнему регистру
                // Интересующие нас строки заданы всегда латиницей, поэтому сделаем просто
                size_t sle = strlen(param_name);
                for(size_t i = 0; i < sle; i++)
                    if(param_name[i] >= 'a' && param_name[i] <= 'z')
                        param_name[i] += 'A' - 'a';
                sle = strlen(param_value);
                for(size_t i = 0; i < sle; i++)
                    if(param_value[i] >= 'a' && param_value[i] <= 'z')
                        param_value[i] += 'A' - 'a';

                //printf("[P-C]\t%s = %s\n", param_name, param_value);
                //fflush(stdout);

                // Теперь разберемся, что за параметр мы считали
                // Это "I"
                if(strcmp(param_name, "I") == 0)
                {
                    unsigned int v = 0;
#if defined _MSC_VER && _MSC_VER >= 1400
                    sscanf_s(param_value, "%u", & v);
#else
                    sscanf(param_value, "%u", & v);
#endif
                    IJK[0] = static_cast<size_t>(v);
                }
                // Это "J"
                else if(strcmp(param_name, "J") == 0)
                {
                    unsigned int v = 0;
#if defined _MSC_VER && _MSC_VER >= 1400
                    sscanf_s(param_value, "%u", & v);
#else
                    sscanf(param_value, "%u", & v);
#endif
                    IJK[1] = static_cast<size_t>(v);
                }
                // Это "K"
                else if(strcmp(param_name, "K") == 0)
                {
                    unsigned int v = 0;
#if defined _MSC_VER && _MSC_VER >= 1400
                    sscanf_s(param_value, "%u", & v);
#else
                    sscanf(param_value, "%u", & v);
#endif
                    IJK[2] = static_cast<size_t>(v);
                }
                // Это что-то с параметром "POINT"
                else if(strcmp(param_value, "POINT") == 0)
                {
                    point_accepted = true;
                }
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
            //ifs >> coords[j];
            coords[j] = read_number(ifs);
        tec_data[i].coord.x = coords[ind[0]];
        tec_data[i].coord.y = coords[ind[1]];
        tec_data[i].value = (float *)malloc(sizeof(float) * variables.size());
        for(size_t k = 0; k < variables.size(); k++)
            //ifs >> tec_data[i].value[k];
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

    if(!ifs.good())
    {
        print_io_error();
        return;
    }
    ifs.close();

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
        triangle_size += (sizeof(COLORREF) + sizeof(float) * 3) * variables.size();
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
        free(triangles[i].color);
        for(size_t k = 0; k < 3; k++)
            free(triangles[i].solution[k]);
    }
    triangles.clear();

    size_t tmp1_size = 4;
    for(size_t i = 0; i < num; i++)
        tmp1_size *= 4;
    triangle * tmp1;
    if(num % 2 == 0) tmp1 = (triangle *)malloc(sizeof(triangle) * tmp1_size);
    else             tmp1 = (triangle *)malloc(sizeof(triangle) * tmp1_size / 4);
    // Посчитаем в локальных координатах
    tmp1[0] = triangle( point(0.0f, 0.0f), point(1.0f, 0.0f), point(0.5f, 0.5f) );
    tmp1[1] = triangle( point(1.0f, 0.0f), point(1.0f, 1.0f), point(0.5f, 0.5f) );
    tmp1[2] = triangle( point(1.0f, 1.0f), point(0.0f, 1.0f), point(0.5f, 0.5f) );
    tmp1[3] = triangle( point(0.0f, 1.0f), point(0.0f, 0.0f), point(0.5f, 0.5f) );
    if(num > 0)
    {
        triangle * tmp2;
        if(num % 2 == 0) tmp2 = (triangle *)malloc(sizeof(triangle) * tmp1_size / 4);
        else             tmp2 = (triangle *)malloc(sizeof(triangle) * tmp1_size);
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
        free(tmp2);
    }

    // Заполняем вектор из треугольников переводя координаты в глобальные и считая цвет
    size_t curr_index = 0;
    triangles.resize(tmp1_size * (nx - 1) * (ny - 1));
    // Обходим все прямоугольники
    for(size_t i = 0; i < nx - 1; i++)
    {
        for(size_t j = 0; j < ny - 1; j++)
        {
            float x0 = tec_data[i * nx + j].coord.x;
            float y0 = tec_data[i * nx + j].coord.y;
            float x1 = tec_data[(i + 1) * nx + j + 1].coord.x;
            float y1 = tec_data[(i + 1) * nx + j + 1].coord.y;
            float hx = x1 - x0;
            float hy = y1 - y0;
            float step_x_i = std::fabs(tec_data[(i + 1) * nx + j].coord.x - x0);
            float step_x_j = std::fabs(tec_data[i * nx + j + 1].coord.x - x0);
            bool native_order = (step_x_i > step_x_j) ? true : false;

            for(size_t tn = 0; tn < tmp1_size; tn++)
            {
                triangle tmp_tr;
                tmp_tr.color = (COLORREF *)malloc(sizeof(COLORREF) * variables.size());
                // Переводим координаты в глобальные
                for(size_t k = 0; k < 3; k++)
                {
                    tmp_tr.nodes[k].x = tmp1[tn].nodes[k].x * hx + x0;
                    tmp_tr.nodes[k].y = tmp1[tn].nodes[k].y * hy + y0;
                    tmp_tr.solution[k] = (float *)malloc(sizeof(float) * variables.size());
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
                            r1 = (x1 - tmp_tr.nodes[k].x) / hx * tec_data[i * nx + j].value[m] +
                                    (tmp_tr.nodes[k].x - x0) / hx * tec_data[(i + 1) * nx + j].value[m];
                            r2 = (x1 - tmp_tr.nodes[k].x) / hx * tec_data[i * nx + j + 1].value[m] +
                                    (tmp_tr.nodes[k].x - x0) / hx * tec_data[(i + 1) * nx + j + 1].value[m];
                        }
                        else
                        {
                            r1 = (x1 - tmp_tr.nodes[k].x) / hx * tec_data[i * nx + j].value[m] +
                                    (tmp_tr.nodes[k].x - x0) / hx * tec_data[i * nx + j + 1].value[m];
                            r2 = (x1 - tmp_tr.nodes[k].x) / hx * tec_data[(i + 1) * nx + j].value[m] +
                                    (tmp_tr.nodes[k].x - x0) / hx * tec_data[(i + 1) * nx + j + 1].value[m];
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
                        r1 = (x1 - barycenter.x) / hx * tec_data[i * nx + j].value[m] +
                                (barycenter.x - x0) / hx * tec_data[(i + 1) * nx + j].value[m];
                        r2 = (x1 - barycenter.x) / hx * tec_data[i * nx + j + 1].value[m] +
                                (barycenter.x - x0) / hx * tec_data[(i + 1) * nx + j + 1].value[m];
                    }
                    else
                    {
                        r1 = (x1 - barycenter.x) / hx * tec_data[i * nx + j].value[m] +
                                (barycenter.x - x0) / hx * tec_data[i * nx + j + 1].value[m];
                        r2 = (x1 - barycenter.x) / hx * tec_data[(i + 1) * nx + j].value[m] +
                                (barycenter.x - x0) / hx * tec_data[(i + 1) * nx + j + 1].value[m];
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
                    tmp_tr.color[v] = RGB(r_color, g_color, b_color);
                }

                // И заносим в вектор
                triangles[curr_index++] = tmp_tr;
            }
        }
    }
    free(tmp1);
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
    hwnd = NULL;
    hbmp = NULL;
    hbmp_is_valid = false;
    memset(&ps, 0, sizeof(PAINTSTRUCT));

    use_purple = false;
    use_legend = true;
    use_light_colors = true;
    use_memory_limit = true;
}

// Деструктор
paintwidget::~paintwidget()
{
    if(hbmp) DeleteObject(hbmp);
    for(size_t i = 0; i < tec_data.size(); i++)
        free(tec_data[i].value);
    for(size_t i = 0; i < triangles.size(); i++)
    {
        free(triangles[i].color);
        for(size_t k = 0; k < 3; k++)
            free(triangles[i].solution[k]);
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
void paintwidget::to_window(float x, float y, int & xl, int & yl) const
{
    // В OpenGL это был бы glOrtho
    const float gl_x0 = -0.06f;
    const float gl_y0 = -0.06f;
    const float gl_x1 = use_legend ? 1.132f : 1.015f;
    const float gl_y1 = 1.02f;
    const float gl_hx = gl_x1 - gl_x0;
    const float gl_hy = gl_y1 - gl_y0;
    xl = (int)((x - gl_x0) / gl_hx * (float)width);
    yl = height - (int)((y - gl_y0) / gl_hy * (float)height);
}

// Отрисовка сцены
void paintwidget::paintEvent()
{
    RECT r;
    memset(&r, 0, sizeof(RECT));
    GetClientRect(hwnd, &r);
    HDC hdc1 = BeginPaint(hwnd, &ps);
    HDC hdc2 = CreateCompatibleDC(hdc1);
    BITMAP bmp;
    HGDIOBJ oldhbmp;
    if(!hbmp || !hbmp_is_valid)
    {
        if(hbmp) DeleteObject(hbmp);
        hbmp = CreateCompatibleBitmap(hdc1, r.right - r.left, r.bottom - r.top);
        oldhbmp = SelectObject(hdc2, hbmp);
        draw(hdc2);
    }
    else
    {
        oldhbmp = SelectObject(hdc2, hbmp);
    }
    GetObject(hbmp, sizeof(bmp), &bmp);
    BitBlt(hdc1, 0, 0, bmp.bmWidth, bmp.bmHeight, hdc2, 0, 0, SRCCOPY);
    SelectObject(hdc2, oldhbmp);
    DeleteDC(hdc2);
    EndPaint(hwnd, &ps);
    ReleaseDC(hwnd, hdc1);
    hbmp_is_valid = true;
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
const COLORREF purple_light_legend_colors[14] =
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
const COLORREF purple_normal_legend_colors[14] =
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
const COLORREF default_light_legend_colors[14] =
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
const COLORREF default_normal_legend_colors[14] =
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

// Печать текста с заданным интервалом между символами
void paintwidget::TextOutSpacingA(HDC hdc, int x, int y, const char * str, size_t len, int spacing)
{
    for(size_t i = 0; i < len; i++)
    {
        TextOutA(hdc, x, y, str + i, 1);
        x += spacing;
    }
}

// Отрисовка сцены на HDC
void paintwidget::draw(HDC hdc_local)
{
    // Геометрия окна
    RECT r;
    memset(&r, 0, sizeof(RECT));
    GetClientRect(hwnd, &r);
    height = r.bottom - r.top;
    width = r.right - r.left;

    POINT pt;
    int x, y;
    HPEN hOldPen;
    HBRUSH hOldBrush;
    HFONT hOldFont;
    POINT tr[3];

    // Заливка области белым цветом
    HPEN hAreaPen = (HPEN)GetStockObject(WHITE_PEN);
    hOldPen = (HPEN)SelectObject(hdc_local, hAreaPen);
    HBRUSH hAreaBrush = (HBRUSH)GetStockObject(WHITE_BRUSH);
    hOldBrush = (HBRUSH)SelectObject(hdc_local, hAreaBrush);
    Rectangle(hdc_local, 0, 0, width, height);
    SelectObject(hdc_local, hOldPen);
    SelectObject(hdc_local, hOldBrush);
    SetBkMode(hdc_local, TRANSPARENT);

    if(!is_loaded) return;

    // Шрифты
    float mono_stretch = 0.77f, serif_stretch = 0.9f;
    int mono_shift = 6, serif_shift = 4;
    int fnt_mono_h = (int)((float)height / 45.0f * mono_stretch) + mono_shift;
    int fnt_mono_w = (int)((float)width / 55.0f * mono_stretch) + mono_shift;
    HFONT fnt_mono = CreateFont(
                (fnt_mono_h < fnt_mono_w ? fnt_mono_h : fnt_mono_w), 0, 0, 0,
                FW_REGULAR, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                FIXED_PITCH | FF_MODERN, NULL);
    int fnt_serif_h = (int)((float)height / 40.0f * serif_stretch) + serif_shift;
    int fnt_serif_w = (int)((float)width / 45.0f * serif_stretch) + serif_shift;
    HFONT fnt_serif = CreateFont(
                (fnt_serif_h < fnt_serif_w ? fnt_serif_h : fnt_serif_w), 0, 0, 0,
                FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                VARIABLE_PITCH | FF_ROMAN, NULL);

    // Узнаем размеры шрифта
    hOldFont = (HFONT)SelectObject(hdc_local, fnt_mono);
    TEXTMETRIC tm;
    GetTextMetrics(hdc_local, &tm);
    SelectObject(hdc_local, hOldFont);
    float font_correct = (float)tm.tmHeight / (float)height * 0.8f;
    int mono_spacing = (int)((float)tm.tmAveCharWidth * 0.91f);

    // Координатная сетка
    HPEN hGridPen = CreatePen(PS_SOLID, 1, RGB(192, 192, 192));
    hOldPen = (HPEN)SelectObject(hdc_local, hGridPen);
    for(size_t i = 0; i <= num_ticks_x; i++)
    {
        float xd = (float)i / (float)num_ticks_x;
        to_window(xd, -0.01f, x, y);
        MoveToEx(hdc_local, x, y, &pt);
        to_window(xd, 1.0f, x, y);
        LineTo(hdc_local, x, y);
    }
    for(size_t i = 0; i <= num_ticks_y; i++)
    {
        float yd = (float)i / (float)num_ticks_y;
        to_window(-0.01f, yd, x, y);
        MoveToEx(hdc_local, x, y, &pt);
        to_window(1.0f, yd, x, y);
        LineTo(hdc_local, x, y);
    }
    SelectObject(hdc_local, hOldPen);
    DeleteObject(hGridPen);

    // Координатные оси
    HPEN hAxisPen = CreatePen(PS_SOLID, 2, RGB(0, 0, 0));
    hOldPen = (HPEN)SelectObject(hdc_local, hAxisPen);
    to_window(0.0f, -0.005f, x, y);
    MoveToEx(hdc_local, x, y, &pt);
    to_window(0.0f, 1.005f, x, y);
    LineTo(hdc_local, x, y);
    to_window(-0.005f, 0.0f, x, y);
    MoveToEx(hdc_local, x, y, &pt);
    to_window(1.005f, 0.0f, x, y);
    LineTo(hdc_local, x, y);
    SelectObject(hdc_local, hOldPen);
    DeleteObject(hAxisPen);

    // Подписи осей
    hOldFont = (HFONT)SelectObject(hdc_local, fnt_serif);
    to_window(0.99f, -0.04f + font_correct, x, y);
    TextOutA(hdc_local, x, y, label_x.c_str(), (int)label_x.length());
    to_window(-0.05f, 0.99f + font_correct, x, y);
    TextOutA(hdc_local, x, y, label_y.c_str(), (int)label_y.length());
    SelectObject(hdc_local, hOldFont);

    // Отрисовка шкалы
    hOldFont = (HFONT)SelectObject(hdc_local, fnt_mono);
    for(size_t i = 0; i < num_ticks_x; i++)
    {
        float xd = (float)i / (float)num_ticks_x;
        float x_real = (float)(std::floor((xd * size_x + min_x) * 10000.0f + 0.5f)) / 10000.0f;
        char st[17];
#if defined _MSC_VER && _MSC_VER >= 1400
        sprintf_s(st, 17, "%.6g", x_real);
#else
        sprintf(st, "%.6g", x_real);
#endif
        to_window(xd - 0.01f, -0.04f + font_correct, x, y);
        TextOutSpacingA(hdc_local, x, y, st, strlen(st), mono_spacing);
    }
    for(size_t i = 0; i < num_ticks_y; i++)
    {
        float yd = (float)i / (float)num_ticks_y;
        float y_real = (float)(std::floor((yd * size_y + min_y) * 10000.0f + 0.5f)) / 10000.0f;
        char st[17];
#if defined _MSC_VER && _MSC_VER >= 1400
        sprintf_s(st, 17, "%.6g", y_real);
#else
        sprintf(st, "%.6g", y_real);
#endif
        to_window(-0.05f, yd - 0.01f + font_correct, x, y);
        TextOutSpacingA(hdc_local, x, y, st, strlen(st), mono_spacing);
    }
    SelectObject(hdc_local, hOldFont);

    // Раскрашивать будем если запрошено сие
    if(draw_color)
    {
        // Отрисовка всех треугольников
        for(size_t i = 0; i < triangles.size(); i++)
        {
            HPEN hTrPen = GetStockPen(NULL_PEN);
            hOldPen = (HPEN)SelectObject(hdc_local, hTrPen);
            HBRUSH hTrBrush;

            // Задаем посчитанный цвет
            hTrBrush = CreateSolidBrush(triangles[i].color[draw_index]);
            hOldBrush = (HBRUSH)SelectObject(hdc_local, hTrBrush);

            // Рисуем
            for(size_t k = 0; k < 3; k++)
            {
                to_window((triangles[i].nodes[k].x - min_x) / size_x, (triangles[i].nodes[k].y - min_y) / size_y, x, y);
                tr[k].x = (LONG)x;
                tr[k].y = (LONG)y;
            }
            Polygon(hdc_local, tr, 3);

            SelectObject(hdc_local, hOldPen);
            SelectObject(hdc_local, hOldBrush);
            DeleteObject(hTrBrush);
        }
    }

    HPEN hIsolPen = GetStockPen(BLACK_PEN);
    hOldPen = (HPEN)SelectObject(hdc_local, hIsolPen);
    // Изолинии рисуем только если оно нам надо
    if(draw_isolines)
    {
        for(size_t i = 0; i < triangles.size(); i++)
        {
            // Теперь рисуем изолинии
            // Будем искать наименьшее значение, большее или равное решению
            // Если значения на разных концах ребра будут разными, значит изолиния проходит через это ребро
            set<float>::const_iterator segment_isol[3];
            for(size_t k = 0; k < 3; k++)
                segment_isol[k] = isolines[draw_index].lower_bound(triangles[i].solution[k][draw_index]);

            // Немного странной фигни
            vector<pair<float, float> > vct;
            vct.reserve(3);
            if(segment_isol[0] != segment_isol[1])
                vct.push_back(make_pair(((triangles[i].nodes[1].x + triangles[i].nodes[0].x) * 0.5f - min_x) / size_x, ((triangles[i].nodes[1].y + triangles[i].nodes[0].y) * 0.5f - min_y) / size_y));
            if(segment_isol[0] != segment_isol[2])
                vct.push_back(make_pair(((triangles[i].nodes[2].x + triangles[i].nodes[0].x) * 0.5f - min_x) / size_x, ((triangles[i].nodes[2].y + triangles[i].nodes[0].y) * 0.5f - min_y) / size_y));
            if(segment_isol[1] != segment_isol[2])
                vct.push_back(make_pair(((triangles[i].nodes[2].x + triangles[i].nodes[1].x) * 0.5f - min_x) / size_x, ((triangles[i].nodes[2].y + triangles[i].nodes[1].y) * 0.5f - min_y) / size_y));

            // А теперь нарисуем, согласно вышеприведенному условию
            if(vct.size() > 1)
            {
                // WinAPI почему-то не доводит линии на один пиксель
                // Исправим это досадное недоразумение
                float direction_x = vct[1].first - vct[0].first;
                float direction_y = vct[1].second - vct[0].second;
                int shift_x = 0, shift_y = 0;
                if(std::fabs(direction_x) > std::fabs(direction_y))
                    shift_x = direction_x > 0 ? 1 : -1;
                else
                    shift_y = direction_y > 0 ? -1 : 1;
                // Ну и нарисуем теперь с учетом поправки
                to_window(vct[0].first, vct[0].second, x, y);
                MoveToEx(hdc_local, x, y, &pt);
                to_window(vct[1].first, vct[1].second, x, y);
                LineTo(hdc_local, x + shift_x, y + shift_y);
            }
        }
    }
    SelectObject(hdc_local, hOldPen);

    // Легенда
    if(use_legend)
    {
        const COLORREF * legend_colors;
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
            static const float y0 = -0.003f;
            static const float dx = 0.0f;
            static const float dy = 0.0702f;
            static const float hx = 0.109f;
            static const float hy = 0.073f;
            HPEN hLegPen = (HPEN)GetStockObject(NULL_PEN);
            hOldPen = (HPEN)SelectObject(hdc_local, hLegPen);
            HBRUSH hLegBrush;
            hLegBrush = CreateSolidBrush(legend_colors[i]);
            hOldBrush = (HBRUSH)SelectObject(hdc_local, hLegBrush);
            int coords[4];
            to_window(x0, y0 + dy * i, coords[0], coords[3]);
            to_window(x0 + hx, y0 + dy * i + hy, coords[2], coords[1]);
            Rectangle(hdc_local, coords[0], coords[1], coords[2], coords[3]);
            SelectObject(hdc_local, hOldPen);
            SelectObject(hdc_local, hOldBrush);
            DeleteObject(hLegBrush);

            SetBkColor(hdc_local, legend_colors[i]);
            hOldFont = (HFONT)SelectObject(hdc_local, fnt_mono);
            char st[17];
            int exponent  = (int)std::floor(std::log10(std::fabs(legend_values[i])));
            if(abs(exponent) < 0) exponent = 0;
            float base   = legend_values[i] * std::pow(10.0f, -1.0f * exponent);
#if defined _MSC_VER && _MSC_VER >= 1400
            sprintf_s(st, 17, "%.2fE%+03d", base, exponent);
#else
            sprintf(st, "%.2fE%+03d", base, exponent);
#endif
            to_window(x0 + dx * i + 0.005f, y0 + dy * i + hy / 2.0f - 0.007f + font_correct, x, y);
            TextOutSpacingA(hdc_local, x, y, st, strlen(st), mono_spacing);
            SelectObject(hdc_local, hOldFont);
        }
        SetBkColor(hdc_local, RGB(255, 255, 255));
    }

    if(draw_vectors)
    {
        HPEN hVecPen = GetStockPen(BLACK_PEN);
        hOldPen = (HPEN)SelectObject(hdc_local, hVecPen);
        HBRUSH hVecBrush = GetStockBrush(BLACK_BRUSH);
        hOldBrush = (HBRUSH)SelectObject(hdc_local, hVecBrush);
        float vec_len = 10.0f;
        int arrow_len = 2;
        for(size_t j = 0; j < ny; j += skip_vec)
        {
            for(size_t i = 0; i < nx; i += skip_vec)
            {
                tecplot_node * v = & tec_data[j * nx + i];
                int begin_x, begin_y;
                to_window((v->coord.x - min_x) / size_x, (v->coord.y - min_y) / size_y, begin_x, begin_y);
                float len_x = v->value[ind_vec_1];
                float len_y = v->value[ind_vec_2];
                float norm = std::sqrt(len_x * len_x + len_y * len_y);
                len_x /= norm;
                len_y /= norm;
                if(len_x == len_x && len_y == len_y)
                {
                    int end_x = (int)(len_x * vec_len) + begin_x, end_y = -(int)(len_y * vec_len) + begin_y;
                    MoveToEx(hdc_local, begin_x, begin_y, &pt);
                    LineTo(hdc_local, end_x, end_y);
                    Ellipse(hdc_local, end_x - arrow_len, end_y - arrow_len, end_x + arrow_len, end_y + arrow_len);
                }
            }
        }
        SelectObject(hdc_local, hOldPen);
        SelectObject(hdc_local, hOldBrush);
    }

    DeleteObject(fnt_serif);
    DeleteObject(fnt_mono);
}
