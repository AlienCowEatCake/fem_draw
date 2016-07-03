#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QList>
#include <QVector>
#include <QSvgGenerator>
#include <QPrinter>
#include <QColorDialog>
#include <QInputDialog>
#include <QClipboard>
#include <QProcess>
#include <QDir>
#include <QStringList>
#include <QApplication>
#include <QTranslator>
#include <QPair>
#include <QMenu>
#include <QSettings>
#include <QLocale>
#include <QResizeEvent>
#include <QObject>
#include <QFont>
#include <QFontDatabase>
#include <algorithm>
#include <cmath>
#include "libs/jo_images.h"

#if defined (Q_OS_WIN) && defined (USE_WIN98_WORKAROUNDS)
#include <windows.h>
#endif

// Конструктор
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Установка минимальных размеров окна
    this->setMinimumHeight(500);
    this->setMinimumWidth(640);

    // Перемещение в центр экрана
    QPoint center = QApplication::desktop()->availableGeometry().center();
    QPoint corner = QApplication::desktop()->availableGeometry().topLeft();
    center.setX(center.x() - this->width() / 2);
    center.setY(center.y() - this->height() / 2);
    if(center.x() <= corner.x() || center.y() <= corner.y())
        this->move(corner);
    else
        this->move(center);

    // Список компонент
    while(ui->comboBox_Color->count() > 0)
        ui->comboBox_Color->removeItem(0);
    while(ui->comboBox_Vectors_U->count() > 0)
        ui->comboBox_Vectors_U->removeItem(0);
    while(ui->comboBox_Vectors_V->count() > 0)
        ui->comboBox_Vectors_V->removeItem(0);

    // Начальные значения элементов управления
    ui->checkBox_Isolines->setChecked(true);
    ui->checkBox_Color->setChecked(true);
    ui->checkBox_Vectors->setChecked(false);
    ui->spinBox_Isolines->setMinimum(0);
    ui->spinBox_Isolines->setMaximum(100);
    ui->spinBox_Isolines->setValue(10);
    ui->spinBox_Vectors->setMinimum(1);
    ui->spinBox_Vectors->setMaximum(10000);
    ui->spinBox_Vectors->setValue(1);
    ui->actionShow_Isolines->setChecked(ui->checkBox_Isolines->isChecked());
    ui->actionShow_Vectors->setChecked(ui->checkBox_Vectors->isChecked());

    // Немного эстетства
    this->setWindowTitle(trUtf8("FEM Draw"));
    connect(ui->actionAbout_Qt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

    // Передача начальных значений виджету
    ui->widget->draw_isolines = ui->checkBox_Isolines->isChecked();
    ui->widget->draw_color = ui->checkBox_Color->isChecked();
    ui->widget->draw_vectors = ui->checkBox_Vectors->isChecked();
    ui->widget->set_isolines_num(ui->spinBox_Isolines->value());
    ui->widget->skip_vec = ui->spinBox_Vectors->value();
    ui->widget->set_div_num(0);
    ui->actionShow_Legend->setChecked(ui->widget->use_legend);
    ui->actionUse_Light_Colors->setChecked(ui->widget->use_light_colors);
    ui->actionUse_Purple_Colors->setChecked(ui->widget->use_purple);
    ui->actionMemory_Limit->setChecked(ui->widget->use_memory_limit);

    // Разрешим обработку drag-and-drop
    setAcceptDrops(true);

    // Зададим умолчательные имена файлов
    last_saved = trUtf8("draw.png");
    last_opened = "";

#if defined (Q_OS_MAC)
    // Очень грустный костыль: под маком очень странная политика работы с фокусом.
    // Из-за этого по-умолчанию фокус устанавливается только на SpinBox'ах. Самый
    // вменяемый способ починить это - заблокировать фокус у всех элементов
    // управления, кроме основного виджета, в котором рисуем.
    ui->checkBox_Color->setFocusPolicy(Qt::NoFocus);
    ui->comboBox_Color->setFocusPolicy(Qt::NoFocus);
    ui->checkBox_Isolines->setFocusPolicy(Qt::NoFocus);
    ui->spinBox_Isolines->setFocusPolicy(Qt::NoFocus);
    ui->checkBox_Vectors->setFocusPolicy(Qt::NoFocus);
    ui->spinBox_Vectors->setFocusPolicy(Qt::NoFocus);
    ui->label->setFocusPolicy(Qt::NoFocus);
    ui->comboBox_Vectors_U->setFocusPolicy(Qt::NoFocus);
    ui->label_2->setFocusPolicy(Qt::NoFocus);
    ui->comboBox_Vectors_V->setFocusPolicy(Qt::NoFocus);
    ui->centralwidget->setFocusPolicy(Qt::StrongFocus);
    ui->centralwidget->setFocus();
#endif
}

