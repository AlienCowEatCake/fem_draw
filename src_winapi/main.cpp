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

#define CNTRL_DRAW_MESH         10001
#define CNTRL_DRAW_ISOL         10002
#define CNTRL_DRAW_COLOR        10003
#define CNTRL_ISOL_NUM_TEXT     10004
#define CNTRL_ISOL_NUM_UPDWN    10005
#define CNTRL_ISOL_NUM_LBL      10006
#define CNTRL_SMOOTH_TEXT       10007
#define CNTRL_SMOOTH_UPDWN      10008
#define CNTRL_SMOOTH_LBL        10009
#define CNTRL_GDI_WIDGET        10010

HWND hwnd;
paintwidget * pdraw;
int min_height;
int min_width;
int isol_min;
int isol_max;
int isol_curr;
int smooth_min;
int smooth_max;
int smooth_curr;

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
        }
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
                    GetDlgItem(hwnd, CNTRL_GDI_WIDGET), NULL,
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

int main()
{
    // Рисовалка
    paintwidget draw;
    pdraw = &draw;

    // Переменные
    isol_min = 0;
    isol_max = 100;
    isol_curr = 10;
    smooth_min = 0;
    smooth_max = 7;
    smooth_curr = 0;

    // Разное
    HINSTANCE hInstance = GetModuleHandle(0);
    WNDCLASS wnd;
    memset(&wnd, 0, sizeof(WNDCLASS));
    wnd.style = CS_HREDRAW | CS_VREDRAW;
    wnd.lpfnWndProc = WndProc;
    wnd.hInstance = hInstance;
    wnd.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wnd.lpszClassName = TEXT("mainwindow");
    RegisterClass(&wnd);

    // Установка минимальных размеров окна
    min_height = 400 + 2 * GetSystemMetrics(SM_CYFRAME) + GetSystemMetrics(SM_CYCAPTION);
    min_width = 600 + 2 * GetSystemMetrics(SM_CXFRAME);

    // Начинаем создавать главное окно
    int window_width = min_width;
    int window_height = min_height;
    HDC hDCScreen = GetDC(NULL);
    hwnd = CreateWindow(
                TEXT("mainwindow"), TEXT("FEMA lab04"),
                WS_OVERLAPPEDWINDOW,
                (GetDeviceCaps(hDCScreen, HORZRES) - window_width) / 2,
                (GetDeviceCaps(hDCScreen, VERTRES) - window_height) / 2,
                window_width, window_height,
                NULL, NULL, hInstance, NULL
                );

    // Замутим чекбокс "Draw mesh"
    CreateWindow(
                WC_BUTTON, TEXT("Draw mesh"),
                WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | WS_TABSTOP,
                10, 0, 100, 25,
                hwnd, (HMENU)CNTRL_DRAW_MESH, hInstance, NULL
                );

    // Замутим чекбокс "Draw isolines"
    CreateWindow(
                WC_BUTTON, TEXT("Draw isolines"),
                WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | WS_TABSTOP,
                115, 0, 115, 25,
                hwnd, (HMENU)CNTRL_DRAW_ISOL, hInstance, NULL
                );

    // Замутим чекбокс "Draw color"
    CreateWindow(
                WC_BUTTON, TEXT("Draw color"),
                WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | WS_TABSTOP,
                235, 0, 100, 25,
                hwnd, (HMENU)CNTRL_DRAW_COLOR, hInstance, NULL
                );

    // Замутим спинбокс о числе изолиний
    CreateWindowEx(
                WS_EX_CLIENTEDGE,
                WC_EDIT, NULL,
                WS_CHILD | WS_VISIBLE | ES_LEFT | WS_BORDER | ES_AUTOHSCROLL |
                ES_NUMBER | WS_TABSTOP,
                340, 0, 50, 25,
                hwnd, (HMENU)CNTRL_ISOL_NUM_TEXT, hInstance, NULL
                );
    CreateUpDownControl(
                WS_CHILD | WS_BORDER | WS_VISIBLE | UDS_ARROWKEYS |
                UDS_ALIGNRIGHT | UDS_SETBUDDYINT,
                0, 0, 0, 0,
                hwnd, CNTRL_ISOL_NUM_UPDWN, hInstance, GetDlgItem(hwnd, CNTRL_ISOL_NUM_TEXT),
                isol_max, isol_min, isol_curr
                );
    CreateWindow(
                WC_STATIC, TEXT("Isolines"),
                WS_CHILD | WS_VISIBLE | SS_CENTERIMAGE,
                400, 0, 70, 25,
                hwnd, (HMENU)CNTRL_ISOL_NUM_LBL, hInstance, NULL
                );

    // Замутим спинбокс о сглаживании
    CreateWindowEx(
                WS_EX_CLIENTEDGE,
                WC_EDIT, NULL,
                WS_CHILD | WS_VISIBLE | ES_LEFT | WS_BORDER | ES_AUTOHSCROLL |
                ES_NUMBER | WS_TABSTOP,
                470, 0, 50, 25,
                hwnd, (HMENU)CNTRL_SMOOTH_TEXT, hInstance, NULL
                );
    CreateUpDownControl(
                WS_CHILD | WS_BORDER | WS_VISIBLE | UDS_ARROWKEYS |
                UDS_ALIGNRIGHT | UDS_SETBUDDYINT,
                0, 0, 0, 0,
                hwnd, CNTRL_SMOOTH_UPDWN, hInstance, GetDlgItem(hwnd, CNTRL_SMOOTH_TEXT),
                smooth_max, smooth_min, smooth_curr
                );
    CreateWindow(
                WC_STATIC, TEXT("Smooth"),
                WS_CHILD | WS_VISIBLE | SS_CENTERIMAGE,
                530, 0, 70, 25,
                hwnd, (HMENU)CNTRL_SMOOTH_LBL, hInstance, NULL
                );

    // Замутим label, в котором будем рисовать
    CreateWindow(
                WC_STATIC, NULL,
                WS_CHILD | WS_VISIBLE,
                0, 25, 600, 375,
                hwnd, (HMENU)CNTRL_GDI_WIDGET, hInstance, NULL
                );

    // Шрифт
    HFONT font_std = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    SendDlgItemMessage(hwnd, CNTRL_DRAW_MESH, WM_SETFONT, (WPARAM)font_std, TRUE);
    SendDlgItemMessage(hwnd, CNTRL_DRAW_ISOL, WM_SETFONT, (WPARAM)font_std, TRUE);
    SendDlgItemMessage(hwnd, CNTRL_DRAW_COLOR, WM_SETFONT, (WPARAM)font_std, TRUE);
    SendDlgItemMessage(hwnd, CNTRL_ISOL_NUM_TEXT, WM_SETFONT, (WPARAM)font_std, TRUE);
    SendDlgItemMessage(hwnd, CNTRL_ISOL_NUM_UPDWN, WM_SETFONT, (WPARAM)font_std, TRUE);
    SendDlgItemMessage(hwnd, CNTRL_ISOL_NUM_LBL, WM_SETFONT, (WPARAM)font_std, TRUE);
    SendDlgItemMessage(hwnd, CNTRL_SMOOTH_TEXT, WM_SETFONT, (WPARAM)font_std, TRUE);
    SendDlgItemMessage(hwnd, CNTRL_SMOOTH_UPDWN, WM_SETFONT, (WPARAM)font_std, TRUE);
    SendDlgItemMessage(hwnd, CNTRL_SMOOTH_LBL, WM_SETFONT, (WPARAM)font_std, TRUE);

    // Зададим умолчательные параметры
    SendMessage(GetDlgItem(hwnd, CNTRL_DRAW_MESH), BM_SETCHECK, BST_CHECKED, 0);
    SendMessage(GetDlgItem(hwnd, CNTRL_DRAW_ISOL), BM_SETCHECK, BST_CHECKED, 0);
    SendMessage(GetDlgItem(hwnd, CNTRL_DRAW_COLOR), BM_SETCHECK, BST_CHECKED, 0);
//    draw.draw_mesh = true;
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
