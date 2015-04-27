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
    ui->comboBox->addItem("ReEx");
    ui->comboBox->addItem("ReEy");
    ui->comboBox->addItem("ReEz");
    ui->comboBox->addItem("ImEx");
    ui->comboBox->addItem("ImEy");
    ui->comboBox->addItem("ImEz");
    ui->comboBox->addItem("abs(E)");
    ui->comboBox_2->addItem("ReEx");
    ui->comboBox_2->addItem("ReEy");
    ui->comboBox_2->addItem("ReEz");
    ui->comboBox_2->addItem("ImEx");
    ui->comboBox_2->addItem("ImEy");
    ui->comboBox_2->addItem("ImEz");
    ui->comboBox_2->addItem("abs(E)");
    ui->comboBox_3->addItem("ReEx");
    ui->comboBox_3->addItem("ReEy");
    ui->comboBox_3->addItem("ReEz");
    ui->comboBox_3->addItem("ImEx");
    ui->comboBox_3->addItem("ImEy");
    ui->comboBox_3->addItem("ImEz");
    ui->comboBox_3->addItem("abs(E)");
    ui->comboBox_3->setCurrentIndex(1);

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
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Tecplot File"), "", tr("Tecplot Data Files (*.dat)"));
    if(fileName.length() == 0) return;
    ui->widget->tec_read(fileName.toStdString());
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

// Событие при изменении числа внутренних сегментов каждого КЭ
void MainWindow::on_spinBox_3_valueChanged(int arg1)
{
    if(arg1 >= ui->spinBox_3->minimum() && arg1 <= ui->spinBox_3->maximum())
        ui->widget->set_div_num(arg1);
    ui->widget->repaint();
}