// Деструктор
MainWindow::~MainWindow()
{
    delete ui;
}

// Событие при изменении размера окна
void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    // Подгонка размеров виджета при изменении размеров окна
    QRect main = ui->centralwidget->geometry();
    QRect widget = ui->widget->geometry();
    int widget_y = ui->verticalSpacer->geometry().top();
    ui->widget->setGeometry(widget.x(), widget_y, main.width() - widget.x(), main.height() - widget_y);
}

// Показать окно
void MainWindow::show()
{
    QMainWindow::show();
    QApplication::postEvent(this, new QResizeEvent(size(), size()));
}

// Исправить отображение локаизованных шрифтов под Windows
void MainWindow::fonts_fix(const QString & language)
{
#if defined (Q_OS_WIN)

    // Отображение название языка -> соответствующая ему WritingSystem
    static QList<QPair<QString, QFontDatabase::WritingSystem> > writingsystem_map =
            QList<QPair<QString, QFontDatabase::WritingSystem> >()
            << qMakePair(QString("en"), QFontDatabase::Latin)
            << qMakePair(QString("ru"), QFontDatabase::Cyrillic);

    // Найдем WritingSystem для текущего языка
    QFontDatabase::WritingSystem current_writingsystem = QFontDatabase::Any;
    for(QList<QPair<QString, QFontDatabase::WritingSystem> >::Iterator it = writingsystem_map.begin(); it != writingsystem_map.end(); ++it)
    {
        if(it->first == language)
        {
            current_writingsystem = it->second;
            break;
        }
    }

    // Шрифт стандартный, по умолчанию
    static QFont default_font = qApp->font();
    // Шрифт Tahoma, если стандартный не поддерживает выбранный язык
    QFont fallback_font = default_font;
    fallback_font.setFamily("Tahoma");

    // Проверим, умеет ли стандартный шрифт писать на новом языке
    static QFontDatabase qfd;
    if(!qfd.families(current_writingsystem).contains(default_font.family(), Qt::CaseInsensitive))
        qApp->setFont(fallback_font);   // Если не умеет - заменим на Tahoma
    else
        qApp->setFont(default_font);    // Если умеет, то вернем его обратно

#if defined (USE_WIN98_WORKAROUNDS)
    // Для Win98 форсированно заменяем шрифты на Tahoma для всех не-английских локалей
    static DWORD dwVersion = (DWORD)(LOBYTE(LOWORD(GetVersion())));
    if(dwVersion <= 4)
    {
        if(language != "en")
            qApp->setFont(fallback_font);
        else
            qApp->setFont(default_font);
    }
#endif

#else

    Q_UNUSED(language);

#endif
}

