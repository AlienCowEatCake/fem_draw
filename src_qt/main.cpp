#include "mainwindow.h"
#include <QMenu>
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
    MainWindow w;
#if defined (Q_OS_MAC)
    void qt_mac_set_dock_menu(QMenu *menu);
    QMenu dock_menu;
    dock_menu.addAction("New Window", &w, SLOT(on_actionNew_Window_triggered()));
    qt_mac_set_dock_menu(&dock_menu);
#endif
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
