#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDesktopWidget>
#include <QFileDialog>
#include <QImage>
#include <QMessageBox>
#include <QImageWriter>
#include "paintwidget.h"

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
    // Открытие файла по имени
    void open_file(QString filename);
private slots:
    // Событие при открытии файла
    void on_actionOpen_Tecplot_File_triggered();
    // Событие при сохранении
    void on_actionSave_Image_File_triggered();
    // Событие при нажатии кнопки Exit
    void on_actionExit_triggered();
    // Событие при изменении уровня интерполяции
    void on_actionIncrease_Interpolation_triggered();
    void on_actionDecrease_Interpolation_triggered();
    // Событие при нажатии кнопки About
    void on_actionAbout_FEM_Draw_triggered();
    // Событие при переключении закраски цветом
    void on_checkBox_Color_clicked();
    // Изменение переменной, которую выводим
    void on_comboBox_Color_currentIndexChanged(int index);
    // Событие при переключении рисования изолиний
    void on_checkBox_Isolines_clicked();
    // Событие при изменении числа изолиний
    void on_spinBox_Isolines_valueChanged(int arg1);
    // Переключение рисовки векторов
    void on_checkBox_Vectors_clicked();
    // Число рисуемых векторов
    void on_spinBox_Vectors_valueChanged(int arg1);
    // Первая переменная вектора
    void on_comboBox_Vectors_U_currentIndexChanged(int index);
    // Вторая переменная вектора
    void on_comboBox_Vectors_V_currentIndexChanged(int index);

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