// Открытие файла по имени
void MainWindow::open_file(const QString & filename)
{
    // Запомним старые значения индексов, чтоб потом восстановить
    size_t old_draw_index = ui->widget->draw_index;
    size_t old_ind_vec_1 = ui->widget->ind_vec_1;
    size_t old_ind_vec_2 = ui->widget->ind_vec_2;

    // Откроем файл
    ui->widget->div_num = 0; // Сбросим значение интерполяции, чтобы не повисло на больших файлах
    ui->widget->tec_read(filename);
    if(!ui->widget->is_loaded)
    {
        this->setWindowTitle(trUtf8("FEM Draw"));
        ui->widget->invalidate();
        return;
    }
    // Ненене, еще не все готово!
    ui->widget->is_loaded = false;

    // Очистим поля в комбобоксах
    while(ui->comboBox_Color->count() > 0)
        ui->comboBox_Color->removeItem(0);
    while(ui->comboBox_Vectors_U->count() > 0)
        ui->comboBox_Vectors_U->removeItem(0);
    while(ui->comboBox_Vectors_V->count() > 0)
        ui->comboBox_Vectors_V->removeItem(0);

    // Заполним поля в комбобоксах
    for(size_t i = 0; i < ui->widget->variables.size(); i++)
    {
        ui->comboBox_Color->addItem(ui->widget->variables[i]);
        ui->comboBox_Vectors_U->addItem(ui->widget->variables[i]);
        ui->comboBox_Vectors_V->addItem(ui->widget->variables[i]);
    }

    // Попытаемся восстановить старые индексы
    if(old_draw_index < ui->widget->variables.size())
        ui->comboBox_Color->setCurrentIndex((int)old_draw_index);
    else
        ui->comboBox_Color->setCurrentIndex(0);

    if(old_ind_vec_1 < ui->widget->variables.size() && old_ind_vec_2 < ui->widget->variables.size())
    {
        ui->comboBox_Vectors_U->setCurrentIndex((int)old_ind_vec_1);
        ui->comboBox_Vectors_V->setCurrentIndex((int)old_ind_vec_2);
    }
    else
    {
        ui->comboBox_Vectors_U->setCurrentIndex(0);
        if(ui->widget->variables.size() >= 2)
            ui->comboBox_Vectors_V->setCurrentIndex(1);
        else
            ui->comboBox_Vectors_V->setCurrentIndex(0);
    }

    // Устанавливаем оптимальное значение для векторов
    if(ui->widget->vect_value < ui->spinBox_Vectors->minimum())
        ui->spinBox_Vectors->setValue(ui->spinBox_Vectors->minimum());
    else if(ui->widget->vect_value > ui->spinBox_Vectors->maximum())
        ui->spinBox_Vectors->setValue(ui->spinBox_Vectors->maximum());
    else
        ui->spinBox_Vectors->setValue(ui->widget->vect_value);

    // Установим заголовок окна
    QString label = filename.split('/').last();
#if defined (Q_OS_WIN32)
    label = label.split('\\').last();
#endif
    if(ui->widget->title.length() > 0)
        label.prepend(ui->widget->title + " - ");
    label.append(" - ").append(trUtf8("FEM Draw"));
    this->setWindowTitle(label);

    // Сохраним директорию, в которой находится файл
    last_opened = "";
    int stop_index = filename.lastIndexOf("/");
#if defined (Q_OS_WIN32)
    int stop_index_win = filename.lastIndexOf("\\");
    if(stop_index_win > stop_index)
        stop_index = stop_index_win;
#endif
    for(int i = 0; i < stop_index; i++)
        last_opened.append(filename[i]);

    // А вот теперь готово
    ui->widget->is_loaded = true;
    ui->widget->invalidate();
}

// Событие при открытии файла
void MainWindow::on_actionOpen_Tecplot_File_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, trUtf8("Open Tecplot File"), last_opened,
                                                    QString("%1 (*.dat *.plt);;%2 (*.*)").arg(trUtf8("Tecplot Data Files")).arg(trUtf8("All Files")));
    if(fileName.length() == 0) return;
    open_file(fileName);
}

// Событие при копировании картинки в буфер обмена
void MainWindow::on_actionCopy_Image_to_Clipboard_triggered()
{
    QImage image(ui->widget->width(), ui->widget->height(), QImage::Format_ARGB32_Premultiplied);
    ui->widget->draw(& image, ui->actionTransparent_Image->isChecked(), true);
    QClipboard * clipboard = QApplication::clipboard();
    clipboard->setImage(image);
}

