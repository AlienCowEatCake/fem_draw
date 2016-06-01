#ifndef APPLICATION_H
#define APPLICATION_H

#include <QApplication>
#include <QString>

/**
 * @brief Класс-обертка над QApplication, выполняющий обработку событий типа QFileOpenEvent
 * @note https://stackoverflow.com/questions/26849866/unable-to-open-file-with-qt-app-on-mac
 */
class Application : public QApplication
{
    Q_OBJECT
public:
    /**
     * @brief Конструктор
     * @param[inout] argc Количество аргументов
     * @param[inout] argv Массив C-строк - значений аргументов
     */
    Application(int & argc, char ** argv);

    /**
     * @brief Получить имя последнего файла, который пришел в QFileOpenEvent
     * @return Имя последнего файла, который пришел в QFileOpenEvent
     */
    const QString & getLastOpenFilename() const;

    /**
     * @brief Узнать, сохранено ли имя последнего файла, который пришел в QFileOpenEvent
     * @return true - имя сохранено, false - имя не сохранено
     */
    bool hasLastOpenFilename() const;

protected:
    /**
     * @brief Обработчик событий
     * @param[in] event Событие
     * @return true - событие распознано и обработано, false - иначе
     */
    bool event(QEvent * event);

signals:
    /**
     * @brief Сигнал о том, что пришло событие QFileOpenEvent
     * @param filename Имя файла
     */
    void openFileEvent(const QString & filename);

private:
    /**
     * @brief Имя последнего файла, который пришел в QFileOpenEvent
     */
    QString m_lastOpenFilename;
};

#endif // APPLICATION_H
