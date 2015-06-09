#include "mainwindow.h"
#include "ui_mainwindow.h"

// Конструктор
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Установка минимальных размеров окна
    this->setMinimumHeight(530);
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
    while(ui->comboBox->count() > 0)
        ui->comboBox->removeItem(0);
    while(ui->comboBox_2->count() > 0)
        ui->comboBox_2->removeItem(0);
    while(ui->comboBox_3->count() > 0)
        ui->comboBox_3->removeItem(0);

    // Начальные значения элементов управления
    ui->checkBox_2->setChecked(true);
    ui->checkBox_3->setChecked(true);
    ui->checkBox_4->setChecked(false);
    ui->spinBox->setMinimum(0);
    ui->spinBox->setMaximum(100);
    ui->spinBox->setValue(10);
    ui->spinBox_2->setMinimum(1);
    ui->spinBox_2->setMaximum(10000);
    ui->spinBox_2->setValue(1);
    ui->spinBox_3->setMinimum(0);
    ui->spinBox_3->setMaximum(7);
    ui->spinBox_3->setValue(0);

    // Немного эстетства
    this->setWindowTitle(trUtf8("VFEM Draw"));

    // Передача начальных значений виджету
    ui->widget->draw_isolines = ui->checkBox_2->isChecked();
    ui->widget->draw_color = ui->checkBox_3->isChecked();
    ui->widget->draw_vectors = ui->checkBox_4->isChecked();
    ui->widget->set_isolines_num(ui->spinBox->value());
    ui->widget->skip_vec = ui->spinBox_2->value();
    ui->widget->set_div_num(ui->spinBox_3->value());
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
    // Подгонка размеров OpenGL виджета при изменении размеров окна
    QRect main = ui->centralwidget->geometry();
    QRect ogl = ui->widget->geometry();
    ui->widget->setGeometry(ogl.x(), ogl.y(), main.width() - ogl.x(), main.height() - ogl.y());
}

// Событие при переключении рисования изолиний
void MainWindow::on_checkBox_2_clicked()
{
    ui->widget->draw_isolines = ui->checkBox_2->isChecked();
    ui->widget->repaint();
}

// Событие при переключении закраски цветом
void MainWindow::on_checkBox_3_clicked()
{
    ui->widget->draw_color = ui->checkBox_3->isChecked();
    ui->widget->repaint();
}

// Событие при изменении числа изолиний
void MainWindow::on_spinBox_valueChanged(int arg1)
{
    if(arg1 >= ui->spinBox->minimum() && arg1 <= ui->spinBox->maximum())
        ui->widget->set_isolines_num(arg1);
    ui->widget->repaint();
}

// Событие при открытии файла
void MainWindow::on_pushButton_clicked()
{
    // Запомним старые значения индексов, чтоб потом восстановить
    size_t old_draw_index = ui->widget->draw_index;
    ui->widget->draw_index = 0;
    size_t old_ind_vec_1 = ui->widget->ind_vec_1;
    ui->widget->ind_vec_1 = 0;
    size_t old_ind_vec_2 = ui->widget->ind_vec_2;
    ui->widget->ind_vec_2 = 0;

    // Откроем файл
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Tecplot File"), "", tr("Tecplot Data Files (*.dat)"));
    if(fileName.length() == 0) return;
    ui->spinBox_3->setValue(0); // Сбросим значение интерполяции, чтобы не повисло на больших файлах
    ui->widget->tec_read(fileName.toStdString());
    // Ненене, еще не все готово!
    ui->widget->is_loaded = false;

    // Очистим поля в комбобоксах
    while(ui->comboBox->count() > 0)
        ui->comboBox->removeItem(0);
    while(ui->comboBox_2->count() > 0)
        ui->comboBox_2->removeItem(0);
    while(ui->comboBox_3->count() > 0)
        ui->comboBox_3->removeItem(0);

    // Заполним поля в комбобоксах
    for(size_t i = 0; i < ui->widget->variables.size(); i++)
    {
        ui->comboBox->addItem(ui->widget->variables[i]);
        ui->comboBox_2->addItem(ui->widget->variables[i]);
        ui->comboBox_3->addItem(ui->widget->variables[i]);
    }

    // Попытаемся восстановить старые индексы
    if(old_draw_index < ui->widget->variables.size())
        ui->comboBox->setCurrentIndex((int)old_draw_index);
    else
        ui->comboBox->setCurrentIndex(0);

    if(old_ind_vec_1 < ui->widget->variables.size())
        ui->comboBox_2->setCurrentIndex((int)old_ind_vec_1);
    else
        ui->comboBox_2->setCurrentIndex(0);

    if(old_ind_vec_2 < ui->widget->variables.size())
        ui->comboBox_3->setCurrentIndex((int)old_ind_vec_2);
    else
        ui->comboBox_3->setCurrentIndex(0);

    // Устанавливаем оптимальное значение для векторов
    if(ui->widget->vect_value < ui->spinBox_2->minimum())
        ui->spinBox_2->setValue(ui->spinBox_2->minimum());
    else if(ui->widget->vect_value > ui->spinBox_2->maximum())
        ui->spinBox_2->setValue(ui->spinBox_2->maximum());
    else
        ui->spinBox_2->setValue(ui->widget->vect_value);

    // А вот теперь готово
    ui->widget->is_loaded = true;
    ui->widget->repaint();
}

// Изменение переменной, которую выводим
void MainWindow::on_comboBox_currentIndexChanged(int index)
{
    ui->widget->draw_index = (size_t)index;
    ui->widget->repaint();
}

// Первая переменная вектора
void MainWindow::on_comboBox_2_currentIndexChanged(int index)
{
    ui->widget->ind_vec_1 = (size_t)index;
    ui->widget->repaint();
}

// Вторая переменная вектора
void MainWindow::on_comboBox_3_currentIndexChanged(int index)
{
    ui->widget->ind_vec_2 = (size_t)index;
    ui->widget->repaint();
}

// Переключение рисовки векторов
void MainWindow::on_checkBox_4_clicked()
{
    ui->widget->draw_vectors = ui->checkBox_4->isChecked();
    ui->widget->repaint();
}

// Число рисуемых векторов
void MainWindow::on_spinBox_2_valueChanged(int arg1)
{
    if(arg1 >= ui->spinBox_2->minimum() && arg1 <= ui->spinBox_2->maximum())
        ui->widget->skip_vec = (size_t)arg1;
    ui->widget->repaint();
}

// Событие при изменении уровня интерполяции
void MainWindow::on_spinBox_3_valueChanged(int arg1)
{
    if(arg1 >= ui->spinBox_3->minimum() && arg1 <= ui->spinBox_3->maximum())
        ui->widget->set_div_num(arg1);
    ui->widget->repaint();
}