// Событие при сохранении
void MainWindow::on_actionSave_Image_File_triggered()
{
    QList<QByteArray> supported_temp = QImageWriter::supportedImageFormats();
    QVector<QString> supported;
    for(QList<QByteArray>::const_iterator it = supported_temp.begin(); it != supported_temp.end(); ++it)
        supported.push_back(QString(*it).toLower());
    supported_temp.clear();
    supported.push_back("svg");
//#if !defined (HAVE_QT5)
//    supported.push_back("ps");
//#endif
    supported.push_back("pdf");

    bool qt_jpg = true;
    if(std::find(supported.begin(), supported.end(), "jpg") == supported.end())
    {
        supported.push_back("jpg");
        qt_jpg = false;
    }

    bool qt_gif = true;
    if(std::find(supported.begin(), supported.end(), "gif") == supported.end())
    {
        supported.push_back("gif");
        qt_gif = false;
    }

    bool qt_tga = true;
    if(std::find(supported.begin(), supported.end(), "tga") == supported.end())
    {
        supported.push_back("tga");
        qt_tga = false;
    }

    std::sort(supported.begin(), supported.end());

    QString formats, formats_all;
    for(QVector<QString>::const_iterator it = supported.begin(); it != supported.end(); ++it)
    {
        if(formats_all.length() > 0)
            formats_all.append(" *.");
        else
            formats_all.append("*.");
        formats_all.append(*it);

        if(formats.length() > 0)
            formats.append(";;");
        formats.append(it->toUpper());
        formats.append(" ");
        formats.append(trUtf8("Images"));
        formats.append(" (*.");
        formats.append(*it);
        formats.append(")");
    }
    formats_all.prepend(trUtf8("All Images").append(" ("));
    formats_all.append(");;");
    formats.prepend(formats_all);

    QString filename = QFileDialog::getSaveFileName(this, trUtf8("Save Image File"), last_saved, formats);
    if(filename.length() == 0) return;

    QString def_ext("png"), ext;
    int found = filename.lastIndexOf('.');
    if(found == -1)
    {
        filename.append(".");
        filename.append(def_ext);
        ext = def_ext;
    }
    else
    {
        ext = filename.right(filename.length() - found - 1).toLower();
        if(std::find(supported.begin(), supported.end(), ext) == supported.end())
        {
            filename.append(".");
            filename.append(def_ext);
            ext = def_ext;
        }
    }
    last_saved = filename;

    bool saved = false;
    if(ext == "svg")
    {
        QSvgGenerator generator;
        generator.setFileName(filename);
        generator.setSize(QSize(ui->widget->width(), ui->widget->height()));
#if !defined (HAVE_LESS_THAN_QT45)
        generator.setViewBox(QRect(0, 0, ui->widget->width(), ui->widget->height()));
        generator.setTitle(ui->widget->title);
        generator.setDescription(trUtf8("Generated with FEM Draw"));
#endif
        ui->widget->draw(& generator, ui->actionTransparent_Image->isChecked(), true);
        saved = true;
    }
//#if !defined (HAVE_QT5)
//    else if(ext == "ps" || ext == "pdf")
//#else
    else if(ext == "pdf")
//#endif
    {
        QPrinter printer(QPrinter::ScreenResolution);
        if(ext == "pdf")
            printer.setOutputFormat(QPrinter::PdfFormat);
//#if !defined (HAVE_QT5)
//        else if(ext == "ps")
//            printer.setOutputFormat(QPrinter::PostScriptFormat);
//#endif
        printer.setOutputFileName(filename);
        printer.setPaperSize(QSizeF(ui->widget->width(), ui->widget->height()), QPrinter::DevicePixel);
        printer.setPageMargins(0, 0, 0, 0, QPrinter::DevicePixel);
        ui->widget->draw(& printer, ui->actionTransparent_Image->isChecked(), true);
        saved = true;
    }
    else if(ext == "jpg" && !qt_jpg)
    {
        QFile file(filename);
        file.open(QIODevice::WriteOnly);
        QDataStream stream(&file);
        if(stream.status() == QDataStream::Ok)
        {
            QImage image(ui->widget->width(), ui->widget->height(), QImage::Format_ARGB32_Premultiplied);
            ui->widget->draw(& image, false);
            jo_write_jpg(stream, image);
            stream.device()->close();
            saved = true;
        }
    }
    else if(ext == "gif" && !qt_gif)
    {
        QFile file(filename);
        file.open(QIODevice::WriteOnly);
        QDataStream stream(&file);
        if(stream.status() == QDataStream::Ok)
        {
            QImage image(ui->widget->width(), ui->widget->height(), QImage::Format_ARGB32_Premultiplied);
            ui->widget->draw(& image, false);
            q_jo_gif_t gif = jo_gif_start(stream, image.width(), image.height());
            jo_gif_frame(gif, image, 0);
            jo_gif_end(gif);
            stream.device()->close();
            saved = true;
        }
    }
    else if(ext == "tga" && !qt_tga)
    {
        QFile file(filename);
        file.open(QIODevice::WriteOnly);
        QDataStream stream(&file);
        if(stream.status() == QDataStream::Ok)
        {
            QImage image(ui->widget->width(), ui->widget->height(), QImage::Format_ARGB32_Premultiplied);
            ui->widget->draw(& image, ui->actionTransparent_Image->isChecked());
            jo_write_tga(stream, image, ui->actionTransparent_Image->isChecked());
            stream.device()->close();
            saved = true;
        }
    }
    else
    {
        QImage image(ui->widget->width(), ui->widget->height(), QImage::Format_ARGB32_Premultiplied);
        ui->widget->draw(& image, ui->actionTransparent_Image->isChecked());
        saved = image.save(filename);
    }

    if(!saved)
    {
        QMessageBox msgBox;
        msgBox.setAttribute(Qt::WA_QuitOnClose);
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.setWindowTitle(trUtf8("Error"));
        msgBox.setText(trUtf8("Error: Can't save file"));
        msgBox.setIcon(QMessageBox::Critical);
#if !defined (Q_OS_MAC)
        msgBox.setWindowIcon(QIcon(":/resources/icon.ico"));
#endif
        msgBox.exec();
    }
}

