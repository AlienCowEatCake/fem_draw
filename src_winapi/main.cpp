#include <iostream>
#include <fstream>
#include <cstdio>
#include <windows.h>
#include <commctrl.h>
#include "paintwidget.h"

#if !defined WC_BUTTON
#define WC_BUTTON   TEXT("Button")
#endif
#if !defined WC_STATIC
#define WC_STATIC   TEXT("Static")
#endif
#if !defined WC_EDIT
#define WC_EDIT     TEXT("Edit")
#endif
#if !defined WC_COMBOBOX
#define WC_COMBOBOX TEXT("Combobox")
#endif

#define CTRL_PAINT_WIDGET                   10000
#define CTRL_CHECKBOX_COLOR                 10001
#define CTRL_COMBOBOX_COLOR                 10002
#define CTRL_CHECKBOX_ISOLINES              10003
#define CTRL_SPINBOX_ISOLINES_TEXT          10004
#define CTRL_SPINBOX_ISOLINES_UPDOWN        10005
#define CTRL_CHECKBOX_VECTORS               10006
#define CTRL_SPINBOX_VECTORS_TEXT           10007
#define CTRL_SPINBOX_VECTORS_UPDOWN         10008
#define CTRL_LABEL_VECTORS_U                10009
#define CTRL_COMBOBOX_VECTORS_U             10010
#define CTRL_LABEL_VECTORS_V                10011
#define CTRL_COMBOBOX_VECTORS_V             10012
#define CTRL_MENU_OPEN                      10013
#define CTRL_MENU_SAVE                      10014
#define CTRL_MENU_EXIT                      10015
#define CTRL_MENU_INCREASE_INTERPOLATION    10016
#define CTRL_MENU_DECREASE_INTERPOLATION    10017
#define CTRL_MENU_ABOUT                     10018

HWND hwnd;
paintwidget * pdraw;
int min_height, min_width;
int isol_min = 0,   isol_max = 100,   isol_curr = 10;
int smooth_min = 0, smooth_max = 7,   smooth_curr = 0;
int vect_min = 1,   vect_max = 10000, vect_curr = 1;

