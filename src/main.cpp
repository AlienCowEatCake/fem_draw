#include "mainwindow.h"
#include <QApplication>

#if defined USE_STATIC_QGIF
Q_IMPORT_PLUGIN(qgif)
#endif
#if defined USE_STATIC_QJPEG
Q_IMPORT_PLUGIN(qjpeg)
#endif
#if defined USE_STATIC_QTIFF
Q_IMPORT_PLUGIN(qtiff)
#endif

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
