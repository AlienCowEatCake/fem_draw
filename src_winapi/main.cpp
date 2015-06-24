#include "paintwidget.h"
#include "resources.h"
#include <commctrl.h>
// https://github.com/lvandeve/lodepng
#include "libs/lodepng.h"
// http://www.jonolick.com/code.html
#include "libs/jo_images.h"

HWND hwnd;
HACCEL haccel;
paintwidget * pdraw;
namespace menu
{
    HMENU hMenu;
    HMENU hFileMenu;
    HMENU hInterpMenu;
    HMENU hAboutMenu;
}
namespace config
{
    int min_height, min_width;
    int isol_min = 0,   isol_max = 100,   isol_curr = 10;
    int smooth_min = 0, smooth_max = 7,   smooth_curr = 0;
    int vect_min = 1,   vect_max = 10000, vect_curr = 1;
}

void widget_redraw()
{
    pdraw->hbmp_is_valid = false;
    RECT r;
    GetClientRect(hwnd, &r);
    InvalidateRect(hwnd, &r, FALSE);
}

// Открытие файла по имени
void open_file(LPTSTR filename)
{
    // Запомним старые значения индексов, чтоб потом восстановить
    size_t old_draw_index = pdraw->draw_index;
    pdraw->draw_index = 0;
    size_t old_ind_vec_1 = pdraw->ind_vec_1;
    pdraw->ind_vec_1 = 0;
    size_t old_ind_vec_2 = pdraw->ind_vec_2;
    pdraw->ind_vec_2 = 0;

    // Откроем файл
    pdraw->div_num = 0; // Сбросим значение интерполяции, чтобы не повисло на больших файлах
    pdraw->tec_read(filename);
    if(!pdraw->is_loaded)
    {
        SetWindowText(hwnd, TEXT("FEM Draw"));
        widget_redraw();
        return;
    }
    // Ненене, еще не все готово!
    pdraw->is_loaded = false;

    // Очистим поля в комбобоксах
    while(ComboBox_GetCount(GetDlgItem(hwnd, CONTROL_COMBOBOX_COLOR)) > 0)
        ComboBox_DeleteString(GetDlgItem(hwnd, CONTROL_COMBOBOX_COLOR), 0);
    while(ComboBox_GetCount(GetDlgItem(hwnd, CONTROL_COMBOBOX_VECTORS_U)) > 0)
        ComboBox_DeleteString(GetDlgItem(hwnd, CONTROL_COMBOBOX_VECTORS_U), 0);
    while(ComboBox_GetCount(GetDlgItem(hwnd, CONTROL_COMBOBOX_VECTORS_V)) > 0)
        ComboBox_DeleteString(GetDlgItem(hwnd, CONTROL_COMBOBOX_VECTORS_V), 0);

    // Заполним поля в комбобоксах
    for(size_t i = 0; i < pdraw->variables.size(); i++)
    {
        SendMessageA(GetDlgItem(hwnd, CONTROL_COMBOBOX_COLOR), CB_ADDSTRING, 0L, (LPARAM)pdraw->variables[i].c_str());
        SendMessageA(GetDlgItem(hwnd, CONTROL_COMBOBOX_VECTORS_U), CB_ADDSTRING, 0L, (LPARAM)pdraw->variables[i].c_str());
        SendMessageA(GetDlgItem(hwnd, CONTROL_COMBOBOX_VECTORS_V), CB_ADDSTRING, 0L, (LPARAM)pdraw->variables[i].c_str());
    }

    // Попытаемся восстановить старые индексы
    if(old_draw_index < pdraw->variables.size())
        ComboBox_SetCurSel(GetDlgItem(hwnd, CONTROL_COMBOBOX_COLOR), (int)old_draw_index);
    else
        ComboBox_SetCurSel(GetDlgItem(hwnd, CONTROL_COMBOBOX_COLOR), 0);

    if(old_ind_vec_1 < pdraw->variables.size())
        ComboBox_SetCurSel(GetDlgItem(hwnd, CONTROL_COMBOBOX_VECTORS_U), (int)old_ind_vec_1);
    else
        ComboBox_SetCurSel(GetDlgItem(hwnd, CONTROL_COMBOBOX_VECTORS_U), 0);

    if(old_ind_vec_2 < pdraw->variables.size())
        ComboBox_SetCurSel(GetDlgItem(hwnd, CONTROL_COMBOBOX_VECTORS_V), (int)old_ind_vec_2);
    else
        ComboBox_SetCurSel(GetDlgItem(hwnd, CONTROL_COMBOBOX_VECTORS_V), 0);

    // Устанавливаем оптимальное значение для векторов
    const size_t vect_bufsize = 16;
    char vect_buf[vect_bufsize];
#if defined _MSC_VER && _MSC_VER >= 1400
    if(pdraw->vect_value < config::vect_min)
        sprintf_s(vect_buf, vect_bufsize, "%d", config::vect_min);
    else if(pdraw->vect_value > config::vect_max)
        sprintf_s(vect_buf, vect_bufsize, "%d", config::vect_max);
    else
        sprintf_s(vect_buf, vect_bufsize, "%d", pdraw->vect_value);
#else
    if(pdraw->vect_value < config::vect_min)
        sprintf(vect_buf, "%d", config::vect_min);
    else if(pdraw->vect_value > config::vect_max)
        sprintf(vect_buf, "%d", config::vect_max);
    else
        sprintf(vect_buf, "%d", pdraw->vect_value);
#endif
    SetWindowTextA(GetDlgItem(hwnd, CONTROL_SPINBOX_VECTORS_TEXT), vect_buf);

    // Установим заголовок окна
    u_string label(filename);
    size_t begin = label.find_last_of(TEXT("\\"));
    if(begin != u_string::npos)
        label = label.substr(begin + 1);
    label.append(TEXT(" - FEM Draw"));
    SetWindowText(hwnd, label.c_str());

    // А вот теперь готово
    pdraw->is_loaded = true;
    widget_redraw();
}