void widget_redraw()
{
    RECT r1, r2;
    GetWindowRect(pdraw->hwnd, &r1);
    GetWindowRect(hwnd, &r2);
    int dx = r2.left + GetSystemMetrics(SM_CXFRAME);
    int dy = r2.top + GetSystemMetrics(SM_CYFRAME) + GetSystemMetrics(SM_CYCAPTION);
    r1.top -= dy;
    r1.bottom -= dy;
    r1.left -= dx;
    r1.right -= dx;
    InvalidateRect(hwnd, &r1, FALSE);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    switch(Msg)
    {
    case WM_COMMAND:    // Нажата кнопка
    {
        /*
        switch(LOWORD(wParam))
        {
        case CNTRL_DRAW_MESH:   // Переключалка рисования сетки
        {
            HWND hwndCheck = GetDlgItem(hwnd, CNTRL_DRAW_MESH);
            LRESULT res = SendMessage(hwndCheck, BM_GETCHECK, 0, 0);
//            if(res == BST_CHECKED)
//                pdraw->draw_mesh = true;
//            if(res == BST_UNCHECKED)
//                pdraw->draw_mesh = false;
            widget_redraw();
        }
        case CNTRL_DRAW_ISOL:   // Переключалка рисования изолиний
        {
            HWND hwndCheck = GetDlgItem(hwnd, CNTRL_DRAW_ISOL);
            LRESULT res = SendMessage(hwndCheck, BM_GETCHECK, 0, 0);
            if(res == BST_CHECKED)
                pdraw->draw_isolines = true;
            if(res == BST_UNCHECKED)
                pdraw->draw_isolines = false;
            widget_redraw();
            break;
        }
        case CNTRL_DRAW_COLOR:   // Переключалка рисования цвета
        {
            HWND hwndCheck = GetDlgItem(hwnd, CNTRL_DRAW_COLOR);
            LRESULT res = SendMessage(hwndCheck, BM_GETCHECK, 0, 0);
            if(res == BST_CHECKED)
                pdraw->draw_color = true;
            if(res == BST_UNCHECKED)
                pdraw->draw_color = false;
            widget_redraw();
            break;
        }
        case CNTRL_ISOL_NUM_TEXT:   // Задание числа изолиний
        {
            const size_t bufsize = 10;
            char str[bufsize];
            int val;
            GetWindowTextA(GetDlgItem(hwnd, CNTRL_ISOL_NUM_TEXT), str, bufsize);
            val = atoi(str);
            if(val != isol_curr)
            {
                if(val <= isol_max && val >= isol_min)
                {
                    isol_curr = val;
                    pdraw->set_isolines_num((size_t)isol_curr);
                    widget_redraw();
                }
                else
                {
#if defined _MSC_VER && _MSC_VER >= 1400
                    sprintf_s(str, bufsize, "%d", isol_curr);
#else
                    sprintf(str, "%d", isol_curr);
#endif
                    SetWindowTextA(GetDlgItem(hwnd, CNTRL_ISOL_NUM_TEXT), str);
                }
            }
            break;
        }
        case CNTRL_SMOOTH_TEXT:     // Задание уровня сглаживания
        {
            const size_t bufsize = 10;
            char str[bufsize];
            int val;
            GetWindowTextA(GetDlgItem(hwnd, CNTRL_SMOOTH_TEXT), str, bufsize);
            val = atoi(str);
            if(val != smooth_curr)
            {
                if(val <= smooth_max && val >= smooth_min)
                {
                    smooth_curr = val;
                    pdraw->set_div_num((size_t)smooth_curr);
                    widget_redraw();
                }
                else
                {
#if defined _MSC_VER && _MSC_VER >= 1400
                    sprintf_s(str, bufsize, "%d", smooth_curr);
#else
                    sprintf(str, "%d", smooth_curr);
#endif
                    SetWindowTextA(GetDlgItem(hwnd, CNTRL_SMOOTH_TEXT), str);
                }
            }
            break;
        }
        }*/
        break;
    }
    case WM_SETTEXT:
        cout << "here" << endl;
        break;
    case WM_SIZE:       // Изменение размера окна
    {
        RECT r;
        GetClientRect(hwnd, &r);
        SetWindowPos(
                    GetDlgItem(hwnd, CTRL_PAINT_WIDGET), NULL,
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
        ((LPMINMAXINFO)lParam)->ptMinTrackSize.x = (LONG)min_width;
        ((LPMINMAXINFO)lParam)->ptMinTrackSize.y = (LONG)min_height;
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

int main()
{
    // Рисовалка
    paintwidget draw;
    pdraw = & draw;

    // Разное
    HINSTANCE hInstance = GetModuleHandle(0);
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
    min_height = 500 + 2 * GetSystemMetrics(SM_CYFRAME) + GetSystemMetrics(SM_CYCAPTION);
    min_width = 640 + 2 * GetSystemMetrics(SM_CXFRAME);

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
    HMENU hMenu = CreateMenu();
    HMENU hFileMenu = CreatePopupMenu();
    AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT)hFileMenu, TEXT("File"));
    AppendMenu(hFileMenu, MF_STRING, CTRL_MENU_OPEN, TEXT("Open Tecplot File\tCtrl+O"));
    AppendMenu(hFileMenu, MF_STRING, CTRL_MENU_SAVE, TEXT("Save Image\tCtrl+S"));
    AppendMenu(hFileMenu, MF_SEPARATOR, (UINT)NULL, TEXT(""));
    AppendMenu(hFileMenu, MF_STRING, CTRL_MENU_EXIT, TEXT("Exit\tCtrl+Q"));
    HMENU hInterpMenu = CreatePopupMenu();
    AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT)hInterpMenu, TEXT("Interpolation"));
    AppendMenu(hInterpMenu, MF_STRING, CTRL_MENU_INCREASE_INTERPOLATION, TEXT("Increase Interpolation\t="));
    AppendMenu(hInterpMenu, MF_STRING, CTRL_MENU_DECREASE_INTERPOLATION, TEXT("Decrease Interpolation\t-"));
    HMENU hAboutMenu = CreatePopupMenu();
    AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT)hAboutMenu, TEXT("About"));
    AppendMenu(hAboutMenu, MF_STRING, CTRL_MENU_ABOUT, TEXT("About FEM Draw"));
    SetMenu(hwnd, hMenu);

    // Замутим чекбокс "Color"
    CreateWindow(
                WC_BUTTON, TEXT("Color"),
                WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | WS_TABSTOP,
                10, 0, 65, 25,
                hwnd, (HMENU)CTRL_CHECKBOX_COLOR, hInstance, NULL
                );
    set_tooltip(hInstance, hwnd, CTRL_CHECKBOX_COLOR, TEXT("Draw color image"));

    // Комбобокс о выборе переменной по которой цвет
    CreateWindow(
                WC_COMBOBOX, NULL,
                WS_CHILD | WS_VISIBLE | CBS_HASSTRINGS | CBS_DROPDOWNLIST | WS_TABSTOP,
                75, 0, 70, 150,
                hwnd, (HMENU)CTRL_COMBOBOX_COLOR, hInstance, NULL
                );
    set_tooltip(hInstance, hwnd, CTRL_COMBOBOX_COLOR, TEXT("Data component for colors and isolines"));

    // Замутим чекбокс "Isolines"
    CreateWindow(
                WC_BUTTON, TEXT("Isolines"),
                WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | WS_TABSTOP,
                160, 0, 80, 25,
                hwnd, (HMENU)CTRL_CHECKBOX_ISOLINES, hInstance, NULL
                );
    set_tooltip(hInstance, hwnd, CTRL_CHECKBOX_ISOLINES, TEXT("Draw isolines"));

    // Замутим спинбокс о числе изолиний
    CreateWindowEx(
                WS_EX_CLIENTEDGE,
                WC_EDIT, NULL,
                WS_CHILD | WS_VISIBLE | ES_LEFT | WS_BORDER | ES_AUTOHSCROLL |
                ES_NUMBER | WS_TABSTOP,
                240, 0, 55, 25,
                hwnd, (HMENU)CTRL_SPINBOX_ISOLINES_TEXT, hInstance, NULL
                );
    CreateUpDownControl(
                WS_CHILD | WS_BORDER | WS_VISIBLE | UDS_ARROWKEYS |
                UDS_ALIGNRIGHT | UDS_SETBUDDYINT,
                0, 0, 0, 0,
                hwnd, CTRL_SPINBOX_ISOLINES_UPDOWN, hInstance, GetDlgItem(hwnd, CTRL_SPINBOX_ISOLINES_TEXT),
                isol_max, isol_min, isol_curr
                );
    set_tooltip(hInstance, hwnd, CTRL_SPINBOX_ISOLINES_TEXT, TEXT("Number of isolines"));
    set_tooltip(hInstance, hwnd, CTRL_SPINBOX_ISOLINES_UPDOWN, TEXT("Number of isolines"));

    // Замутим чекбокс "Vectors"
    CreateWindow(
                WC_BUTTON, TEXT("Vectors"),
                WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | WS_TABSTOP,
                310, 0, 80, 25,
                hwnd, (HMENU)CTRL_CHECKBOX_VECTORS, hInstance, NULL
                );
    set_tooltip(hInstance, hwnd, CTRL_CHECKBOX_VECTORS, TEXT("Draw vectors"));

    // Замутим спинбокс о числе пропускаемых векторов
    CreateWindowEx(
                WS_EX_CLIENTEDGE,
                WC_EDIT, NULL,
                WS_CHILD | WS_VISIBLE | ES_LEFT | WS_BORDER | ES_AUTOHSCROLL |
                ES_NUMBER | WS_TABSTOP,
                390, 0, 55, 25,
                hwnd, (HMENU)CTRL_SPINBOX_VECTORS_TEXT, hInstance, NULL
                );
    CreateUpDownControl(
                WS_CHILD | WS_BORDER | WS_VISIBLE | UDS_ARROWKEYS |
                UDS_ALIGNRIGHT | UDS_SETBUDDYINT,
                0, 0, 0, 0,
                hwnd, CTRL_SPINBOX_VECTORS_UPDOWN, hInstance, GetDlgItem(hwnd, CTRL_SPINBOX_VECTORS_TEXT),
                vect_max, vect_min, vect_curr
                );
    set_tooltip(hInstance, hwnd, CTRL_SPINBOX_VECTORS_TEXT, TEXT("Number of skipped values"));
    set_tooltip(hInstance, hwnd, CTRL_SPINBOX_VECTORS_UPDOWN, TEXT("Number of skipped values"));

    // Надпись "U:"
    CreateWindow(
                WC_STATIC, TEXT("U:"),
                WS_CHILD | WS_VISIBLE | SS_CENTERIMAGE,
                455, 0, 25, 25,
                hwnd, (HMENU)CTRL_LABEL_VECTORS_U, hInstance, NULL
                );

    // Комбобокс о выборе первой переменной по которой строятся вектора
    CreateWindow(
                WC_COMBOBOX, NULL,
                WS_CHILD | WS_VISIBLE | CBS_HASSTRINGS | CBS_DROPDOWNLIST | WS_TABSTOP,
                470, 0, 70, 150,
                hwnd, (HMENU)CTRL_COMBOBOX_VECTORS_U, hInstance, NULL
                );
    set_tooltip(hInstance, hwnd, CTRL_COMBOBOX_VECTORS_U, TEXT("Data component for first vector axis"));

    // Надпись "V:"
    CreateWindow(
                WC_STATIC, TEXT("V:"),
                WS_CHILD | WS_VISIBLE | SS_CENTERIMAGE,
                550, 0, 25, 25,
                hwnd, (HMENU)CTRL_LABEL_VECTORS_V, hInstance, NULL
                );

    // Комбобокс о выборе второй переменной по которой строятся вектора
    CreateWindow(
                WC_COMBOBOX, NULL,
                WS_CHILD | WS_VISIBLE | CBS_HASSTRINGS | CBS_DROPDOWNLIST | WS_TABSTOP,
                565, 0, 70, 150,
                hwnd, (HMENU)CTRL_COMBOBOX_VECTORS_V, hInstance, NULL
                );
    set_tooltip(hInstance, hwnd, CTRL_COMBOBOX_VECTORS_V, TEXT("Data component for second vector axis"));

    // Замутим label, в котором будем рисовать
    CreateWindow(
                WC_STATIC, NULL,
                WS_CHILD | WS_VISIBLE,
                0, 25, 600, 375,
                hwnd, (HMENU)CTRL_PAINT_WIDGET, hInstance, NULL
                );

    // Шрифт
    HFONT font_std = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    SendDlgItemMessage(hwnd, CTRL_PAINT_WIDGET, WM_SETFONT, (WPARAM)font_std, TRUE);
    SendDlgItemMessage(hwnd, CTRL_CHECKBOX_COLOR, WM_SETFONT, (WPARAM)font_std, TRUE);
    SendDlgItemMessage(hwnd, CTRL_COMBOBOX_COLOR, WM_SETFONT, (WPARAM)font_std, TRUE);
    SendDlgItemMessage(hwnd, CTRL_CHECKBOX_ISOLINES, WM_SETFONT, (WPARAM)font_std, TRUE);
    SendDlgItemMessage(hwnd, CTRL_SPINBOX_ISOLINES_TEXT, WM_SETFONT, (WPARAM)font_std, TRUE);
    SendDlgItemMessage(hwnd, CTRL_SPINBOX_ISOLINES_UPDOWN, WM_SETFONT, (WPARAM)font_std, TRUE);
    SendDlgItemMessage(hwnd, CTRL_CHECKBOX_VECTORS, WM_SETFONT, (WPARAM)font_std, TRUE);
    SendDlgItemMessage(hwnd, CTRL_SPINBOX_VECTORS_TEXT, WM_SETFONT, (WPARAM)font_std, TRUE);
    SendDlgItemMessage(hwnd, CTRL_SPINBOX_VECTORS_UPDOWN, WM_SETFONT, (WPARAM)font_std, TRUE);
    SendDlgItemMessage(hwnd, CTRL_LABEL_VECTORS_U, WM_SETFONT, (WPARAM)font_std, TRUE);
    SendDlgItemMessage(hwnd, CTRL_COMBOBOX_VECTORS_U, WM_SETFONT, (WPARAM)font_std, TRUE);
    SendDlgItemMessage(hwnd, CTRL_LABEL_VECTORS_V, WM_SETFONT, (WPARAM)font_std, TRUE);
    SendDlgItemMessage(hwnd, CTRL_COMBOBOX_VECTORS_V, WM_SETFONT, (WPARAM)font_std, TRUE);

    // Зададим умолчательные параметры
    SendMessage(GetDlgItem(hwnd, CTRL_CHECKBOX_COLOR), BM_SETCHECK, BST_CHECKED, 0);
    SendMessage(GetDlgItem(hwnd, CTRL_CHECKBOX_ISOLINES), BM_SETCHECK, BST_CHECKED, 0);
    SendMessage(GetDlgItem(hwnd, CTRL_CHECKBOX_VECTORS), BM_SETCHECK, BST_UNCHECKED, 0);
/*
    draw.draw_isolines = true;
    draw.draw_color = true;
    draw.set_isolines_num((size_t)isol_curr);
    draw.set_div_num((size_t)smooth_curr);
    draw.hwnd = GetDlgItem(hwnd, CNTRL_GDI_WIDGET);
    draw.hdc = GetDC(draw.hwnd);
    SetBkColor(draw.hdc, RGB(255, 255, 255));

    draw.draw_vectors = true;
    draw.skip_vec = 10;
    draw.ind_vec_1 = 2;
    draw.ind_vec_2 = 3;
    draw.tec_read("../fem_draw/examples/plot.dat");
*/
    // Покажем окно и запустим обработчик сообщений
    ShowWindow(hwnd, SW_SHOWNORMAL);
    UpdateWindow(hwnd);
    MSG msg;
    while(GetMessage(&msg, NULL, 0, 0))
    {
        if(!IsDialogMessage(hwnd, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return 0;
}
