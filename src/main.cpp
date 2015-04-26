#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    /*
    QGLFormat fmt;
    fmt.setSampleBuffers(true);
    fmt.setSamples(16); // 2, 4, 8, 16
    QGLFormat::setDefaultFormat(fmt);
    */
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