// Событие при открытии файла
void on_actionOpen_Tecplot_File_triggered()
{
    OPENFILENAME ofn;
    TCHAR szFile[260] = TEXT("");
    ZeroMemory(& ofn, sizeof(OPENFILENAME));
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = TEXT("Tecplot Data Files (*.dat *.plt)\0*.dat;*.plt\0All Files (*.*)\0*.*\0");
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = TEXT("Open Tecplot File");
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_READONLY | OFN_HIDEREADONLY;
    if(GetOpenFileName(& ofn) != TRUE) return;
    open_file(ofn.lpstrFile);
}

// Событие при переключении прозрачности
void on_actionTransparent_Image_triggered()
{
    DWORD state = GetMenuState(menu::hFileMenu, (UINT)CONTROL_MENU_TRANSPARENT, MF_BYCOMMAND);
    if(!(state & MF_CHECKED))
        CheckMenuItem(menu::hFileMenu, (UINT)CONTROL_MENU_TRANSPARENT, MF_CHECKED | MF_BYCOMMAND);
    else
        CheckMenuItem(menu::hFileMenu, (UINT)CONTROL_MENU_TRANSPARENT, MF_UNCHECKED | MF_BYCOMMAND);
}

// Конвертер из битмапов bmp в другие форматы
void bmp2rgb(const char * lpbitmap, LONG width, LONG height, unsigned colors, bool transparent, unsigned char * image_rgb)
{
    assert(colors == 3 || colors == 4);
    for(unsigned y = 0; y < (unsigned)height; y++)
    {
        for(unsigned x = 0; x < (unsigned)width; x++)
        {
            unsigned bmpos = 4 * ((height - y - 1) * width + x);
            unsigned newpos = colors * (y * width + x);
            image_rgb[newpos + 0] = *(((unsigned char *)lpbitmap) + bmpos + 2);
            image_rgb[newpos + 1] = *(((unsigned char *)lpbitmap) + bmpos + 1);
            image_rgb[newpos + 2] = *(((unsigned char *)lpbitmap) + bmpos + 0);
            if(transparent)
            {
                // У винды тупой битмап, в альфа канале ничего не хранит
                // Так что замутим псевдо-прозрачность: все что белое, будем считать прозрачным
                if(colors == 4)
                {
                    if(image_rgb[newpos + 0] == 255 && image_rgb[newpos + 1] == 255 && image_rgb[newpos + 2] == 255)
                        image_rgb[newpos + 3] = 0;
                    else
                        image_rgb[newpos + 3] = 255;
                }
            }
            else
            {
                if(colors == 4) image_rgb[newpos + 3] = 255;
            }
        }
    }
}

