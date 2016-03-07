#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDesktopWidget>
#include <QFileDialog>
#include <QImage>
#include <QMessageBox>
#include <QImageWriter>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>

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
    // Пришло drag-and-drop сообщение
    void dragEnterEvent(QDragEnterEvent * event);
    void dragMoveEvent(QDragMoveEvent * event);
    void dragLeaveEvent(QDragLeaveEvent * event);
    void dropEvent(QDropEvent * event);
private slots:
    // Событие при открытии файла
    void on_actionOpen_Tecplot_File_triggered();
    // Событие при копировании картинки в буфер обмена
    void on_actionCopy_Image_to_Clipboard_triggered();
    // Событие при сохранении
    void on_actionSave_Image_File_triggered();
    // Событие при нажатии кнопки Exit
    void on_actionExit_triggered();
    // Событие при изменении уровня интерполяции
    void on_actionIncrease_Interpolation_triggered();
    void on_actionDecrease_Interpolation_triggered();
    // Событие при переключении рисования изолиний из меню
    void on_actionShow_Isolines_triggered();
    // Событие при запросе конфигурации цвета изолиний
    void on_actionIsolines_Color_triggered();
    // Событие при запросе конфигурации толщины изолиний
    void on_actionIsolines_Width_triggered();
    // Событие при переключении рисования векторов из меню
    void on_actionShow_Vectors_triggered();
    // Событие при запросе конфигурации цвета векторов
    void on_actionVectors_Color_triggered();
    // Событие при запросе конфигурации толщины векторов
    void on_actionVectors_Width_triggered();
    // Событие при запросе конфигурации длины векторов
    void on_actionVectors_Length_triggered();
    // Событие при запросе конфигурации размера стрелок векторов
    void on_actionArrowSize_triggered();
    // Событие при переключении рисования легенды
    void on_actionShow_Legend_triggered();
    // Событие при переключении использования светлых цветов
    void on_actionUse_Light_Colors_triggered();
    // Событие при переключении использования фиолетовых цветов
    void on_actionUse_Purple_Colors_triggered();
    // Событие при переключении использования ограничения памяти
    void on_actionMemory_Limit_triggered();
    // Событие при нажатии кнопки About
    void on_actionAbout_FEM_Draw_triggered();
    // Событие при нажатии кнопки About Libraries
    void on_actionAbout_Third_Party_Libraries_triggered();
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
protected:
    // Последняя директория с файлом для открытия
    QString last_opened;
    // Последнее имя файла для сохранения
    QString last_saved;

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
