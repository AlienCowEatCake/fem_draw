#-------------------------------------------------
#
# Project created by QtCreator 2014-11-03T09:55:33
#
#-------------------------------------------------

TARGET = fem_draw_qt
TEMPLATE = app
CONFIG += warn_on

QT += core gui svg
INCLUDEPATH += src_qt

greaterThan(QT_MAJOR_VERSION, 4) {
    QT += widgets printsupport
    DEFINES += HAVE_QT5
}

lessThan(QT_VERSION, 4.5.0) {
    DEFINES += HAVE_LESS_THAN_QT45
}

SOURCES += \
    src_qt/libs/jo_jpeg.cpp \
    src_qt/libs/jo_gif.cpp \
    src_qt/libs/jo_tga.cpp \
    src_qt/Application.cpp \
    src_qt/paintwidget.cpp \
    src_qt/main.cpp \
    src_qt/mainwindow.cpp

HEADERS += \
    src_qt/libs/jo_images.h \
    src_qt/Application.h \
    src_qt/paintwidget.h \
    src_qt/mainwindow.h

FORMS += \
    src_qt/mainwindow.ui

RESOURCES += resources/icon.qrc
QMAKE_RESOURCE_FLAGS += -threshold 0 -compress 9

win32 {
    RC_FILE += src_qt/resources.rc
}

macx {
    QMAKE_INFO_PLIST = resources/Info.plist
    ICON = resources/fem_draw.icns
    TARGET = "FEM Draw"
    QMAKE_CXXFLAGS += -Wno-invalid-constexpr
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

# qmake CONFIG+=use_static_qjpeg
use_static_qjpeg {
    QTPLUGIN += qjpeg
    DEFINES += USE_STATIC_QJPEG
}

# qmake CONFIG+=use_static_qtiff
use_static_qtiff {
    QTPLUGIN += qtiff
    DEFINES += USE_STATIC_QTIFF
}