// Событие при сохранении
void on_actionSave_Image_File_triggered()
{
    // Разберемся с прозрачностью
    bool transparent = GetMenuState(menu::hFileMenu, (UINT)CONTROL_MENU_TRANSPARENT, MF_BYCOMMAND) & MF_CHECKED ? true : false;

    // Откроем файл
    OPENFILENAME ofn;
    TCHAR szFile[260] = TEXT("draw.png");
    ZeroMemory(& ofn, sizeof(OPENFILENAME));
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    ofn.lpstrDefExt = TEXT("png");
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = TEXT("All Images (*.png *.bmp *.jpg *.gif *.tga)\0*.png;*.bmp;*.jpg;*.gif;*.tga\0PNG Images (*.png)\0*.png\0BMP Images (*.bmp)\0*.bmp\0JPG Images (*.jpg)\0*.jpg\0GIF Images (*.gif)\0*.gif\0TGA Images (*.tga)\0*.tga\0");
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = TEXT("Save Image File");
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
    if(GetSaveFileName(& ofn) != TRUE) return;

    // Определим, что за тип изображения нам надо сохранить
    enum filetypes { TYPE_PNG, TYPE_BMP, TYPE_JPG, TYPE_GIF, TYPE_TGA };
    filetypes filetype = TYPE_PNG;
    u_string fileName(ofn.lpstrFile);
    size_t found = fileName.find_last_of(TEXT("."));
    if(found == u_string::npos) fileName.append(TEXT(".png"));
    else
    {
        u_string ext = fileName.substr(found + 1);
        for(u_string::iterator it = ext.begin(); it != ext.end(); it++)
            if(*it >= 'A' && *it <= 'Z') *it -= 'A' - 'a';
        if(ext == TEXT("bmp")) filetype = TYPE_BMP;
        else if(ext == TEXT("jpg")) filetype = TYPE_JPG;
        else if(ext == TEXT("gif")) filetype = TYPE_GIF;
        else if(ext == TEXT("tga")) filetype = TYPE_TGA;
        else if(ext != TEXT("png")) fileName.append(TEXT(".png"));
    }

    // Создадим все что нужно и запустим отрисовку
    RECT r;
    GetClientRect(pdraw->hwnd, &r);
    HDC hdc1 = BeginPaint(pdraw->hwnd, & pdraw->ps);
    HDC hdc2 = CreateCompatibleDC(hdc1);
    HBITMAP hbmp = pdraw->hbmp;

    // Создадим bmp-изображение
    // https://msdn.microsoft.com/en-us/library/windows/desktop/dd183402(v=vs.85).aspx
    BITMAP bmp;
    GetObject(hbmp, sizeof(BITMAP), &bmp);
    BITMAPFILEHEADER bmfHeader;
    BITMAPINFOHEADER bi;
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = bmp.bmWidth;
    bi.biHeight = bmp.bmHeight;
    bi.biPlanes = 1;
    bi.biBitCount = 32;
    bi.biCompression = BI_RGB;
    bi.biSizeImage = 0;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed = 0;
    bi.biClrImportant = 0;
    DWORD dwBmpSize = ((bmp.bmWidth * bi.biBitCount + 31) / 32) * 4 * bmp.bmHeight;
    HANDLE hDIB = GlobalAlloc(GHND, dwBmpSize);
    char *lpbitmap = (char *)GlobalLock(hDIB);
    GetDIBits(hdc2, hbmp, 0, (UINT)bmp.bmHeight, lpbitmap, (BITMAPINFO *)&bi, DIB_RGB_COLORS);

    // Сохраним отрисованное в bmp файл
    if(filetype == TYPE_BMP)
    {
        HANDLE hFile = CreateFile(fileName.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if(hFile == INVALID_HANDLE_VALUE)
        {
            MessageBox(hwnd, TEXT("Error: Can't save file"), TEXT("Error"), MB_OK | MB_ICONERROR);
            goto finish;
        }
        DWORD dwSizeofDIB = dwBmpSize + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
        bmfHeader.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER);
        bmfHeader.bfSize = dwSizeofDIB;
        bmfHeader.bfType = 0x4D42;
        DWORD dwBytesWritten = 0;
        WriteFile(hFile, (LPSTR)&bmfHeader, sizeof(BITMAPFILEHEADER), &dwBytesWritten, NULL);
        WriteFile(hFile, (LPSTR)&bi, sizeof(BITMAPINFOHEADER), &dwBytesWritten, NULL);
        WriteFile(hFile, (LPSTR)lpbitmap, dwBmpSize, &dwBytesWritten, NULL);
        CloseHandle(hFile);
    }
    // Сохраним отрисованное в png файл
    else if(filetype == TYPE_PNG)
    {
        HANDLE hFile = CreateFile(fileName.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if(hFile == INVALID_HANDLE_VALUE)
        {
            MessageBox(hwnd, TEXT("Error: Can't save file"), TEXT("Error"), MB_OK | MB_ICONERROR);
            goto finish;
        }
        int num_channels = transparent ? 4 : 3;
        unsigned char * image_raw = (unsigned char *)malloc(bi.biWidth * bi.biHeight * num_channels);
        bmp2rgb(lpbitmap, bi.biWidth, bi.biHeight, num_channels, transparent, image_raw);
        unsigned char * image_png = NULL;
        size_t image_png_size;
        unsigned error = 1;
        if(transparent)
            error = lodepng_encode32(&image_png, &image_png_size, image_raw, bi.biWidth, bi.biHeight);
        else
            error = lodepng_encode24(&image_png, &image_png_size, image_raw, bi.biWidth, bi.biHeight);
        if(error)
        {
            free(image_raw);
            if(image_png) free(image_png);
            CloseHandle(hFile);
            MessageBox(hwnd, TEXT("Error: Can't save file"), TEXT("Error"), MB_OK | MB_ICONERROR);
            goto finish;
        }
        DWORD dwBytesWritten = 0;
        WriteFile(hFile, (LPSTR)image_png, (DWORD)image_png_size, &dwBytesWritten, NULL);
        free(image_raw);
        free(image_png);
        CloseHandle(hFile);
    }
    // Сохраним отрисованное в tga файл
    else if(filetype == TYPE_TGA)
    {
        int num_channels = transparent ? 4 : 3;
        unsigned char * image_raw = (unsigned char *)malloc(bi.biWidth * bi.biHeight * num_channels);
        bmp2rgb(lpbitmap, bi.biWidth, bi.biHeight, num_channels, transparent, image_raw);
        bool status = jo_write_tga(fileName.c_str(), image_raw, bi.biWidth, bi.biHeight, num_channels);
        free(image_raw);
        if(!status)
        {
            MessageBox(hwnd, TEXT("Error: Can't save file"), TEXT("Error"), MB_OK | MB_ICONERROR);
            goto finish;
        }
    }
    // Сохраним отрисованное в jpg файл
    else if(filetype == TYPE_JPG)
    {
        unsigned char * image_raw = (unsigned char *)malloc(bi.biWidth * bi.biHeight * 3);
        bmp2rgb(lpbitmap, bi.biWidth, bi.biHeight, 3, false, image_raw);
        bool status = jo_write_jpg(fileName.c_str(), image_raw, bi.biWidth, bi.biHeight, 3, 0);
        free(image_raw);
        if(!status)
        {
            MessageBox(hwnd, TEXT("Error: Can't save file"), TEXT("Error"), MB_OK | MB_ICONERROR);
            goto finish;
        }
    }
    // Сохраним отрисованное в gif файл
    else if(filetype == TYPE_GIF)
    {
        unsigned char * image_raw = (unsigned char *)malloc(bi.biWidth * bi.biHeight * 4);
        // Товарищ Jon Olick в своей рисовалке не запилил прозрачность
        // Пруф: http://www.jonolick.com/home/gif-writer
        // При этом ему не помешало затребовать памяти на альфа-канал
        // Ну что же, может быть когда-нибудь запилю как положено, а пока так
        bmp2rgb(lpbitmap, bi.biWidth, bi.biHeight, 4, false, image_raw);
        jo_gif_t gif = jo_gif_start(fileName.c_str(), (short)bi.biWidth, (short)bi.biHeight, 0, 255);
        if(!gif.fp)
        {
            free(image_raw);
            MessageBox(hwnd, TEXT("Error: Can't save file"), TEXT("Error"), MB_OK | MB_ICONERROR);
            goto finish;
        }
        jo_gif_frame(&gif, image_raw, 0, false);
        jo_gif_end(&gif);
        free(image_raw);
    }

    // Удаляем всякий мусор
finish:
    GlobalUnlock(hDIB);
    GlobalFree(hDIB);
    pdraw->paintEvent();
    EndPaint(pdraw->hwnd, & pdraw->ps);
    DeleteObject(hdc2);
}

// Событие при нажатии кнопки Exit
void on_actionExit_triggered()
{
    exit(0);
}

// Событие при увеличении уровня интерполяции
void on_actionIncrease_Interpolation_triggered()
{
    if(pdraw->div_num < 7)
    {
        pdraw->set_div_num(pdraw->div_num + 1);
        widget_redraw();
    }
}

// Событие при уменьшении уровня интерполяции
void on_actionDecrease_Interpolation_triggered()
{
    if(pdraw->div_num > 0)
    {
        pdraw->set_div_num(pdraw->div_num - 1);
        widget_redraw();
    }
}

// Событие при нажатии кнопки About
void on_actionAbout_FEM_Draw_triggered()
{
    u_stringstream sstr;
    sstr << TEXT("FEM Draw v1.0 beta4 (WinAPI)\n\n")
         << TEXT("http://fami-net.dlinkddns.com/gitlab/peter/fem_draw\n")
         << TEXT("License: GNU GPL v3\n\n")
         << TEXT("Copyright (c) 2014 - 2015\n")
         << TEXT("Peter Zhigalov <peter.zhigalov@gmail.com>");
    MessageBox(hwnd, sstr.str().c_str(), TEXT("About"), MB_OK | MB_ICONINFORMATION);
}

// Событие при переключении закраски цветом
void on_checkBox_Color_clicked()
{
    LRESULT res = SendMessage(GetDlgItem(hwnd, CONTROL_CHECKBOX_COLOR), BM_GETCHECK, 0, 0);
    bool draw_color = pdraw->draw_color;
    if(res == BST_CHECKED)
        draw_color = true;
    if(res == BST_UNCHECKED)
        draw_color = false;
    if(draw_color != pdraw->draw_color)
    {
        pdraw->draw_color = draw_color;
        widget_redraw();
    }
}

// Изменение переменной, которую выводим
void on_comboBox_Color_currentIndexChanged()
{
    int index = ComboBox_GetCurSel(GetDlgItem(hwnd, CONTROL_COMBOBOX_COLOR));
    if((size_t)index != pdraw->draw_index)
    {
        pdraw->draw_index = (size_t)index;
        if(pdraw->draw_color || pdraw->draw_isolines)
            widget_redraw();
    }
}

// Событие при переключении рисования изолиний
void on_checkBox_Isolines_clicked()
{
    LRESULT res = SendMessage(GetDlgItem(hwnd, CONTROL_CHECKBOX_ISOLINES), BM_GETCHECK, 0, 0);
    bool draw_isolines = pdraw->draw_isolines;
    if(res == BST_CHECKED)
        draw_isolines = true;
    if(res == BST_UNCHECKED)
        draw_isolines = false;
    if(draw_isolines != pdraw->draw_isolines)
    {
        pdraw->draw_isolines = draw_isolines;
        widget_redraw();
    }
}

// Событие при изменении числа изолиний
void on_spinBox_Isolines_valueChanged()
{
    const size_t bufsize = 16;
    char str[bufsize];
    GetWindowTextA(GetDlgItem(hwnd, CONTROL_SPINBOX_ISOLINES_TEXT), str, bufsize);
    int val = atoi(str);
    if(val != config::isol_curr)
    {
        if(val <= config::isol_max && val >= config::isol_min)
        {
            config::isol_curr = val;
            pdraw->set_isolines_num((size_t)config::isol_curr);
            if(pdraw->draw_isolines)
                widget_redraw();
        }
        else
        {
#if defined _MSC_VER && _MSC_VER >= 1400
            sprintf_s(str, bufsize, "%d", config::isol_curr);
#else
            sprintf(str, "%d", config::isol_curr);
#endif
            SetWindowTextA(GetDlgItem(hwnd, CONTROL_SPINBOX_ISOLINES_TEXT), str);
        }
    }
}

// Переключение рисовки векторов
void on_checkBox_Vectors_clicked()
{
    LRESULT res = SendMessage(GetDlgItem(hwnd, CONTROL_CHECKBOX_VECTORS), BM_GETCHECK, 0, 0);
    bool draw_vectors = pdraw->draw_vectors;
    if(res == BST_CHECKED)
        draw_vectors = true;
    if(res == BST_UNCHECKED)
        draw_vectors = false;
    if(draw_vectors != pdraw->draw_vectors)
    {
        pdraw->draw_vectors = draw_vectors;
        widget_redraw();
    }
}

// Число рисуемых векторов
void on_spinBox_Vectors_valueChanged()
{
    const size_t bufsize = 16;
    char str[bufsize];
    GetWindowTextA(GetDlgItem(hwnd, CONTROL_SPINBOX_VECTORS_TEXT), str, bufsize);
    int val = atoi(str);
    if(val != config::vect_curr)
    {
        if(val <= config::vect_max && val >= config::vect_min)
        {
            config::vect_curr = val;
            pdraw->skip_vec = (size_t)config::vect_curr;
            if(pdraw->draw_vectors)
                widget_redraw();
        }
        else
        {
#if defined _MSC_VER && _MSC_VER >= 1400
            sprintf_s(str, bufsize, "%d", config::vect_curr);
#else
            sprintf(str, "%d", config::vect_curr);
#endif
            SetWindowTextA(GetDlgItem(hwnd, CONTROL_SPINBOX_VECTORS_TEXT), str);
        }
    }
}

// Первая переменная вектора
void on_comboBox_Vectors_U_currentIndexChanged()
{
    int index = ComboBox_GetCurSel(GetDlgItem(hwnd, CONTROL_COMBOBOX_VECTORS_U));
    if((size_t)index != pdraw->ind_vec_1)
    {
        pdraw->ind_vec_1 = (size_t)index;
        if(pdraw->draw_vectors)
            widget_redraw();
    }
}

// Вторая переменная вектора
void on_comboBox_Vectors_V_currentIndexChanged()
{
    int index = ComboBox_GetCurSel(GetDlgItem(hwnd, CONTROL_COMBOBOX_VECTORS_V));
    if((size_t)index != pdraw->ind_vec_2)
    {
        pdraw->ind_vec_2 = (size_t)index;
        if(pdraw->draw_vectors)
            widget_redraw();
    }
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    switch(Msg)
    {
    case WM_COMMAND:    // Нажата кнопка
    {
        switch(LOWORD(wParam))
        {
        case CONTROL_MENU_OPEN: // Событие при открытии файла
        {
            on_actionOpen_Tecplot_File_triggered();
            break;
        }
        case CONTROL_MENU_TRANSPARENT: // Событие при переключении прозрачности
        {
            on_actionTransparent_Image_triggered();
            break;
        }
        case CONTROL_MENU_SAVE: // Событие при сохранении
        {
            on_actionSave_Image_File_triggered();
            break;
        }
        case CONTROL_MENU_EXIT: // Событие при нажатии кнопки Exit
        {
            on_actionExit_triggered();
            break;
        }
        case CONTROL_MENU_INCREASE_INTERPOLATION: // Событие при увеличении уровня интерполяции
        {
            on_actionIncrease_Interpolation_triggered();
            break;
        }
        case CONTROL_MENU_DECREASE_INTERPOLATION: // Событие при уменьшении уровня интерполяции
        {
            on_actionDecrease_Interpolation_triggered();
            break;
        }
        case CONTROL_MENU_ABOUT: // Событие при нажатии кнопки About
        {
            on_actionAbout_FEM_Draw_triggered();
            break;
        }
        case CONTROL_CHECKBOX_COLOR: // Событие при переключении закраски цветом
        {
            on_checkBox_Color_clicked();
            break;
        }
        case CONTROL_COMBOBOX_COLOR: // Изменение переменной, которую выводим
        {
            on_comboBox_Color_currentIndexChanged();
            break;
        }
        case CONTROL_CHECKBOX_ISOLINES: // Событие при переключении рисования изолиний
        {
            on_checkBox_Isolines_clicked();
            break;
        }
        case CONTROL_SPINBOX_ISOLINES_TEXT: // Событие при изменении числа изолиний
        {
            on_spinBox_Isolines_valueChanged();
            break;
        }
        case CONTROL_CHECKBOX_VECTORS: // Переключение рисовки векторов
        {
            on_checkBox_Vectors_clicked();
            break;
        }
        case CONTROL_SPINBOX_VECTORS_TEXT: // Число рисуемых векторов
        {
            on_spinBox_Vectors_valueChanged();
            break;
        }
        case CONTROL_COMBOBOX_VECTORS_U: // Первая переменная вектора
        {
            on_comboBox_Vectors_U_currentIndexChanged();
            break;
        }
        case CONTROL_COMBOBOX_VECTORS_V: // Вторая переменная вектора
        {
            on_comboBox_Vectors_V_currentIndexChanged();
            break;
        }
        }
        break;
    }
    case WM_SIZE:       // Изменение размера окна
    {
        RECT r;
        GetClientRect(hwnd, &r);
        SetWindowPos(
                    GetDlgItem(hwnd, CONTROL_PAINT_WIDGET), NULL,
                    0, 0, r.right - r.left, r.bottom - r.top - 25,
                    SWP_NOMOVE | SWP_NOOWNERZORDER
                    );
        widget_redraw();
        break;
    }
    case WM_PAINT:
    case WM_PRINT:
    case WM_PRINTCLIENT:
        DefWindowProc(hWnd, Msg, wParam, lParam);
        pdraw->paintEvent();
        break;
    case WM_GETMINMAXINFO:  // Задаем минимальные размеры окна
        ((LPMINMAXINFO)lParam)->ptMinTrackSize.x = (LONG)config::min_width;
        ((LPMINMAXINFO)lParam)->ptMinTrackSize.y = (LONG)config::min_height;
        break;
    case WM_DESTROY:    // Закрытие окна
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, Msg, wParam, lParam);
    }
    return 0;
}