// Событие при нажатии кнопки New Window
void MainWindow::on_actionNew_Window_triggered()
{
    QProcess::startDetached(QApplication::applicationFilePath(), QStringList(), QDir::currentPath());
}

// Событие при нажатии кнопки Exit
void MainWindow::on_actionExit_triggered()
{
    close();
}

// Событие при изменении уровня интерполяции
void MainWindow::on_actionIncrease_Interpolation_triggered()
{
    if(ui->widget->div_num < 10)
    {
        size_t old_value = ui->widget->div_num;
        ui->widget->set_div_num(ui->widget->div_num + 1);
        if(ui->widget->div_num == old_value + 1)
            ui->widget->invalidate();
        else
        {
            QMessageBox msgBox;
            msgBox.setAttribute(Qt::WA_QuitOnClose);
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setDefaultButton(QMessageBox::Ok);
            msgBox.setWindowTitle(trUtf8("Error"));
            msgBox.setText(trUtf8("Error: Memory limit exceeded"));
            msgBox.setIcon(QMessageBox::Critical);
#if !defined (Q_OS_MAC)
            msgBox.setWindowIcon(QIcon(":/resources/icon.ico"));
#endif
            msgBox.exec();
        }
    }
    else
    {
        QMessageBox msgBox;
        msgBox.setAttribute(Qt::WA_QuitOnClose);
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.setWindowTitle(trUtf8("Error"));
        msgBox.setText(trUtf8("Error: More than 10x interpolation is not supported"));
        msgBox.setIcon(QMessageBox::Critical);
#if !defined (Q_OS_MAC)
        msgBox.setWindowIcon(QIcon(":/resources/icon.ico"));
#endif
        msgBox.exec();
    }
}

void MainWindow::on_actionDecrease_Interpolation_triggered()
{
    if(ui->widget->div_num > 0)
    {
        ui->widget->set_div_num(ui->widget->div_num - 1);
        ui->widget->invalidate();
    }
}

// Событие при переключении рисования изолиний из меню
void MainWindow::on_actionShow_Isolines_triggered()
{
    ui->checkBox_Isolines->click();
}

// Событие при запросе конфигурации цвета изолиний
void MainWindow::on_actionIsolines_Color_triggered()
{
#if !defined (HAVE_LESS_THAN_QT45)
    QColor color = QColorDialog::getColor(ui->widget->isolines_config.color, this, trUtf8("Select Isolines Color"));
#else
    QColor color = QColorDialog::getColor(ui->widget->isolines_config.color, this);
#endif
    if(color != ui->widget->isolines_config.color)
    {
        ui->widget->isolines_config.color = color;
        ui->widget->invalidate();
    }
}

// Событие при запросе конфигурации толщины изолиний
void MainWindow::on_actionIsolines_Width_triggered()
{
    bool ok = false;
    float width = (float)QInputDialog::getDouble(this, trUtf8("Width"), trUtf8("Select Isolines Width:"),
                                                 (double)ui->widget->isolines_config.width, 1.0, 10.0, 1, &ok);
    if(ok && fabs(width - ui->widget->isolines_config.width) > 1e-5)
    {
        ui->widget->isolines_config.width = width;
        ui->widget->invalidate();
    }
}

// Событие при переключении рисования векторов из меню
void MainWindow::on_actionShow_Vectors_triggered()
{
    ui->checkBox_Vectors->click();
}

// Событие при запросе конфигурации цвета векторов
void MainWindow::on_actionVectors_Color_triggered()
{
#if !defined (HAVE_LESS_THAN_QT45)
    QColor color = QColorDialog::getColor(ui->widget->vectors_config.color, this, trUtf8("Select Vectors Color"));
#else
    QColor color = QColorDialog::getColor(ui->widget->isolines_config.color, this);
#endif
    if(color != ui->widget->vectors_config.color)
    {
        ui->widget->vectors_config.color = color;
        ui->widget->invalidate();
    }
}

