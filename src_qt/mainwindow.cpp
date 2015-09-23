#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSvgGenerator>

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

    // Немного эстетства
    this->setWindowTitle("FEM Draw");
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
    last_saved = "draw.png";
    last_opened = "";
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
    ui->widget->setGeometry(widget.x(), widget.y(), main.width() - widget.x(), main.height() - widget.y());
}

// Открытие файла по имени
void MainWindow::open_file(QString filename)
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
        this->setWindowTitle("FEM Draw");
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
#if defined _WIN32
    label = label.split('\\').last();
#endif
    if(ui->widget->title.length() > 0)
        label.prepend(ui->widget->title + " - ");
    label.append(" - FEM Draw");
    this->setWindowTitle(label);

    // Сохраним директорию, в которой находится файл
    last_opened = "";
    int stop_index = filename.lastIndexOf("/");
#if defined _WIN32
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
    QString fileName = QFileDialog::getOpenFileName(this, trUtf8("Open Tecplot File"), last_opened, "Tecplot Data Files (*.dat *.plt);;All Files (*.*)");
    if(fileName.length() == 0) return;
    open_file(fileName);
}

// Событие при сохранении
void MainWindow::on_actionSave_Image_File_triggered()
{
    QList<QByteArray> supported = QImageWriter::supportedImageFormats();
    QString formats, formats_all;
    for(QList<QByteArray>::const_iterator it = supported.begin(); it != supported.end(); ++it)
    {
        QString record(* it);
        if(formats_all.length() > 0)
            formats_all.append(" *.");
        else
            formats_all.append("*.");
        formats_all.append(record);

        if(formats.length() > 0)
            formats.append(";;");
        formats.append(record.toUpper());
        formats.append(" Images (*.");
        formats.append(record);
        formats.append(")");
    }
    formats.append(";;SVG Images (*.svg)");
    formats_all.append(" *.svg");
    supported.push_back("svg");
    formats_all.prepend("All Images (");
    formats_all.append(");;");
    formats.prepend(formats_all);

    bool is_svg = false;
    QString fileName = QFileDialog::getSaveFileName(this, trUtf8("Save Image File"), last_saved, formats);
    if(fileName.length() == 0) return;
    int found = fileName.lastIndexOf('.');
    if(found == -1)
    {
        fileName.append(".png");
    }
    else
    {
        QString ext = fileName.right(fileName.length() - found - 1);
        bool finded = false;
        for(QList<QByteArray>::const_iterator it = supported.begin(); it != supported.end() && !finded; ++it)
        {
            if(ext.compare(QString(* it), Qt::CaseInsensitive) == 0)
                finded = true;
        }
        if(!finded)
            fileName.append(".png");
        if(ext.compare("svg", Qt::CaseInsensitive) == 0)
            is_svg = true;
    }
    last_saved = fileName;

    bool saved = false;
    if(is_svg)
    {
        QSvgGenerator generator;
        generator.setFileName(fileName);
        generator.setSize(QSize(ui->widget->width(), ui->widget->height()));
        generator.setViewBox(QRect(0, 0, ui->widget->width(), ui->widget->height()));
        generator.setTitle(ui->widget->title);
        generator.setDescription(trUtf8("An SVG drawing created by FEM Draw."));
        ui->widget->draw(& generator, ui->actionTransparent_Image->isChecked());
        saved = true;
    }
    else
    {
        QImage image(ui->widget->width(), ui->widget->height(), QImage::Format_ARGB32_Premultiplied);
        ui->widget->draw(& image, ui->actionTransparent_Image->isChecked());
        saved = image.save(fileName);
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
        msgBox.setWindowIcon(QIcon(":/resources/icon.ico"));
        msgBox.exec();
    }
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
            msgBox.setWindowIcon(QIcon(":/resources/icon.ico"));
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
        msgBox.setWindowIcon(QIcon(":/resources/icon.ico"));
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
    msgBox.setWindowTitle("About");
    msgBox.setText("<b>FEM Draw v1.3 (Qt)</b><br><br>"
                   "<a href=\"https://fami.codefreak.ru/osp/fem_draw/\">https://fami.codefreak.ru/osp/fem_draw/</a><br>"
                   "License: <a href=\"http://www.gnu.org/copyleft/gpl.html\">GNU GPL v3</a><br><br>"
                   "Copyright &copy; 2014-2015<br>"
                   "Peter Zhigalov &lt;<a href=\"mailto:peter.zhigalov@gmail.com\">peter.zhigalov@gmail.com</a>&gt;");
    msgBox.setIconPixmap(QPixmap::fromImage(QImage(":/resources/icon_64.png")));
    msgBox.setWindowIcon(QIcon(":/resources/icon.ico"));
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
    ui->widget->draw_isolines = ui->checkBox_Isolines->isChecked();
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
    ui->widget->draw_vectors = ui->checkBox_Vectors->isChecked();
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
            msgBox.setText(trUtf8("Error: You can open only one file simultaneously!"));
            msgBox.setIcon(QMessageBox::Critical);
            msgBox.setWindowIcon(QIcon(":/resources/icon.ico"));
            msgBox.exec();
        }
        else
            open_file(url_list.at(0).toLocalFile());
    }
}