// Функция, устанавливающая подсказки
void set_tooltip(HINSTANCE hInstance, HWND hwnd, int item, LPTSTR text)
{
    // Кому тут надо подсказку?
    HWND hButton = GetDlgItem(hwnd, item);
    // Создать подсказку
    HWND hTooltip = CreateWindow(
                TOOLTIPS_CLASS, NULL, WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,
                CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                hButton, (HMENU)NULL, hInstance, NULL
                );
    // Прикрепить подсказку
    TOOLINFO ti;
    memset(& ti, 0, sizeof(TOOLINFO));
    ti.cbSize = sizeof(TOOLINFO);
    ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
    ti.uId = (UINT)hButton;
    ti.lpszText = text;
    ti.hinst = hInstance;
    SendMessage(hTooltip, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO) & ti);
}

#if defined UNICODE || defined _UNICODE
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR lpCmdLine, int nCmdShow)
#else
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR lpCmdLine, int nCmdShow)
#endif
{
    // Рисовалка
    paintwidget draw;
    pdraw = & draw;

    // Разное
    bool use_arg = u_strlen(lpCmdLine) > 2 ? true : false;
    WNDCLASS wnd;
    memset(&wnd, 0, sizeof(WNDCLASS));
    wnd.style = CS_HREDRAW | CS_VREDRAW;
    wnd.lpfnWndProc = WndProc;
    wnd.hInstance = hInstance;
    wnd.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wnd.lpszClassName = TEXT("mainwindow");
    wnd.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
    RegisterClass(&wnd);

    // Установка минимальных размеров окна
    config::min_height = 500 + 2 * GetSystemMetrics(SM_CYFRAME) + GetSystemMetrics(SM_CYCAPTION);
    config::min_width = 640 + 2 * GetSystemMetrics(SM_CXFRAME);

    // Начинаем создавать главное окно
    int window_width = 640 + 2 * GetSystemMetrics(SM_CXFRAME);
    int window_height = 570 + 2 * GetSystemMetrics(SM_CYFRAME) + GetSystemMetrics(SM_CYCAPTION);
    HDC hDCScreen = GetDC(NULL);
    hwnd = CreateWindow(
                TEXT("mainwindow"), TEXT("FEM Draw"),
                WS_OVERLAPPEDWINDOW,
                (GetDeviceCaps(hDCScreen, HORZRES) - window_width) / 2,
                (GetDeviceCaps(hDCScreen, VERTRES) - window_height) / 2,
                window_width, window_height,
                NULL, NULL, hInstance, NULL
                );

    // Замутим меню
    menu::hMenu = CreateMenu();
    menu::hFileMenu = CreatePopupMenu();
    AppendMenu(menu::hMenu, MF_STRING | MF_POPUP, (UINT)menu::hFileMenu, TEXT("File"));
    AppendMenu(menu::hFileMenu, MF_STRING, CONTROL_MENU_OPEN, TEXT("Open Tecplot File\tCtrl+O"));
    AppendMenu(menu::hFileMenu, MF_SEPARATOR, (UINT)NULL, TEXT(""));
    AppendMenu(menu::hFileMenu, MF_STRING | MF_UNCHECKED, CONTROL_MENU_TRANSPARENT, TEXT("Transparent Image"));
    AppendMenu(menu::hFileMenu, MF_STRING, CONTROL_MENU_SAVE, TEXT("Save Image\tCtrl+S"));
    AppendMenu(menu::hFileMenu, MF_SEPARATOR, (UINT)NULL, TEXT(""));
    AppendMenu(menu::hFileMenu, MF_STRING, CONTROL_MENU_EXIT, TEXT("Exit\tCtrl+Q"));
    menu::hInterpMenu = CreatePopupMenu();
    AppendMenu(menu::hMenu, MF_STRING | MF_POPUP, (UINT)menu::hInterpMenu, TEXT("Interpolation"));
    AppendMenu(menu::hInterpMenu, MF_STRING, CONTROL_MENU_INCREASE_INTERPOLATION, TEXT("Increase Interpolation\t="));
    AppendMenu(menu::hInterpMenu, MF_STRING, CONTROL_MENU_DECREASE_INTERPOLATION, TEXT("Decrease Interpolation\t-"));
    menu::hAboutMenu = CreatePopupMenu();
    AppendMenu(menu::hMenu, MF_STRING | MF_POPUP, (UINT)menu::hAboutMenu, TEXT("About"));
    AppendMenu(menu::hAboutMenu, MF_STRING, CONTROL_MENU_ABOUT, TEXT("About FEM Draw"));
    SetMenu(hwnd, menu::hMenu);
    haccel = LoadAccelerators(hInstance, TEXT("APP_ACCELERATORS"));

    // Замутим чекбокс "Color"
    CreateWindow(
                WC_BUTTON, TEXT("Color"),
                WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | WS_TABSTOP,
                10, 0, 64, 25,
                hwnd, (HMENU)CONTROL_CHECKBOX_COLOR, hInstance, NULL
                );
    set_tooltip(hInstance, hwnd, CONTROL_CHECKBOX_COLOR, TEXT("Draw color image"));

    // Комбобокс о выборе переменной по которой цвет
    CreateWindow(
                WC_COMBOBOX, NULL,
                WS_CHILD | WS_VISIBLE | CBS_HASSTRINGS | CBS_DROPDOWNLIST | WS_TABSTOP,
                75, 0, 70, 150,
                hwnd, (HMENU)CONTROL_COMBOBOX_COLOR, hInstance, NULL
                );
    set_tooltip(hInstance, hwnd, CONTROL_COMBOBOX_COLOR, TEXT("Data component for colors and isolines"));

    // Замутим чекбокс "Isolines"
    CreateWindow(
                WC_BUTTON, TEXT("Isolines"),
                WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | WS_TABSTOP,
                160, 0, 79, 25,
                hwnd, (HMENU)CONTROL_CHECKBOX_ISOLINES, hInstance, NULL
                );
    set_tooltip(hInstance, hwnd, CONTROL_CHECKBOX_ISOLINES, TEXT("Draw isolines"));

    // Замутим спинбокс о числе изолиний
    CreateWindowEx(
                WS_EX_CLIENTEDGE,
                WC_EDIT, NULL,
                WS_CHILD | WS_VISIBLE | ES_LEFT | WS_BORDER | ES_AUTOHSCROLL |
                ES_NUMBER | WS_TABSTOP,
                240, 0, 55, 25,
                hwnd, (HMENU)CONTROL_SPINBOX_ISOLINES_TEXT, hInstance, NULL
                );
    CreateUpDownControl(
                WS_CHILD | WS_BORDER | WS_VISIBLE | UDS_ARROWKEYS |
                UDS_ALIGNRIGHT | UDS_SETBUDDYINT,
                0, 0, 0, 0,
                hwnd, CONTROL_SPINBOX_ISOLINES_UPDOWN, hInstance, GetDlgItem(hwnd, CONTROL_SPINBOX_ISOLINES_TEXT),
                config::isol_max, config::isol_min, config::isol_curr
                );
    set_tooltip(hInstance, hwnd, CONTROL_SPINBOX_ISOLINES_TEXT, TEXT("Number of isolines"));
    set_tooltip(hInstance, hwnd, CONTROL_SPINBOX_ISOLINES_UPDOWN, TEXT("Number of isolines"));

    // Замутим чекбокс "Vectors"
    CreateWindow(
                WC_BUTTON, TEXT("Vectors"),
                WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | WS_TABSTOP,
                310, 0, 79, 25,
                hwnd, (HMENU)CONTROL_CHECKBOX_VECTORS, hInstance, NULL
                );
    set_tooltip(hInstance, hwnd, CONTROL_CHECKBOX_VECTORS, TEXT("Draw vectors"));

    // Замутим спинбокс о числе пропускаемых векторов
    CreateWindowEx(
                WS_EX_CLIENTEDGE,
                WC_EDIT, NULL,
                WS_CHILD | WS_VISIBLE | ES_LEFT | WS_BORDER | ES_AUTOHSCROLL |
                ES_NUMBER | WS_TABSTOP,
                390, 0, 55, 25,
                hwnd, (HMENU)CONTROL_SPINBOX_VECTORS_TEXT, hInstance, NULL
                );
    CreateUpDownControl(
                WS_CHILD | WS_BORDER | WS_VISIBLE | UDS_ARROWKEYS |
                UDS_ALIGNRIGHT | UDS_SETBUDDYINT,
                0, 0, 0, 0,
                hwnd, CONTROL_SPINBOX_VECTORS_UPDOWN, hInstance, GetDlgItem(hwnd, CONTROL_SPINBOX_VECTORS_TEXT),
                config::vect_max, config::vect_min, config::vect_curr
                );
    set_tooltip(hInstance, hwnd, CONTROL_SPINBOX_VECTORS_TEXT, TEXT("Number of skipped values"));
    set_tooltip(hInstance, hwnd, CONTROL_SPINBOX_VECTORS_UPDOWN, TEXT("Number of skipped values"));

    // Надпись "U:"
    CreateWindow(
                WC_STATIC, TEXT("U:"),
                WS_CHILD | WS_VISIBLE | SS_CENTERIMAGE,
                455, 0, 14, 25,
                hwnd, (HMENU)CONTROL_LABEL_VECTORS_U, hInstance, NULL
                );

    // Комбобокс о выборе первой переменной по которой строятся вектора
    CreateWindow(
                WC_COMBOBOX, NULL,
                WS_CHILD | WS_VISIBLE | CBS_HASSTRINGS | CBS_DROPDOWNLIST | WS_TABSTOP,
                470, 0, 70, 150,
                hwnd, (HMENU)CONTROL_COMBOBOX_VECTORS_U, hInstance, NULL
                );
    set_tooltip(hInstance, hwnd, CONTROL_COMBOBOX_VECTORS_U, TEXT("Data component for first vector axis"));

    // Надпись "V:"
    CreateWindow(
                WC_STATIC, TEXT("V:"),
                WS_CHILD | WS_VISIBLE | SS_CENTERIMAGE,
                550, 0, 14, 25,
                hwnd, (HMENU)CONTROL_LABEL_VECTORS_V, hInstance, NULL
                );

    // Комбобокс о выборе второй переменной по которой строятся вектора
    CreateWindow(
                WC_COMBOBOX, NULL,
                WS_CHILD | WS_VISIBLE | CBS_HASSTRINGS | CBS_DROPDOWNLIST | WS_TABSTOP,
                565, 0, 70, 150,
                hwnd, (HMENU)CONTROL_COMBOBOX_VECTORS_V, hInstance, NULL
                );
    set_tooltip(hInstance, hwnd, CONTROL_COMBOBOX_VECTORS_V, TEXT("Data component for second vector axis"));

    // Замутим label, в котором будем рисовать
    CreateWindow(
                WC_STATIC, NULL,
                WS_CHILD | WS_VISIBLE,
                0, 25, 600, 375,
                hwnd, (HMENU)CONTROL_PAINT_WIDGET, hInstance, NULL
                );

    // Шрифт
    HFONT font_std = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    SendDlgItemMessage(hwnd, CONTROL_PAINT_WIDGET, WM_SETFONT, (WPARAM)font_std, TRUE);
    SendDlgItemMessage(hwnd, CONTROL_CHECKBOX_COLOR, WM_SETFONT, (WPARAM)font_std, TRUE);
    SendDlgItemMessage(hwnd, CONTROL_COMBOBOX_COLOR, WM_SETFONT, (WPARAM)font_std, TRUE);
    SendDlgItemMessage(hwnd, CONTROL_CHECKBOX_ISOLINES, WM_SETFONT, (WPARAM)font_std, TRUE);
    SendDlgItemMessage(hwnd, CONTROL_SPINBOX_ISOLINES_TEXT, WM_SETFONT, (WPARAM)font_std, TRUE);
    SendDlgItemMessage(hwnd, CONTROL_SPINBOX_ISOLINES_UPDOWN, WM_SETFONT, (WPARAM)font_std, TRUE);
    SendDlgItemMessage(hwnd, CONTROL_CHECKBOX_VECTORS, WM_SETFONT, (WPARAM)font_std, TRUE);
    SendDlgItemMessage(hwnd, CONTROL_SPINBOX_VECTORS_TEXT, WM_SETFONT, (WPARAM)font_std, TRUE);
    SendDlgItemMessage(hwnd, CONTROL_SPINBOX_VECTORS_UPDOWN, WM_SETFONT, (WPARAM)font_std, TRUE);
    SendDlgItemMessage(hwnd, CONTROL_LABEL_VECTORS_U, WM_SETFONT, (WPARAM)font_std, TRUE);
    SendDlgItemMessage(hwnd, CONTROL_COMBOBOX_VECTORS_U, WM_SETFONT, (WPARAM)font_std, TRUE);
    SendDlgItemMessage(hwnd, CONTROL_LABEL_VECTORS_V, WM_SETFONT, (WPARAM)font_std, TRUE);
    SendDlgItemMessage(hwnd, CONTROL_COMBOBOX_VECTORS_V, WM_SETFONT, (WPARAM)font_std, TRUE);

    // Зададим умолчательные параметры
    SendMessage(GetDlgItem(hwnd, CONTROL_CHECKBOX_COLOR), BM_SETCHECK, BST_CHECKED, 0);
    SendMessage(GetDlgItem(hwnd, CONTROL_CHECKBOX_ISOLINES), BM_SETCHECK, BST_CHECKED, 0);
    SendMessage(GetDlgItem(hwnd, CONTROL_CHECKBOX_VECTORS), BM_SETCHECK, BST_UNCHECKED, 0);

    // Начальные значения элементов управления
    draw.set_isolines_num((size_t)config::isol_curr);
    draw.set_div_num((size_t)config::smooth_curr);
    draw.hwnd = GetDlgItem(hwnd, CONTROL_PAINT_WIDGET);

    // Если подан аргумент, значит откроем файл
    if(use_arg)
    {
        LPTSTR cmd_real = lpCmdLine;
        if(lpCmdLine[0] == TEXT('"'))
        {
            cmd_real++;
            cmd_real[u_strlen(cmd_real) - 1] = 0;
        }
        open_file(cmd_real);
    }

    // Покажем окно и запустим обработчик сообщений
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);
    MSG msg;
    while(GetMessage(&msg, NULL, 0, 0))
    {
        if(!haccel || !TranslateAccelerator(hwnd, haccel, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return 0;
}
