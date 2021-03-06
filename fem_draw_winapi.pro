#-------------------------------------------------
#
# Project created by QtCreator 2014-11-03T09:55:33
#
#-------------------------------------------------

TARGET = fem_draw_winapi
TEMPLATE = app
CONFIG -= app_bundle
CONFIG -= qt
CONFIG -= embed_manifest_exe
CONFIG += warn_on

INCLUDEPATH += src_winapi

SOURCES += \
    src_winapi/libs/lodepng.cpp \
    src_winapi/libs/jo_jpeg.cpp \
    src_winapi/libs/jo_gif.cpp \
    src_winapi/libs/jo_tga.cpp \
    src_winapi/paintwidget.cpp \
    src_winapi/main.cpp

HEADERS += \
    src_winapi/libs/lodepng.h \
    src_winapi/libs/jo_images.h \
    src_winapi/resources.h \
    src_winapi/paintwidget.h \
    src_winapi/compilersupport.h \
    src_winapi/common.h

RC_FILE += src_winapi/resources.rc

win32-g++* {
    QMAKE_LIBS += -lgdi32 -lcomctl32 -luser32 -lcomdlg32 -lshell32
} else {
    QMAKE_LIBS += gdi32.lib comctl32.lib user32.lib comdlg32.lib shell32.lib
}

*g++*|*clang* {
    QMAKE_CXXFLAGS *= -Wno-write-strings
    QMAKE_CXXFLAGS_WARN_ON *= -Wextra
    QMAKE_CXXFLAGS_RELEASE -= -O2
    QMAKE_CXXFLAGS_RELEASE *= -O3
    QMAKE_CXXFLAGS_RELEASE *= -DNDEBUG
}

*msvc* {
    QMAKE_CXXFLAGS_RELEASE -= -O2
    QMAKE_CXXFLAGS_RELEASE *= -Ox
    QMAKE_CFLAGS_RELEASE                    -= -Zc:strictStrings
    QMAKE_CFLAGS_RELEASE_WITH_DEBUGINFO     -= -Zc:strictStrings
    QMAKE_CFLAGS_DEBUG                      -= -Zc:strictStrings
    QMAKE_CFLAGS                            -= -Zc:strictStrings
    QMAKE_CXXFLAGS_RELEASE                  -= -Zc:strictStrings
    QMAKE_CXXFLAGS_RELEASE_WITH_DEBUGINFO   -= -Zc:strictStrings
    QMAKE_CXXFLAGS_DEBUG                    -= -Zc:strictStrings
    QMAKE_CXXFLAGS                          -= -Zc:strictStrings
}
