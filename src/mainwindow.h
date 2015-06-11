#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDesktopWidget>
#include <QFileDialog>
#include <QImage>
#include <QMessageBox>
#include <QImageWriter>
#include "glwidget.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    // Конструктор
    explicit MainWindow(QWidget *parent = 0);
    // Деструктор
    ~MainWindow();
    // Событие при изменении размера окна
    void resizeEvent(QResizeEvent *event);
private slots:
    // Событие при переключении рисования изолиний
    void on_checkBox_2_clicked();
    // Событие при переключении закраски цветом
    void on_checkBox_3_clicked();
    // Событие при изменении числа изолиний
    void on_spinBox_valueChanged(int arg1);
    // Событие при открытии файла
    void on_actionOpen_Tecplot_File_triggered();
    // Изменение переменной, которую выводим
    void on_comboBox_currentIndexChanged(int index);
    // Первая переменная вектора
    void on_comboBox_2_currentIndexChanged(int index);
    // Вторая переменная вектора
    void on_comboBox_3_currentIndexChanged(int index);
    // Переключение рисовки векторов
    void on_checkBox_4_clicked();
    // Число рисуемых векторов
    void on_spinBox_2_valueChanged(int arg1);
    // Событие при изменении уровня интерполяции
    void on_actionIncrease_Interpolation_triggered();
    void on_actionDecrease_Interpolation_triggered();
    // Событие при сохранении
    void on_actionSave_Image_File_triggered();
    // Событие при нажатии кнопки About
    void on_actionAbout_FEM_Draw_triggered();
    // Событие при нажатии кнопки Exit
    void on_actionExit_triggered();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