// Событие при запросе конфигурации толщины векторов
void MainWindow::on_actionVectors_Width_triggered()
{
    bool ok = false;
    float width = (float)QInputDialog::getDouble(this, trUtf8("Width"), trUtf8("Select Vectors Width:"),
                                                 (double)ui->widget->vectors_config.width, 1.0, 10.0, 1, &ok);
    if(ok && fabs(width - ui->widget->vectors_config.width) > 1e-5)
    {
        ui->widget->vectors_config.width = width;
        ui->widget->invalidate();
    }
}

// Событие при запросе конфигурации длины векторов
void MainWindow::on_actionVectors_Length_triggered()
{
    bool ok = false;
    float length = (float)QInputDialog::getDouble(this, trUtf8("Length"), trUtf8("Select Vectors Length:"),
                                                  (double)ui->widget->vectors_config.length, 1.0, 100.0, 1, &ok);
    if(ok && fabs(length - ui->widget->vectors_config.length) > 1e-5)
    {
        ui->widget->vectors_config.length = length;
        ui->widget->invalidate();
    }
}

// Событие при запросе конфигурации размера стрелок векторов
void MainWindow::on_actionArrowSize_triggered()
{
    bool ok = false;
    float arrow_size = (float)QInputDialog::getDouble(this, trUtf8("Arrow Size"), trUtf8("Select Arrow Size:"),
                                                      (double)ui->widget->vectors_config.arrow_size, 1.0, 25.0, 1, &ok);
    if(ok && fabs(arrow_size - ui->widget->vectors_config.arrow_size) > 1e-5)
    {
        ui->widget->vectors_config.arrow_size = arrow_size;
        ui->widget->invalidate();
    }
}

// Функция для применения локализации
void MainWindow::update_translations(QString language)
{
    // Отображение название языка -> соответствующая ему менюшка
    static QList<QPair<QString, QAction *> > languages_map = QList<QPair<QString, QAction *> >()
            << qMakePair(QString("en"), ui->actionEnglish)
            << qMakePair(QString("ru"), ui->actionRussian);

    // Определим системную локаль
    static QString system_lang;
    if(system_lang.isEmpty())
    {
        QString system_locale = QLocale::system().name().toLower();
        for(QList<QPair<QString, QAction *> >::Iterator it = languages_map.begin(); it != languages_map.end(); ++it)
        {
            if(system_locale.startsWith(it->first))
            {
                system_lang = it->first;
                break;
            }
        }
        if(system_lang.isEmpty())
            system_lang = "en";
    }

    // Посмотрим в настройки, не сохранен ли случайно в них язык
    QSettings settings;
    if(language.isEmpty())
        language = settings.value("Language", system_lang).toString();
    else
        settings.setValue("Language", language);

    // Удалим старый перевод, установим новый
    static QTranslator translator;
    if(!translator.isEmpty())
        qApp->removeTranslator(&translator);
    translator.load(QLocale(language), "fem_draw_qt", "_", ":/l10ns");
    qApp->installTranslator(&translator);
    ui->retranslateUi(this);

    // Пофиксим шрифты
    fonts_fix(language);

    // Пробежим по меню и проставим галочку на нужном нам языке и снимем с остальных
    for(QList<QPair<QString, QAction *> >::Iterator it = languages_map.begin(); it != languages_map.end(); ++it)
        it->second->setChecked(it->first == language);

    // Меню в доке OS X также нуждается в переводе
    update_dock_menu();

    // Также следует пересчитать геометрию виждетов
    QApplication::postEvent(this, new QResizeEvent(size(), size()));
}

// Функция, устанавливающая меню в доке OS X
void MainWindow::update_dock_menu()
{
#if defined (Q_OS_MAC)
    void qt_mac_set_dock_menu(QMenu *menu);
    static QMenu dock_menu;
    dock_menu.clear();
    dock_menu.addAction(qApp->translate("Dock", "New Window"), this, SLOT(on_actionNew_Window_triggered()));
    qt_mac_set_dock_menu(&dock_menu);
#endif
}

// Событие при включении английского языка
void MainWindow::on_actionEnglish_triggered()
{
    update_translations("en");
}

// Событие при включении русского языка
void MainWindow::on_actionRussian_triggered()
{
    update_translations("ru");
}

