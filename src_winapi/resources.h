#ifndef RESOURCES_H
#define RESOURCES_H

#define IDI_ICON1 42

#define CONTROL_PAINT_WIDGET                10000
#define CONTROL_CHECKBOX_COLOR              10001
#define CONTROL_COMBOBOX_COLOR              10002
#define CONTROL_CHECKBOX_ISOLINES           10003
#define CONTROL_SPINBOX_ISOLINES_TEXT       10004
#define CONTROL_SPINBOX_ISOLINES_UPDOWN     10005
#define CONTROL_CHECKBOX_VECTORS            10006
#define CONTROL_SPINBOX_VECTORS_TEXT        10007
#define CONTROL_SPINBOX_VECTORS_UPDOWN      10008
#define CONTROL_LABEL_VECTORS_U             10009
#define CONTROL_COMBOBOX_VECTORS_U          10010
#define CONTROL_LABEL_VECTORS_V             10011
#define CONTROL_COMBOBOX_VECTORS_V          10012
#define CONTROL_MENU_OPEN                   10013
#define CONTROL_MENU_TRANSPARENT            10014
#define CONTROL_MENU_SAVE                   10015
#define CONTROL_MENU_EXIT                   10016
#define CONTROL_MENU_INCREASE_INTERPOLATION 10017
#define CONTROL_MENU_DECREASE_INTERPOLATION 10018
#define CONTROL_MENU_ABOUT                  10019

#include <windows.h>
#include <commctrl.h>

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
#if !defined VK_ADD
#define VK_ADD            0x6B
#endif
#if !defined VK_SUBTRACT
#define VK_SUBTRACT       0x6D
#endif
#if !defined VK_OEM_MINUS
#define VK_OEM_MINUS      0xBD
#endif
#if !defined VK_OEM_PLUS
#define VK_OEM_PLUS       0xBB
#endif

#endif // RESOURCES_H
