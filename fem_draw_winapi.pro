#-------------------------------------------------
#
# Project created by QtCreator 2014-11-03T09:55:33
#
#-------------------------------------------------

TARGET = fem_draw_winapi
TEMPLATE = app
CONFIG -= app_bundle
CONFIG -= qt
CONFIG += warn_on

INCLUDEPATH += src_winapi

SOURCES += \
    src_winapi/paintwidget.cpp \
    src_winapi/main.cpp

HEADERS += \
    src_winapi/paintwidget.h

RC_FILE += resources/icon.rc
DEFINES += IDI_ICON1=42

win32-g++* {
    QMAKE_LIBS += -lgdi32 -lcomctl32 -luser32
} else {
    QMAKE_LIBS += gdi32.lib comctl32.lib user32.lib
    QMAKE_LFLAGS_WINDOWS += -ENTRY:"mainCRTStartup"
}

*g++*|*clang* {
    QMAKE_CXXFLAGS_WARN_ON *= -Wextra
    QMAKE_CXXFLAGS_RELEASE -= -O2
    QMAKE_CXXFLAGS_RELEASE *= -O3
    QMAKE_CXXFLAGS_RELEASE *= -DNDEBUG
}

*msvc* {
    QMAKE_CXXFLAGS_RELEASE -= -O2
    QMAKE_CXXFLAGS_RELEASE *= -Ox
    DEFINES += _CRT_SECURE_NO_WARNINGS
    DEFINES += _USE_MATH_DEFINES
}