// Событие при переключении рисования легенды
void MainWindow::on_actionShow_Legend_triggered()
{
    ui->widget->use_legend = ui->actionShow_Legend->isChecked();
    ui->widget->invalidate();
}

// Событие при переключении использования светлых цветов
void MainWindow::on_actionUse_Light_Colors_triggered()
{
    ui->widget->use_light_colors = ui->actionUse_Light_Colors->isChecked();
    ui->widget->set_div_num(ui->widget->div_num);
    ui->widget->invalidate();
}

// Событие при переключении использования фиолетовых цветов
void MainWindow::on_actionUse_Purple_Colors_triggered()
{
    ui->widget->use_purple = ui->actionUse_Purple_Colors->isChecked();
    ui->widget->set_div_num(ui->widget->div_num);
    ui->widget->invalidate();
}

// Событие при переключении использования ограничения памяти
void MainWindow::on_actionMemory_Limit_triggered()
{
    ui->widget->use_memory_limit = ui->actionMemory_Limit->isChecked();
}

// Событие при нажатии кнопки About
void MainWindow::on_actionAbout_FEM_Draw_triggered()
{
    QMessageBox msgBox;
    msgBox.setAttribute(Qt::WA_QuitOnClose);
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.setWindowTitle(trUtf8("About"));
    msgBox.setText(QString("<b>%1 v1.7 (Qt)</b>").arg(trUtf8("FEM Draw")).replace(" ", "&nbsp;"));
    msgBox.setInformativeText(QString(
                   "<a href=\"https://fami.codefreak.ru/osp/fem_draw/\">https://fami.codefreak.ru/osp/fem_draw/</a><br>"
                   "%1: <a href=\"http://www.gnu.org/copyleft/gpl.html\">GNU GPL v3</a><br><br>"
                   "Copyright &copy; 2014-2016<br>"
                   "%2 &lt;<a href=\"mailto:peter.zhigalov@gmail.com\">peter.zhigalov@gmail.com</a>&gt;"
                   ).arg(trUtf8("License")).arg(trUtf8("Peter Zhigalov")).replace(" ", "&nbsp;").replace("a&nbsp;href", "a href"));
    msgBox.setIconPixmap(QPixmap::fromImage(QImage(":/resources/icon_64.png")));
#if !defined (Q_OS_MAC)
    msgBox.setWindowIcon(QIcon(":/resources/icon.ico"));
#endif
    msgBox.exec();
}

// Событие при нажатии кнопки About Libraries
void MainWindow::on_actionAbout_Third_Party_Libraries_triggered()
{
    QMessageBox msgBox;
    msgBox.setAttribute(Qt::WA_QuitOnClose);
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.setWindowTitle(trUtf8("About Third Party Libraries"));
    msgBox.setText(QString("<b>%1:</b>").arg(trUtf8("Third Party Libraries")).replace(" ", "&nbsp;"));
    msgBox.setInformativeText(QString(
                   "<table border=\"0\">"
                   "<tr><td style=\"padding-right:8%\">%1:</td><td>Jon Olick JPEG Writer</td></tr>"
                   "<tr><td style=\"padding-right:8%\">%2:</td><td>public domain</td></tr>"
                   "<tr><td style=\"padding-right:8%\">%3:</td><td><a href=\"http://www.jonolick.com/code.html\">http://www.jonolick.com/code.html</td></tr>"
                   "</table><br>"
                   "<table border=\"0\">"
                   "<tr><td style=\"padding-right:8%\">%1:</td><td>Jon Olick GIF Writer</td></tr>"
                   "<tr><td style=\"padding-right:8%\">%2:</td><td>public domain</td></tr>"
                   "<tr><td style=\"padding-right:8%\">%3:</td><td><a href=\"http://www.jonolick.com/code.html\">http://www.jonolick.com/code.html</td></tr>"
                   "</table><br>"
                   "<table border=\"0\">"
                   "<tr><td style=\"padding-right:8%\">%1:</td><td>Jon Olick TGA Writer</td></tr>"
                   "<tr><td style=\"padding-right:8%\">%2:</td><td>public domain</td></tr>"
                   "<tr><td style=\"padding-right:8%\">%3:</td><td><a href=\"http://www.jonolick.com/code.html\">http://www.jonolick.com/code.html</td></tr>"
                   "</table>"
                   ).arg(trUtf8("Library")).arg(trUtf8("License")).arg(trUtf8("Website"))
                    .replace(" ", "&nbsp;").replace("a&nbsp;href", "a href").replace("table&nbsp;border", "table border").replace("td&nbsp;style", "td style"));
    msgBox.setIconPixmap(QPixmap::fromImage(QImage(":/resources/icon_64.png")));
#if !defined (Q_OS_MAC)
    msgBox.setWindowIcon(QIcon(":/resources/icon.ico"));
#endif
    msgBox.exec();
}

