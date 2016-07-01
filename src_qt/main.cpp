#include "mainwindow.h"
#include "Application.h"

#if defined (USE_STATIC_QJPEG)
Q_IMPORT_PLUGIN(qjpeg)
#endif
#if defined (USE_STATIC_QTIFF)
Q_IMPORT_PLUGIN(qtiff)
#endif
#if defined (USE_STATIC_QICO)
Q_IMPORT_PLUGIN(qico)
#endif

int main(int argc, char *argv[])
{
    Application a(argc, argv);
    a.setOrganizationDomain("fami.codefreak.ru");
    a.setOrganizationName("fami_net_team");
    a.setApplicationName("fem_draw");
    a.setApplicationVersion("1.7");
    MainWindow w;
    w.update_translations();
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
    else if(a.hasLastOpenFilename())
    {
        w.open_file(a.getLastOpenFilename());
    }
    QObject::connect(&a, SIGNAL(openFileEvent(const QString &)), &w, SLOT(open_file(const QString &)));
    w.show();
    return a.exec();
}
