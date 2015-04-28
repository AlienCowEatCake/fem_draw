#-------------------------------------------------
#
# Project created by QtCreator 2014-11-03T09:55:33
#
#-------------------------------------------------

TARGET = vfem_draw
TEMPLATE = app

QT += core gui
INCLUDEPATH += src

greaterThan(QT_MAJOR_VERSION, 4) {
    QT += widgets
    DEFINES += HAVE_QT5
}

SOURCES += \
    src/glwidget.cpp \
    src/main.cpp \
    src/mainwindow.cpp

HEADERS += \
    src/glwidget.h \
    src/mainwindow.h

FORMS += \
    src/mainwindow.ui

*g++*|*clang* {
#    QMAKE_CXXFLAGS *= -ansi
#    QMAKE_CXXFLAGS *= -pedantic
#    QMAKE_CXXFLAGS_WARN_ON *= -Wextra
    QMAKE_CXXFLAGS_RELEASE -= -O2
    QMAKE_CXXFLAGS_RELEASE *= -O3
#    QMAKE_CXXFLAGS_RELEASE *= -march=native
#    QMAKE_CXXFLAGS_RELEASE *= -mtune=native
    QMAKE_CXXFLAGS_RELEASE *= -DNDEBUG
}

*msvc* {
    QMAKE_CXXFLAGS_RELEASE -= -O2
    QMAKE_CXXFLAGS_RELEASE *= -Ox
    DEFINES += _CRT_SECURE_NO_WARNINGS
    DEFINES += _USE_MATH_DEFINES
}