// Событие при переключении закраски цветом
void MainWindow::on_checkBox_Color_clicked()
{
    ui->widget->draw_color = ui->checkBox_Color->isChecked();
    ui->widget->invalidate();
}

// Изменение переменной, которую выводим
void MainWindow::on_comboBox_Color_currentIndexChanged(int index)
{
    if(ui->widget->draw_index != (size_t)index)
    {
        ui->widget->draw_index = (size_t)index;
        if(ui->widget->draw_color || ui->widget->draw_isolines)
            ui->widget->invalidate();
    }
}

// Событие при переключении рисования изолиний
void MainWindow::on_checkBox_Isolines_clicked()
{
    bool status = ui->checkBox_Isolines->isChecked();
    ui->actionShow_Isolines->setChecked(status);
    ui->widget->draw_isolines = status;
    ui->widget->invalidate();
}

// Событие при изменении числа изолиний
void MainWindow::on_spinBox_Isolines_valueChanged(int arg1)
{
    if(ui->widget->isolines_num != (size_t)arg1)
    {
        if(arg1 >= ui->spinBox_Isolines->minimum() && arg1 <= ui->spinBox_Isolines->maximum())
        {
            ui->widget->set_isolines_num(arg1);
            if(ui->widget->draw_isolines)
                ui->widget->invalidate();
        }
    }
}

// Переключение рисовки векторов
void MainWindow::on_checkBox_Vectors_clicked()
{
    bool status = ui->checkBox_Vectors->isChecked();
    ui->actionShow_Vectors->setChecked(status);
    ui->widget->draw_vectors = status;
    ui->widget->invalidate();
}

// Число рисуемых векторов
void MainWindow::on_spinBox_Vectors_valueChanged(int arg1)
{
    if(ui->widget->skip_vec != (size_t)arg1)
    {
        if(arg1 >= ui->spinBox_Vectors->minimum() && arg1 <= ui->spinBox_Vectors->maximum())
        {
            ui->widget->skip_vec = (size_t)arg1;
            if(ui->widget->draw_vectors)
                ui->widget->invalidate();
        }
    }
}

// Первая переменная вектора
void MainWindow::on_comboBox_Vectors_U_currentIndexChanged(int index)
{
    if(ui->widget->ind_vec_1 != (size_t)index)
    {
        ui->widget->ind_vec_1 = (size_t)index;
        if(ui->widget->draw_vectors)
            ui->widget->invalidate();
    }
}

// Вторая переменная вектора
void MainWindow::on_comboBox_Vectors_V_currentIndexChanged(int index)
{
    if(ui->widget->ind_vec_2 != (size_t)index)
    {
        ui->widget->ind_vec_2 = (size_t)index;
        if(ui->widget->draw_vectors)
            ui->widget->invalidate();
    }
}

// Пришло drag-and-drop сообщение
void MainWindow::dragEnterEvent(QDragEnterEvent* event)
{
    event->acceptProposedAction();
}

void MainWindow::dragMoveEvent(QDragMoveEvent* event)
{
    event->acceptProposedAction();
}

void MainWindow::dragLeaveEvent(QDragLeaveEvent* event)
{
    event->accept();
}

void MainWindow::dropEvent(QDropEvent * event)
{
    const QMimeData* mime_data = event->mimeData();
    if(mime_data->hasUrls())
    {
        QList<QUrl> url_list = mime_data->urls();
        if(url_list.size() > 1)
        {
            QMessageBox msgBox;
            msgBox.setAttribute(Qt::WA_QuitOnClose);
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setDefaultButton(QMessageBox::Ok);
            msgBox.setWindowTitle(trUtf8("Error"));
            msgBox.setText(trUtf8("Error: You can open only one file simultaneously"));
            msgBox.setIcon(QMessageBox::Critical);
#if !defined (Q_OS_MAC)
            msgBox.setWindowIcon(QIcon(":/resources/icon.ico"));
#endif
            msgBox.exec();
        }
        else
            open_file(url_list.at(0).toLocalFile());
    }
}

