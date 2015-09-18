#ifndef RESOURCES_H
#define RESOURCES_H

#define IDI_ICON1   42
#define IDB_LOGO64  43
#define IDC_HAND_FB 44

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
#define CONTROL_MENU_USELEGEND              10019
#define CONTROL_MENU_USELIGHTCOLORS         10020
#define CONTROL_MENU_USEPURPLE              10021
#define CONTROL_MENU_USEMEMORYLIMIT         10022
#define CONTROL_MENU_ABOUT                  10023
#define CONTROL_MENU_ABOUT_LIBS             10024
#define ABOUT_LABEL_VERSION                 10100
#define ABOUT_LABEL_SITE                    10101
#define ABOUT_LABEL_LICENSE                 10102
#define ABOUT_LABEL_GPL3                    10103
#define ABOUT_LABEL_COPYRIGHT               10104
#define ABOUT_LABEL_AUTHOR_BEGIN            10105
#define ABOUT_LABEL_AUTHOR_EMAIL            10106
#define ABOUT_LABEL_AUTHOR_END              10107
#define ABOUT_BUTTON_OK                     10108
#define ABOUT_LABEL_LOGO                    10109
#define ABOUT_LIBS_LABEL_LIBS               10200
#define ABOUT_LIBS_LABEL_LIB1               10201
#define ABOUT_LIBS_LABEL_LODEPNG            10202
#define ABOUT_LIBS_LABEL_LICENSE1           10203
#define ABOUT_LIBS_LABEL_ZLIB               10204
#define ABOUT_LIBS_LABEL_WEBSITE1           10205
#define ABOUT_LIBS_LABEL_URL_PNG            10206
#define ABOUT_LIBS_LABEL_LIB2               10207
#define ABOUT_LIBS_LABEL_JPG                10208
#define ABOUT_LIBS_LABEL_LICENSE2           10209
#define ABOUT_LIBS_LABEL_LIC_JPG            10210
#define ABOUT_LIBS_LABEL_WEBSITE2           10211
#define ABOUT_LIBS_LABEL_URL_JPG            10212
#define ABOUT_LIBS_LABEL_LIB3               10213
#define ABOUT_LIBS_LABEL_GIF                10214
#define ABOUT_LIBS_LABEL_LICENSE3           10215
#define ABOUT_LIBS_LABEL_LIC_GIF            10216
#define ABOUT_LIBS_LABEL_WEBSITE3           10217
#define ABOUT_LIBS_LABEL_URL_GIF            10218
#define ABOUT_LIBS_LABEL_LIB4               10219
#define ABOUT_LIBS_LABEL_TGA                10220
#define ABOUT_LIBS_LABEL_LICENSE4           10221
#define ABOUT_LIBS_LABEL_LIC_TGA            10222
#define ABOUT_LIBS_LABEL_WEBSITE4           10223
#define ABOUT_LIBS_LABEL_URL_TGA            10224
#define ABOUT_LIBS_BUTTON_OK                10225
#define ABOUT_LIBS_LABEL_LOGO               10226

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
