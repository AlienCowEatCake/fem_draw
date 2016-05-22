#include "mainwindow.h"
#include <QApplication>

#if defined (USE_STATIC_QJPEG)
Q_IMPORT_PLUGIN(qjpeg)
#endif
#if defined (USE_STATIC_QTIFF)
Q_IMPORT_PLUGIN(qtiff)
#endif

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    if(argc > 1)
    {
        string filename;
        for(int i = 1; i < argc; i++)
        {
            filename.append(argv[i]);
            if(i + 1 < argc)
                filename.append(" ");
        }
        w.open_file(QString::fromLocal8Bit(filename.c_str()));
    }
    w.show();
    return a.exec();
}
